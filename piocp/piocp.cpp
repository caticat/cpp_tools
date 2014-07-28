#include "piocp.h"

using std::string;

PER_IO_CONTEXT::PER_IO_CONTEXT()
{
	memset(&overlapped,0,sizeof(overlapped));
	memset(&cbuf,0,PIOCP_MAX_BUFFER_LEN);
	sock = INVALID_SOCKET;
	wsaBuf.buf = cbuf;
	wsaBuf.len = PIOCP_MAX_BUFFER_LEN;
	opType = NULL_POSTED;
}

PER_IO_CONTEXT::~PER_IO_CONTEXT()
{
	RELEASE_SOCKET(sock);
}

void PER_IO_CONTEXT::ResetBuffer()
{
	memset(cbuf,0,PIOCP_MAX_BUFFER_LEN);
}

PER_SOCKET_CONTEXT::PER_SOCKET_CONTEXT()
{
	sock = INVALID_SOCKET;
	memset(&addr,0,sizeof(addr));
}

PER_SOCKET_CONTEXT::~PER_SOCKET_CONTEXT()
{
	RELEASE_SOCKET(sock);

	// 释放掉所有的IO上下文数据
	for (itIoContextArr_t it = ioContextArr.begin(); it != ioContextArr.end(); ++it)
		delete *it;
	ioContextArr.clear();
}

PER_IO_CONTEXT* PER_SOCKET_CONTEXT::GetNewIoContext()
{
	PER_IO_CONTEXT* p = new PER_IO_CONTEXT;
	ioContextArr.push_back(p);
	return p;
}

void PER_SOCKET_CONTEXT::RemoveIoContext(PER_IO_CONTEXT* pIoContext)
{
	PIOCP_CHECK_POINTER(pIoContext,"RemoveContext空指针\n");
	for (itIoContextArr_t it = ioContextArr.begin(); it != ioContextArr.end(); ++it)
	{
		if (pIoContext == *it)
		{
			RELEASE(pIoContext);
			ioContextArr.erase(it);
			break;
		}
	}
}

PIOCP::PIOCP() : 
				m_nThreads(0),
				m_hShutdownEvent(NULL),
				m_hIOCompletionPort(NULL),
				m_phWorkerThreads(NULL),
				m_strIP(PIOCP_DEFAULT_IP),
				m_nPort(PIOCP_DEFAULT_PORT),
				m_lpfnAcceptEx(NULL),
				m_lpfnGetAcceptExSockAddrs(NULL)

{
	
}

PIOCP::~PIOCP()
{
	this->Stop();
}

bool PIOCP::Start()
{
	if (!_LoadSocketLib())
	{
		printf_s("加载socket库失败\n");
		return false;
	}
	else
	{
		printf_s("加载socket库成功\n");
	}

	InitializeCriticalSection(&m_csSocketContextArr); // 初始化线程互斥量

	// 信号的使用
	// 一个Event被创建以后,可以用OpenEvent()API来获得它的Handle,用CloseHandle()来关闭它,
	// 用SetEvent()或PulseEvent()来设置它使其有信号,用ResetEvent()来使其无信号,
	// 用WaitForSingleObject()或WaitForMultipleObjects()来等待其变为有信号
	// lpEventAttributes 一般为NULL
	// bManualReset 创建的Event是自动复位还是人工复位.如果true,人工复位,一旦该Event被设置为有信号,则它一直会等到ResetEvent()API被调用时才会恢复为无信号.
	// bInitialState 初始状态,true,有信号,false无信号
	// lpName 事件对象的名称。您在OpenEvent函数中可能使用。
	m_hShutdownEvent = CreateEvent(NULL,TRUE,FALSE,NULL); // 建立系统退出事件通知

	if (!_InitializeIOCP())
	{
		printf_s("初始化IOCP失败\n");
		return false;
	}
	else
	{
		printf_s("初始化IOCP完毕\n");
	}

	if (!_InitializeListenSocket())
	{
		printf_s("Listen Socket初始化失败\n");
		_DeInitialize();
		return false;
	}
	else
	{
		printf_s("Listen Socket初始化完毕\n");
	}

	printf_s("系统准备就绪，等待连接\n");
	return true;
}

