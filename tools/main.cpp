#include <iostream>
#include "plog.h"

using std::cout;
using std::endl;

int main()
{
	CPLog log;
	log.init("E:/","Error",".txt",true,true);
	log.log("1111");
	log.log("%d:%s",2222,"lalala");
	log.log("3333");

	return 0;
}
