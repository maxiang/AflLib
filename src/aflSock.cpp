#ifdef _WIN32
	#include <windows.h>
	#ifndef _WIN32_WCE
		#include <process.h>
	#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aflSock.h"

#ifdef _MSC_VER
	#pragma comment(lib, "wsock32.lib")
	#pragma comment(lib, "winmm.lib")
#endif

#ifndef _WIN32
	#include <errno.h>
#endif
/*
#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define new new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtsetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 
*/
//namespace AFL::Sock
namespace AFL{namespace SOCK{

//------------------------------------------------------------
WSAObject::WSAObject()
{
#ifdef _WIN32
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2,2), &WSAData);
#endif
}
WSAObject::~WSAObject()
{
#ifdef _WIN32
	WSACleanup();
#endif
}
WSAObject Sock::m_wsaObject;

Sock::Sock()
{
	m_Sofd = 0;
	m_baccept = false;
	m_nPort = 0;
	m_nCondition = SOCK_NON;
}

Sock::~Sock()
{
	close();
}
void Sock::close()
{
	if(m_Sofd)
	{
		shutdown(m_Sofd,1);
		shutdown(m_Sofd,2);
	#ifdef _WIN32
		closesocket(m_Sofd);
	#else
		::close(m_Sofd);
	#endif
		m_Sofd = 0;
		m_nPort = 0;
	}
}

void Sock::setArgp(bool bFlag)const
{
	INT flag = bFlag;
	ioctlsocket(m_Sofd,FIONBIO,(ULONG*)&flag);
}

INT Sock::getSockOpt(INT nOptName)
{
	if(!m_Sofd)
		return false;
	INT nBuff;
	INT nLength = sizeof(INT);
	#ifdef _WIN32
		getsockopt(m_Sofd, SOL_SOCKET,nOptName,(char*)&nBuff,&nLength);
	#else
		if(getsockopt(m_Sofd, SOL_SOCKET,nOptName,(char*)&nBuff,(socklen_t*)&nLength) < 0)
			return getError();
	#endif
	return nBuff;
}
bool Sock::create(USHORT usPort,INT nType,INT nProtocol)
{
	close();
	m_nPort = usPort;
	m_Sofd = socket(AF_INET,nType,nProtocol);
	if(m_Sofd != INVALID_SOCKET)
	{
		sockaddr_in inAddr;
		ZeroMemory((char*)&inAddr,sizeof(sockaddr_in));
		inAddr.sin_family = AF_INET;
		inAddr.sin_port = htons(usPort);
		*(LPDWORD)&inAddr.sin_addr = INADDR_ANY;
		m_nCondition = SOCK_CREATE;
		return bind(m_Sofd,(sockaddr *)&inAddr,sizeof(sockaddr)) != SOCKET_ERROR;
	}
	return false;
}

bool Sock::listen( INT nconnectionBacklog ) const
{
	return ::listen(m_Sofd,nconnectionBacklog) != SOCKET_ERROR;
}

INT Sock::getError() const
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}
bool Sock::sendWake(LPCSTR mac)
{
	UINT data[6];
	sscanf(mac,"%02x%*1c%02x%*1c%02x%*1c%02x%*1c%02x%*1c%02x",
		&data[0],&data[1],&data[2],&data[3],&data[4],&data[5]);

	INT i;
	BYTE b[6];
	for(i=0;i<6;i++)
		b[i] = (BYTE)data[i];

	BYTE buff[1024];
	memset(buff,0xff,6);
	for(i=0;i<20;i++)
		memcpy(buff+6+i*6,b,6);

	create(0,SOCK_DGRAM);
	setSockOpt(SO_BROADCAST,true);
	sendTo(buff,132,7);
	return true;
}

