#include <iostream>
#include <vector>
#include <string>
#include "plog.h"
#include "MysqlProxy.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

//typedef vector<vector<string>> mysqlResTab_t;
//typedef vector<vector<string>>::iterator itMysqlResTab_t;
//typedef vector<string> mysqlResRow_t;
//typedef vector<string>::iterator itMysqlResRow_t;

int main()
{
	/* log的使用
	CPLog log;
	log.init("E:/","Error",".txt",true,true);
	log.log("1111");
	log.log("%d:%s",2222,"lalala");
	log.log("3333");
	*/

	/* 数据库代理的使用
	MysqlProxy db;
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

	return 0;
}