void PIOCP::Stop()
{
	if ((m_pListenContext != NULL) && (m_pListenContext->sock != INVALID_SOCKET))
	{
		SetEvent(m_hShutdownEvent); // 激活关闭网络服务通知

		for (int16 i = 0; i < m_nThreads; ++i)
		{
			PostQueuedCompletionStatus(m_hIOCompletionPort,0,(DWORD)EXIT_CODE,NULL); // 通知所有完成端口线程退出
		}

		WaitForMultipleObjects(m_nThreads,m_phWorkerThreads,TRUE,INFINITE); // 等待所有完成端口线程退出

		this->_ClearSocketContext(); // 清空服务器记录的已经连接客户端信息列表

		this->_DeInitialize(); // 释放其他资源

		RELEASE(m_pListenContext);

		_UnLoadSocketLib();

		printf_s("已经停止网络服务\n");
	}
}

string PIOCP::GetLocalIP()
{
	// 获得本地主机名
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);
	hostent FAR* lpHostEnt = gethostbyname(hostname);
	if (lpHostEnt == NULL)
	{
		return PIOCP_DEFAULT_IP;
	}

	// 取得IP地址列表中第一个返回的IP（因为一台主机可能会绑定多个IP）
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// 将IP地址转化为字符串
	in_addr inAddr;
	memmove(&inAddr,lpAddr,4);
	m_strIP = inet_ntoa(inAddr);
	return m_strIP;
}

void PIOCP::SetPort(int port)
{
	m_nPort = port;
}

bool PIOCP::_LoadSocketLib()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != NO_ERROR)
	{
		printf_s("初始化WinSock2.2失败\n");
		return false;
	}
	return true;
}

void PIOCP::_UnLoadSocketLib()
{
	WSACleanup();
}

bool PIOCP::_InitializeIOCP()
{
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0); // 创建完成端口
	if (m_hIOCompletionPort == NULL)
	{
		printf_s("创建完成端口失败，错误码：%d\n",WSAGetLastError());
		return false;
	}

	m_nThreads = _GetNoOfProcessor() * WORKER_THREAD_PER_PROCESSOR; // 根据本机中的处理器数量，建立对应的线程数

	m_phWorkerThreads = new HANDLE[m_nThreads]; // 为工作者线程初始化指针

	// 建立工作者线程
	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; ++i)
	{
		THREADPARAMS_WORKER* pThreadParam = new THREADPARAMS_WORKER;
		pThreadParam->nThreadNo = i+1;
		pThreadParam->pPIOCP = this;
		m_phWorkerThreads[i] = CreateThread(0,0,_WorkerThread,(void*)pThreadParam,0,&nThreadID);
	}

	printf_s("建立工作者线程(_WorkerThread)%d个\n",m_nThreads);
	return true;
}

