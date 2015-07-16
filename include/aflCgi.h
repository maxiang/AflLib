//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLCGI
//-----------------------------------------------------

#ifdef _WIN32
	#pragma warning( disable : 4786 )	//STLの警告外し
	#include <windows.h>
#endif

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <list>
#include <fstream>

#include "aflStd.h"
#include "aflSock.h"
namespace AFL{ namespace CGI {
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HttpUrl
{
public:
	bool setUrl(LPCSTR url);

	LPCSTR getProto()const{return m_proto.c_str();}
	INT getPort()const{return m_port;}
	LPCSTR getHost()const{return m_host.c_str();}
	LPCSTR getPath()const{return m_path.c_str();}
	LPCSTR getParam()const{return m_param.c_str();}
protected:
	std::string m_proto;
	INT m_port;
	std::string m_host;
	std::string m_path;
	std::string m_param;
};

//------------------------------------------------------------
// ClientInst
// クライアントインスタンス管理
//------------------------------------------------------------
class ClientInst
{
	friend class MethodBuffer;
public:
	ClientInst(SOCK::Sock* sock);
	~ClientInst();
	SOCK::Sock* getSock()const;
	CHAR getChar();
	INT read(LPSTR dest,INT size);
	void setParam(LPCSTR string);
	void setUrl(LPCSTR url);
	Thread* getThread();
	INT getContentLength()const;
	LPCSTR GET()const;
	LPCSTR getHeader(LPCSTR name);
	LPCSTR getMultipartData()const;
	void setMutipartData(LPSTR data);
	LPCSTR getPath()const{return m_path;}
protected:
	Thread m_thread;
	SOCK::Sock* m_sock;
	std::map<String,String> m_params;
	String m_method;
	String m_path;
	String m_get;
	INT m_contentLength;
	LPSTR m_multipartData;
	INT m_multipartPoint;
};

struct Cookie
{
	String value;
	String path;
	time_t expires;
};
class MethodFile
{
friend class MethodBuffer;
public:
	MethodFile();
	bool save(LPCSTR fileName) const;
	LPCSTR getName()const {return m_name.c_str();}
	FILE* getFile()const{return m_file;}
	INT getSize()const{return m_size;}
protected:
	std::string m_name;
	std::string m_lname;
	INT m_size;
	FILE* m_file;
};

class MethodBuffer
{
friend class Method;
public:
	MethodBuffer();
	~MethodBuffer();
protected:
	void init(LPCSTR getData,LPCSTR cookieData,LPCSTR path=NULL);
	void setClientInst(ClientInst* clientInst){m_clientInst = clientInst;}
	void init(LPCSTR path=NULL);
	bool isGET(LPCSTR name) const;
	bool isPOST(LPCSTR name) const;
	bool isCOOKIE(LPCSTR name) const;
	bool isFILE(LPCSTR name) const;
	bool isENV(LPCSTR name) const;
	LPCSTR ENV(LPCSTR name) const;
	LPCSTR GET(LPCSTR name,INT index=0) const;
	LPCSTR POST(LPCSTR name,INT index=0);
	LPCSTR POST() const
	{
		return m_methodPOSTData.c_str();
	}
	INT getPostLength()
	{
		return m_methodPOSTData.length();
	}
	LPCSTR COOKIE(LPCSTR name) const;
	const MethodFile* FILE(LPCSTR name,INT index=0) const;
	
	static bool decodeURI(std::string& dest,LPCSTR src);
	static bool getDecodeChar(std::string& dest,LPCSTR& src);
	bool createGET(LPCSTR data);
	bool createPOST(LPCSTR data);
	bool createCOOKIE(LPCSTR data);
	bool createGET();
	bool createPOST();
	bool createENV();
	bool createCOOKIE();
	bool createMultipart();
	bool createEnvMap(std::map<std::string,std::string>& map,LPCSTR envName);
	bool createEnvMap2(std::map<std::string,std::string>& map,LPCSTR data);
	INT isMultipartString(std::vector<CHAR>& buff,INT& length);
	bool readHeader(std::map<std::string,std::map<std::string,std::string> >&map,INT& length);
	CHAR getChar()
	{
		if(m_clientInst)
			return m_clientInst->getChar();
		return getchar();
	}
	bool readMultipartPost(INT& length,LPCSTR name);
	bool readMultipartFile(INT& length,LPCSTR name,LPCSTR fileName);
	void convertHtmlToBin(std::string& string);

	bool m_init;
	ClientInst* m_clientInst;

	std::multimap<std::string,MethodFile> m_methodFILE;
	std::multimap<std::string,std::string> m_methodGET;
	std::multimap<std::string,std::string> m_methodPOST;
	std::map<std::string,std::string> m_methodCOOKIE;
	std::map<std::string,std::string> m_methodENV;
	std::string m_methodPOSTData;

