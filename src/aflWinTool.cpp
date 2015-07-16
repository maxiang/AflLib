#include <windows.h>


#include "aflWinTool.h"
#ifndef _WIN32_WCE
	#include <direct.h>
	#include <sys/stat.h> 
#endif
#include <stdio.h>
#include <stdlib.h>
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
		#define CHECK_MEMORY_LEAK
#endif
//----------------------------------------------------

#if _MSC_VER && !defined(_WIN32_WCE)
	#pragma comment(lib, "imm32.lib")
	#pragma comment(lib, "winmm.lib")
#endif 
#if defined(_WIN32_WCE)
	#define timeGetTime GetTickCount
#endif
//namespace AFL::Windows
namespace AFL{namespace WINDOWS{
bool deleteDirectory(LPCWSTR path)
{
	WString pathSrc = path;
	if(pathSrc.back() != L'/' && pathSrc.back() != L'\\')
		pathSrc += L"\\";
	
	WString pathFind = pathSrc;
	pathFind += L"*.*";

	WIN32_FIND_DATAW findData;
	HANDLE handle = FindFirstFileW(pathFind,&findData);
	if(handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(wcscmp(findData.cFileName,L".") == 0 || wcscmp(findData.cFileName,L"..") == 0)
				continue;
			WString pathName = pathSrc;
			pathName += findData.cFileName;
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				deleteDirectory(pathName);
			}
			else
			{
				DeleteFileW(pathName);
			}
		
		}while(FindNextFileW(handle,&findData));
		FindClose(handle);
	}
	return RemoveDirectoryW(path) != FALSE;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Stdout
// 標準出力横流し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Stdout::Stdout()
{
	//パイプの作成
	CreatePipe(&m_readHandle, &m_writeHandle, NULL, 0);
	INT handle = _open_osfhandle((intptr_t)m_writeHandle, _O_WTEXT);

	m_oldStdout = *stdout;
	*stdout = *_fdopen(handle, "wt");
	setvbuf(stdout,NULL,_IONBF ,0);

	m_thread.startThread(CLASSPROC(this,Stdout,onThread));
}
Stdout::~Stdout()
{
	CloseHandle(m_readHandle);
	fclose(stdout);
	*stdout = m_oldStdout;
	while(m_thread.isActiveThread())
		Sleep(1);
}
void Stdout::setCallback(ClassProc& proc)
{
	m_classProc = proc;
}
void Stdout::onThread(LPVOID data)
{
	DWORD size,length;
	CHAR buff[1024];
	while(PeekNamedPipe(m_readHandle,NULL,0,NULL,&length,NULL))
	{
		DWORD readSize;
		while(length)
		{
			size = length;
			if(size > 1022)
				size = 1022;
			ReadFile(m_readHandle,buff,size,&readSize,NULL);
			buff[readSize] = 0;
			buff[readSize+1] = 0;
			Message m = {buff,size};
			m_classProc.call(&m);
			length -= size;
		}
		Sleep(10);
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// FRect
// 矩形サイズ小数点
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

FLOAT FRect::getWidth()const
{
	return right - left;
}
FLOAT FRect::getHeight()const
{
	return bottom - top;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Event
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Event::Event()
{
	m_hEvent = CreateEvent(NULL,false,false,NULL);
}
Event::~Event()
{
	release();
}
void Event::release()
{
	if(m_hEvent)
	{
		setEvent();
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}
void Event::setEvent()
{
	SetEvent(m_hEvent);
}

HANDLE Event::getHandle()const
{
	return m_hEvent;
}
void Event::wait()const
{
	WaitForSingleObject(m_hEvent,INFINITE);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Semaphore
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Semaphore::Semaphore()
{
	m_hEvent = CreateSemaphore(NULL,0,100000,NULL);
}
Semaphore::~Semaphore()
{
	CloseHandle(m_hEvent);
}
void Semaphore::clear()
{
	while(ReleaseSemaphore(m_hEvent,1,NULL));
}
bool Semaphore::release(LONG lCount)
{
	return ReleaseSemaphore(m_hEvent,lCount,NULL)!=0;
}
HANDLE Semaphore::getHandle()const
{
	return m_hEvent;
}
void Semaphore::wait()const
{
	WaitForSingleObject(m_hEvent,INFINITE);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mutex
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Mutex::Mutex()
{
	m_mutex = NULL;
}
Mutex::~Mutex()
{
	if(m_mutex)
	{
		ReleaseMutex( m_mutex );
		CloseHandle( m_mutex );
	}
}
bool Mutex::create(LPCSTR name)
{
	m_mutex = CreateMutexA(NULL, true, name);
	if(m_mutex && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex( m_mutex );
		CloseHandle( m_mutex );
		m_mutex = NULL;
	}

	return m_mutex != NULL;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AdviseTimer
// 周期呼び出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
AdviseTimer::AdviseTimer()
{
	//DirectShow用のIReferenceClockのGUID
	static const GUID IID_IReferenceClock = {0x56a86897,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70};
	static const GUID CLSID_SystemClock = {0xe436ebb1, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70};
	//COMを利用可能に
	::CoInitializeEx(NULL,COINIT_MULTITHREADED);

	//DirectShow用IReferenceClockを利用可能に
	::CoCreateInstance( CLSID_SystemClock , NULL , CLSCTX_INPROC_SERVER , IID_IReferenceClock , (LPVOID*)&m_pReferenceClock);
	//周期カウンタの初期化
	m_dwAdviseToken = 0;
}
AdviseTimer::~AdviseTimer()
{
	stopTimer();
	//IReferenceClockの解放
	m_pReferenceClock->Release();
	//COM利用完了
	//::CoUninitialize();
}
bool AdviseTimer::startTimer(DWORD dwTime)
{
	stopTimer();
	//現在のカウントを開始カウンタに設定
	REFERENCE_TIME rtBase;
	m_pReferenceClock->GetTime(&rtBase);
	//セマフォイベントの設定
	return m_pReferenceClock->AdvisePeriodic(rtBase,dwTime,(HEVENT)m_semaphore.getHandle(),&m_dwAdviseToken) == S_OK;
}
bool AdviseTimer::stopTimer()
{
	if(!m_dwAdviseToken)
		return false;
	//イベントの動作停止
	bool bFlag = m_pReferenceClock->Unadvise(m_dwAdviseToken) == S_OK;
	m_dwAdviseToken = 0;
	return bFlag;
}
void AdviseTimer::waitTimer()
{
	//時間が来るまで待機
	m_semaphore.wait();
}
bool AdviseTimer::release(LONG lCount)
{
	return m_semaphore.release();
}
REFERENCE_TIME AdviseTimer::getTime()
{
	//現在のカウントを取得
	REFERENCE_TIME rtBase;
	m_pReferenceClock->GetTime(&rtBase);
	return rtBase;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// BenchCounter
// ベンチマーク用カウンタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
BenchCounter::BenchCounter()
{
#ifndef _WIN32_WCE
	TIMECAPS timeCaps;
	timeGetDevCaps(&timeCaps,sizeof(TIMECAPS));
	m_period = timeCaps.wPeriodMin;

	timeBeginPeriod(m_period);
#endif
}
BenchCounter::~BenchCounter()
{
#ifndef _WIN32_WCE
	timeEndPeriod(m_period);
#endif
}
void BenchCounter::clear()
{
	m_count = timeGetTime();
}
UINT BenchCounter::get() const
{
	return timeGetTime() - m_count;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PathName
// ファイル名とパス制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#if !defined(_WIN32_WCE)
bool PathName::setFileName(LPCSTR pFileName)
{
	return setFileName(UCS2(pFileName));
}
bool PathName::setFileName(LPCWSTR pFileName)
{
	WCHAR cPath[_MAX_PATH];
	WCHAR cDrive[_MAX_DRIVE];
	WCHAR cDir[_MAX_DIR];
	WCHAR cFname[_MAX_FNAME];
	WCHAR cExt[_MAX_EXT];

	int i;
	std::wstring fileName;
	if(pFileName[0] == '"')
	{
		for(i=1;pFileName[i] && pFileName[i]!='"';i++)
			fileName += pFileName[i];
	}
	else
		fileName = pFileName;
	
	_wfullpath(cPath,fileName.c_str(),_MAX_PATH);
	_wsplitpath(cPath,cDrive,cDir,cFname,cExt);
	
	m_strFullPath = cPath;
	m_strDrive = cDrive;
	m_strDir = cDir;
	m_strFname = cFname;
	m_strExt = cExt;
	
	m_strPath = m_strDrive.c_str();
	m_strPath += m_strDir.c_str();

	m_strFileName = m_strFname.c_str();
	m_strFileName += m_strExt.c_str();

	return true;
}
std::wstring PathName::relativePath(LPCWSTR pFileName)
{
	std::wstring strFileName;
	PathName fileName(pFileName);

	LPCWSTR pBasePath = fileName.getPathNameW();
	LPCWSTR pPath = getPathNameW();

	INT i;
	INT iBaseCount;
	INT iLength = (INT)wcsspn(pBasePath,pPath);
	if(iLength)
	{
		for(;iLength>0;iLength--)
			if(pBasePath[iLength] == L'\\' || pBasePath[iLength] == L'/')
				break;
		iLength++;

		for(iBaseCount=0,i=iLength;pBasePath[i];i++)
			if(pBasePath[i] == L'\\' || pBasePath[i] == L'/')
				iBaseCount++;
		if(pPath[i] || pBasePath[i])
		{
			for(i=0;i<iBaseCount;i++)
				strFileName += L"..\\";

			strFileName += pPath + iLength;
		}
	}
	else
		strFileName += pPath;

	strFileName += m_strFileName.c_str();
	
	return strFileName;
}
bool PathName::pushPath()
{
	WCHAR cPath[_MAX_PATH];
	m_strTempPath = _wgetcwd(cPath,_MAX_PATH);
	return true;
}
bool PathName::popPath()
{
	if(m_strTempPath[0] >= 'A' && m_strTempPath[0] <= 'Z')
		_chdrive(m_strTempPath[0] - 'A' + 1);
	else if(m_strTempPath[0] >= 'a' && m_strTempPath[0] <= 'z')
		_chdrive(m_strTempPath[0] - 'a' + 1);
	_wchdir(m_strTempPath.c_str());
	return true;
}

bool PathName::changePath()
{
	LPCWSTR drive = m_strDrive.c_str();
	if(drive[0] >= 'A' && drive[0] <= 'Z')
		_chdrive(drive[0] - 'A' + 1);
	else if(drive[0] >= 'a' && drive[0] <= 'z')
		_chdrive(drive[0] - 'a' + 1);
	_wchdir(m_strPath.c_str());
	return true;
}
LPCSTR PathName::getFileName()const
{
	return SJIS(m_strFileName);
}
LPCSTR PathName::getPathName()const
{
	return SJIS(m_strPath.c_str());
}
LPCSTR PathName::getSFileName()const
{
	return SJIS(m_strFname.c_str());
}
LPCWSTR PathName::getFileNameW()const
{
	return m_strFileName.c_str();
}
LPCWSTR PathName::getPathNameW()const
{
	return m_strPath.c_str();
}
LPCWSTR PathName::getSFileNameW()const
{
	return m_strFname.c_str();
}
void PathName::getPathDir(String& dest)
{
	dest.clear();
	if(m_strDrive.c_str()[0])
		dest.printf("%ls",m_strDrive.c_str());
	dest.appendf("%ls",m_strDir.c_str());
}
void PathName::getPathDir(WString& dest)
{
	dest.clear();
	if(m_strDrive.c_str()[0])
		dest.printf(L"%s",m_strDrive.c_str());
	dest.appendf(L"%s",m_strDir.c_str());
}
#endif
//------------------------------------------------------------
// Rect3D
// ３次元座標管理
//------------------------------------------------------------

//-----------------------------------------------------
//	初期化
//-----------------------------------------------------
Rect3D::Rect3D()
{
	left=right=top=bottom=high=low=0;
}
//-----------------------------------------------------
//	初期化
//-----------------------------------------------------
Rect3D::Rect3D(int nLeft,int nTop,int nHigh,int nRight,int nBottom,int nLow)
{
	setRect(nLeft,nTop,nHigh,nRight,nBottom,nLow);
}
Rect3D::Rect3D(int nLeft,int nTop,int nRight,int nBottom)
{
	setRect(nLeft,nTop,nRight,nBottom);
}
//-----------------------------------------------------
//	座標設定
//-----------------------------------------------------
void Rect3D::setRect(int nLeft,int nTop,int nHigh,int nRight,int nBottom,int nLow)
{
	left = nLeft; right=nRight;
	top = nTop; bottom = nBottom;
	high = nHigh; low = nLow;
}
void Rect3D::setRect(int nLeft,int nTop,int nRight,int nBottom)
{
	left = nLeft; right=nRight;
	top = nTop; bottom = nBottom;
	high = 0; low = 0;
}
//-----------------------------------------------------
//	座標移動
//-----------------------------------------------------
void Rect3D::offsetRect(int nX,int nY,int nZ)
{
	left += nX; right += nX;
	top += nY; bottom += nY;
	high += nZ; low += nZ;
}
//-----------------------------------------------------
//	全て0かどうかの判定
//-----------------------------------------------------
bool Rect3D::isRectNull() const
{
	if(left | right | top | bottom | high | low)
		return false;
	else
		return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Font
// フォント系
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------------
Font::Font()
{
	ZeroMemory(&m_logFont,sizeof(LOGFONT));
	m_hFont = 0;
	m_logFont.lfWeight = FW_NORMAL;
	m_logFont.lfHeight = 16;
	m_logFont.lfCharSet = DEFAULT_CHARSET;
	m_logFont.lfQuality = DEFAULT_QUALITY;
	setFontName(L"ＭＳ ゴシック");
}
//-----------------------------------------------------
Font::~Font()
{
	deleteFont();
}
//-----------------------------------------------------
void Font::setFontName(LPCWSTR name)
{
	WString dest = name;
	wcscpy(m_logFont.lfFaceName,dest);
}
//-----------------------------------------------------
bool Font::createFont(LPLOGFONTW logFont)
{
	deleteFont();
	if(logFont)
		m_logFont = *logFont;
	m_hFont = ::CreateFontIndirectW(&m_logFont);
	return m_hFont!=0;
}

//-----------------------------------------------------
void Font::deleteFont()
{
	if(m_hFont)
	{
		::DeleteObject(m_hFont);
		m_hFont = 0;
	}
}
//-----------------------------------------------------
bool Font::getFontSize(LPSIZE pSize,LPCSTR pText,INT iLength,INT iLimitWidth,bool mline) 
{
	return getFontSize(pSize,*this,pText,iLength,iLimitWidth,mline);
}
//-----------------------------------------------------
bool Font::getFontSize(LPSIZE pSize,LPCWSTR pText,INT iLength,INT iLimitWidth,bool mline) 
{
	return getFontSize(pSize,*this,pText,iLength,iLimitWidth,mline);
}
//-----------------------------------------------------
bool Font::getFontSize(LPSIZE pSize,HFONT hFont,LPCSTR pText,INT iLength,INT iLimitWidth,bool mline)
{
	WString work;
	work = pText;
	return getFontSize(pSize,hFont,work.c_str(),iLength,iLimitWidth,mline);
}


//-----------------------------------------------------
bool Font::getFontSize(LPSIZE pSize,HFONT hFont,LPCWSTR pText,INT iLength,INT iLimitWidth,bool mline)
{
	if(iLength < 0)
		iLength = lstrlenW(pText);
	//フォントサイズの取得
	SIZE sizeFont;
	INT i;
	HDC hdmDC = CreateCompatibleDC(NULL);
	HFONT holdFont = (HFONT)SelectObject(hdmDC,hFont);

	GetTextExtentPoint32W(hdmDC,L" ",1,&sizeFont);

	INT iWidth = 0;
	INT iHeight = 0;
	INT iMaxWidth = 0;
	for(i=0;i<iLength;i++)
	{
		if(pText[i] == '\t')
		{
			GetTextExtentPoint32W(hdmDC,L" ",1,&sizeFont);
			iWidth += sizeFont.cx*4;
			iWidth -= iWidth % (sizeFont.cx*4);
		}
		else if(pText[i] != '\n')
			GetTextExtentPoint32W(hdmDC,pText+i,1,&sizeFont);
		else
			sizeFont.cx = 0;
		if(pText[i] == '\n' || iLimitWidth>0 && sizeFont.cx + iWidth > iLimitWidth)
		{
			if(!mline)
				break;
			iHeight += sizeFont.cy;
			if(iWidth > iMaxWidth)
				iMaxWidth = iWidth;
			iWidth = sizeFont.cx;
		}
		else
			iWidth += sizeFont.cx;
	}
	iHeight += sizeFont.cy;
	SelectObject(hdmDC,holdFont);
	DeleteDC(hdmDC);
	
	if(iWidth > iMaxWidth)
		iMaxWidth = iWidth;
	
	if(pSize)
	{
		pSize->cx = iMaxWidth;
		pSize->cy = iHeight;
	}
	return true;
}
//-----------------------------------------------------
Font::operator HFONT()
{
	if(!m_hFont)createFont();return m_hFont;
}

void Font::setSize(int nWidth,int nHeight)
{
	m_logFont.lfWidth = nWidth;m_logFont.lfHeight=nHeight;
}
void Font::setSize(int nHeight)
{
	HDC hdc = GetDC(NULL);
	m_logFont.lfWidth = 0;
	m_logFont.lfHeight= -MulDiv(nHeight,::GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
	ReleaseDC(NULL,hdc);
}
int Font::getSize()
{
	return m_logFont.lfHeight;
}
void Font::setBold(int iBold)
{
	m_logFont.lfWeight = iBold;
}
INT Font::getBold()const
{
	return m_logFont.lfWeight;
}
INT Font::getItalic()const
{
	return m_logFont.lfItalic;
}
void Font::setUnderline(bool flag)
{
	m_logFont.lfUnderline = flag;
}
void Font::setItalic(INT value)
{
	m_logFont.lfItalic = value;
}
LPCWSTR Font::getFontName()const
{
	GetObjectW(m_hFont,sizeof(LOGFONTW),(LPVOID)&m_logFont);
	return m_logFont.lfFaceName;
}
LPLOGFONTW Font::getLogFont()
{
	return &m_logFont;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fep
// FEP制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------------
Fep::Fep()
{
	m_hWnd = 0;
	m_pCandidateList = NULL;
}
//-----------------------------------------------------
Fep::~Fep()
{
	release();
}
//-----------------------------------------------------
bool Fep::release()
{
	//コンテキストの解放
	if(m_pCandidateList)
	{
		delete m_pCandidateList;
		m_pCandidateList = NULL;
	}

	return true;
}
//-----------------------------------------------------
bool Fep::setWindow(HWND hWnd)
{
	release();
	if(hWnd)
	{
		//既存のFEPの破壊
		HWND hFep = ImmGetDefaultIMEWnd(hWnd);
		//SendMessage(hFep,WM_CLOSE,0,0);

		//ウインドウに関連づけられているFEPの取得
		m_hWnd = hWnd;
	}
	return true;
}
//-----------------------------------------------------
bool Fep::isOpen() const
{
	//FEPの活動状態を取得
	HIMC hImc = ImmGetContext(m_hWnd);
	bool bRet = ImmGetOpenStatus(hImc)!=false;
	ImmReleaseContext(m_hWnd,hImc);
	return bRet;
}
//-----------------------------------------------------
bool Fep::setOpenStatus(bool bFlag)
{
	HIMC hImc = ImmGetContext(m_hWnd);
	bool bRet = ImmSetOpenStatus(hImc,bFlag)!=false;
	ImmReleaseContext(m_hWnd,hImc);
	return bRet;
}
//-----------------------------------------------------
bool Fep::getStatus(LPDWORD pdwConversion,LPDWORD pdwSentence) const
{
	//FEPの入力モード
	 HIMC hImc = ImmGetContext(m_hWnd);
	bool bRet = ImmGetConversionStatus(hImc,pdwConversion,pdwSentence)!=false;
	ImmReleaseContext(m_hWnd,hImc);
	return bRet;
}
//-----------------------------------------------------
bool Fep::getString(std::string& strBuff) const
{
	//FEP入力文字列の取得
	LONG lByte;
	 HIMC hImc = ImmGetContext(m_hWnd);
	lByte = ImmGetCompositionStringA(hImc,GCS_COMPSTR,NULL,0);
	if(lByte > 0)
	{
		LPSTR pBuff = NEW CHAR[lByte+1];
		ImmGetCompositionStringA(hImc,GCS_COMPSTR,pBuff,lByte);
		pBuff[lByte] = 0;
		strBuff = pBuff;
		delete[] pBuff;
	}
	ImmReleaseContext(m_hWnd,hImc);
	return true;
}
bool Fep::getString(std::wstring& strBuff) const
{
	//FEP入力文字列の取得
	LONG lByte;
	 HIMC hImc = ImmGetContext(m_hWnd);
	lByte = ImmGetCompositionStringW(hImc,GCS_COMPSTR,NULL,0);
	if(lByte > 0)
	{
		LPWSTR pBuff = (LPWSTR)new CHAR[lByte+2];
		ImmGetCompositionStringW(hImc,GCS_COMPSTR,pBuff,lByte);
		pBuff[lByte/2] = 0;
		strBuff = pBuff;
		delete[] pBuff;
	}
	ImmReleaseContext(m_hWnd,hImc);
	return true;
}
//-----------------------------------------------------
INT Fep::getConvertPos() const
{
	//変換位置の取得
	DWORD dwBuff[2];
	HIMC hImc = ImmGetContext(m_hWnd);
	if(ImmGetCompositionString(hImc,GCS_COMPCLAUSE,dwBuff,sizeof(DWORD)*2) <= 0)
	{
		ImmReleaseContext(m_hWnd,hImc);
		return 0;
	}
	ImmReleaseContext(m_hWnd,hImc);
	return dwBuff[1];
}
//-----------------------------------------------------
INT Fep::getCursorPos() const
{
	INT ret;
	//カーソル位置の取得
	HIMC hImc = ImmGetContext(m_hWnd);
	ret = ImmGetCompositionString(hImc,GCS_CURSORPOS,NULL,0);
	ImmReleaseContext(m_hWnd,hImc);
	if(ret)
		return ret;
	//変換位置
	ret = getConvertPos();
	return ret;
}
//-----------------------------------------------------
INT Fep::getCountCandidate()
{
	//前回使った変換候補の解放
	if(m_pCandidateList)
	{
		delete m_pCandidateList;
		m_pCandidateList = NULL;
	}
	//変換候補の取得
	HIMC hImc = ImmGetContext(m_hWnd);
	INT iCandidateSize = ImmGetCandidateList(hImc,0,NULL,0);
	if(!iCandidateSize)
	{
		ImmReleaseContext(m_hWnd,hImc);
		return 0;
	}
	ImmReleaseContext(m_hWnd,hImc);
	m_pCandidateList = (LPCANDIDATELIST)new BYTE[iCandidateSize];
	ImmGetCandidateList(hImc,0,m_pCandidateList,iCandidateSize);
	return m_pCandidateList->dwCount;
}
//-----------------------------------------------------
INT Fep::getCandidateIndex() const
{
	//現在選択中の変換候補の取得
	return m_pCandidateList->dwSelection;
}
//-----------------------------------------------------
LPCSTR Fep::getCandidateString(INT iIndex) const
{
	//指定されたインデックスの変換候補を取得
	if(!m_pCandidateList)
		return NULL;
	return (LPCSTR)((LPBYTE)m_pCandidateList+m_pCandidateList->dwOffset[iIndex]);
}
//-----------------------------------------------------
INT Fep::getCandidateStart() const
{
	//変換候補インデックスの開始位置の取得
	return m_pCandidateList->dwPageStart;
}
//-----------------------------------------------------
INT Fep::getCandidateEnd() const
{
	//変換候補インデックスの終了位置の取得
	int iCount = (int)(m_pCandidateList->dwPageStart + m_pCandidateList->dwPageSize);
	if(iCount > (int)m_pCandidateList->dwCount)
		iCount = m_pCandidateList->dwCount;
	return iCount;
}
//-----------------------------------------------------
HWND Fep::getWindow()const
{
	return m_hWnd;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClipBoard
// クリップボード制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool ClipBoard::setString(LPCSTR string)
{
	INT length = (INT)strlen(string);
	HGLOBAL global = GlobalAlloc(GHND, length + 1);
	if(global)
	{
		LPBYTE work = (LPBYTE)GlobalLock(global);
		CopyMemory(work,string,length+1);
		GlobalUnlock(global);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, global);
		CloseClipboard();
	}
	return true;
}
bool ClipBoard::setString(LPCWSTR string)
{
	INT size = ((INT)wcslen(string) + 1)*2;
	HGLOBAL global = GlobalAlloc(GHND, size);
	if(global)
	{
		LPBYTE work = (LPBYTE)GlobalLock(global);
		CopyMemory(work,string,size);
		GlobalUnlock(global);
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, global);
		CloseClipboard();
	}
	return true;
}
bool ClipBoard::getString(WString& string)
{
	string.clear();
	if( !::IsClipboardFormatAvailable(CF_UNICODETEXT) )
		return false;

	if( !::OpenClipboard(NULL) )
		return false;

	HANDLE hMem = ::GetClipboardData(CF_UNICODETEXT);
	LPCWSTR data= (LPCWSTR)::GlobalLock(hMem);
	string = data;
	::GlobalUnlock(hMem);
	::CloseClipboard();

	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TextArea
// テキストサイズ計算用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
TextArea::TextArea()
{
	m_font.setFontName(L"ＭＳ ゴシック");
	m_font.createFont();
	m_width = 640;
	m_mask = false;
}
void TextArea::setText(LPCWSTR text)
{
	m_text = text;
}

void TextArea::addText(LPCSTR text)
{
	addText(UCS2(text));
}
void TextArea::addText(LPCWSTR src)
{
	INT point = 0;
	m_text += src;
	
	if(m_areaList.size())
	{
		point = m_areaList.back().point;
		m_height -= m_font.getSize();
		m_areaList.pop_back();
	}
	compute(m_width,point);
}
void TextArea::compute(INT areaWidth)
{
	m_height = 0;
	m_width = areaWidth;
	m_areaList.clear();
	compute(areaWidth,0);
}
void TextArea::insertText(INT point,WCHAR text)
{
	WCHAR work[2];
	work[0] = text;
	work[1] = 0;
	m_text.insert(point,&text,1);
}

void TextArea::insertText(INT point,LPCWSTR text)
{
	m_text.insert(point,text);
}
void TextArea::getPoint(LPPOINT pos,INT point)
{
	INT i;
	std::list<AREA>::iterator it;
	INT x = 0;
	INT y = 0;
	for(it=m_areaList.begin();it!=m_areaList.end();++it)
	{
		if(point <= it->point + (INT)it->length)
		{
			INT length = point - it->point;
			LPCWSTR text = m_text.c_str() + it->point;
			HDC hdc = CreateCompatibleDC(NULL);
			HFONT oldFont = (HFONT)SelectObject(hdc,getFont());
			INT spaceWidth = getCharWidth(hdc,L' ');
			for(i=0;i<point-it->point;i++)
			{
				if(text[i] == '\t')
				{
					x += spaceWidth*4;
					x -= x % (spaceWidth*4);
				}
				else
					x += getCharWidth(hdc,text[i]);
			}
			SelectObject(hdc,oldFont);
			DeleteDC(hdc);
			if(x > m_width-getFontHeight())
			{
				x = 0;
				y += getFontHeight();
			}
			pos->x = x;
			pos->y = y;
			return;
		}
		y += getFontHeight();
	}
	pos->x = 0;
	pos->y = 0;
	
}
INT TextArea::getPoint(INT x,INT y)
{
	if(y < 0)
		return 0;
	INT j;
	INT count = y / m_font.getSize();
	INT i = 0;
	std::list<AREA>::iterator it;
	for(it=m_areaList.begin();it!=m_areaList.end();++it)
	{
		if(i == count)
		{
			INT point = it->point;
			LPCWSTR text = m_text.c_str() + point;
			INT width = 0;
			HDC hdc = CreateCompatibleDC(NULL);
			HFONT oldFont = (HFONT)SelectObject(hdc,m_font);
			INT spaceWidth = getCharWidth(hdc,L' ');
			for(j=0;j<(INT)it->length;j++)
			{
				if(!m_mask && text[j] == '\t')
				{
					width += spaceWidth*4;
					width -= width % (spaceWidth*4);
				}
				else
					width += getCharWidth(hdc,text[j]);
				if(x < width)
					break;
			}
			SelectObject(hdc,oldFont);
			DeleteDC(hdc);
			return point+j;
		}
		i++;
	}
	return (INT)m_text.length();
}
INT TextArea::getCharWidth(HDC hdc,WCHAR text)
{
	SIZE size;
	if(m_mask)
		GetTextExtentPoint32W(hdc,L"*",1,&size);
	else
		GetTextExtentPoint32W(hdc,&text,1,&size);
	return size.cx;
}
void TextArea::compute(INT areaWidth,INT point)
{
	 m_width = areaWidth;

	INT length = (INT)m_text.length();
	LPCWSTR text = m_text.c_str();
	INT start = point;
	//フォントサイズの取得
	INT i;
	HDC hdmDC = CreateCompatibleDC(NULL);
	HFONT holdFont = (HFONT)SelectObject(hdmDC,m_font);

	//半角スペースのサイズを取得
	INT spaceWidth = getCharWidth(hdmDC,L' ');
	INT charWidth;
	INT charHeight = m_font.getSize();
	INT iWidth = 0;
	INT iHeight = 0;
	for(i=start;i<length;i++)
	{
		if(text[i] == '\t')
		{
			iWidth += spaceWidth*4;
			iWidth -= iWidth % (spaceWidth*4);
		}
		else if(text[i] == '\n')
			charWidth = 0;
		else
			charWidth = getCharWidth(hdmDC,text[i]);
		if(text[i] == '\n' || areaWidth > 0 && charWidth + iWidth >= areaWidth)
		{
			AREA a = {start,(DWORD)(i - start)};
			m_areaList.push_back(a);
			start = i;
			if(text[start] == '\n')
				start++;

			iHeight += charHeight;
			iWidth = charWidth;
		}
		else
			iWidth += charWidth;
	}
	iHeight += charHeight;
	m_height += iHeight;
	SelectObject(hdmDC,holdFont);
	DeleteDC(hdmDC);
	
	
	AREA a = {start,(DWORD)(i - start)};
	m_areaList.push_back(a);
}

INT TextArea::getAreaHeight() const
{
	return m_height;
}
std::list<TextArea::AREA>& TextArea::getArea()
{
	return m_areaList;
}
LPCWSTR TextArea::getString() const
{
	return m_text.c_str();
}
HFONT TextArea::getFont()
{
	return (HFONT)m_font;
}
INT TextArea::getFontHeight()
{
	return m_font.getSize();
}
INT TextArea::getLength()const
{
	return (INT)m_text.length();
}
void TextArea::eraseText(INT point,INT size)
{
	m_text.erase(point,size);
}
void TextArea::setMask(bool flag)
{
	m_mask = flag;
}
bool TextArea::isMask() const
{
	return m_mask;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Variant
// VARIANT型管理用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Variant& Variant::operator=(const Variant& value)
{
	VariantClear(this);
	VariantCopy(this,&value);
	return *this;
}
Variant::Variant()
{
	vt = VT_EMPTY;
}
Variant::Variant(VARIANT value)
{
	*(VARIANT*)this = value;
}
Variant::Variant(LPCWSTR value)
{
	vt = VT_BSTR;
	bstrVal  = SysAllocString(value);
}
Variant::Variant(UINT value)
{
	vt = VT_UINT;
	uintVal = value;
}
Variant::Variant(INT value)
{
	vt = VT_INT;
	intVal = value;
}
Variant::~Variant()
{
	VariantClear(this);
}
INT Variant::toInt() const
{
	VARIANT v;
	v.vt = VT_EMPTY;
	VariantChangeType(&v,this,0,VT_I4);
	return v.intVal;
}
LPCWSTR Variant::toString()
{
	VariantChangeType(this,this,0,VT_BSTR);
	if(vt == VT_BSTR)
		return bstrVal;
	return L"";
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AXObject
// ActiveXオブジェクトパーサ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
AXObject::AXObject()
{
	CoInitialize(NULL);
	m_disp = NULL;
}
AXObject::AXObject(Variant& value)
{
	CoInitialize(NULL);
	m_disp = value.pdispVal;
	m_disp->AddRef();
}
AXObject::AXObject(IDispatch* disp)
{
	CoInitialize(NULL);
	disp->AddRef();
	m_disp = disp;
}
AXObject::AXObject(LPCWSTR name)
{
	CoInitialize(NULL);

	m_disp = NULL;
	CLSID clsid;
	if(CLSIDFromProgID(name,&clsid) == S_OK)
	{
		CoCreateInstance(clsid,NULL,CLSCTX_SERVER, IID_IDispatch,(void**)&m_disp);
	}
}
AXObject& AXObject::operator=(IDispatch* disp)
{
	if(m_disp)
		m_disp->Release();
	disp->AddRef();
	m_disp = disp;
	return *this;
}
AXObject::~AXObject()
{
	if(m_disp)
		m_disp->Release();
	CoUninitialize();
}
Variant AXObject::invoke(OLECHAR* name, int nArgs, ...)
{
	if(!m_disp)
		return Variant();
	DISPID dispID=0;
	if(m_disp->GetIDsOfNames(IID_NULL,&name,1,LOCALE_USER_DEFAULT,&dispID) != S_OK)
		return Variant();

	INT i;
	va_list ap;

	VARIANTARG *pvArgs = NULL;
	if(nArgs > 0)
	{
		pvArgs = new VARIANTARG[nArgs];

		va_start(ap, nArgs);
		for(i=0;i<nArgs;i++)
		{
			VariantInit(&pvArgs[i]);
			VARIANT* v = va_arg(ap, VARIANT*);
			pvArgs[i] = *v;
		}
		va_end(ap);
	}
	// DISPPARAMS の設定
	DISPPARAMS dispParams;
	dispParams.rgvarg = pvArgs;
	dispParams.rgdispidNamedArgs = NULL;
	dispParams.cArgs = nArgs;
	dispParams.cNamedArgs = 0;

	VARIANT result;
	HRESULT hr = m_disp->Invoke(dispID, IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_METHOD,
				&dispParams, &result,NULL, NULL);

	// VARIANT 配列の後始末
	for(i=0;i<nArgs;i++)
	{
		VariantClear(&pvArgs[i]);
	}

	if(pvArgs != NULL)
	{
		delete[] pvArgs;
		pvArgs = NULL;
	}
	return Variant(result);
}
Variant AXObject::get(OLECHAR* name, int nArgs, ...)
{
	if(!m_disp)
		return Variant();
	DISPID dispID=0;
	if(m_disp->GetIDsOfNames(IID_NULL,&name,1,LOCALE_USER_DEFAULT,&dispID) != S_OK)
		return Variant();

	INT i;
	va_list ap;

	VARIANTARG *pvArgs = NULL;
	if(nArgs > 0)
	{
		pvArgs = new VARIANTARG[nArgs];

		va_start(ap, nArgs);
		for(i=0;i<nArgs;i++)
		{
			VariantInit(&pvArgs[i]);
			VARIANT* v = va_arg(ap, VARIANT*);
			pvArgs[i] = *v;
		}
		va_end(ap);
	}
	// DISPPARAMS の設定
	DISPPARAMS dispParams;
	dispParams.rgvarg = pvArgs;
	dispParams.rgdispidNamedArgs = NULL;
	dispParams.cArgs = nArgs;
	dispParams.cNamedArgs = 0;

	VARIANT result;
	HRESULT hr = m_disp->Invoke(dispID, IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET,
				&dispParams, &result,NULL, NULL);

	// VARIANT 配列の後始末
	for(i=0;i<nArgs;i++)
	{
		VariantClear(&pvArgs[i]);
	}

	if(pvArgs != NULL)
	{
		delete[] pvArgs;
		pvArgs = NULL;
	}
	return Variant(result);
}
AXObject::operator IDispatch*()
{
	return m_disp;
}

void AXObject::p()
{
	FILE* file = fopen("test.txt","at");

	INT i;
	LPFUNCDESC desc;
	ITypeInfo* info;
	BSTR data;
	m_disp->GetTypeInfo(0,0,&info);
	for(i=0;;i++)
	{
		if(info->GetFuncDesc(i,&desc) != S_OK)
			break;
		UINT s;
		info->GetNames(desc->memid,&data,1,&s);

		Variant variant = get(data);

		fwprintf(file,L"%s : %s\n",data,variant.toString());
		info->ReleaseFuncDesc(desc);
	}
	fclose(file);
	info->Release();
}

Registy::Registy()
{
	m_key = NULL;
}
Registy::~Registy()
{
	if(m_key)
		RegCloseKey(m_key);
}
bool Registy::open(LPCWSTR compName,HKEY key)
{
	if(RegConnectRegistryW(compName,key,&m_key) == ERROR_SUCCESS)
		return true;
	return false;
}
bool Registy::close()
{
	if(!m_key)
		return false;
	RegCloseKey(m_key);
	return true;
}
bool Registy::readKey(LPCWSTR subKey,LPCWSTR name,DWORD& value)
{
	HKEY key;
	if(RegOpenKeyExW(m_key,subKey,0,KEY_WRITE|KEY_READ|KEY_WOW64_64KEY,&key) != ERROR_SUCCESS)
		return false;

	bool flag = false;
	DWORD dataType = REG_DWORD;
	DWORD size;
	if(RegQueryValueExW(key,name,0,&dataType,NULL,&size) == ERROR_SUCCESS)
	{
		DWORD data;
		RegQueryValueExW(key,name,0,&dataType,(LPBYTE)&data,&size);
		value = data;
		flag = true;
	}

	RegCloseKey(key);
	return flag;
}
bool Registy::readKey(LPCWSTR subKey,LPCWSTR name,WString& value)
{
	HKEY key;
	if(RegOpenKeyExW(m_key,subKey,0,KEY_WRITE|KEY_READ|KEY_WOW64_64KEY,&key) != ERROR_SUCCESS)
		return false;

	bool flag = false;
	DWORD dataType = REG_SZ;
	DWORD size;
	if(RegQueryValueExW(key,name,0,&dataType,NULL,&size) == ERROR_SUCCESS)
	{
		LPBYTE data = NEW BYTE[size];
		RegQueryValueExW(key,name,0,&dataType,data,&size);
		value = (LPCWSTR)data;
		flag = true;
	}

	RegCloseKey(key);
	return flag;
}
bool Registy::writeKey(LPCWSTR subKey,LPCWSTR name,LPCWSTR value)
{
	HKEY key;
	if(RegOpenKeyExW(m_key,subKey,0,KEY_WRITE|KEY_READ|KEY_WOW64_64KEY,&key) != ERROR_SUCCESS)
		return false;

	bool flag = false;;
	if(RegSetValueExW(key,name,0,REG_SZ,(LPCBYTE)value,(DWORD)wcslen(value)*2) == ERROR_SUCCESS)
		flag = true;
	RegCloseKey(key);
	return flag;
}
bool Registy::writeKey(LPCWSTR subKey,LPCWSTR name,DWORD value)
{
	HKEY key;
	if(RegOpenKeyExW(m_key,subKey,0,KEY_WRITE|KEY_READ|KEY_WOW64_64KEY,&key) != ERROR_SUCCESS)
		return false;

	bool flag = false;;
	if(RegSetValueExW(key,name,0,REG_DWORD,(LPCBYTE)&value,sizeof(value)) == ERROR_SUCCESS)
		flag = true;
	RegCloseKey(key);
	return flag;
}

//namespace
};};

