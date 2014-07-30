#pragma once

/*
	csv读取类
	行，列最小值为1 索引从1开始
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
	enum PCsvErrorType
	{
		PCsv_NoError, // 没有错误
		PCsv_FileNotFound, // 文件没有找到
		PCsv_IllegalForm, // 错误的文件格式
	};

public:
	PCsv() : m_path(""),m_width(0),m_height(0),m_errNo(PCsv_NoError) {}

public:
	bool Load(const char* path); // 加载文件

public:
	typedef std::map<int,std::string> row_t; // 行
	typedef std::map<int,row_t> tab_t; // 表
	typedef row_t::iterator rowIt_t; // 行 迭代器
	typedef row_t::const_iterator crowIt_t; // 行 常量迭代器
	typedef tab_t::iterator tabIt_t; // 表 迭代器
	typedef tab_t::const_iterator ctabIt_t; // 表 常量迭代器

	inline const std::string& GetVal(int row,int col); // 获取内容
	inline const row_t& GetRow(int row); // 获取行
	inline const tab_t& GetTab(); // 获取表
	inline int GetWidth(); // 获取表宽度
	inline int GetHeight(); // 获取表高度
	inline int GetErrorNo(); // 获取错误码

private:
	inline bool _ResolveRow(int rowNo,std::string rowData); // 解析数据
	inline void _SetErrNo(PCsvErrorType errNo); // 设置错误码

private:
	std::string m_path; // 文件路径
	tab_t m_tab; // 数据表
	int m_width; // 表宽，列数
	int m_height; // 表高，行数
	int m_errNo; // 错误码，加载数据报错信息
};
