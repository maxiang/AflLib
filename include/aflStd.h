#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLSTD

#include <string>
#include <map>
#include <vector>
#include <cstdarg>

//-----------------------------------------------------
// UNIX Windows 共通化用
#if defined(_WIN32) | defined(_WIN32_WCE)
	#ifndef _WINDOWS_
		#include <windows.h>
	#endif
	#if !defined(_WIN32_WCE)
		#include <process.h>
		#include <io.h>
		#include <fcntl.h>
	#endif
	#define setBinary(a) _setmode(_fileno(a),_O_BINARY)
	#define THANDLE HANDLE
	typedef int socklen_t;
	#define getCurrentThreadID() GetCurrentThreadId()
	#pragma warning(disable : 4996)

	#if _DEBUG
		#define D(a,...) {String s;s.printf(a,__VA_ARGS__);OutputDebugStringA(s.c_str());}
		#define DLINE(a) D("%s(%d): %s\n",__FILE__,__LINE__,a)
	#else
		#define D(a,...)
		#define DLINE(a)
	#endif
#else
	#include <sys/stat.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <netdb.h>


	#define setBinary(a)
	#define Sleep(msec)	usleep(msec*1000)
	#define ZeroMemory(dest,length)	memset(dest,0,length)
	#define CopyMemory(dest,src,length) memcpy(dest,src,length)
	#define getCurrentThreadID() (INT)pthread_self()

	typedef size_t SIZE_T;
	typedef void VOID;
	typedef struct tagRect{int left,top,right,bottom;}RECT,*LPRECT,*PRECT;
	typedef const char *LPCTSTR,*LPCSTR,*PCTSTR,*PCSTR;
	typedef const wchar_t  *LPCWSTR,*PCWSTR;
	typedef wchar_t  WCHAR;
	typedef char *LPTSTR,*LPSTR,*PTSTR,*PSTR;
	typedef unsigned long DWORD,*LPDWORD,*PDWORD;
	typedef unsigned short WORD,*PWORD,*LPWORD;
	typedef unsigned char BYTE,*PBYTE,*LPBYTE,UINT8;
	typedef const unsigned char CBYTE,*PCBYTE,*LPCBYTE;
	typedef unsigned int UINT,*PUINT,*LPUINT,BOOL,UINT32;
	typedef uint64_t UINT64;
	typedef int64_t INT64;
	typedef unsigned long ULONG,*PULONG,LPULONG;
	typedef unsigned short USHORT,*PUSHORT,*LPUSHORT;
	typedef char CHAR,TCHAR,*PCHAR,*LPCHAR;
	typedef unsigned char UCHAR,*PUCHAR,*LPUCHAR;
	typedef short SHORT,*PSHORT,*LPSHORT;
	typedef void *LPVOID,*PVOID;
	typedef int *PINT,INT,HANDLE;
	typedef void const* LPCVOID;
	typedef float FLOAT,*PFLOAT;
	#define THANDLE pthread_t
	#define _byteswap_ulong __builtin_bswap32  
	#define TRUE 1
	#define FALSE 0

	#define SOCKET int
	#define IPPORT_SMTP 25
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
	#define ioctlsocket ioctl
	#define closesocket close
	#define _fstat fstat
	#define _stat stat
	#define _fileno fileno
	#define _kbhit kbhit
	
	inline long timeGetTime()
	{
		timeval tv;
		gettimeofday(&tv,NULL);
		return tv.tv_usec/1000;
	}
	#define _rotl(x,r)  ((x << r) | (x >> (32 - r)))
	#define D(a,...)
	#define DLINE(a)
#endif
//-----------------------------------------------------
int strprintf(std::string& dest,const char *format,va_list argptr);
int strprintf(std::string& dest,const char *format, ...);
int strprintf(std::wstring& dest,const wchar_t *format,va_list argptr);
int strprintf(std::wstring& dest,const wchar_t *format, ...);
#define foreach(a,b) for(a=(b).begin();a!=(b).end();++a)
#define foreach_reverse(a,b) for(a=(b).rbegin();a!=(b).rend();++a)

