#if !defined(_WIN32_WCE)
	#include <sys/stat.h> 
#endif
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include "aflStd.h"


//----------------------------------------------------
//メモリリークテスト用
#if _MSC_VER && !defined(_WIN32_WCE)
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
#endif
//----------------------------------------------------

#if _MSC_VER && !defined(_WIN32_WCE)
	#pragma comment(lib, "winmm.lib")
#endif 



int strprintf(std::string& dest,const char *format,va_list argptr)
{
	INT ret;
	char *buff;
	#if defined(_WIN32_WCE)
		buff = new char [0x10000];
		ret = _vsnprintf(buff, 0x10000,format, argptr);
	#elif defined(_WIN32)
		int length = _vscprintf(format, argptr);
		if(length < 0)
			return 0;
		buff = new char [length + 1];
		ret = vsprintf(buff, format, argptr);
	#else
		ret = vasprintf(&buff,format, argptr);
	#endif
	if(buff)
	{
		dest = buff;
		delete[] buff;
	}
	return ret;
}
int strprintf(std::string& dest,const char *format, ...)
{
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	return ret;
}

int strprintf(std::wstring& dest,const wchar_t *format,va_list argptr)
{
	INT ret;
	wchar_t *buff;
	#if defined(_WIN32_WCE)
		buff = new wchar_t [0x10000];
		ret = _vsnwprintf(buff, 0x10000,format, argptr);
	#elif defined(_WIN32)
		int length = _vscwprintf(format, argptr);
		buff = new wchar_t [length + 1];
		ret = vswprintf(buff, format, argptr);
	#else
		ret = 0;//vaswprintf(&buff,format, argptr);
	#endif
	if(buff)
	{
		dest = buff;
		delete[] buff;
	}
	return ret;
}
int strprintf(std::wstring& dest,const wchar_t *format, ...)
{
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	return ret;
}


