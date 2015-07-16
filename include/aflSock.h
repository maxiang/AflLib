#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_Sock

#include "aflStd.h"
#include <string>
#ifdef _WIN32_WCE
	#include "Winsock2.h"
#endif

#ifndef _WIN32
	#define WSAEWOULDBLOCK EINPROGRESS
#endif
#include <stdio.h>
//namespace AFL::Sock
namespace AFL{namespace SOCK{

typedef struct tagICMPHDR
{
	UCHAR	Type;			
	UCHAR	Code;			
	USHORT	Checksum;		
	USHORT	ID;				
	USHORT	Seq;			
	CHAR	Data;
}ICMPHDR,*PICMPHDR,*LPICMPHDR;
typedef struct tagTCPHDR
{
	USHORT	SrcPort;			
	USHORT	DestPort;			
	ULONG	Seq;		
	ULONG	Ack;
	USHORT	Dm2;
	USHORT  Size;
	ULONG	Dm1;
	BYTE	Data[1];			
}TCPHDR,*PTCPHDR,*LPTCPHDR;

typedef struct tagIPHDR
{
	UCHAR	VIHL;			
	UCHAR	TOS;			
	SHORT	TotLen;			
	SHORT	ID;				
	SHORT	FlagOff;		
	UCHAR	TTL;			
	UCHAR	Protocol;		
	USHORT	Checksum;		
	in_addr inAddrSrc;	
	in_addr inAddrDst;
	union
	{
		ICMPHDR icmpHdr;
		TCPHDR tcpHdr;
	};
}IPHDR,*PIPHDR,*LPIPHDR;


class WSAObject
{
public:
	WSAObject();
	~WSAObject();
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Sock
// ソケット用基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Sock 
{
public:
	enum
	{
		SOCK_NON,
		SOCK_CREATE,
		SOCK_CONNECTING,
		SOCK_CONNECT,
		SOCK_CONNECTERROR
	};

	operator const SOCKET()const{return m_Sofd;}
	Sock();
	virtual ~Sock();
	void attach(SOCKET sock);

	bool create(USHORT usPort=0 ,INT nType=SOCK_STREAM,INT nProtocol=0);
	bool listen( INT nconnectionBacklog = 5 ) const;
	
	bool connect(sockaddr_in* pInAddr,INT timeOut=0);
	bool connect(DWORD dwHost,unsigned short usPort,INT timeOut=0);
	bool connect(const char* pHostName,unsigned short usPort,INT timeOut=0);
	
	bool accept(Sock& Sock,INT nTimeOut=0);
	bool accept(SOCKET& Sock,INT nTimeOut=0);
	Sock* accept(INT nTimeOut=0);
	void close();
	INT send(const void* lpBuf, INT nBufLen, INT nFlags = 0 ,INT nTimeOut=0) const;
	INT send2(const void* lpBuf, INT nBufLen, INT nFlags = 0 ,INT nTimeOut=0) const;
	INT send( const void* lpBuf) const;
	INT send2( const void* lpBuf) const;
	bool getRecvPort() const;
	INT sendTo(const void* lpBuf, INT nBufLen,sockaddr_in* sockaddrTo,INT nFlags=0)
		{return sendTo(lpBuf,nBufLen,(sockaddr*)sockaddrTo,nFlags);}
	INT sendTo(const void* lpBuf, INT nBufLen,sockaddr* sockaddrTo,INT nFlags=0);
	INT sendTo(const void* lpBuf, INT nBufLen, INT nPort,const char* pAdr=NULL,INT nFlags=0);
	INT recvFrom(void* lpBuf, INT nBufLen,INT nFlags=0,sockaddr_in* pAdrFrom=NULL);
	INT getSockOpt(INT nOptName);

	INT setSockOpt(INT nOptName,INT bOption){return setsockopt(m_Sofd, SOL_SOCKET,nOptName,(char*)&bOption,sizeof(INT));}
	INT setSockOpt2(INT nOptName,INT bOption){return setsockopt(m_Sofd, IPPROTO_IP,nOptName,(char*)&bOption,sizeof(INT));}
	bool isConnect()const{return ::send(m_Sofd,"",0,MSG_OOB) != SOCKET_ERROR;}
	bool isBuffer()const{return ::send(m_Sofd,"",0,MSG_OOB) != SOCKET_ERROR;}
	bool select(fd_set* pReadFds,fd_set* pWriteFds,fd_set* pExceptFds,INT nTime);
	bool waitSend(INT timeout=1000);
	bool waitSelect(INT nTime);
	bool wait();
	INT recv( void* lpBuf, INT nBufLen, INT nFlags = 0,INT nTimeOut=0);
	bool isRead();

	void setSockAddr(sockaddr_in* pSockAddr,DWORD dwHost,u_short usPort);
	void setSockAddr(sockaddr_in* pSockAddr,const char* pHostName,u_short usPort);
	void setArgp(bool bFlag)const;

	INT getCondition()const{return m_nCondition;}
	void setCondition(INT nCondition){m_nCondition = nCondition;}
	UINT getPort(){return m_nPort;}

	INT getPeerAdr(sockaddr_in* pInAddr) const;
	DWORD getPeerIP() const;
	bool getPeerIP(char* pBuf) const;
	UINT getPeerPort() const;
	INT getSockAdr(sockaddr_in* pInAddr) const;
	bool getSockIP(char* pBuf) const;
	DWORD getSockIP() const;
	INT getSockPort() const;
	
	bool isAccept()const{return m_baccept;}
	INT getError()const;

	bool sendWake(LPCSTR mac);
protected:
	static WSAObject m_wsaObject;

	SOCKET m_Sofd;
	volatile INT m_nCondition;
	volatile bool m_baccept;
	
	INT m_nPort;

};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Socket
// ソケットスレッド対応
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Socket : public ThreadProc
{
public:
	struct SockData
	{
		Socket* pSock;
		INT iDataSize;
		LPBYTE pbyData;
	};
	Socket();
	~Socket();
	Sock* getSock();
	bool close();
	bool isConnect()const{return m_bConnect;}
	bool connect(LPCSTR pHost,INT iPort);
	int send(LPCSTR pString);
	int send(LPCVOID pvData,INT iSize);
	void setConnect(ClassProc& classProc);
	void setRecv(ClassProc& classProc);
	void setClose(ClassProc& classProc);
	
	virtual DWORD onThreadProc(LPVOID pvData);
	virtual void onSocketConnect(bool bConnect){}
	virtual void onSocketClose(){}
	virtual void onSocketRecv(INT iSize,LPVOID pvAddr){}

protected:
	Sock m_Sock;
	std::string m_strHostName;
	INT m_iPort;
	volatile bool m_bThreadValid;

	ClassProc m_callConnect;
	ClassProc m_callClose;
	ClassProc m_callRecv;
	Thread m_aflThread;

	LPBYTE m_pbyRecvBuff;
	INT m_iRecvBuffSize;
	bool m_bConnect;
};


#ifdef _WIN32
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Ping
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Ping
{
	typedef struct tagIPInfo
		{BYTE byTtl;BYTE byTos;BYTE byIPFlags;BYTE byOptSize;LPBYTE pbyOptions;}IPINFO,*PIPINFO,*LPIPINFO;
	typedef struct tagIcmpEcho
		{DWORD dwSource;DWORD dwStatus;DWORD dwRTTime;WORD wDataSize;WORD wReserved;LPVOID pData;IPINFO infoIP;}ICMPECHO,*PICMPECHO,*LPICMPECHO;
	typedef HANDLE WINAPI IcmpCreateFile(void); 
	typedef BOOL WINAPI IcmpCloseHandle(HANDLE IcmpHandle); 
	typedef DWORD WINAPI IcmpSendEcho(HANDLE IcmpHandle,DWORD dwDestinationAddress,
			 LPVOID RequestData,WORD RequestSize,IPINFO* pRequestOptions,LPVOID ReplyBuffer,
			 DWORD ReplySize,DWORD Timeout); 
public:
	Ping();
	~Ping();
	INT ping(LPCSTR hostAdr,INT timeout=5000);
	BOOL trace(LPCSTR hostAdr);
	

protected:
	DWORD getAddress(LPCSTR pString);
	virtual void onTrace(LPCSTR pString,int nNumber,DWORD dwAdr,int nPing){}

	HANDLE m_hICMP;
	HINSTANCE m_hinsIcmp;
	IcmpCreateFile* m_pIcmpCreateFile;
	IcmpCloseHandle* m_pIcmpCloseHandle;
	IcmpSendEcho* m_pIcmpSendEcho;
};
#else
class Ping
{
	#define ICMP_ECHOREQ 8
public:
	Ping();
	~Ping();
	BOOL ping(PCSTR pString);
	BOOL trace(LPCSTR pString);
protected:
	DWORD getAddress(LPCSTR pString);
	WORD getCheckSum(USHORT* pAddr, int nLen);
	virtual void onTrace(LPCSTR pString,int nNumber,DWORD dwAdr,int nPing);

};
#endif
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PingThread
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PingThread : public Ping
{
public:
	PingThread();
	bool ping(LPCSTR pString);
	DWORD threadPing(LPVOID pvData);
	void onTrace(LPCSTR pString,int nNumber,DWORD dwAdr,int iPing);
	void setTrace(ClassProc* pTrace);
protected:
	ClassProc m_aflClassCallBack;
	std::string m_strHostName;
	ClassProc* m_pTrace;
	Thread m_aflThread;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HttpUrl
{
public:
	bool setUrl(LPCSTR url);

	LPCSTR getProto()const;
	INT getPort()const;
	LPCSTR getHost()const;
	LPCSTR getPath()const;
protected:
	std::string m_proto;
	INT m_port;
	std::string m_host;
	std::string m_path;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpData
// httpデータ受信
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HttpData
{
public:
	bool read(LPCSTR urlAddr);
	LPCSTR getHeader()const;
	LPCSTR getBody()const;
	void setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass);
	size_t getBodyLength()const{return m_body.length();}
	void setHeader(LPCSTR name,LPCSTR value)
	{
		m_outHeader[name] = value;
	}
protected:
	std::string m_header;
	std::string m_body;
	std::map<String,String> m_outHeader;

	String m_proxyName;
	INT m_proxyPort;
	String m_proxyID;
	String m_proxyPass;

};

//namespace
};};

#define __INC_Sock
#endif