	std::string m_script;
	std::string m_url;
	std::string m_contentType;
	std::string m_multipartString;
	std::string m_path;
};
class Method
{
public:
	static bool isGET(LPCSTR name);
	static bool isPOST(LPCSTR name);
	static bool isCOOKIE(LPCSTR name);
	static bool isFILE(LPCSTR name);
	static bool isENV(LPCSTR name);
	static LPCSTR ENV(LPCSTR name);
	static LPCSTR GET(LPCSTR name,INT index=0);
	static LPCSTR POST(LPCSTR name,INT index=0);
	static LPCSTR POST();
	static INT getPostLength();
	static LPCSTR COOKIE(LPCSTR name);
	static LPCSTR URL();
	static LPCSTR SCRIPT();
	static const MethodFile* FILE(LPCSTR name,INT index=0);
	static const MethodFile* FILE(INT index,std::string& name);
	static void POST(std::vector<LPCSTR>& dest,LPCSTR name);
	static void init(LPCSTR path=NULL);
	static void setClientInst(ClientInst* clientInst);
	static bool decodeURI(std::string& dest,LPCSTR data){return MethodBuffer::decodeURI(dest,data);}

	static INT numPOST(LPCSTR name=NULL);
	static INT numGET(LPCSTR name=NULL);
	static INT numFILE(LPCSTR name=NULL);
protected:
	static std::map<INT,MethodBuffer> m_methodBuffer;
};
class Stream
{
public:
	Stream(bool enable=false);
	~Stream();
	void setCompress(bool flag)
	{
#ifdef ZipWriter
		m_compress = flag;
#endif
	}
	bool outHeader();
	bool outBody();
	bool setCookie(LPCSTR name,LPCSTR value="",LPCSTR path="/",time_t expires=-1);
	bool outTab();
	void incLevel(){m_level++;}
	void decLevel(){m_level--;}
	INT printf(LPCSTR format,...);
	INT puts(LPCSTR string);
	INT out(LPCSTR string);
	size_t outFile(LPCSTR fileName);
	void write(LPVOID data,INT size);
	void setHeader(LPCSTR name,LPCSTR value);
	void setEnable(bool flag){m_enable = flag;}
	void block();
	void setFile(LPCSTR fileName,LPCSTR localName=NULL,INT size=0,LPCSTR type=NULL);
	void setFileName(LPCSTR fileName){m_fileName=fileName;}
	LPCSTR getBody()const{return m_stream.c_str();}
	void setSock(SOCK::Sock* sock){m_socket = sock;}
	bool send(LPCVOID src);
	bool send(LPCVOID src, INT length, bool binary = true);
	bool getByte(LPBYTE value);
	bool read(LPVOID dest, int size);

	void setWebSocketKey(LPCSTR key);
	bool recvWebSocket(LPVOID data, INT* length);

protected:
	bool m_compress;
	bool m_output;
	bool m_enable;
	int m_level;
	std::string m_stream;
	std::map<std::string,std::string> m_header;
	std::map<std::string,Cookie> m_cookie;
	INT m_length;

	bool m_fileError;
	String m_fileName;
	String m_localName;
	String m_fileType;
	INT m_fileSize;
	SOCK::Sock* m_socket;

};
struct WEBSockData
{
	Stream* stream;
	LPVOID data;
	INT length;
};
//------------------------------------------------------------
// HttpServer
// 実験用仮想サーバ用
//------------------------------------------------------------
class HttpServer
{
public:
	HttpServer();
	virtual ~HttpServer();
	bool start();
	void setProc(ClassProc proc)
	{
		m_proc = proc;
	}

protected:
	DWORD procServer(LPVOID pvoid);
	void connect(ClientInst* clientInst);
	DWORD proc(ClientInst* clientInst);
	virtual void call(Stream* s);

	SOCK::Sock m_listener;
	std::list<ClientInst*> m_clientInst;
	std::list<ClientInst*> m_clientInstEnd;
	Critical m_critical;
	String m_documentRoot;
	INT m_port;
	volatile bool m_serverActive;
	Thread m_serverThread;
	ClassProc m_proc;



};

void TextToHtml(String& dest,LPCSTR src);

#define TEXTSTRING(a) ((LPCSTR)TEXTString(a))
class TEXTString
{
public:
	TEXTString(LPCSTR src)
	{
		//特殊文字の変換
		INT i;
		for(i=0;src[i];i++)
		{
			if(src[i] == ',')
				m_string += "%2C";
			else if(src[i] == '\n')
				m_string += "%0A";
			else if(src[i] == '\r')
				m_string += "%0D";
			else if(src[i] == '%')
				m_string += "%25";
			else
				m_string += src[i];
		}
	}
	operator LPCSTR() const {return m_string.c_str();}
protected:
	String m_string;
};


}}
//-----------------------------------------------------
#define __INC_AFLCGI
#endif	// __INC_AFLCGI
//-----------------------------------------------------