bool PIOCP::_InitializeListenSocket()
{
	m_pListenContext = new PER_SOCKET_CONTEXT; // 生成用于监听的socket上下文信息
	m_pListenContext->sock = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED); // 需要使用重叠IO，必须的使用WSASocket来建立socket，才可以支持重叠IO操作
	if (m_pListenContext->sock == INVALID_SOCKET)
	{
		printf_s("初始化socket失败，错误码：%d\n",WSAGetLastError());
		return false;
	}
	else
	{
		printf_s("WSASocket完成\n");
	}

	// 将ListenSocket绑定到完成端口上
	if (CreateIoCompletionPort((HANDLE)m_pListenContext->sock,m_hIOCompletionPort,(DWORD)m_pListenContext,0) == NULL)
	{
		printf_s("将ListenSocket绑定至完成端口失败，错误码：%d\n",WSAGetLastError());
		return false;
	}
	else
	{
		printf_s("将ListenSocket绑定至完成端口成功。\n");
	}

	// 服务器监听地址端口设置
	SOCKADDR_IN serverAddr;
	memset(&serverAddr,0,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY); // 绑定任何可用的ip
	//serverAddr.sin_addr.s_addr = inet_addr(m_strIP.GetString()); // 绑定指定的ip
	serverAddr.sin_port = htons(m_nPort);

	// listenSocket与地址绑定
	if (bind(m_pListenContext->sock,(sockaddr*)&serverAddr,sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf_s("bind函数错误\n");
		return false;
	}
	else
	{
		printf_s("bind地址与监听socket成功\n");
	}

	// 开始监听
	if (listen(m_pListenContext->sock,SOMAXCONN) == SOCKET_ERROR)
	{
		printf_s("listen函数执行错误\n");
		return false;
	}
	else
	{
		printf_s("listen执行成功\n");
	}

	// 使用AcceptEx函数，因为这个是属于WinSock2规范之外的微软另外提供的扩展函数
	// 所以需要额外获取一下函数的指针，
	// 获取AcceptEx函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL
		))
	{
		printf_s("WSAIoctl未能获取AcceptEx函数指针，错误码：%d\n",WSAGetLastError());
		this->_DeInitialize();
		return false;
	}

	// 获取GetAcceptExSockAddrs函数指针
	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	if (SOCKET_ERROR == WSAIoctl(
		m_pListenContext->sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockAddrs,
		sizeof(guidGetAcceptExSockAddrs),
		&m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL
		))
	{
		printf_s("WSAIoctl未能获取GetAcceptExSockAddrs指针，错误码：%d\n",WSAGetLastError());
		this->_DeInitialize();
		return false;
	}

	// 为AcceptEx准备参数，然后投递AcceptEx I/O请求
	for (int i = 0; i < MAX_POST_ACCEPT; ++i)
	{
		PER_IO_CONTEXT* pIoContext = m_pListenContext->GetNewIoContext();
		if (!this->_PostAccept(pIoContext))
		{
			m_pListenContext->RemoveIoContext(pIoContext);
			printf_s("初始化监听 投递Accept失败\n");
			return false;
		}
	}

	printf_s("投递%d个AcceptEx请求完毕\n",MAX_POST_ACCEPT);
	return true;
}

void PIOCP::_DeInitialize()
{
	DeleteCriticalSection(&m_csSocketContextArr);

	RELEASE_HANDLE(m_hShutdownEvent); // 关闭系统退出事件

	// 释放工作者线程句柄指针
	for (int i = 0; i < m_nThreads; ++i)
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}
	RELEASE_ARR(m_phWorkerThreads);

	RELEASE_HANDLE(m_hIOCompletionPort); // 关闭IOCP句柄

	RELEASE(m_pListenContext); // 释放监听上下文

	printf_s("释放资源完毕\n");
}

bool PIOCP::_PostAccept(PER_IO_CONTEXT* pIoContext)
{
	PIOCP_CHECK_BOOL_BOOL((INVALID_SOCKET != m_pListenContext->sock),"_PostAccept监听socket无效\n");

	// 准备参数
	DWORD dwBytes = 0;
	pIoContext->opType = ACCEPT_POSTED;
	WSABUF* pWSABuf = &pIoContext->wsaBuf;
	OVERLAPPED* pOL = &pIoContext->overlapped;

	// 为以后新连接的客户端准备好socket（这是与传统accept的最大区别）
	pIoContext->sock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	if (pIoContext->sock == INVALID_SOCKET)
	{
		printf_s("创建用于accept的socket失败，错误码：%d\n",WSAGetLastError());
		return false;
	}

	// 投递AcceptEx
	if (FALSE == m_lpfnAcceptEx(m_pListenContext->sock,pIoContext->sock,pWSABuf->buf,
		pWSABuf->len-((sizeof(SOCKADDR_IN)+16)*2),sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&dwBytes,pOL))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			printf_s("投递AcceptEx请求失败，错误码：%d\n",WSAGetLastError());
			return false;
		}
	}

	return true;
}

bool PIOCP::_PostRecv(PER_IO_CONTEXT* pIoContext)
{
	PIOCP_CHECK_POINTER_BOOL(pIoContext,"_PostRecv无效的ioContext指针\n");

	// 初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF* pWSABuf = &pIoContext->wsaBuf;
	OVERLAPPED* pOL = &pIoContext->overlapped;

	pIoContext->ResetBuffer();
	pIoContext->opType = RECV_POSTED;

	// 投递WSARecv请求
	int nBytesRecv = WSARecv(pIoContext->sock,pWSABuf,1,&dwBytes,&dwFlags,pOL,NULL);
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		printf_s("投递WSARecv失败\n");
		return false;
	}
	return true;
}

