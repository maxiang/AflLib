#pragma once

#include <string>
#include <map>
#include <list>
#include <strmif.h>
#include "aflStd.h"


//namespace AFL::Windows
namespace AFL{namespace WINDOWS{
bool deleteDirectory(LPCWSTR path);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Stdout
// 標準出力横流し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Stdout
{
public:
	struct Message
	{
		LPVOID data;
		DWORD size;
	};
	Stdout();
	~Stdout();
	void setCallback(ClassProc& proc);
protected:
	virtual void onThread(LPVOID data);

	FILE m_oldStdout;
	HANDLE m_readHandle;
	HANDLE m_writeHandle;
	Thread m_thread;
	ClassProc m_classProc;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// FRect
// 矩形サイズ小数点
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct FRect
{
	FLOAT top,left,bottom,right;
	FLOAT getWidth()const;
	FLOAT getHeight()const;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Event
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Event
{
public:
	Event();
	~Event();
	void release();
	void setEvent();
	HANDLE getHandle()const;
	void wait()const;
protected:
	HANDLE m_hEvent;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Semaphore
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Semaphore
{
public:
	Semaphore();
	~Semaphore();
	void clear();
	bool release(LONG lCount=1);
	HANDLE getHandle()const;
	void wait()const;

protected:
	HANDLE m_hEvent;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mutex
// 同期制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Mutex
{
public:
	Mutex();
	~Mutex();
	bool create(LPCSTR name);
protected:
	HANDLE m_mutex;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflAdviseTimer
// 周期呼び出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class AdviseTimer
{
public:
	AdviseTimer();
	~AdviseTimer();
	bool startTimer(DWORD dwTime);
	bool stopTimer();
	void waitTimer();
	bool release(LONG lCount=1);
	LONGLONG getTime();
protected:
	struct IReferenceClock* m_pReferenceClock;
	DWORD_PTR m_dwAdviseToken;
	Semaphore m_semaphore;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflBenchCounter
// ベンチマーク用カウンタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class BenchCounter
{
public:
	BenchCounter();
	~BenchCounter();
	void clear();
	UINT get() const;
protected:
	UINT m_count;
	INT m_period;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PathName
// ファイル名とパス制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PathName
{
public:
	PathName(){}
	PathName(LPCSTR pFileName){setFileName(pFileName);}
	PathName(LPCWSTR pFileName){setFileName(pFileName);}
	bool setFileName(LPCSTR pFileName);
	bool setFileName(LPCWSTR pFileName);
	bool pushPath();
	bool popPath();
	bool changePath();
	std::string relativePath(LPCSTR pFileName);
	std::wstring relativePath(LPCWSTR pFileName);

	LPCSTR getFileName()const;
	LPCSTR getPathName()const;
	LPCSTR getSFileName()const;

	LPCWSTR getFileNameW()const;
	LPCWSTR getPathNameW()const;
	LPCWSTR getSFileNameW()const;

	void getPathDir(String& dest);
	void getPathDir(WString& dest);
protected:
	WString m_strFullPath;
	WString m_strDrive;
	WString m_strDir;
	WString m_strFname;
	WString m_strExt;
	WString m_strPath;
	WString m_strFileName;

	std::wstring m_strTempPath;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Rect3D
// RECT 3Dバージョン
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Rect3D : public RECT
{
public:
	operator LPRECT(){return this;}
	Rect3D();
	Rect3D(int nLeft,int nTop,int nHigh,int nRight,int nBottom,int nLow);
	Rect3D(int nLeft,int nTop,int nRight,int nBottom);
	void setRect(int nLeft,int nTop,int nRight,int nBottom);
	void setRect(int nLeft,int nTop,int nHigh,int nRight,int nBottom,int nLow);
	void offsetRect(int nX,int nY,int nZ=0);
	bool isRectNull() const;
	int getWidth(){return right - left;}
	int getHeight(){return bottom - top;}
	int high,low;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Font
// フォント系
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Font
{
public:
	Font();
	~Font();
	operator HFONT();

	void setSize(int nWidth,int nHeight);
	void setSize(int nHeight);
	int getSize();
	void setBold(int iBold);
	INT getBold()const;
	INT getItalic()const;
	void setUnderline(bool flag);
	void setItalic(INT value);
	void setFontName(LPCWSTR pString);
	bool createFont(LPLOGFONTW logFont=NULL);
	void deleteFont();
	bool getFontSize(LPSIZE pSize,LPCSTR pText,INT iLength=-1,INT iLimitWidth=0,bool mline=true);
	bool getFontSize(LPSIZE pSize,LPCWSTR pText,INT iLength=-1,INT iLimitWidth=0,bool mline=true);
	static bool getFontSize(LPSIZE pSize,HFONT hFont,LPCSTR pText,INT iLength=-1,INT iLimitWidth=0,bool mline=true);
	static bool getFontSize(LPSIZE pSize,HFONT hFont,LPCWSTR pText,INT iLength=-1,INT iLimitWidth=0,bool mline=true);
	LPCWSTR getFontName()const;
	LPLOGFONTW getLogFont();
protected:
	HFONT m_hFont;
	LOGFONTW m_logFont;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fep
// FEP制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Fep
{
public:
	Fep();
	~Fep();
	bool release();
	bool setWindow(HWND hWnd);
	bool isOpen() const;
	bool getStatus(LPDWORD pdwConversion,LPDWORD pdwSentence) const;
	bool getString(std::string& strBuff) const;
	bool getString(std::wstring& strBuff) const;
	INT getConvertPos() const;
	INT getCursorPos() const;
	INT getCountCandidate();
	INT getCandidateIndex() const;
	LPCSTR getCandidateString(INT iIndex) const;
	INT getCandidateStart() const;
	INT getCandidateEnd() const;
	bool setOpenStatus(bool bFlag);
	HWND getWindow()const;
protected:
	HWND m_hWnd;
	LPCANDIDATELIST m_pCandidateList;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ClipBoard
// クリップボード制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class ClipBoard
{
public:
	static bool setString(LPCSTR string);
	static bool setString(LPCWSTR string);
	static bool getString(WString& string);

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TextArea
// テキストサイズ計算用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TextArea
{
public:
	struct AREA
	{
		INT point;
		DWORD length;
	};
	TextArea();
	void addText(LPCSTR text);
	void addText(LPCWSTR src);
	void addChar(WCHAR text);
	void setText(LPCWSTR text);
	void insertText(INT point,LPCWSTR text);
	void insertText(INT point,WCHAR text);
	void getPoint(LPPOINT pos,INT point);
	INT getPoint(INT x,INT y);
	INT getCharWidth(HDC hdc,WCHAR text);
	void compute(INT areaWidth);
	void compute(INT areaWidth,INT point);
	INT getAreaHeight() const;
	LPCWSTR getString() const;
	HFONT getFont();
	INT getFontHeight();
	INT getLength()const;
	void eraseText(INT point,INT size);
	void setMask(bool flag);
	bool isMask() const;
	std::list<AREA>& getArea();
protected:
	WINDOWS::Font m_font;
	std::list<AREA> m_areaList;
	WString m_text;
	INT m_width;
	INT m_height;
	bool m_mask;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Variant
// VARIANT型管理用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Variant : public VARIANT
{
public:
	Variant& operator=(const Variant& value);
	Variant();
	Variant(VARIANT value);
	Variant(LPCWSTR value);
	Variant(UINT value);
	Variant(INT value);
	~Variant();
	INT toInt() const;
	LPCWSTR toString();
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AXObject
// ActiveXオブジェクトパーサ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class AXObject
{
public:
	AXObject();
	AXObject(Variant& value);
	AXObject(IDispatch* disp);
	AXObject(LPCWSTR name);
	AXObject& operator=(IDispatch* disp);
	~AXObject();
	Variant invoke(OLECHAR* name, int nArgs=0, ...);
	Variant get(OLECHAR* name, int nArgs=0, ...);
	operator IDispatch*();

	void p();
protected:
	IDispatch* m_disp;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Registy
// Registy操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Registy
{
public:
	Registy();
	~Registy();
	bool open(LPCWSTR compName,HKEY key=HKEY_LOCAL_MACHINE);
	bool close();
	bool readKey(LPCWSTR subKey,LPCWSTR name,WString& value);
	bool readKey(LPCWSTR subKey,LPCWSTR name,DWORD& value);
	bool writeKey(LPCWSTR subKey,LPCWSTR name,LPCWSTR value);
	bool writeKey(LPCWSTR subKey,LPCWSTR name,DWORD value);

protected:
	HKEY m_key;
};

//namespace
};};