bool Sock::connect(sockaddr_in* pInAddr,INT timeout)
{
	m_nCondition = SOCK_CONNECTING;
	create();
	if(timeout)
		setArgp(true);
	if(::connect(m_Sofd,(sockaddr*)pInAddr,sizeof(sockaddr)) == SOCKET_ERROR)
	{
		INT err = getError();
		if(err != WSAEWOULDBLOCK)
		{
			m_nCondition = SOCK_CONNECTERROR;
			return false;
		}

		fd_set fds;
		select(NULL,&fds,NULL,timeout);
		err = getSockOpt(SO_ERROR);
		if(err)
		{
			m_nCondition = SOCK_CONNECTERROR;
			return false;
		}
	}
	m_nCondition = SOCK_CONNECT;
	if(timeout)
		setArgp(false);
	return true;
}
bool Sock::connect(DWORD dwHost,unsigned short usPort,INT timeout)
{
	sockaddr_in sockAddr;
	setSockAddr(&sockAddr,dwHost,usPort);
	return connect(&sockAddr,timeout);
}
bool Sock::connect(const char* pHostName,unsigned short usPort,INT timeout)
{
	sockaddr_in sockAddr;
	setSockAddr(&sockAddr,pHostName,usPort);
	return connect(&sockAddr,timeout);
}
void Sock::setSockAddr(sockaddr_in* pSockAddr,DWORD dwHost,u_short usPort)
{
	ZeroMemory(pSockAddr,sizeof(sockaddr_in));
	pSockAddr->sin_family = AF_INET;
	pSockAddr->sin_addr.s_addr = dwHost;
	pSockAddr->sin_port = htons(usPort);
}
void Sock::setSockAddr(sockaddr_in* pSockAddr,const char* pHostName,u_short usPort)
{
	hostent* pHost;
	ZeroMemory(pSockAddr,sizeof(sockaddr_in));
	pSockAddr->sin_family = AF_INET;
	pHost = gethostbyname(pHostName);
	if(pHost)
		memmove(&pSockAddr->sin_addr,pHost->h_addr,pHost->h_length);
	else
		pSockAddr->sin_addr.s_addr = inet_addr(pHostName);
	pSockAddr->sin_port = htons(usPort);
}
void Sock::attach(SOCKET sock)
{
	if(m_Sofd)
		close();
	m_Sofd = sock;
}
bool Sock::accept(Sock& Sock,INT timeout)
{
	if(accept(Sock.m_Sofd,timeout))
	{
		Sock.setCondition(SOCK_CONNECT);
		return true;
	}
	return false;
}
bool Sock::accept(SOCKET& Sock,INT timeout)
{
	if(!m_Sofd)
		return false;
	INT nSize = sizeof(sockaddr_in);
	struct sockaddr Addr;
	
	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;
	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(*this,&readFD);
	if(timeout < 0)
		::select((int)*this+1,&readFD,NULL,NULL,NULL);
	else
		::select((int)*this+1,&readFD,NULL,NULL,&it);
	if(!FD_ISSET(*this,&readFD))
		return false;

	m_baccept = true;
	SOCKET hTemp = ::accept(m_Sofd,&Addr,(socklen_t*)&nSize);
	if(INVALID_SOCKET != hTemp)
	{
		Sock = hTemp;
		return  true;
	}
	return  false;
}
Sock* Sock::accept(INT timeout)
{
	SOCKET sock;
	if(!accept(sock,timeout))
		return NULL;
	Sock* pSock = new Sock;
	pSock->attach(sock);
	return  pSock;
}
INT Sock::sendTo(const void* lpBuf, INT nBufLen,sockaddr* sockaddrTo,INT nFlags)
{
	return sendto(m_Sofd,(const char*)lpBuf,nBufLen,nFlags,sockaddrTo,sizeof(sockaddr_in));
}

INT Sock::sendTo( const void* lpBuf, INT nBufLen, INT nPort,const char* pAdr,INT nFlags)
{
	if(!m_Sofd)
		return SOCKET_ERROR;
	hostent* pHost;
	sockaddr_in sockaddrTo;
	memset((char*)&sockaddrTo,0,sizeof(sockaddr_in));
	sockaddrTo.sin_family = AF_INET;
	sockaddrTo.sin_port = htons((USHORT)nPort);
	
	if(pAdr)
	{
		sockaddrTo.sin_addr.s_addr = inet_addr(pAdr);
		if(sockaddrTo.sin_addr.s_addr == INADDR_NONE)
		{
			pHost = gethostbyname(pAdr);
			if(!pHost)
				return 0;
			memmove(&sockaddrTo.sin_addr,pHost->h_addr,pHost->h_length);
		}
	}
	else
	{
		sockaddrTo.sin_addr.s_addr = INADDR_BROADCAST;
	}
	return sendTo((const char*)lpBuf,nBufLen,(sockaddr *)&sockaddrTo,nFlags);
}