bool PIOCP::_DoAccept(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* pServerAddr = NULL;
	SOCKADDR_IN* pClientAddr = NULL;
	int serverAddrLen = sizeof(SOCKADDR_IN), clientAddrLen = sizeof(SOCKADDR_IN);

	// 获取客户端、服务器地址信息，同时获取客户端发来的第一组数据
	m_lpfnGetAcceptExSockAddrs(pIoContext->wsaBuf.buf,pIoContext->wsaBuf.len-((sizeof(SOCKADDR_IN)+16)*2),
		sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,(LPSOCKADDR*)&pServerAddr,&serverAddrLen,(LPSOCKADDR*)&pClientAddr,&clientAddrLen);

	printf_s("客户端[%s:%d]连入,str:%s\n",inet_ntoa(pClientAddr->sin_addr),ntohs(pClientAddr->sin_port),pIoContext->cbuf);
	// TODO:PJ 数据处理没有做

	// pSocketContext是listenSocket上的context，还需要监听下一个连接，所以需要新建一个socketContext对应新连入的socket
	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->sock = pIoContext->sock;
	memcpy(&(pNewSocketContext->addr),pClientAddr,sizeof(SOCKADDR_IN));

	// 将新连入的socket绑定完成端口
	if (!_AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		printf_s("_DoAccept绑定完成端口失败\n");
		return false;
	}

	// 建立newSocketContext下的newIoContext用于在这个socket上投递的第一个Recv请求
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->opType = RECV_POSTED;
	pNewIoContext->sock = pNewSocketContext->sock;
	if (!_PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveIoContext(pNewIoContext);
		printf_s("_DoAccept上第一次投递接收请求失败\n");
		return false;
	}

	// 投递成功，将有效客户端信息添加到socketContextArr中统一管理
	_AddSocketContext(pNewSocketContext);
	
	// 将ListenSocket的ioContext重置，准备投递新的AcceptEx
	pIoContext->ResetBuffer();
	return _PostAccept(pIoContext);
}

bool PIOCP::_DoRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_IO_CONTEXT* pIoContext)
{
	// 取数据
	SOCKADDR_IN* pClientAddr = &(pSocketContext->addr);
	printf_s("收到[%s:%d]信息：%s\n",inet_ntoa(pClientAddr->sin_addr),ntohs(pClientAddr->sin_port),pIoContext->wsaBuf.buf);

	// 投递下一个WSARecv请求
	return _PostRecv(pIoContext);
}

void PIOCP::_AddSocketContext(PER_SOCKET_CONTEXT* pSocketContext)
{
	PIOCP_CHECK_POINTER(pSocketContext,"_AddSocketContext参数pSocketContext空指针\n");

	EnterCriticalSection(&m_csSocketContextArr);

	m_socketContextArr.push_back(pSocketContext);

	LeaveCriticalSection(&m_csSocketContextArr);
}

void PIOCP::_RemoveSocketContext(PER_SOCKET_CONTEXT* pSocketContext)
{
	PIOCP_CHECK_POINTER(pSocketContext,"_RemoveSocketContext参数pSocketContext空指针\n");

	EnterCriticalSection(&m_csSocketContextArr);

	for (itSocketContextArr_t it = m_socketContextArr.begin(); it != m_socketContextArr.end(); ++it)
	{
		if (*it == pSocketContext)
		{
			RELEASE(pSocketContext);
			m_socketContextArr.erase(it);
			break;
		}
	}

	LeaveCriticalSection(&m_csSocketContextArr);
}

void PIOCP::_ClearSocketContext()
{
	EnterCriticalSection(&m_csSocketContextArr);

	for (itSocketContextArr_t it = m_socketContextArr.begin(); it != m_socketContextArr.end(); ++it)
	{
		delete *it;
	}
	m_socketContextArr.clear();

	LeaveCriticalSection(&m_csSocketContextArr);
}

