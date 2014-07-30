#include "pcsv.h"

using std::string;
using std::fstream;
using std::make_pair;

static std::string pcsv_emptyString = ""; // 空字符串
static PCsv::row_t pcsv_emptyRow; // 空行

bool PCsv::Load(const char* path)
{
	m_path = path;

	fstream fileStream;
	fileStream.open(m_path.c_str(),std::ios::in);
	if (!fileStream)
	{
		_SetErrNo(PCsv_FileNotFound);
		return false;
	}

	int bufLen = 10240; // buf长度
	char* buf = new char[bufLen];
	int rowNo = 1; // 行号从1开始
	while (!fileStream.eof())
	{
		fileStream.getline(buf,bufLen);
		if (strlen(buf) == 0)
			continue;
		if (!_ResolveRow(rowNo,buf))
		{
			delete [] buf;
			_SetErrNo(PCsv_IllegalForm);
			return false;
		}
		++rowNo;
	}
	m_height = rowNo-1;
	delete [] buf;
	return true;
}

const std::string& PCsv::GetVal(int row,int col)
{
	if ((row > m_height) || (col > m_width) || (row < 1) || (col < 1))
		return pcsv_emptyString;

	tabIt_t tabIt = m_tab.find(row);
	if (tabIt == m_tab.end())
		return pcsv_emptyString;

	rowIt_t rowIt = tabIt->second.find(col);
	if (rowIt == tabIt->second.end())
		return pcsv_emptyString;
	return rowIt->second;
}

const PCsv::row_t& PCsv::GetRow(int row)
{
	if (row > m_height)
		return pcsv_emptyRow;

	tabIt_t tabIt = m_tab.find(row);
	if (tabIt == m_tab.end())
		return pcsv_emptyRow;

	return tabIt->second;
}

const PCsv::tab_t& PCsv::GetTab()
{
	return m_tab;
}

int PCsv::GetWidth()
{
	return m_width;
}

int PCsv::GetHeight()
{
	return m_height;
}

int PCsv::GetErrorNo()
{
	return m_errNo;
}

bool PCsv::_ResolveRow(int rowNo,string rowData)
{
	int posBegin = 0;
	int posEnd = 0;
	string tmp = "";
	int colNo = 1;
	row_t row;
	while ((posEnd = rowData.find(",",posBegin)) != rowData.npos)
	{
		row.insert(make_pair(colNo++,rowData.substr(posBegin,posEnd-posBegin)));
		posBegin = posEnd+1;
	}
	row.insert(make_pair(colNo,rowData.substr(posBegin)));
	if (m_width == 0)
		m_width = colNo;

	m_tab.insert(make_pair(rowNo,row));
	return m_width == colNo;
}

void PCsv::_SetErrNo(PCsvErrorType errNo)
{
	m_errNo = errNo;
}
