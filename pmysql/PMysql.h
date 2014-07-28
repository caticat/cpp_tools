#pragma once
#include "stdafx.h"

/* 数据库代理的使用
	PMysql db;
	if (!db.Connect("192.168.0.183",3306,"kuafu","root","","gbk"))
	{
		cout << "connect mysql error.\n";
		return 1;
	}
	string sql = "select id,name from item_template limit 10;";
	if (!db.Query(sql.c_str()))
	{
		cout << "query sql error.\n";
		return 1;
	}
	tab_t resTab;
	db.StoreResult(resTab);
	for (itTab_t it = resTab.begin(); it != resTab.end(); ++it)
	{
		for (itRow_t itRow = it->begin(); itRow != it->end(); ++itRow)
		{
			cout << "res:" << *itRow << endl;
		}
	}
	db.Close();
*/

typedef std::vector<std::string> row_t;
typedef std::vector<row_t> tab_t;
typedef row_t::iterator itRow_t;
typedef tab_t::iterator itTab_t;

struct DBConfig;

class DLL_API PMysql
{
public:
	bool Connect(const DBConfig& dbConfig);
	bool Connect(const char* host, int port, const char* database, const char* name, const char* password, const char* charSet = NULL);
	bool Query(const char* sql);
	void StoreResult(tab_t& res);
	void Close();

public:
	void SetCharSet(const char* charSet);
private:
	void ShowError();
private:
	MYSQL mysql;
};