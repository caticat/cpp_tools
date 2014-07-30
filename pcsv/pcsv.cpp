#include "pcsv.h"

using std::string;
using std::fstream;
using std::make_pair;

int PCsv::Load(string& path)
{
	m_path = path;

	fstream fileStream;
	fileStream.open(m_path.c_str(),std::ios::in);
	if (!fileStream)
		return PCsv_FileNotFound;

	int bufLen = 10240; // buf长度
	char* buf = new char[bufLen];
	int rowNo = 0; // 行号从0开始
	while (!fileStream.eof())
	{
		fileStream.getline(buf,bufLen);
		if (!_ResolveRow(rowNo,buf))
		{
			delete [] buf;
			return PCsv_IllegalForm;
		}
		++rowNo;
	}
	delete [] buf;
	return PCsv_NoError;
}

const std::string& PCsv::GetVal(int row,int col)
{
	if ((row > m_height) || (col > m_width))
		return m_emptyString;

	tabIt_t tabIt = m_tab.find(row);
	if (tabIt == m_tab.end())
		return m_emptyString;

	rowIt_t rowIt = tabIt->second.find(col);
	if (rowIt == tabIt->second.end())
		return m_emptyString;
	return rowIt->second;
}

const row_t& PCsv::GetRow(int row)
{
	if (row > m_height)
		return m_emptyString;

	tabIt_t tabIt = m_tab.find(row);
	if (tabIt == m_tab.end())
		return m_emptyString;

	return tabIt->second;
}

const tab_t& PCsv::GetTab()
{
	return m_tab;
}

bool PCsv::_ResolveRow(int rowNo,string rowData)
{
	int posBegin = 0;
	int posEnd = 0;
	string tmp = "";
	int colNo = 0;
	row_t row;
	while ((posEnd = rowData.find(",",posBegin)) != rowData.npos)
	{
		row.insert(make_pair(colNo++,rowData.substr(posBegin,posEnd-posBegin)));
		posBegin = posEnd+1;
	}
	row.insert(make_pair(colNo,rowData.substr(posBegin)));
	if (m_width == 0)
		m_width = colNo;
	
	return m_width == colNo;
}