namespace AFL{

template<class T1, class T2> inline T1 find(const T1 begin, const T1 end, const T2& value)
{
	T1 it;
	for(it=begin;it!=end;it++)
	{
		if(memcmp(&*it,&value,sizeof(T2)) == 0)
			return it;
	}
	return end;
}
class ClassDescBase
{
public:
	virtual LPVOID create() = 0;
};

template<class T>
class ClassDescripter : public ClassDescBase
{
public:
	LPVOID create()
	{
		return (LPVOID)new T();
	}
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SP
// シェアードポインタ擬き
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

template<class T> class SP
{
public:
	SP()
	{
		m_object = NULL;
		m_count = NULL;
	}
	SP(T* object)
	{
		m_object = NULL;
		m_count = NULL;
		set(object);
	}
	SP(T& object)
	{
		m_count = NULL;
		m_object = &object;
	}
	SP(const SP& sp)
	{
		m_object = NULL;
		m_count = NULL;
		if(sp.m_object)
		{
			m_object = sp.m_object;
			m_count = sp.m_count;
			if(m_count)
				++*m_count;
		}

	}
	SP& operator=(const SP& sp)
	{
		release();
		if(sp.m_object)
		{
			m_object = sp.m_object;
			m_count = sp.m_count;
			if(m_count)
				++*m_count;
		}
		return *this;
	}
	bool operator==(const SP& sp) const
	{
		return m_object == sp.m_object;
	}
	void set(T* object)
	{
		release();
		if(object)
		{
			m_object = object;
			m_count = new int;
			*m_count = 1;
		}
	}
	T* get() const
	{
		return m_object;
	}
	T* operator->() const
	{
		return m_object;
	}
	~SP()
	{
		release();
	}

	bool operator!=(const SP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator<=(const SP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator<(const SP& sp) const
	{
		return *m_object < *sp.m_object;
	}
	bool operator>=(const SP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator>(const SP& sp) const
	{
		return *m_object < *sp.m_object;
	}
	INT getCount() const
	{
		return *m_count;
	}
protected:
	void release()
	{
		if(m_object)
		{
			if(m_count && !--*m_count)
			{
				delete m_object;
				delete m_count;
				m_object = NULL;
				m_count = NULL;
			}
		}
	}

	T* m_object;
	int* m_count;
};
template<class T> class CP
{
public:
	CP()
	{
		m_object = NULL;
	}
	CP(T* object)
	{
		m_object = NULL;
		set(object);
	}
	CP(const CP& sp)
	{
		m_object = NULL;
		if(sp.m_object)
		{
			m_object = sp.m_object;
			m_object->AddRef();
		}

	}
	CP& operator=(const CP& sp)
	{
		release();
		if(sp.m_object)
		{
			m_object = sp.m_object;
			m_object->AddRef();
		}
		return *this;
	}
	operator bool() const
	{
		return m_object != NULL;
	}
	operator T*() const
	{
		return m_object;
	}
	bool operator==(const CP& sp) const
	{
		return m_object == sp.m_object;
	}
	void set(T* object)
	{
		release();
		if(object)
		{
			m_object = object;
		}
	}
	T* get() const
	{
		return m_object;
	}
	T* operator->() const
	{
		return m_object;
	}
	~CP()
	{
		release();
	}

	bool operator!=(const CP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator<=(const CP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator<(const CP& sp) const
	{
		return *m_object < *sp.m_object;
	}
	bool operator>=(const CP& sp) const
	{
		return m_object == sp.m_object;
	}
	bool operator>(const CP& sp) const
	{
		return *m_object < *sp.m_object;
	}
protected:
	void release()
	{
		if(m_object)
		{
			m_object->Release();
			m_object = NULL;
		}
	}
	T* m_object;
};

//文字コード変換
void BASE64(std::string& dest, void* src, int size);
void AtoB64(std::string& dest, LPCSTR src);
void EUCtoSJIS(std::string& dest,std::string& src);
void EUCtoSJIS(std::string& dest,const char* src);
void UTF8toUCS2(std::wstring& dest,LPCSTR src,INT size = -1);
void UCS2toUTF8(std::string& dest,LPCWSTR src);
void SJIStoUTF8(std::string& dest,LPCSTR src);
void UTF8toSJIS(std::string& dest,LPCSTR src);
void EUCtoUTF8(std::string& dest,LPCSTR src);
void SJIStoJIS(std::string& dest,PCSTR pSrc);
void JIStoSJIS(std::string& dest,PCSTR pSrc);



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// String
// 文字列制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


class String : public std::string
{
public:
	operator LPCSTR()const{return c_str();}
	String(){}
	String(LPCSTR value) : std::string(value){}
	String(LPCWSTR value);
	String(INT value);
	String(FLOAT value);
	String& operator=(LPCWSTR value);
	String& operator=(LPCSTR value);
	String& operator=(INT value);
	String& operator=(FLOAT value);
	INT vprintf(LPCSTR format,va_list argptr);
	INT printf(LPCSTR format, ...);
	INT appendf(LPCSTR format, ...);
	INT toInt() const;
	FLOAT toFloat() const;
	void toUpper() const;
};

class StrPrint : public String
{
public:
	StrPrint(LPCSTR format, ...);
};


class WString : public std::wstring
{
public:
	operator LPCWSTR()const{return c_str();}
	WString(){}
	WString(LPCSTR value);
	WString(LPCWSTR value) : std::wstring(value){}
	WString(INT value);
	WString(DWORD value);
	WString(FLOAT value);
	WString& operator=(LPCWSTR value);
	WString& operator=(LPCSTR value);
	WString& operator=(INT value);
	WString& operator=(DWORD value);
	WString& operator=(FLOAT value);
	INT vprintf(LPCWSTR format,va_list argptr);
	INT printf(LPCWSTR format, ...);
	INT appendf(LPCWSTR format, ...);
	INT toInt() const;
	FLOAT toFloat() const;
	void toUpper() const;

};

#ifdef _UNICODE
typedef WString TString;
#else
typedef String TString;
#endif



#define UTF8TOUCS2(a) ((LPCWSTR)Ucs2(a,false))

#if defined(_WIN32) || defined(_WIN32_WCE)
	#define UCS2(a) ((LPCWSTR)AFL::Ucs2(a,true))
#else
	#define UCS2(a) ((LPCWSTR)AFL::Ucs2(a,false))
#endif

class Ucs2
{
public:
	Ucs2(LPCSTR src,bool sjis);
	operator LPCWSTR() const;
protected:
	WString m_string;
};
#define UTF8(a) ((LPCSTR)Utf8(a))
class Utf8
{
public:
	Utf8(LPCSTR src);
	Utf8(LPCWSTR src);
	operator LPCSTR() const;
protected:
	String m_string;
};
#define SJIS(a) ((LPCSTR)Sjis(a))
class Sjis
{
public:
	Sjis(LPCSTR src);
	Sjis(LPCWSTR src);
	operator LPCSTR() const;
protected:
	String m_string;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// BinaryStream
// バイナリーストリーム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class BinaryStream
{
public:
	BinaryStream();
	bool load(LPCSTR fileName);
	void write(LPCVOID addr,size_t size);
	void write(LPCSTR addr);
	LPVOID getData();
	INT getSize()const;
	INT printf(LPCSTR format, ...);

	INT setSeek(INT seek,INT origin)
	{
		if(origin == SEEK_SET)
			m_seek = seek;
		else
			m_seek += seek;
		return m_seek;
	}
	INT getImageWord()
	{
		if(m_seek+2 < (INT)m_stream.size())
		{
			LPBYTE data = (LPBYTE)getData();
			m_seek += 2;
			return (data[0] << 8) + data[1];
		}
		return -1;
	}
	INT getImageDWord()
	{
		if(m_seek+4 < (INT)m_stream.size())
		{
			LPBYTE data = (LPBYTE)getData();
			m_seek += 4;
			return (data[0] << 24) +(data[1] << 16) +(data[2] << 8)  + data[3];
		}
		return -1;

	}
	INT read(LPVOID dest,INT size)
	{
		if(m_seek+size < (INT)m_stream.size())
		{
			memcpy(dest,getData(),size);
			m_seek += size;
			return size;
		}
		return 0;
	}
protected:
	std::vector<BYTE> m_stream;
	INT m_seek;
};
//文字コード判別
#define isSJIS(a) ((unsigned char)a >= 0x81 && (unsigned char)a <= 0x9f || (unsigned char)a >= 0xe0 && (unsigned char)a<=0xfc)
#define isSJIS2(a) ((unsigned char)a >= 0x40 && (unsigned char)a <= 0x7e || (unsigned char)a >= 0x80 && (unsigned char)a<=0xfc)
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflTClassProc
// クラス関数コールバック用関数テンプレート
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
template<class T,class ARGTYPE,DWORD (T::*function)(ARGTYPE)>
class TProc
{
public:
	static DWORD proc(LPVOID pclass,ARGTYPE pvoid)
	{
		return ((T*)pclass->*function)((ARGTYPE)pvoid);
	}
	static LPVOID getAdr(){return (LPVOID)&TProc::proc;}
};
#define PROC(a) (::AFL::ClassProc(a))
#if defined(_WIN32) | defined(_WIN32_WCE)
	#define CLASSPROC(a,b,c) (::AFL::ClassProc(a,::AFL::TProc<b,LPVOID,(DWORD (b::*)(LPVOID))&b::c>::getAdr()))
#else
	#define CLASSPROC(a,b,c) (::AFL::ClassProc(a,::AFL::TProc<b,LPVOID,&b::c>::getAdr()))
#endif
#define CLASSPROC2(a,b,c,d) (::AFL::ClassProc(a,::AFL::TProc<b,d,&b::c>::getAdr()))


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflCall
// クラス関数呼び出し補助クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class ClassProc
{
public:
	ClassProc();
	ClassProc(LPVOID pclass,LPVOID function);
	ClassProc(LPVOID function);
	DWORD call(LPVOID pvoid=NULL);
	bool isAddress()const;
	bool operator==(const ClassProc& classProc) const;
protected:
	LPVOID m_function;
	LPVOID m_pClass;
};



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClassCallBack
// クラス関数コールバック用基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef class ClassCallBack AFLCLASSCALLBACK,*PAFLCLASSCALLBACK,*LPAFLCLASSCALLBACK;
class ClassCallBack
{
public:
	virtual ~ClassCallBack(){}
	virtual DWORD callProcess(LPVOID pvData) = 0;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClassCallBack
// クラス関数コールバック用派生テンプレート
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
template<class T> class TClassCallBack : public ClassCallBack
{
public:
	TClassCallBack();
	virtual ~TClassCallBack(){}
	TClassCallBack(T* pClass,DWORD (T::*pAddress)(LPVOID));
	void setAddress(T* pClass,DWORD (T::*pAddress)(LPVOID));
	DWORD callProcess(LPVOID pvData);
	bool isAddress()const{return m_pClass && m_pAddress;}
private:
	T* m_pClass;
	DWORD (T::*m_pAddress)(LPVOID);
};

template<class T> TClassCallBack<T>::TClassCallBack()
{
	m_pClass = NULL;
	m_pAddress = NULL;
}
template<class T> TClassCallBack<T>::TClassCallBack(T* pClass,DWORD (T::*pAddress)(LPVOID))
{
	m_pClass = pClass;
	m_pAddress = pAddress;
}
template<class T> void TClassCallBack<T>::setAddress(T* pClass,DWORD (T::*pAddress)(LPVOID))
{
	m_pClass = pClass;
	m_pAddress = pAddress;
}
template<class T> DWORD TClassCallBack<T>::callProcess(LPVOID pvData)
{
	if(m_pClass && m_pAddress)
		return (m_pClass->*m_pAddress)((LPVOID)pvData);
	return 0;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// aflThread
// スレッド実行用基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Thread
{
public:
	static THANDLE createThread(LPVOID pAddress,LPVOID pvData=NULL,LPDWORD pdwId=0);
	static bool closeThread(THANDLE hThread);
	Thread();
	~Thread();
	bool closeThread();
	bool getExitCodeThread(PDWORD pdwCode=NULL);
	bool isActiveThread() const;
	bool startThread(ClassProc paflClassCallBack,LPVOID pvData=NULL);
	THANDLE getThreadHandle()const{return m_hThread;}
	DWORD getThreadID()const{return m_dwThreadID;}
	static DWORD getCurrentID();
protected:
	static DWORD threadProcServer(LPVOID pVoid);
	DWORD threadProcRouter(LPVOID pvData);

	THANDLE m_hThread;						//スレッドハンドル
	DWORD m_dwThreadID;						//スレッドID
	volatile  DWORD m_dwExitCode;			//終了コード
	volatile  bool m_bEnable;				//状態
	volatile  bool m_bStart;				//状態
	ClassProc m_paflClassCallBack;	//メンバコールバック用
};

class ThreadProc
{
	friend class Thread;
protected:
	virtual DWORD onThreadProc(LPVOID pvData){return 0;}
};
//------------------------------------------------------------
// SyncObject
// 同期制御用
//------------------------------------------------------------
class SyncObject
{
public:
	virtual ~SyncObject(){};
	virtual bool lock() = 0;
	virtual bool unlock() = 0;
};

//------------------------------------------------------------
// Critical
// 同期制御用
//------------------------------------------------------------
#ifdef _WIN32
class Critical : public SyncObject
{
public:
	Critical(){::InitializeCriticalSection(&m_Sect);}
	~Critical(){::DeleteCriticalSection(&m_Sect);}
	bool lock(){::EnterCriticalSection(&m_Sect);return TRUE;}
	bool unlock(){::LeaveCriticalSection(&m_Sect);return TRUE;}
protected:
	CRITICAL_SECTION m_Sect;
};
#else
class Critical : public SyncObject
{
public:
	Critical(){::pthread_mutex_init(&m_Sect,NULL);}
	~Critical(){::pthread_mutex_destroy(&m_Sect);}
	bool lock(){return ::pthread_mutex_lock(&m_Sect);}
	bool unlock(){return ::pthread_mutex_unlock(&m_Sect);}
protected:
	pthread_mutex_t m_Sect;
};
#endif

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// File
// ファイル制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class File
{

public:
	enum
	{
		file_binary = 0,
		file_ascii= 1,
		file_in = 2,
		file_out = 4
	};
	
	File();
	virtual ~File();
	bool open(LPCSTR fileName,LPCSTR mode="rb");
	bool open(LPCWSTR fileName,LPCWSTR mode=L"rb");
	bool close();
	INT read(LPVOID pVoid,INT iSize) const;
	INT write(LPVOID pvoid,INT size) const;
	INT64 getLength() const;
	INT64 tell() const;
	bool isEof() const;
	bool isOpen() const;
	void setSeek(INT64 offset, INT origin) const;
	LPSTR gets(LPSTR pString,INT iSize);

protected:
	LPVOID m_pFileHandle;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Std
// 汎用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Std
{
public:
	static int GetChr(const char* pData,char Data,int nLength);
	static int GetChr(const char* pData,char Data);
	static int GetNum16(const char* pData,int nLength);
	static char* StrCnv(const char* pData,char* pString1,char* pString2);
	static char* StrCnv1(const char* pData,int nCount,char* const ppString[]);
	static LPSTR replacString(LPCSTR pData,int nCount,LPCSTR pString[]);
};

struct cmpConvert
{
	bool operator()(const String& s1,const String& s2) const
	{
		if(s1.length() > s2.length())
			return true;
		if(s1.length() == s2.length())
			return ((std::string)s1) < ((std::string)s2);
		return false;
	}
};
struct cmpConvertW
{
	bool operator()(const WString& s1,const WString& s2) const
	{
		if(s1.length() > s2.length())
			return true;
		if(s1.length() == s2.length())
			return ((std::wstring)s1) < ((std::wstring)s2);
		return false;
	}
};

typedef std::map<String,String,struct cmpConvert> ConvertItem;
typedef std::map<WString,WString,struct cmpConvertW> ConvertItemW;
void replaceString(std::string& dest,LPCSTR src,ConvertItem* item);
void replaceString(std::wstring& dest,LPCWSTR src,ConvertItemW* item);


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Debug
// デバッグ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Debug
{
public:
	static bool out(LPCSTR string, ...);
	static bool open();
	static void setLevel(INT level);

private:
	static INT m_level;
	static FILE* m_file;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TimeString
// 時間フォーマット用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TimeString
{
public:
	LPCSTR getDateTime();
protected:
	String m_value;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SleepTimer
// ウエイト制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifdef _WIN32
class SleepTimer
{
public:
	SleepTimer();
	~SleepTimer();
	bool sleep(DWORD t);
	void wake();
protected:
	HANDLE m_handle;
};
#else
class SleepTimer
{
public:
	SleepTimer();
	~SleepTimer();
	bool sleep(DWORD t);
	void wake();
};
#endif
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TimeCounter
// 進行速度維持用カウンタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TimeCounter
{
public:
	TimeCounter();
	~TimeCounter();
	void resetTime();
	DWORD getTime() const;
	INT getCount();
	void sleep() const;
	DWORD sleepTime() const;

	void setEnable(bool bFlag);
	void setTimeSync(DWORD dwTime);
	DWORD getTimeSync(){return m_dwSyncTime;}
	bool isEnable()const{return m_bEnable;}
protected:
	DWORD m_dwSyncTime;
	DWORD m_dwStartTime;
	INT m_iNowCount;
	DWORD m_dwSleepTime;
	bool m_bEnable;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ThreadTimer
// 時間制御ループスレッド
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class ThreadTimer
{
public:
	ThreadTimer();
	~ThreadTimer();
	void start(LPVOID pvoid=NULL);
	void stop();
	void stopSignal();
	DWORD getInterval();
	void setInterval(DWORD dwInterval);
	void setRender(const ClassProc& paflClassProc);
	void setStart(const ClassProc& paflClassProc);
	void setEnd(const ClassProc& paflClassProc);
	void setAction(const ClassProc& paflClassProc);
	void setIdle(const ClassProc& paflClassProc);
	bool isWait()const{return m_aflTimeCounter.isEnable();}
	void setWait(bool bFlag){m_aflTimeCounter.setEnable(bFlag);}
	void lock(){m_critical.lock();}
	void unlock(){m_critical.unlock();}
protected:
	DWORD onProc(LPVOID pvoid);
private:
	TimeCounter m_aflTimeCounter;
	Thread m_thread;
	volatile bool m_bEnable;
	volatile bool m_bEnter;

	ClassProc m_aflProc;

	ClassProc m_paflProcStart;
	ClassProc m_paflProcEnd;
	ClassProc m_paflProcAction;
	ClassProc m_paflProcIdle;
	ClassProc m_paflProcRender;
	SleepTimer m_sleep;
	Critical m_critical;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonObject
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class JsonObject
{
public:
	virtual void getString(String& dest,int level=0) = 0;
	virtual ~JsonObject(){}
};
template<typename T> class JsonData : public JsonObject
{
public:
	JsonData(T data)
	{
		m_data = data;
	}
	void getString(String& dest,int level=0)
	{
		dest = getData();
	}
	T& getData()
	{
		return m_data;
	}
protected:
	T m_data;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonData
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

template<> class JsonData<String> : public JsonObject
{
public:
	JsonData(String data);
	void getString(String& dest, int level = 0);
	String& getData();
protected:
	String m_data;
};
template<> class JsonData<LPCSTR> : public JsonObject
{
public:
	JsonData(LPCSTR data);
	void getString(String& dest, int level = 0);
	String& getData();
protected:
	String m_data;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonArray
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class JsonArray : public JsonObject
{
public:
	void getString(String& dest,int level=0);
	void add(JsonObject* object);

protected:
	std::vector<SP<JsonObject> > m_data;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonHash
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class JsonHash : public JsonObject
{
public:
	void add(LPCSTR name, JsonObject* object);
	void getString(String& dest, int level = 0);
protected:
	std::map<String, SP<JsonObject> > m_data;
};
template<class T> JsonObject* createJson(T value)
{
	return new JsonData<T>(value);
}
//namespace
}
#define __INC_AFLSTD
#endif
