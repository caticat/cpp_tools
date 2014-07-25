#pragma once

/*
	日志输出类

	#include "plog.h"

	// 测试方法
	int main()
	{
		CPLog log;
		log.init("E:/test/","Error",".txt",true,true);
		log.log("111");
		log.log("222");
		log.log("333");
		log.log("%s:%d","lala",256);
		return 0;
	}
*/

#include <string>
#include <fstream>

#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API CPLog
{
public:
	CPLog();
	~CPLog();

public:
	bool init(const std::string& path,const std::string& prifix,const char* fileType = NULL,bool showTime = false,bool showConsole = false); // 日志路径，输出前缀，是否输出当前时间，是否有当前默认控制台输出
	void log(const char* format,...); // 日志输出

private:
	void refreshTime(bool isSimple = false); // 更新时间字符串

private:
	std::fstream* m_pFos; // 文件流指针
	std::string m_strPrifix; // 前缀
	bool m_bShowTime; // 是否显示时间
	bool m_bShowConsole; // 是否输出到控制台（默认输出方式）
	char m_cstrTime[32]; // 当时时间字符串
	char m_cstrLog[256]; // 日志输出文字
};