bool Sock::waitSelect(INT timeout)
{
	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;

	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(*this,&readFD);
	::select((int)*this+1,&readFD,NULL,NULL,&it);
	if(FD_ISSET(*this,&readFD))
		return true;
	return false;
}
bool Sock::waitSend(INT timeout)
{
	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;

	fd_set writeFD;
	FD_ZERO(&writeFD);
	FD_SET(*this,&writeFD);
	if(::select((int)*this+1,NULL,&writeFD,NULL,&it) <= 0)
		return false;
	if(FD_ISSET(*this,&writeFD))
		return true;
	return false;
}
bool Sock::select(fd_set* pReadFds,fd_set* pWriteFds,fd_set* pExceptFds,INT timeout)
{
	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;

	if(pReadFds)
	{
		FD_ZERO(pReadFds);
		FD_SET(*this,pReadFds);
	}
	if(pWriteFds)
	{
		FD_ZERO(pWriteFds);
		FD_SET(*this,pWriteFds);
	}
	if(pExceptFds)
	{
		FD_ZERO(pExceptFds);
		FD_SET(*this,pExceptFds);
	}
	return ::select((int)*this+1,pReadFds,pWriteFds,pExceptFds,&it) != SOCKET_ERROR;
}
INT Sock::recvFrom(void* lpBuf, INT nBufLen,INT nFlags,sockaddr_in* pAdrFrom)
{
	sockaddr_in AdrFrom;
	if(!m_Sofd)
		return SOCKET_ERROR;
	if(!pAdrFrom)
		pAdrFrom = &AdrFrom;

	timeval it={0,0};
	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(*this,&readFD);
	::select((int)*this+1,&readFD,NULL,NULL,&it);
	if(FD_ISSET(*this,&readFD))
		return recvfrom(m_Sofd,(char*)lpBuf,nBufLen,nFlags,(sockaddr *)pAdrFrom,(socklen_t*)&nBufLen);
	return 0;
}
INT Sock::send(const void* lpBuf, INT nBufLen, INT nFlags,INT timeout) const
{
	if(!m_Sofd)
		return SOCKET_ERROR;

	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;
	fd_set writeFD;
	FD_ZERO(&writeFD);
	FD_SET(*this,&writeFD);
	if(::select((int)*this+1,NULL,&writeFD,NULL,&it) == SOCKET_ERROR)
		return SOCKET_ERROR;
	if(FD_ISSET(*this,&writeFD))
		return ::send(m_Sofd,(LPCSTR)lpBuf,nBufLen,nFlags);
	return 0;
}

INT Sock::send2(const void* lpBuf, INT nBufLen, INT nFlags,INT timeout) const
{
	if(!m_Sofd)
		return SOCKET_ERROR;

	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;
	fd_set writeFD;
	FD_ZERO(&writeFD);
	FD_SET(*this,&writeFD);

	INT size = nBufLen;
	while(size)
	{
		if(::select((int)*this+1,NULL,&writeFD,NULL,&it) == SOCKET_ERROR)
			return SOCKET_ERROR;
		if(FD_ISSET(*this,&writeFD))
		{
			INT ret = ::send(m_Sofd,(LPCSTR)lpBuf+(nBufLen - size),size,nFlags);
			if(ret == -1)
				break;
			if(ret > 0)
				size -= ret;
		}
		else
			Sleep(1);
	}
	return nBufLen - size;
}
INT Sock::send2( const void* lpBuf) const
{
	if(!m_Sofd)
		return SOCKET_ERROR;
	if(lpBuf)
		return send2((const char*)lpBuf,(int)strlen((const char*)lpBuf));
	return -1;
}

INT Sock::send( const void* lpBuf) const
{
	if(!m_Sofd)
		return SOCKET_ERROR;
	if(lpBuf)
		return ::send(m_Sofd,(const char*)lpBuf,(int)strlen((const char*)lpBuf),0);
	return -1;
}
bool Sock::isRead()
{
	if(!m_Sofd)
		return false;
	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(*this,&readFD);

	timeval it = {0,0};
	::select((int)*this+1,&readFD,NULL,NULL,&it);
	return FD_ISSET(*this,&readFD) != 0;
}
INT Sock::recv( void* lpBuf, INT nBufLen, INT nFlags,INT timeout)
{
	if(!m_Sofd)
		return SOCKET_ERROR;

	INT nRet = 0;
	timeval it;
	it.tv_sec = timeout/1000;
	it.tv_usec = (timeout%1000)*1000;
	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(*this,&readFD);
	INT ret;
	if(timeout >= 0)
		ret = ::select((int)*this+1,&readFD,NULL,NULL,&it);
	else
		ret = ::select((int)*this+1,&readFD,NULL,NULL,NULL);
	if(ret == SOCKET_ERROR)
		return SOCKET_ERROR;
	if(FD_ISSET(*this,&readFD))
	{
		nRet = ::recv(m_Sofd,(char*)lpBuf,nBufLen,nFlags);
		if(nRet <= 0)
			return SOCKET_ERROR;
	}

	return nRet;
}

