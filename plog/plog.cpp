#include "plog.h"

#include <iostream>
#include <stdarg.h>
#include <time.h>

using std::cout;
using std::endl;
using std::ios;
using std::string;

CPLog::CPLog() : m_pFos(NULL),
	m_strPrifix(""),
	m_bShowTime(false),
	m_bShowConsole(false)
{
	memset(m_cstrTime,0,sizeof(m_cstrTime));
}

CPLog::~CPLog()
{
	log("日志文件结束");
	if (m_pFos != NULL)
	{
		m_pFos->close();
		m_pFos = NULL;
	}
}

// 初始化日志配置
// @param const string& path 日志路径
// @param const string& prifix 输出前缀
// @param ,const char* fileType 扩展名，默认为空
// @param bool showTime = false 是否输出当前时间，默认不输出
// @param bool showConsole = false 是否有当前默认控制台输出，默认不输出
// @return 是否初始化成功
bool CPLog::init(const string& path,const string& prifix,const char* fileType,bool showTime,bool showConsole)
{
	m_pFos = new std::fstream;
	if (m_pFos == NULL)
	{
		cout << "初始化日志文件流失败。\n";
		return false;
	}

	string fullPath = path;
	fullPath += prifix;
	fullPath += "_";
	refreshTime(true);
	fullPath += m_cstrTime;
	if (fileType != NULL)
		fullPath += fileType;

	m_pFos->open(fullPath.c_str(),ios::app); // 创建、追加更新
	if (!m_pFos)
	{
		cout << "创建/打开日志文件失败。\n";
		return false;
	}

	m_strPrifix = prifix;
	m_bShowTime = showTime;
	m_bShowConsole = showConsole;

	log("日志文件创建");
	return true;
}

// 日志输出
void CPLog::log(const char* format,...)
{
	if (m_pFos == NULL)
	{
		cout << "无法记录日志，日志文件不存在。\n";
		return;
	}
	if (format == NULL)
	{
		cout << "记录日志格式为空。\n";
		return;
	}

	memset(m_cstrLog,0,sizeof(m_cstrLog));

	va_list vl;
	va_start(vl,format);

	if (m_bShowTime)
	{
		refreshTime();
		strcpy_s(m_cstrLog,sizeof(m_cstrLog),m_cstrTime);
		vsprintf_s(m_cstrLog+strlen(m_cstrTime),sizeof(m_cstrLog)-strlen(m_cstrTime),format,vl);
	}
	else
		vsprintf_s(m_cstrLog,sizeof(m_cstrLog),format,vl);

	va_end(vl);

	m_pFos->write(m_cstrLog,strlen(m_cstrLog));
	*m_pFos<<'\n';

	if (m_bShowConsole)
		cout << "[" << m_strPrifix.c_str() << "]" << m_cstrLog << endl;

}

// 更新时间字符串
// @param bool isSimple 是否是简单时间，默认不是
void CPLog::refreshTime(bool isSimple)
{
	memset(m_cstrTime,0,sizeof(m_cstrTime));
	time_t curTime(time(NULL));
	tm tm;
	localtime_s(&tm,&curTime);
	if (isSimple)
		sprintf_s(m_cstrTime,"%d-%02d-%02d %02d_%02d_%02d",(tm.tm_year+1900),(tm.tm_mon+1),(tm.tm_mday),tm.tm_hour,tm.tm_min,tm.tm_sec);
	else
		sprintf_s(m_cstrTime,"[%d-%02d-%02d %02d:%02d:%02d]",(tm.tm_year+1900),(tm.tm_mon+1),(tm.tm_mday),tm.tm_hour,tm.tm_min,tm.tm_sec);
}