namespace AFL{
void UCS2toUTF8(std::string& dest,LPCWSTR src)
{
	dest.clear();
	#ifdef __ANDROID__
		const unsigned long* work = (const unsigned long*)src;
	#else
		const unsigned short* work = (const unsigned short*)src;
	#endif
	for(;*work;work++)
	{
		if(*work < 0x80)
			dest += (char)*work;
		else if(*work < 0x800)
		{
			dest += (char)(0xc0 | (*work >> 6));
			dest += (char)(0x80 | (*work & 0x3f));
		}
		else
		{
			dest += (char)(0xe0 | (*work >> 12));
			dest += (char)(0x80 | ((*work >> 6) & 0x3f));
			dest += (char)(0x80 | (*work & 0x3f));
		}
	}
}
void UTF8toSJIS(std::string& dest,LPCSTR src)
{
	//UCS2をSJISに変換
	WString work;
	UTF8toUCS2(work,src);
	((String*)&dest)->printf("%ls",work.c_str());
}
#ifdef _WIN32
void EUCtoUTF8(std::string& dest,LPCSTR src)
{
	std::string work;
	EUCtoSJIS(work,src);
	SJIStoUTF8(dest,work.c_str());
}
void SJIStoUTF8(std::string& dest,LPCSTR src)
{
	//SJISをUTF16に変換
	wchar_t* buff = new wchar_t[strlen(src)+1];
	swprintf(buff,L"%hs",src);
	//UTF16をUTF8に変換
	UCS2toUTF8(dest,buff);
	delete[] buff;
}
void SJIStoUCS2(std::wstring& dest,LPCSTR src)
{
	//SJISをUTF16に変換
	INT size = MultiByteToWideChar(0,0,src,-1,NULL,0);
	wchar_t* buff = new wchar_t[size];
	MultiByteToWideChar(0,0,src,-1,buff,size);
	dest = buff;
	delete[] buff;
}
void UCS2toSJIS(std::string& dest,LPCWSTR src)
{
	//SJISをUTF16に変換
	char* buff = new char[wcslen(src)*2+1];
	sprintf(buff,"%ls",src);
	dest = buff;
	delete[] buff;
}
#endif
void StoJ(PSTR ch1, PSTR ch2)
{
	int high=(UCHAR)*ch1;
	int low=(UCHAR)*ch2;

   if(high <= 0x9f)
	   high -= 0x71;
   else
	   high -= 0xb1;
   high=high*2+1;
   if(low > 0x7f)
	   --low;
   if(low >= 0x9e)
   {
	   low -= 0x7d;
	   ++high;
   }
   else
	   low -= 0x1f;
   *ch1 = (unsigned char)high;
   *ch2 = (unsigned char)low;
}
void JIStoSJIS(std::string& dest,PCSTR pSrc)
{
	int i;
	BOOL bJis = FALSE;
	for(i=0;pSrc[i];)
	{
		if(pSrc[i] == '\x1b')
		{
			if(pSrc[i+1] == '$')
				bJis = TRUE;
			else if(pSrc[i+1] == '(')
				bJis = FALSE;
			i += 3;
			continue;
		}
		if(bJis)
		{
			if (pSrc[i] % 2)
			{
				dest += ((pSrc[i] + 1) / 2) + 0x70;
				dest += pSrc[i+1] + 0x1f;
			}
			else
			{
				dest += (pSrc[i] / 2) + 0x70;
				dest += pSrc[i+1] + 0x7d;
			}
		    if((UCHAR)dest[dest.length()-2] >= 0xa0)
				dest[dest.length()-2] = dest[dest.length()-2] + 0x40;
			if((UCHAR)dest[dest.length()-1] >= 0x7f)
				dest[dest.length()-1] = dest[dest.length()-1] + 1;
			i += 2;
		}
		else
		{
			dest += pSrc[i];
			i++;
		}
	}
}


void SJIStoJIS(std::string& dest,PCSTR pSrc)
{
	String work;
	BOOL bFlag = FALSE;
	int nSrcP = 0;
	while(pSrc[nSrcP])
	{
		if(isSJIS(pSrc[nSrcP]))
		{
			if(!bFlag)
			{
				work += 0x1b;
				work += 0x24;
				work += 0x42;
				bFlag = TRUE;
			}
			UCHAR Dt1,Dt2;
			Dt1 = (UCHAR)pSrc[nSrcP];
			Dt2 = (UCHAR)pSrc[nSrcP+1];
			nSrcP+=2;
			StoJ((PSTR)&Dt1,(PSTR)&Dt2);
			work += Dt1;
			work += Dt2;
		}
		else
		{
			if(bFlag)
			{
				work += 0x1b;
				work += 0x28;
				work += 0x42;
				bFlag = FALSE;
			}
			work += pSrc[nSrcP];
			nSrcP++;
		}
	}
	if(bFlag)
	{
		work += 0x1b;
		work += 0x28;
		work += 0x4a;
	}
	dest = work;
}
void UTF8toUCS2(std::wstring& dest,LPCSTR src,INT size)
{
	dest.clear();
	const unsigned char* work = (const unsigned char*)src;
	INT i;
	for(i=0;*work && (size==-1 || i < size);work++)
	{
		INT code;
		INT c = *work;
		if((c & 0xf0) == 0xe0)
		{
			code = ((*work & 0x0f) << 12) + ((*(work+1) & 0x3f)<<6) + (*(work+2) & 0x3f);
			work += 2;
		}
		else if((c & 0xe0) == 0xc0)
		{
			code = ((*work & 0x1f) << 6) + (*(work+1) & 0x3f);
			work++;
		}
		else
		{
			code = *work;
		}
		dest += code;
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Ucs2::Ucs2(LPCSTR src,bool sjis)
{

	#if defined(_WIN32) || defined(_WIN32_WCE)
		if(sjis)
			SJIStoUCS2(m_string,src);
		else
			UTF8toUCS2(m_string,src);
	#else
		UTF8toUCS2(m_string,src);
	#endif
}
Ucs2::operator LPCWSTR() const
{
	return m_string;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Utf8::Utf8(LPCSTR src)
{
	#if defined(_WIN32) || defined(_WIN32_WCE)
		SJIStoUTF8(m_string,src);
	#else
		m_string = src;
	#endif
}
Utf8::Utf8(LPCWSTR src)
{
	UCS2toUTF8(m_string,src);
}
Utf8::operator LPCSTR() const
{
	return m_string.c_str();
}
Sjis::Sjis(LPCSTR src)
{
	UTF8toSJIS(m_string,src);
}
Sjis::Sjis(LPCWSTR src)
{
#if defined(_WIN32) | defined(_WIN32_WCE)
	UCS2toSJIS(m_string,src);
#endif
}
Sjis::operator LPCSTR() const
{
	return m_string.c_str();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// EUCtoSJIS
// 文字コード変換
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void EUCtoSJIS(std::string& dest,std::string& src)
{
	EUCtoSJIS(dest,src.c_str());
}
void EUCtoSJIS(std::string& dest,const char* src)
{
	std::string work;
	char c1,c2;
	char s1,s2;
	int i;
	for(i=0;src[i];)
	{
		if(src[i] & 0x80)
		{
			s1 = src[i] & 0x7f;
			s2 = src[i+1] & 0x7f;
			if (s1 % 2)
			{
				c1 = ((s1 + 1) / 2) + 0x70;
				c2 = s2 + 0x1f;
			}
			else
			{
				c1 = (s1 / 2) + 0x70;
				c2 = s2 + 0x7d;
			}
			if((unsigned char)c1 >= 0xa0)
				c1 += 0x40;
			if((unsigned char)c2 >= 0x7f)
				c2 += 1;
			work += c1;
			work += c2;
			i += 2;
		}
		else
		{
			work += src[i];
			i++;
		}
	}
	dest = work;
}
void BASE64(std::string& dest, void* src,int size)
{
	static const CHAR BASE64CHAR[] =
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"
	};
	int i, j;
	int	iBit = 2;
	unsigned char cWork = 0;
	unsigned char* data = (unsigned char*)src;
	int lineCount = 0;
	for (int pt = 0; pt < size; pt++)
	{
		if (lineCount)
			dest += "\n";

		lineCount = 0;
		for (i = 0; pt<size && lineCount<76; i++)
		{
			cWork = cWork << (6 - (iBit - 2));
			cWork = cWork | (data[pt] >> iBit);
			dest += BASE64CHAR[cWork & 0x3f];
			lineCount++;
			cWork = data[pt++];
			iBit += 2;

			if (iBit >= 8)
			{
				dest += BASE64CHAR[cWork & 0x3f];
				cWork = 0;
				iBit = 2;
				lineCount++;
			}
		}
	}
	cWork = cWork << (6 - (iBit - 2));
	dest += BASE64CHAR[cWork & 0x3f];
	INT count = 4 - (INT)dest.length() % 4;
	if (count == 4)
		count = 0;
	for (j = 0; j < count; j++)
		dest += '=';
	//}

}
void AtoB64(std::string& dest,LPCSTR src)
{
	static const CHAR BASE64CHAR[] = 
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"
	};
	int i,j;
	int	iBit = 2;
	unsigned char cWork = 0;
	unsigned char* data = (unsigned char*)src;
	int lineCount = 0;
	while(*data)
	{
		if(lineCount)
			dest += "\n";

		lineCount = 0;
		for(i=0;*data && lineCount<76;i++)
		{
			cWork = cWork << ( 6 - (iBit-2) );
			cWork = cWork | (*data >> iBit );
			dest += BASE64CHAR[ cWork & 0x3f ];
			lineCount++;
			cWork = *data++;
			iBit += 2;

			if( iBit >= 8 )
			{
				dest += BASE64CHAR[ cWork & 0x3f ];
				cWork = 0;
				iBit = 2;
				lineCount++;
			}
		}
	}
		cWork = cWork << ( 6 - (iBit-2) );
		dest += BASE64CHAR[ cWork & 0x3f ];
		INT count = 4-(INT)dest.length()%4;
		if(count == 4)
			count = 0;
		for( j = 0 ; j < count; j++ )
			dest += '=';
	//}

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// String
// 文字列制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

String::String(LPCWSTR value)
{
#if defined(_WIN32) | defined(_WIN32_WCE)
	UCS2toSJIS(*this, value);
#else
	UCS2toUTF8(*this, value);
#endif
}


String::String(INT value)
{
	*this = value;
}
String::String(FLOAT value)
{
	*this = value;
}
String& String::operator=(LPCSTR value)
{
	*(std::string*)this = value;
	return *this;
}
String& String::operator=(INT value)
{
	printf("%s%d",c_str(),value);
	return *this;
}
String& String::operator=(FLOAT value)
{
	printf("%s%f",c_str(),value);
	return *this;
}

String& String::operator=(LPCWSTR value)
{
#if defined(_WIN32) | defined(_WIN32_WCE)
	UCS2toSJIS(*this,value);
#else
	UCS2toUTF8(*this,value);
#endif
	return *this;
}

INT String::vprintf(LPCSTR format,va_list argptr)
{
	return strprintf(*this,format,argptr);
}
INT String::printf(LPCSTR format, ...)
{
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(*this,format,param_list);
	va_end(param_list);
	return ret;
}
INT String::appendf(LPCSTR format, ...)
{
	va_list param_list;

	std::string dest;
	va_start(param_list, format);
	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);
	*this += dest;
	return ret;
}
INT String::toInt() const
{
	return atoi(c_str());
}
FLOAT String::toFloat() const
{
	return (FLOAT)atof(c_str());
}
void String::toUpper() const
{
	INT i;
	INT len = (INT)length();
	for(i=0;i<len;i++)
	{
		(*(std::string*)this)[i] = toupper((*(std::string*)this)[i]);
	}
}

StrPrint::StrPrint(LPCSTR format, ...)
{
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(*this,format,param_list);
	va_end(param_list);
}
WString::WString(LPCSTR value)
{
	#ifdef _WIN32
		SJIStoUCS2(*this,value);
	#else
		UTF8toUCS2(*this,value);
	#endif
}
WString::WString(INT value)
{
	*this = value;
}
WString::WString(FLOAT value)
{
	*this = value;
}
WString::WString(DWORD value)
{
	*this = value;
}
WString& WString::operator=(LPCSTR value)
{
	#ifdef _WIN32
		SJIStoUCS2(*this,value);
	#else
		UTF8toUCS2(*this,value);
	#endif
	return *this;
}
WString& WString::operator=(LPCWSTR value)
{
	*(std::wstring*)this = value;
	return *this;
}
WString& WString::operator=(INT value)
{
	printf(L"%s%d",c_str(),value);
	return *this;
}
WString& WString::operator=(DWORD value)
{
	printf(L"%s%u",c_str(),value);
	return *this;
}
WString& WString::operator=(FLOAT value)
{
	printf(L"%s%f",c_str(),value);
	return *this;
}

INT WString::vprintf(LPCWSTR format,va_list argptr)
{
	return strprintf(*this,format,argptr);
}
INT WString::printf(LPCWSTR format, ...)
{
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(*this,format,param_list);
	va_end(param_list);
	return ret;
}
INT WString::appendf(LPCWSTR format, ...)
{
	va_list param_list;

	std::wstring dest;
	va_start(param_list, format);
	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);
	*this += dest;
	return ret;
}
INT WString::toInt() const
{
	#if defined(_WIN32) | defined(_WIN32_WCE)
		return _wtoi(c_str());
	#else
		return 0;
	#endif
}
FLOAT WString::toFloat() const
{
	FLOAT value;
	swscanf(c_str(),L"%f",&value);
	return value;
}
#ifdef _WIN32
void WString::toUpper() const
{
	INT i;
	INT len = (INT)length();
	for(i=0;i<len;i++)
	{
		(*(std::wstring*)this)[i] = towupper((*(std::wstring*)this)[i]);
	}
}
#endif

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// BinaryStream
// バイナリーストリーム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
BinaryStream::BinaryStream()
{
	m_seek = 0;
}
bool BinaryStream::load(LPCSTR fileName)
{
	FILE* file = fopen(fileName,"rb");
	if(!file)
		return false;
	fseek(file,0,SEEK_END);
	INT length = (INT)ftell(file);
	fseek(file,0,SEEK_SET);

	m_stream.resize(length);
	fread(&m_stream[0],length,1,file);
	
	fclose(file);
	return true;
}

void BinaryStream::write(LPCVOID addr,size_t size)
{
	size_t streamSize = m_stream.size();
	m_stream.resize(streamSize+size);
	memcpy(&m_stream[streamSize],addr,size);
}
void BinaryStream::write(LPCSTR addr)
{
	write(addr,strlen(addr));
}
LPVOID BinaryStream::getData()
{
	return &m_stream[0]+m_seek;
}
INT BinaryStream::getSize()const
{
	return (INT)m_stream.size();
}
INT BinaryStream::printf(LPCSTR format, ...)
{	
	String work;
	va_list param_list;

	va_start(param_list, format);
	INT ret = strprintf(work,format,param_list);
	va_end(param_list);
	write(work.c_str());
	return ret;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClassProc
// クラス関数呼び出し補助クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ClassProc::ClassProc()
{
	m_pClass = NULL;
	m_function = NULL;

}
ClassProc::ClassProc(LPVOID pclass,LPVOID function)
{
	m_pClass = pclass;
	m_function = function;
}
ClassProc::ClassProc(LPVOID function)
{
	m_pClass = NULL;
	m_function = function;
}
DWORD ClassProc::call(LPVOID pvoid)
{
	if(m_function)
	{
		if(m_pClass)
			return ((DWORD (*)(LPVOID,LPVOID))m_function)(m_pClass,pvoid);
		else
			return ((DWORD (*)(LPVOID))m_function)(pvoid);
	}
	return 0;
}
bool ClassProc::isAddress()const
{
	return m_function!=NULL;
}
bool ClassProc::operator==(const ClassProc& classProc) const
{
	if(m_function == classProc.m_function && m_pClass == classProc.m_pClass)
		return true;
	return false;
}
//------------------------------------------------------------
DWORD Thread::getCurrentID()
{
	#ifdef _WIN32
		return GetCurrentThreadId();
	#else			//POSIXスレッド
		return pthread_self();
	#endif
}
//------------------------------------------------------------
THANDLE Thread::createThread(LPVOID pAddress,LPVOID pvData,LPDWORD pdwId)
{
	THANDLE hThread;
	#ifdef _WIN32
		#if defined(_MT) && !defined(_WIN32_WCE) //Cランタイムスレッド
			hThread = (THANDLE)_beginthreadex(NULL,0,(unsigned(__stdcall*)(LPVOID))pAddress,pvData,0,(LPUINT)pdwId);	//スレッドの発生
		#else		//WIN32APIスレッド
			hThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)pAddress,pvData,0,pdwId);				//スレッドの発生
		#endif
	#else			//POSIXスレッド
		pthread_create(&hThread,NULL,(LPVOID(*)(LPVOID))pAddress,pvData);
	#endif
	return hThread;
}
//------------------------------------------------------------
bool Thread::closeThread(THANDLE hThread)
{
	if(hThread)
	{
	#ifdef _WIN32	//WIN32ハンドル用
		::CloseHandle(hThread);
	#else			//POSIXハンドル用
		pthread_detach(hThread);
	#endif
		return true;
	}
	return false;
}
//------------------------------------------------------------
Thread::Thread()
{
	m_hThread = 0;
	m_dwThreadID = 0;
	m_dwExitCode = 0;
	m_bEnable = false;
}
//------------------------------------------------------------
Thread::~Thread()
{
	closeThread();
}
//------------------------------------------------------------
bool Thread::closeThread()
{
	bool bRet = closeThread(m_hThread);
	m_hThread = 0;
	return bRet;
}
//------------------------------------------------------------
bool Thread::getExitCodeThread(PDWORD pdwCode)
{
	if(!m_bEnable)
	{
		*pdwCode = m_dwExitCode;
		return true;
	}
	return false;
}
//------------------------------------------------------------
bool Thread::isActiveThread() const
{
	return m_bEnable;
}
//------------------------------------------------------------
bool Thread::startThread(ClassProc paflClassCallBack,LPVOID pvData)
{
	closeThread();	//既に存在するスレッドを停止

	//スレッドに渡す値の設定
	LPVOID adwThreadData[] = {this,pvData};
	LPVOID pdwThreadData = NEW LPVOID[sizeof(adwThreadData)/sizeof(LPVOID)];
	memcpy(pdwThreadData,adwThreadData,sizeof(adwThreadData));
	
	m_paflClassCallBack = paflClassCallBack;

	m_bStart = false;
	//スレッドの作成
	m_hThread = createThread((LPVOID)threadProcServer,pdwThreadData,&m_dwThreadID);
	//スレッド作成後のウエイト
	while(!m_bStart)
		Sleep(0);
	return m_hThread != 0;
}
//------------------------------------------------------------
DWORD Thread::threadProcServer(LPVOID pVoid)
{
	//スレッド初期化用データの設定
	LPVOID* ppThreadData = (LPVOID*)pVoid;
	Thread* pThread = (Thread*)ppThreadData[0];
	LPVOID pvData = ppThreadData[1];
	delete[] ppThreadData;
	//仮想関数の呼び出し
	pThread->m_bStart = true;
	pThread->m_bEnable = true;
	pThread->m_dwExitCode = pThread->threadProcRouter(pvData);
	pThread->m_bEnable = false;
	return pThread->m_dwExitCode;
}
DWORD Thread::threadProcRouter(LPVOID pvData)
{
	if(m_paflClassCallBack.isAddress())
		return m_paflClassCallBack.call(pvData);	//ユーザクラスのメンバ関数へ
	return 0;
}


//------------------------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// File
// ファイル制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
File::File()
{
	m_pFileHandle = NULL;
}
File::~File()
{
	close();
}
bool File::open(LPCSTR fileName,LPCSTR mode)
{
	close();
	m_pFileHandle = fopen(fileName,mode);
	return m_pFileHandle != 0;
}
#ifdef _WIN32
bool File::open(LPCWSTR fileName,LPCWSTR mode)
{
	close();
	m_pFileHandle = _wfopen(fileName,mode);
	return m_pFileHandle != 0;
}
#endif

bool File::close()
{
	if(!m_pFileHandle)
		return false;
	return fclose((FILE*)m_pFileHandle) == 0;
}
bool File::isOpen() const
{
	return m_pFileHandle != 0;
}
INT File::read(LPVOID pVoid,INT iSize) const
{
	if(m_pFileHandle)
		return (INT)fread(pVoid,1,iSize,(FILE*)m_pFileHandle);
	return 0;
}
INT File::write(LPVOID pVoid,INT iSize) const
{
	if(m_pFileHandle)
		return (INT)fwrite(pVoid,1,iSize,(FILE*)m_pFileHandle);
	return 0;
}
INT64 File::getLength() const
{
	INT64 pt = tell();
	setSeek(0,SEEK_END);
	INT64 size = tell();
	setSeek(pt,SEEK_SET);
	return size;
}
INT64 File::tell() const
{
#ifdef _WIN32
	return _ftelli64((FILE*)m_pFileHandle);
#else
	return ftello((FILE*)m_pFileHandle);
#endif
}
bool File::isEof() const
{
	return feof((FILE*)m_pFileHandle)!=0;
}
void File::setSeek(INT64 offset, int origin) const
{
#ifdef _WIN32
	_fseeki64((FILE*)m_pFileHandle, offset, origin);
#else
	fseeko((FILE*)m_pFileHandle, offset, origin);
#endif
}
LPSTR File::gets(LPSTR pString,INT iSize)
{
	return fgets(pString,iSize,(FILE*)m_pFileHandle);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Std
// 汎用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int Std::GetChr(const char* pData,char Data,int nLength)
{
	int i,j;
	for(i=0,j=0;i<nLength;i++)
	{
		if(pData[i] == Data)
			j++;
	}
	return j;
}
int Std::GetChr(const char* pData,char Data)
{
	int i,j;
	for(i=0,j=0;pData[i] != 0;i++)
	{
		if(pData[i] == Data)
			j++;
	}
	return j;
}
int Std::GetNum16(const char* pData,int nLength)
{
	int nData = 0;
	for(int i=0;i < nLength;i++)
	{
		nData *= 16;
		if(pData[i] >= '0' && pData[i] <= '9')
		{
			nData += (int)pData[i] - '0';
		}
		else if(pData[i] >= 'A' && pData[i] <= 'F')
		{
			nData += (int)pData[i] - 'A' + 10;
		}
		else if(pData[i] >= 'a' && pData[i] <= 'f')
		{
			nData += (int)pData[i] - 'a' + 10;
		}
	}
	return nData;
}
char* Std::StrCnv(const char* pData,char* pString1,char* pString2)
{
	int i,j;
	const char* pBuff;
	int nLength1 = (int)strlen(pString1);
	int nLength2 = (int)strlen(pString2);
	pBuff=pData;
	for(i=0;;i++)
	{
		pBuff = strstr(pBuff,pString1);
		if(pBuff == NULL)
			break;
		pBuff++;
	}
	char* pDest = NEW char[1 + strlen(pData) + (nLength2 - nLength1) * i];

	pBuff=pData;
	for(i=0,j=0;pData[i] != 0;)
	{
		pBuff = strstr(pBuff,pString1);
		if(pBuff != NULL)
		{
			pBuff++;
		}
		for(;&pData[i+1] != pBuff && pData[i] != 0;i++,j++)
			pDest[j] = pData[i];
		if(pBuff != NULL)
		{
			strncpy(&pDest[j],pString2,nLength2);
			i+=nLength1;
			j+=nLength2;
		}
	}
	pDest[j] = 0;
	return pDest;
}
LPSTR Std::replacString(LPCSTR pData,int nCount,LPCSTR pString[])
{
	int i,j,k,l;
	int* pnCount = NEW int[nCount];
	const char* pBuff = pData;
	
	for(j=0;j<nCount;j++)
	{
		pnCount[j] = 0;
	}
	for(i=0;;i++)
	{
		const char* pBuff2 = 0;
		for(l=0,k=0;k<nCount;k++)
		{
			const char* pBuff3 = strstr(pBuff,pString[k*2]);
			if((pBuff3 < pBuff2 || pBuff2 == 0) && pBuff3 != 0)
			{
				pBuff2 = pBuff3;
				l = k;  	
			}
		}
		if(pBuff2==0)
			break;
		pBuff = pBuff2;
		++pnCount[l];
		pBuff+=strlen(pString[l*2]);
	}
	
	int nLength=0;
	for(j=0;j<nCount;j++)
	{
		nLength += ((int)strlen(pString[j*2+1]) - (int)strlen(pString[j*2])) * pnCount[j];
	}
	char* pDest = NEW char[1 + strlen(pData) + nLength];
	pBuff=pData;

	for(i=0,j=0;pData[i] != 0;)
	{
		const char* pBuff2 = 0;
		for(l=0,k=0;k<nCount;k++)
		{
			const char* pBuff3 = strstr(pBuff,pString[k*2]);
			if((pBuff3 < pBuff2 || pBuff2 == 0) && pBuff3 != 0)
			{
				pBuff2 = pBuff3;
				l = k;
			}
		}
		k=l;
		pBuff = pBuff2;

		for(;&pData[i] != pBuff && pData[i] != 0;i++,j++)
			pDest[j] = pData[i];
		if(pBuff != NULL)
		{
			pBuff += strlen(pString[k*2]);
			strcpy(&pDest[j],pString[k*2+1]);
			i+=(int)strlen(pString[k*2]);
			j+=(int)strlen(pString[k*2+1]);
		}
	}
	pDest[j] = 0;
	delete pnCount;
	return pDest;
}



void replaceString(std::string& dest,LPCSTR src,ConvertItem* item)
{
	dest.clear();
	ConvertItem::iterator it;
	LPCSTR work = src;
	while(*work)
	{
		for(it = (*item).begin();it != (*item).end();++it)
		{

			INT length = (INT)it->first.length();
			if(strncmp(work,it->first.c_str(),length) == 0)
			{
				dest += it->second;
				work += length;
				break;
			}
		}
		if(it == (*item).end())
		{
			dest += *work++;
		}
	}

}
void replaceString(std::wstring& dest,LPCWSTR src,ConvertItemW* item)
{
	dest.clear();
	ConvertItemW::iterator it;
	LPCWSTR work = src;
	while(*work)
	{
		for(it = (*item).begin();it != (*item).end();++it)
		{

			INT length = (INT)it->first.length();
			if(wcsncmp(work,it->first.c_str(),length) == 0)
			{
				dest += it->second;
				work += length;
				break;
			}
		}
		if(it == (*item).end())
		{
			dest += *work++;
		}
	}

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Debug
// デバッグ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

bool Debug::out(LPCSTR string, ...)
{
	if(m_level == 0 || !open())
		return false;
	va_list argList;
	va_start(argList, string);
	vfprintf(m_file,string,argList);
	va_end (argList);
	return true;
}
bool Debug::open()
{
	if(m_file)
		return true;
	m_file = fopen("debug.txt","wt");
	if(m_file)
		return true;
	return false;
}
void Debug::setLevel(INT level)
{
	m_level = level;
}

INT Debug::m_level = 0;
FILE* Debug::m_file = NULL;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TimeString
// 時間フォーマット用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

LPCSTR TimeString::getDateTime()
{
	time_t timeData;
	time(&timeData);
	tm t = *localtime(&timeData);
	m_value.printf("%04d%02d%02d%02d%02d%02d",
		t.tm_year+1900,t.tm_mon+1,t.tm_mday,
		t.tm_hour,t.tm_min,t.tm_sec);
	return m_value;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SleepTimer
// ウエイト制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifdef _WIN32
SleepTimer::SleepTimer()
{
	m_handle = CreateWaitableTimer(NULL,false,NULL);
}
SleepTimer::~SleepTimer()
{
	CloseHandle(m_handle);
}
bool SleepTimer::sleep(DWORD t)
{
	LARGE_INTEGER li;
	li.QuadPart = -(LONGLONG)(t)*10000;
	SetWaitableTimer(m_handle,&li,NULL,NULL,NULL,false);
	return WaitForSingleObject(m_handle,INFINITE) == WAIT_OBJECT_0;
}
void SleepTimer::wake()
{
	LARGE_INTEGER li;
	li.QuadPart = 0;
	SetWaitableTimer(m_handle,&li,NULL,NULL,NULL,false);
	//CancelWaitableTimer(m_handle);
}
#else
SleepTimer::SleepTimer()
{
}
SleepTimer::~SleepTimer()
{
}
bool SleepTimer::sleep(DWORD t)
{
	Sleep(1000);
	return true;
}
void SleepTimer::wake()
{
}
#endif

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TimeCounter
// 進行速度維持用カウンタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
TimeCounter::TimeCounter()
{
#ifdef _WIN32
	//タイマーの分解能設定
	TIMECAPS timeCaps;
	timeGetDevCaps(&timeCaps,sizeof(TIMECAPS));       
	timeBeginPeriod(timeCaps.wPeriodMin);
#endif
	m_bEnable = true;
	m_dwSyncTime = 10;
	resetTime();
}
TimeCounter::~TimeCounter()
{
#ifdef _WIN32
	//タイマーの分解能復帰
	TIMECAPS timeCaps;
	timeGetDevCaps(&timeCaps,sizeof(TIMECAPS));       
	timeEndPeriod(timeCaps.wPeriodMin);
#endif
}
void TimeCounter::setTimeSync(DWORD dwTime)
{
	m_dwSyncTime = dwTime;
	resetTime();
}
void TimeCounter::resetTime()
{
	m_iNowCount = 0;
	m_dwStartTime = timeGetTime();
}
DWORD TimeCounter::getTime() const
{
	return timeGetTime() - m_dwStartTime;
}
INT TimeCounter::getCount()
{
	//ウエイト無効のスキップ処理
/*	if(!m_bEnable)
	{
		m_iNowCount =  getTime() / m_dwSyncTime;
		m_dwSleepTime = 0;
		return 1;
	}
*/
	//通常処理
	INT iOldCount;
	DWORD dwTime;
	INT iCount;

	iOldCount = m_iNowCount;
	dwTime = getTime();
	if(m_dwSyncTime)
		m_iNowCount =  dwTime / m_dwSyncTime;
	else
		return 1;
	iCount = m_iNowCount - iOldCount;
	
	if(!iCount)
		m_dwSleepTime = dwTime % m_dwSyncTime;
	else
		m_dwSleepTime = 0;
	return iCount;
}
void TimeCounter::sleep() const
{
	if(m_bEnable)
		Sleep(m_dwSleepTime);
	else
		Sleep(0);
}
DWORD TimeCounter::sleepTime() const
{
	if(m_bEnable && m_dwSyncTime)
		return m_dwSleepTime;
	return 0;
}void TimeCounter::setEnable(bool bFlag)
{
	m_bEnable = bFlag;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ThreadTimer
// 時間制御ループスレッド
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

ThreadTimer::ThreadTimer()
{
	m_bEnable = false;
	m_bEnter = false;

#if defined(_WIN32) | defined(_WIN32_WCE)
	//分解能を最小に設定
	TIMECAPS caps;
	timeGetDevCaps(&caps,sizeof(caps));
	::timeBeginPeriod(caps.wPeriodMin);
#endif
}
ThreadTimer::~ThreadTimer()
{
	stop();
#if defined(_WIN32) | defined(_WIN32_WCE)
	//分解能の設定を解除
	TIMECAPS caps;
	timeGetDevCaps(&caps,sizeof(caps));
	::timeEndPeriod(caps.wPeriodMin);
#endif
}
void ThreadTimer::start(LPVOID pvoid)
{
	stop();
	m_aflProc = CLASSPROC(this,ThreadTimer,onProc);
	m_aflTimeCounter.resetTime();
	m_bEnable = true;
	m_bEnter = false;
	m_thread.startThread(m_aflProc,pvoid);
}
void ThreadTimer::stopSignal()
{
	m_bEnable = false;
	m_sleep.wake();
}
void ThreadTimer::stop()
{
	INT i;
	m_bEnable = false;
	for(i=0;i<3000 && m_bEnter;i++)
		Sleep(1);
}
DWORD ThreadTimer::getInterval()
{
	return m_aflTimeCounter.getTimeSync();
}
void ThreadTimer::setInterval(DWORD dwInterval)
{
	m_aflTimeCounter.setTimeSync(dwInterval);
}
void ThreadTimer::setRender(const ClassProc& paflClassProc)
{
	m_paflProcRender = paflClassProc;
}

void ThreadTimer::setStart(const ClassProc& paflClassProc)
{
	m_paflProcStart = paflClassProc;
}
void ThreadTimer::setEnd(const ClassProc& paflClassProc)
{
	m_paflProcEnd = paflClassProc;
}
void ThreadTimer::setAction(const ClassProc& paflClassProc)
{
	m_paflProcAction = paflClassProc;
}
void ThreadTimer::setIdle(const ClassProc& paflClassProc)
{
	m_paflProcIdle = paflClassProc;
}

DWORD ThreadTimer::onProc(LPVOID pvoid)
{
	m_bEnter = true;

	if(m_paflProcStart.isAddress())
		m_paflProcStart.call(pvoid);

	while(m_bEnable)
	{
		m_critical.lock();

		INT i;
		//必要動作回数
		INT iCount = m_aflTimeCounter.getCount();

		//実行処理用ループ
		if(m_bEnable && m_paflProcAction.isAddress()) 
		{
			for(i=0;m_bEnable && i<iCount;i++)
				m_paflProcAction.call(pvoid);
		}
		//アイドル処理用
		if(m_bEnable && m_paflProcIdle.isAddress())
		{
			m_paflProcIdle.call(pvoid);
		}
		//レンダリング用
		if(m_bEnable && m_paflProcRender.isAddress())
		{
			//レンダリングの必要性を確認
			if(m_bEnable && (iCount > 0 || !m_aflTimeCounter.isEnable()))
			{
				m_paflProcRender.call(pvoid);
			}
		}
		m_critical.unlock();
		if(m_bEnable)
			m_sleep.sleep(m_aflTimeCounter.sleepTime());
	}

	if(m_paflProcEnd.isAddress())
		m_paflProcEnd.call(pvoid);

	m_bEnter = false;

	return 0;
}
void JsonEscape(String& dest,LPCSTR src)
{
	ConvertItem item;
	item["\""] = "\\\"";
	item["\n"] = "\\n";

	replaceString(dest, src, &item);
}

JsonData<String>::JsonData(String data)
{
	m_data = data;
}
void JsonData<String>::getString(String& dest, int level)
{
	String data;
	JsonEscape(data, getData().c_str());
	dest.printf("\"%s\"", data.c_str());
}
String& JsonData<String>::getData()
{
	return m_data;
}

JsonData<LPCSTR>::JsonData(LPCSTR data)
{
	m_data = data;
}
void JsonData<LPCSTR>::getString(String& dest, int level)
{
	String data;
	JsonEscape(data, getData().c_str());
	dest.printf("\"%s\"", data.c_str());
}
String& JsonData<LPCSTR>::getData()
{
	return m_data;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonArray
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void JsonArray::getString(String& dest,int level)
{
	bool flag = false;
	//dest = getData();
	dest = "[";
	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{
		String work;
		(*it)->getString(work, level);
		if (flag)
			dest.appendf(",%s", work.c_str());
		else
		{
			flag = true;
			dest.appendf("%s", work.c_str());
		}
	}
	dest += "]";
}
void JsonArray::add(JsonObject* object)
{
	m_data.push_back(object);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// JsonHash
// Jsonデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void JsonHash::add(LPCSTR name, JsonObject* object)
{
	m_data[name] = object;
}
void JsonHash::getString(String& dest, int level)
{
	bool flag = false;
	//dest = getData();
	dest.clear();
	if (level)
		dest += "\n";

	int i;
	for (i = 0; i < level; i++)
		dest += "\t";
	dest += "{\n";
	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{


		String name;
		JsonEscape(name, it->first.c_str());
		String work;
		it->second->getString(work,level+1);
		if (flag)
			dest += ",\n";
		for (i = 0; i < level + 1; i++)
			dest += "\t";
		dest.appendf("\"%s\":%s", name.c_str(), work.c_str());
		flag = true;
	}
	dest += "\n";
	for (i = 0; i < level; i++)
		dest += "\t";
	dest += "}";
}
//namespace
};