INT Sock::getSockAdr(sockaddr_in* pInAddr) const
{
	INT nSize = sizeof(sockaddr_in);
	return getsockname(*this,(sockaddr*)pInAddr,(socklen_t*)&nSize);
}
bool Sock::getSockIP(char* pBuf) const
{
	sockaddr_in inAddr;
	ZeroMemory(&inAddr,sizeof(sockaddr_in));
	getSockAdr(&inAddr);
	strcpy(pBuf,inet_ntoa(inAddr.sin_addr)); 
	return true;
}
DWORD Sock::getSockIP() const
{
	sockaddr_in inAddr;
	ZeroMemory(&inAddr,sizeof(sockaddr_in));
	getSockAdr(&inAddr);
	return *(LPDWORD)&inAddr.sin_addr; 
}
INT Sock::getSockPort() const
{
	sockaddr_in inAddr;
	ZeroMemory(&inAddr,sizeof(sockaddr_in));
	getSockAdr(&inAddr);
	return htons(inAddr.sin_port);
}
DWORD Sock::getPeerIP() const
{
	sockaddr_in inAddr;
	ZeroMemory(&inAddr,sizeof(sockaddr_in));
	getPeerAdr(&inAddr);
	return *(LPDWORD)&inAddr.sin_addr; 
}