bool PIOCP::_AssociateWithIOCP(PER_SOCKET_CONTEXT* pSocketContext)
{
	PIOCP_CHECK_POINTER_BOOL(pSocketContext,"_AssociateWithIOCP参数pSocketContext空指针\n");

	if (CreateIoCompletionPort((HANDLE)pSocketContext->sock,m_hIOCompletionPort,(DWORD)pSocketContext,0) == NULL)
	{
		printf_s("绑定socket和完成端口错误，错误码：%d\n",WSAGetLastError());
		return false;
	}

	return true;
}

bool PIOCP::_HandleError(PER_SOCKET_CONTEXT* pSocketContext,DWORD dwError)
{
	PIOCP_CHECK_POINTER_BOOL(pSocketContext,"_HandleError参数pSocketContext空指针\n");

	if (WAIT_TIMEOUT == dwError) // 超时
	{
		if (!_IsSocketAlive(pSocketContext->sock)) // 客户端连接断开了
		{
			printf_s("检测到客户端异常退出\n");
			_RemoveSocketContext(pSocketContext);
			return true;
		}
		else
		{
			printf_s("网络操作超时，重试中...\n");
			return true;
		}
	}
	else if (ERROR_NETNAME_DELETED == dwError) // 客户端异常退出了
	{
		printf_s("检测到客户端异常退出\n");
		_RemoveSocketContext(pSocketContext);
		return true;
	}
	else
	{
		printf_s("完成端口操作出现错误，线程退出，错误码：%d\n",WSAGetLastError());
		return false;
	}
}

DWORD WINAPI PIOCP::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	PIOCP* pPIOCP = pParam->pPIOCP;
	int nThreadNo = pParam->nThreadNo;
	printf_s("工作者线程启动，线程ID：%d\n",nThreadNo);

	OVERLAPPED* pOverlapped = NULL;
	PER_SOCKET_CONTEXT* pSocketContext = NULL;
	DWORD dwBytesTransfered = 0;

	// 循环处理请求，直道接收到ShutdownEvent为止
	while (WAIT_OBJECT_0 != WaitForSingleObject(pPIOCP->m_hShutdownEvent,0))
	{
		BOOL bReturn = GetQueuedCompletionStatus(
			pPIOCP->m_hIOCompletionPort,
			&dwBytesTransfered,
			(PULONG_PTR)&pSocketContext,
			&pOverlapped,
			INFINITE);

		if (EXIT_CODE == (DWORD)pSocketContext) // 收到退出信号，退出循环
		{
			break;
		}

		if (!bReturn) // 是否出现错误
		{
			DWORD dwErr = GetLastError();
			if (!pPIOCP->_HandleError(pSocketContext,dwErr))
			{
				break;
			}
			continue;
		}
		else
		{
			// CONTAINING_RECORD 根据结构体中的某成员的指针来推算出该结构体的指针!
			// 需要提供:结构体中某个成员变量的地址, 该结构体的原型, 该结构体中的某个成员变量(与前面要是同一个变量)
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped,PER_IO_CONTEXT,overlapped);
			if ((0 == dwBytesTransfered) && (RECV_POSTED == pIoContext->opType || SEND_POSTED == pIoContext->opType)) // 客户端正常退出，主动断开连接时调用
			{
				printf_s("客户端[%s:%d]断开连接\n",inet_ntoa(pSocketContext->addr.sin_addr),htons(pSocketContext->addr.sin_port));
				pPIOCP->_RemoveSocketContext(pSocketContext);
				continue;
			}
			else
			{
				switch (pIoContext->opType)
				{
				case ACCEPT_POSTED:
					{
						pPIOCP->_DoAccept(pSocketContext,pIoContext);
						break;
					}
				case RECV_POSTED:
					{
						pPIOCP->_DoRecv(pSocketContext,pIoContext);
						break;
					}
				case SEND_POSTED:
					{
						// TODO:PJ 这里没有写
						break;
					}
				default:
					{
						printf_s("_WorkerThread中的opType类型异常，异常类型:%d.\n",pIoContext->opType);
						break;
					}
				}
			}
		}
	}

	printf_s("工作者线程ID:%d退出\n",nThreadNo);
	RELEASE(lpParam);
	return 0;
}

int PIOCP::_GetNoOfProcessor()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的
bool PIOCP::_IsSocketAlive(SOCKET s)
{
	if (send(s,"",0,0) == SOCKET_ERROR)
		return false;
	return true;
}
