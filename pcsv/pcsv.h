#pragma once

/*
	csv读取类
*/

#include <string>
#include <fstream>
#include <map>

#ifdef DLL_EXPORT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API PCsv
{
public:
	PCsv() : m_path(""),m_width(0),m_height(0) {}

public:
	int Load(std::string& path);

public:
	typedef std::map<int,std::string> row_t; // 行
	typedef std::map<int,row_t> tab_t; // 表
	typedef row_t::iterator rowIt_t; // 行 迭代器
	typedef tab_t::iterator tabIt_t; // 表 迭代器

	inline const std::string& GetVal(int row,int col); // 获取内容
	inline const row_t& GetRow(int row); // 获取行
	inline const tab_t& GetTab(); // 获取表

private:
	inline bool _ResolveRow(int rowNo,std::string rowData); // 解析数据

private:
	std::string m_path; // 文件路径
	tab_t m_tab; // 数据表
	int m_width; // 表宽，列数
	int m_height; // 表高，行数

private:
	static std::string m_emptyString; // 空字符串
	static row_t m_emptyRow; // 空行
	static tab_t m_emptyTab; // 空表
};

enum DLL_API PCsvErrorType
{
	PCsv_NoError, // 没有错误
	PCsv_FileNotFound, // 文件没有找到
	PCsv_IllegalForm, // 错误的文件格式
};
