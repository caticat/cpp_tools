#include "stdafx.h"
#include "PMysql.h"
#include "DBConfig.h"
#pragma comment(lib,"libmysql.lib")

bool PMysql::Connect(const DBConfig& dbConfig)
{
	return Connect(dbConfig.m_host.c_str(),dbConfig.m_port,dbConfig.m_database.c_str(),dbConfig.m_name.c_str(),dbConfig.m_password.c_str(),dbConfig.m_charset.c_str());
}

bool PMysql::Connect(const char* host, int port, const char* database, const char* name, const char* password, const char* charSet)
{
	if ((host == NULL) || (database == NULL) || (name == NULL) || (password == NULL))
		return false;
	mysql_init(&mysql);
	if (!mysql_real_connect(&mysql,host,name,password,database,port,NULL,0))
	{
		ShowError();
		return false;
	}
	if (charSet != NULL)
		SetCharSet(charSet);
	return true;
}

bool PMysql::Query(const char* sql)
{
	if (sql == NULL)
		return false;
	if (mysql_real_query(&mysql,sql,strlen(sql)) != 0)
	{
		ShowError();
		return false;
	}
	return true;
}

void PMysql::StoreResult(tab_t& res)
{
	res.clear();
	MYSQL_RES* pRes = mysql_store_result(&mysql);
	if (pRes == NULL)
		return;
	row_t tmpData;
	int fieldCnt = mysql_num_fields(pRes);
	char** pRow = NULL;
	while ((pRow = mysql_fetch_row(pRes)) != NULL)
	{
		tmpData.clear();
		for (int i = 0; i < fieldCnt; ++i)
		{
			tmpData.push_back(pRow[i]);
		}
		res.push_back(tmpData);
	}
}

void PMysql::Close()
{
	mysql_close(&mysql);
}

void PMysql::SetCharSet(const char* charSet)
{
	if (charSet == NULL)
		return;
	mysql_set_character_set(&mysql,charSet);
}

void PMysql::ShowError()
{
	std::cout << mysql_error(&mysql) << std::endl;
}

