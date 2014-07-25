#include <iostream>
#include <vector>
#include <string>
#include "piocp.h"
#include "plog.h"
#include "pmysql.h"
#include "pjson.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

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

	// 完成端口测试
	PIOCP iocp;
	iocp.SetPort(12345);
	if (!iocp.Start())
	{
		printf_s("服务器启动失败！\n");
		return 2;
	}

	// 主线程停止循环
	string cmd = "";
	while (true)
	{
		std::cin>>cmd;
		if ((cmd == "quit") || (cmd == "exit")) // 退出程序
		{
			iocp.Stop(); // 关闭IOCP监听服务
			printf_s("服务器关闭\n");
			break;
		}
	}

	return 0;
}
