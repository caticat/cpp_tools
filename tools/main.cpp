#include <iostream>
#include <vector>
#include <string>
#include "piocp.h"
//#include "pmsg.h"
#include "plog.h"
#include "pmysql.h"
#include "pjson.h"
#include "pcsv.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;


//void callBackFunc(PER_SOCKET_CONTEXT* pPSock,char* data,uint16 dataLen);
//void onCloseFunc(PSocket* pPSock,uint16 closeType);

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
	//*/

	/*
	// pmsg测试
	PMsg pmsg;
	cout << "proto:" << pmsg.GetProto() << ";len:" << pmsg.GetDataLen() << ";pos:" << pmsg.GetPos() << endl;
	pmsg.SetProto(1);
	cout << "proto:" << pmsg.GetProto() << ";len:" << pmsg.GetDataLen() << ";pos:" << pmsg.GetPos() << endl;
	int8 a = 1;
	pmsg.Write(a);
	int16 b = 2;
	int32 c = 3;
	pmsg<<b<<c<<"haha";
	cout << "proto:" << pmsg.GetProto() << ";len:" << pmsg.GetDataLen() << ";pos:" << pmsg.GetPos() << endl;
	
	int8 d = 0;
	int16 e = 0;
	int32 f = 0;
	string g = "";
	pmsg>>d>>e>>f>>g;
	cout << "proto:" << pmsg.GetProto() << ";len:" << pmsg.GetDataLen() << ";pos:" << pmsg.GetPos() << endl;
	cout << "d:" << (int)d << ",e:"<<e<<",f:"<<f<<",g:"<<g<<endl;
	//*/

	/* pmsg字符串测试
	PMsg pmsg;
	string b = "11111";
	pmsg<<"aaaa"<<b.c_str()<<b;
	string a,c,d;
	pmsg>>a>>c>>d;
	cout << a << endl;
	cout << c << endl;
	cout << d << endl;
	//*/

	/*
	// 完成端口测试
	PIOCP iocp;
	if (!iocp.Start(12345,callBackFunc,onCloseFunc))
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
	//*/

	/*
	PCsv pcsv;

	// 加载文件
	if (!pcsv.Load("E:/test/tools/example/test.csv"))
	{
		printf_s("出错，错误码：%d\n",pcsv.GetErrorNo());
		return 0;
	}
	// 宽高输出
	printf_s("表宽：%d,表高：%d\n",pcsv.GetWidth(),pcsv.GetHeight());

	// 按坐标取数据
	printf_s("11:%s,44:%s\n",pcsv.GetVal(1,1).c_str(),pcsv.GetVal(4,4).c_str());
	printf_s("首:%s,末:%s\n",pcsv.GetVal(1,1).c_str(),pcsv.GetVal(pcsv.GetHeight(),pcsv.GetWidth()).c_str());

	// 按行取数据
	const PCsv::row_t& row = pcsv.GetRow(2);
	for (PCsv::crowIt_t crowIt = row.begin(); crowIt != row.end(); ++crowIt)
	{
		printf_s("row no:%d, cnt:%s\n",crowIt->first,crowIt->second.c_str());
	}

	// 一次全部取出来
	const PCsv::tab_t& tab = pcsv.GetTab();
	for (PCsv::ctabIt_t ctabIt = tab.begin(); ctabIt != tab.end(); ++ctabIt)
	{
		printf_s("行号：%d\n",ctabIt->first);
		for (PCsv::crowIt_t crowIt = ctabIt->second.begin(); crowIt != ctabIt->second.end(); ++crowIt)
		{
			printf_s("row no:%d, cnt:%s\n",crowIt->first,crowIt->second.c_str());
		}
	}
	//*/

	return 0;
}

//void callBackFunc(PER_SOCKET_CONTEXT* pPSock,char* data,uint16 dataLen)
//{
//	PMsg pmsg(data,dataLen);
//	int32 id = 0;
//	string msg;
//	pmsg>>msg>>id>>id;
//	cout << "proto:" << pmsg.GetProto() << ",dataLen:" << dataLen << ",len:" << pmsg.GetDataLen() << ",data:" << msg << ",id:" << id << endl;
//	pPSock->Send(data,dataLen);
//}
//
//void onCloseFunc(PSocket* pPSock,uint16 closeType)
//{
//	printf_s("客户端主动对出了，sock：%d,closeType:%d\n",pPSock->sock,closeType);
//}