INT Sock::getPeerAdr(sockaddr_in* pInAddr) const
{
	INT nSize = sizeof(sockaddr_in);
	return getpeername(*this,(sockaddr*)pInAddr,(socklen_t*)&nSize);
}
bool Sock::getPeerIP(char* pBuf) const
{
	sockaddr_in inAddr;
	getPeerAdr(&inAddr);
	strcpy(pBuf,inet_ntoa(inAddr.sin_addr)); 
	return true;
}
UINT Sock::getPeerPort() const
{
	sockaddr_in inAddr;
	getPeerAdr(&inAddr);
	return htons(inAddr.sin_port);
}
bool Sock::wait()
{
	bool bFlag = false;
	INT nClock = clock();


	while(clock() < nClock+60000)
	{
		char Buff[101];
		INT nByte = recv(Buff,100);
		if(nByte!=SOCKET_ERROR)
		{
			nClock = clock();
			Buff[nByte] = 0;
			if(strchr(Buff,'\n'))
			{
				bFlag=true;
				break;
			}
		}
		else
		{
			if(bFlag)
				break;
		}
		Sleep(10);
	}
	return bFlag;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Socket
// ソケットスレッド対応
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Socket::Socket()
{
	m_bThreadValid = false;

	m_iRecvBuffSize = 8192;
	m_pbyRecvBuff = new BYTE[m_iRecvBuffSize];
	m_bConnect = false;

}
Socket::~Socket()
{
	close();
	delete m_pbyRecvBuff;
}
Sock* Socket::getSock()
{
	return &m_Sock;
}
bool Socket::close()
{
	m_Sock.close();
	m_bThreadValid = false;

	m_aflThread.closeThread();
	return true;
}
bool Socket::connect(LPCSTR pHost,INT iPort)
{
	m_strHostName = pHost;
	m_iPort = iPort;

	//スレッド設定
	m_bThreadValid = true;
	m_aflThread.startThread(CLASSPROC(this,Socket,onThreadProc));
	
	return true;
}
int Socket::send(LPCSTR pString)
{
	return m_Sock.send(pString);
}
int Socket::send(LPCVOID pvData,INT iSize)
{
	return m_Sock.send(pvData,iSize);
}
DWORD Socket::onThreadProc(LPVOID pvData)
{
	bool bConnect = m_Sock.connect(m_strHostName.c_str(),m_iPort);
	m_bConnect = bConnect;	
	//接続動作完了の通知
	m_callConnect.call(this);

	onSocketConnect(bConnect);
	if(!bConnect)
	{
		return 0;
	}
	
	while(m_bThreadValid)
	{
		INT iSize = m_Sock.recv(m_pbyRecvBuff,m_iRecvBuffSize,0,1000);
		if(iSize == SOCKET_ERROR)
		{
			m_bConnect = false;
			m_callClose.call(this);
			onSocketClose();
			break;
		}
		else if(iSize > 0)
		{
			SockData sockData = {this,iSize,m_pbyRecvBuff};
			m_callRecv.call(&sockData);
			onSocketRecv(iSize,m_pbyRecvBuff);
		}
	}
	return 0;
}
void Socket::setConnect(ClassProc& classProc)
{
	m_callConnect = classProc;
}
void Socket::setRecv(ClassProc& classProc)
{
	m_callRecv = classProc;
}
void Socket::setClose(ClassProc& classProc)
{
	m_callClose = classProc;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Ping
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifdef _WIN32
//-------------------------------------------
// 見てのとおりコンストラクタ
//-------------------------------------------
Ping::Ping()
{
	//----------------------------
	//WinSockの初期化
	WSADATA WSAData;
	WSAStartup(MAKEWORD(1,1), &WSAData);

	//----------------------------
	//生ソケットの代わりにICMP-DLLを使用
	m_hinsIcmp = LoadLibraryW(L"icmp.dll");
	if(m_hinsIcmp)
	{
		#ifdef _WIN32_WCE
			m_pIcmpCreateFile = (IcmpCreateFile*)GetProcAddressW(m_hinsIcmp,L"IcmpCreateFile");
			m_pIcmpCloseHandle = (IcmpCloseHandle*)GetProcAddressW(m_hinsIcmp,L"IcmpCloseHandle");
			m_pIcmpSendEcho = (IcmpSendEcho*)GetProcAddressW(m_hinsIcmp,L"IcmpSendEcho");
		#else
			m_pIcmpCreateFile = (IcmpCreateFile*)GetProcAddress(m_hinsIcmp,"IcmpCreateFile");
			m_pIcmpCloseHandle = (IcmpCloseHandle*)GetProcAddress(m_hinsIcmp,"IcmpCloseHandle");
			m_pIcmpSendEcho = (IcmpSendEcho*)GetProcAddress(m_hinsIcmp,"IcmpSendEcho");
		#endif
	}
	//----------------------------
	//ICMPのハンドルを取得
	m_hICMP = 0;
	if(m_pIcmpCreateFile)
	{
		m_hICMP = m_pIcmpCreateFile();
	}
}

//-------------------------------------------
// 見てのとおりデストラクタ
//-------------------------------------------
Ping::~Ping()
{
	//----------------------------
	//さらばICMPハンドル
	if(m_hICMP)
		m_pIcmpCloseHandle(m_hICMP);
	//----------------------------
	//さらばICMPライブラリ
	if(m_hinsIcmp)
		FreeLibrary(m_hinsIcmp);
	//----------------------------
	//さらばWinSock
	WSACleanup();
}
//-------------------------------------------
// ドメイン/IPアドレス文字列を32Bit値に変換、失敗したら0
// ソケット系の文献はけっこう回りくどくこれを書いてある
//-------------------------------------------
DWORD Ping::getAddress(LPCSTR pString)
{
	int i;
	DWORD dwAddr = 0;
	for (i = 0; i < 3; i++)
	{
		if (pString)
		{
			dwAddr = inet_addr(pString);
			if (dwAddr == INADDR_NONE)
			{
				LPHOSTENT pHost = gethostbyname(pString);
				if (pHost)
					dwAddr = *(LPDWORD)*pHost->h_addr_list;
				else
					dwAddr = 0;
			}
		}
		Sleep(1);
	}
	return dwAddr;
}
//-------------------------------------------
// TraceRoute実行
//-------------------------------------------
BOOL Ping::trace(LPCSTR pString)
{
	if(m_hICMP)
	{
		int i;
		IPINFO infoIP;
		ICMPECHO echoICMP;
	
		DWORD dwAdr = getAddress(pString);
		for(i=1;i<255;i++)
		{
			ZeroMemory(&infoIP,sizeof(IPINFO));
			infoIP.byTtl = i;
			if(m_pIcmpSendEcho(m_hICMP,dwAdr,NULL,0,&infoIP,&echoICMP,sizeof(ICMPECHO),5000))
				onTrace(pString,i , echoICMP.dwSource , echoICMP.dwRTTime);
			if(dwAdr == echoICMP.dwSource)
				return TRUE;
		}
	}
	return FALSE;
}
//-------------------------------------------
// PING実行
//-------------------------------------------
INT Ping::ping(LPCSTR hostAdr,INT timeout)
{
	if(m_hICMP)
	{
		IPINFO infoIP;
		ICMPECHO echoICMP;
	
		DWORD dwAdr = getAddress(hostAdr);
		if(dwAdr == 0)
			return -1;
		ZeroMemory(&infoIP,sizeof(IPINFO));
		infoIP.byTtl = 255;
		if(m_pIcmpSendEcho(m_hICMP,dwAdr,NULL,0,&infoIP,&echoICMP,sizeof(ICMPECHO),timeout))
		{
			if(echoICMP.dwStatus)
				return -1;
			//onTrace(pString,257 - echoICMP.infoIP.byTtl , echoICMP.dwSource , echoICMP.dwRTTime);
			return echoICMP.dwRTTime;
		}
	}
	return -1;
}
#else
Ping::Ping()
{
#ifdef _WIN32
	//----------------------------
	//WinSockの初期化
	WSADATA WSAData;
	WSAStartup(MAKEWORD(1,1), &WSAData);
#endif
}
Ping::~Ping()
{
#ifdef _WIN32
	//----------------------------
	//さらばWinSock
	WSACleanup();
#endif
}

BOOL Ping::ping(PCSTR pString)
{
	IPHDR ipHDR;
	ICMPHDR icmpHDR;
	SOCKET sock;
	DWORD dwTime;
	sockaddr_in sockaddrTo;
	sockaddr_in sockaddrRecv;
	int nSize = sizeof(sockaddr);

	//---------------------------
	//Select用変数準備
	timeval timeVal={5,0};
	fd_set fdSet;
	FD_ZERO(&fdSet);
	
	//---------------------------
	//ICMPヘッダ設定
	ZeroMemory(&icmpHDR,sizeof(ICMPHDR));
	icmpHDR.Type = ICMP_ECHOREQ;
	icmpHDR.Checksum = getCheckSum((PWORD)&icmpHDR, sizeof(ICMPHDR));
	
	//---------------------------
	//生ソケット作成
	sock = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	if(sock == SOCKET_ERROR)
		return FALSE;

	//---------------------------
	//送信アドレス設定
	memset((char*)&sockaddrTo,0,sizeof(sockaddr_in));
	sockaddrTo.sin_family = AF_INET;
	sockaddrTo.sin_addr.s_addr = getAddress(pString);

	//時間の記録
	dwTime = timeGetTime();
	//ICMPメッセージを送信
	sendto(sock,(LPSTR)&icmpHDR,sizeof(ICMPHDR),0,(sockaddr*)&sockaddrTo,sizeof(sockaddr));
	//受信までタイムアウト5秒で待機
	FD_SET(sock,&fdSet);
	select(sock+1,&fdSet,NULL,NULL,&timeVal);
	if(!FD_ISSET(sock,&fdSet))
		return FALSE;
	//応答メッセージ受信
	recvfrom(sock,(char*)&ipHDR,sizeof(ipHDR),0,(sockaddr*)&sockaddrRecv,(socklen_t*)&nSize);
	//応答時間を出す
	dwTime = timeGetTime()-dwTime;
	//オーバライド用仮想関数の呼び出し
	onTrace(pString,257 - ipHDR.TTL, *(PDWORD)&ipHDR.inAddrSrc,dwTime);
	closesocket(sock);

	return TRUE;
}
BOOL Ping::trace(LPCSTR pString)
{
	int i;
	IPHDR ipHDR;
	ICMPHDR icmpHDR;
	SOCKET sock;
	DWORD dwTime;
	sockaddr_in sockaddrTo;
	sockaddr_in sockaddrRecv;
	int nSize = sizeof(sockaddr);

	
	//---------------------------
	//Select用変数準備
	timeval timeVal={5,0};
	fd_set fdSet;
	FD_ZERO(&fdSet);
	
	//---------------------------
	//ICMPヘッダ設定
	ZeroMemory(&icmpHDR,sizeof(ICMPHDR));
	icmpHDR.Type = ICMP_ECHOREQ;
	icmpHDR.Checksum = getCheckSum((PWORD)&icmpHDR, sizeof(ICMPHDR));
	
	//---------------------------
	//生ソケット作成
	sock = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	if(sock == SOCKET_ERROR)
		return FALSE;

	//---------------------------
	//送信アドレス設定
	memset((char*)&sockaddrTo,0,sizeof(sockaddr_in));
	sockaddrTo.sin_family = AF_INET;
	sockaddrTo.sin_addr.s_addr = getAddress(pString);

	for(i=1;i<255;i++)
	{
		//TTL設定（ルーティングの寿命）
		setsockopt(sock,IPPROTO_IP,IP_TTL,(char*)&i,sizeof(int));
		//時間の記録
		dwTime = timeGetTime();
		//ICMPメッセージを送信
		sendto(sock,(LPSTR)&icmpHDR,sizeof(ICMPHDR),0,(sockaddr*)&sockaddrTo,sizeof(sockaddr));
		//受信までタイムアウト5秒で待機
		FD_SET(sock,&fdSet);
		select(sock+1,&fdSet,NULL,NULL,&timeVal);
		if(!FD_ISSET(sock,&fdSet))
			return FALSE;
		//応答メッセージ受信
		recvfrom(sock,(char*)&ipHDR,sizeof(ipHDR),0,(sockaddr*)&sockaddrRecv,(socklen_t*)&nSize);
		//応答時間を出す
		dwTime = timeGetTime()-dwTime;
		//オーバライド用仮想関数の呼び出し
		onTrace(pString,i, *(PDWORD)&ipHDR.inAddrSrc,dwTime);
		//目的地まで到着したら終了
		if(sockaddrTo.sin_addr.s_addr == *(PDWORD)&ipHDR.inAddrSrc)
		{
			closesocket(sock);
			return TRUE;
		}
	}
	closesocket(sock);
	return FALSE;
}
DWORD Ping::getAddress(LPCSTR pString)
{
	DWORD dwAddr = 0;
	if(pString)
	{
		dwAddr = inet_addr(pString);
		if(dwAddr == INADDR_NONE)
		{
			hostent* pHost = gethostbyname(pString);
			if(pHost)
				dwAddr = *(LPDWORD)*pHost->h_addr_list;
		}
	}
	return dwAddr;
}
WORD Ping::getCheckSum(USHORT* pAddr, int nLen)
{
	int i;
	int nSum = 0;
	WORD *pData = pAddr;

	for(i=nLen;i>1;i-=2)
		nSum += *pData++;
	if(i)
		nSum += *(UCHAR*)pData;

	nSum = (nSum >> 16) + (nSum & 0xffff);
	nSum += (nSum >> 16);	
	return (WORD)~nSum;
}
void Ping::onTrace(LPCSTR pString,int nNumber,DWORD dwAdr,int nPing)
{
//	printf("%d %s %d\n",nNumber,inet_ntoa(*(in_addr*)&dwAdr),nPing);
}
#endif
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PingThread
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PingThread::PingThread()
{
	m_pTrace = NULL;
}
bool PingThread::ping(LPCSTR pString)
{
	m_strHostName = pString;
	//m_aflClassCallBack.setAddress(this,PROC(PingThread,threadPing));
	m_aflThread.startThread(m_aflClassCallBack,0);
	return true;
}
DWORD PingThread::threadPing(LPVOID pvData)
{
	Ping::ping(m_strHostName.c_str());
	return 0;
}

#pragma warning(push)
#pragma warning(disable:4312)

void PingThread::onTrace(LPCSTR pString,int nNumber,DWORD dwAdr,int iPing)
{
	LPVOID pData[] ={(LPVOID)this,(LPVOID)pString,reinterpret_cast<LPVOID>(dwAdr),(LPVOID)iPing};
	if(m_pTrace)
		m_pTrace->call((LPVOID)pData);
}

#pragma warning( pop )

void PingThread::setTrace(ClassProc* pTrace)
{
	m_pTrace = pTrace;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HttpUrl::setUrl(LPCSTR url)
{
	std::string work;
	LPCSTR buff,buff2;

	//プロトコル部の確認
	buff = strchr(url,':');
	if(buff)
	{
		work.assign(url,buff-url);
		url = buff + 1;
	}
	if(work != "http")
		return false;
	m_proto = "http";

	//絶対指定確認
	if(strncmp(url,"//",2))
		return false;
	url += 2;

	//ホスト部
	work.clear();
	buff2 = strchr(url,'/');
	buff = strchr(url,':');
	if(buff2 && buff > buff2)
		buff = NULL;
	if(buff)
	{
		work.assign(url,buff-url-1);
		url = buff + 1;

		INT port = 0;
		for(;*url && *url>='0' && *url<='9';url++)
		{
			port *= 10;
			port += *url - '0';
		}
		m_port = port;
	}
	else
	{
		buff = strchr(url,'/');
		if(buff)
		{
			work.assign(url,buff-url);
			url = buff;
		}
		else
			work = url;
		m_port = 80;
		
	}
	m_host = work;
	m_path = url;
	if(m_path.length() == 0)
		m_path = '/';

	return true;
}
LPCSTR HttpUrl::getProto() const
{
	return m_proto.c_str();
}
INT HttpUrl::getPort() const
{
	return m_port;
}
LPCSTR HttpUrl::getHost() const
{
	return m_host.c_str();
}
LPCSTR HttpUrl::getPath() const
{
	return m_path.c_str();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpData
// httpデータ受信
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HttpData::read(LPCSTR urlAddr)
{
	//URL成分の分解
	HttpUrl url;
	if(!url.setUrl(urlAddr))
		return false;

	//コマンドの生成
	String command;
	command.printf("GET %s HTTP/1.1\r\n",urlAddr);

	//接続
	AFL::SOCK::Sock socket;
	if(m_proxyName.length())
	{
		//プロキシ接続
		if(!socket.connect(m_proxyName,m_proxyPort,5000))
			return false;
		if(m_proxyPass.length())
		{
			String userData;
			userData.printf("%s:%s",m_proxyID.c_str(),m_proxyPass.c_str());
			String passString;
			AtoB64(passString,userData);

			String authString;
			authString.printf("Proxy-Authorization: Basic %s\r\n",passString.c_str());
			command += authString;
		}
	}
	else
	{
		if(!socket.connect(url.getHost(),url.getPort(),5000))
			return false;
	}

	String host;
	host.printf("HOST: %s\r\n",url.getHost());
	command += host;

	command += "User-Agent: Mozilla/5.0 (Windows NT 5.0; rv:11.0) Gecko/20100101 Firefox/11.0\r\n"
			"Connection: close\r\n";

	std::map<String,String>::iterator it;
	for(it=m_outHeader.begin();it!=m_outHeader.end();++it)
	{
		command.appendf("%s: %s\r\n",it->first.c_str(),it->second.c_str());
	}
	command.append("\n");

	socket.send(command.c_str(),(INT)command.length());

	LPCSTR headerLast = NULL;

	std::string work;
	CHAR buff[0xFFFF];
	int size;
	while((size = socket.recv(buff,sizeof(buff),0,3000)) > 0)
	{
		work.append(buff,size);
		headerLast = strstr(work.c_str(),"\r\n\r\n");
		if(headerLast)
			break;
	}
	if(!headerLast)
		return false;

	//bool header = true;
	//ヘッダの受け取り
	LPCSTR data = work.c_str();
	INT headerSize = (INT)(headerLast-data);
	m_header.assign(data,headerSize+2);
	work.erase(0,headerSize+4);

	LPCSTR contentLength = strstr(m_header.c_str(),"Content-Length:");
	INT length = 0;
	if(contentLength)
		sscanf(contentLength+15,"%d",&length);

	INT buffSize = sizeof(buff);
	while((size = socket.recv(buff,buffSize,0,5000)) > 0)
	{
		work.append(buff,size);
		//printf("(%d)",size);
		if(length)
		{
			if(buffSize > length-(INT)work.size())
				buffSize = length-(INT)work.size();
			if(work.size() == length)
				break;
		}
	}

	m_body.assign(work.c_str(),work.size());
	
	return true;
}
LPCSTR HttpData::getHeader() const
{
	return m_header.c_str();
}
LPCSTR HttpData::getBody() const
{
	return m_body.c_str();
}
void HttpData::setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass)
{
	m_proxyName = name;
	m_proxyPort = port;
	m_proxyID = id;
	m_proxyPass = pass;
}

//namespace
};};

