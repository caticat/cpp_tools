#pragma once
#include "stdafx.h"

typedef std::vector<std::string> row_t;
typedef std::vector<row_t> tab_t;
typedef row_t::iterator itRow_t;
typedef tab_t::iterator itTab_t;

struct DBConfig;

class DLL_API MysqlProxy
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