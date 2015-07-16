#pragma once

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <commctrl.h>
#include <tchar.h>
#include "aflStd.h"
#include "aflWinTool.h"
#if defined(_WIN64)
	typedef LONG_PTR D_PTR;
#else
	typedef LONG D_PTR;
#endif
#if defined(_WIN32_WCE)
	#define SetClassLongPtr SetClassLong
	#define SetWindowLongPtr SetWindowLong
	#define GetWindowLongPtr GetWindowLong
#endif

namespace AFL{namespace WINDOWS{

enum WMAPP
{
	WM_APP_SETCURSOR = WM_APP+200,
	WM_APP_CREATE,
	WM_APP_DESTROY
};
class Window;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Message
// ウインドウメッセージ引き渡し用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Message
{
public:
	Message(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam);
	HWND getWnd()const;
	UINT getMsg()const;
	WPARAM getWParam()const;
	LPARAM getLParam()const;
	WORD getWParamLow()const;
	WORD getWParamHi()const;
	WORD getLParamLow()const;
	WORD getLParamHi()const;
	LRESULT getResult()const;
	void setWParam(WPARAM wParam);
	void setLParam(LPARAM lParam);
	void setResult(LRESULT result);
	void setDefault(bool flag);
	bool isDefaultProc()const;
	Window* getWindow()const;
	INT getX()const;
	INT getY()const;
	DWORD getChar() const;

protected:
	HWND m_hwnd;
	UINT m_msg;
	WPARAM m_wParam;
	LPARAM m_lParam;
	LRESULT m_result;
	bool m_default;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Hook
// ウインドウイベントフッククラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct HookMessage
{
	INT code;
	WPARAM wp;
	LPARAM lp;
};
class HookBase
{
protected:
	friend class Hook;
	HookBase();
	~HookBase();
	void addHook(ClassProc& classProc);
	void delHook(ClassProc& classProc);
	static LRESULT CALLBACK hookProc(int nCode, WPARAM wp, LPARAM lp);
	static HHOOK m_hHook;
	static std::list<ClassProc> m_classProc;
};
class Hook
{
public:
	static void addHook(ClassProc& classProc);
	static void delHook(ClassProc& classProc);
protected:
	static HookBase m_hookBase;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowManage
// ウインドウ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class WindowManage
{
friend class Window;

public:
	~WindowManage();
protected:
	WindowManage();
	void init();

	void addWindow(Window* window);
	void delWindow(Window* window);
	void setHandle(Window* window);
	DWORD message(bool wait) const;
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	Window* getWindow(HWND hwnd);

	//HWND createWindow(LPCREATESTRUCTW);
	void destroyWindow(HWND hwnd);
	void onCreate(Message* m);
	void onDestory(Message* m);

	INT getWindowCount(DWORD threadID=0) const;


	ATOM m_atom;
	std::set<Window*> m_windowList;
	std::map<HWND,Window*> m_windowMap;
	Window* m_windowMaker;
	Critical m_critical;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Window
// ウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-





class Window
{
friend class WindowManage;
public:
	enum CHILD_STYLE
	{
		CHILD_AUTO,
		CHILD_NORMAL,
		CHILD_CLIENT,
		CHILD_TOP,
		CHILD_BOTTOM,
		CHILD_LEFT,
		CHILD_RIGHT,
		CHILD_CENTER,
		CHILD_EQUARL_VERT, //縦割り
		CHILD_EQUARL_HORIZ //横割り
	};

	operator HWND();
	HWND getWnd()const;

	Window();
	virtual ~Window();
	static void init();
	void setClassName(LPCSTR name);
	void setClassName(LPCWSTR name);
	bool createWindowA(LPCSTR className=NULL,LPCSTR windowTitle=NULL,DWORD style=0,HWND parentWnd=NULL,UINT id=0);
	bool createWindow(LPCWSTR className=NULL,LPCWSTR windowTitle=NULL,DWORD style=0,HWND parentWnd=NULL,UINT id=0);
	bool createWindow(HWND hwnd);

	bool closeWindow();
	void setNextFocus(HWND hwnd);
	HWND getNextFocus()const;
	void dragAcceptFiles(bool flag=true);

	LONG_PTR getClassLong(INT index)const;
	LONG_PTR setClassLong(INT index,LONG_PTR value)const;
	LONG_PTR getWindowLong(INT index)const;
	LONG_PTR setWindowLong(INT index,LONG_PTR value)const;
	void setBackColor(COLORREF color);
	COLORREF getBackColor()const;

	virtual bool createChildWindow(HWND hwnd=NULL);
	bool addChild(Window* window,CHILD_STYLE style=CHILD_AUTO);

	bool isWindow()const;
	bool showWindow(int cmdShow=SW_SHOW);
	HWND setActive();
	HCURSOR setCursor(HCURSOR hCursor);
	bool postMessage(UINT msg,WPARAM wParam=0,LPARAM lParam=0);
	LRESULT sendMessage(UINT msg,WPARAM wParam=0,LPARAM lParam=0) const;
	LRESULT sendMasterMessage(UINT msg,WPARAM wParam=0,LPARAM lParam=0) const;
	bool clientToScreen(LPPOINT point);
	bool setWindowPos(HWND hwndAfter=NULL,int x=0,int y=0,int cx=0, int cy=0,UINT flag=SWP_NOMOVE|SWP_NOSIZE);

	bool setScrollRange(INT bar,INT value,bool flag=true) const
	{
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.nMax = value;
		info.nMin = 0;
		info.nPage = getClientHeight();
		info.nPos = 0;
		info.nTrackPos = 0;
		info.fMask = SIF_PAGE | SIF_RANGE;

		return SetScrollInfo(getWnd(),bar,&info,flag)!=false; 
	}
	void setScrollPos(INT bar,INT pos,bool flag=true)
	{
		if(pos == -1)
		{
			SCROLLINFO info;
			info.fMask = SIF_PAGE | SIF_RANGE;
			info.cbSize = sizeof(SCROLLINFO);
			getScrollInfo(bar,&info);
			pos = info.nMax - info.nPage;
		}
		SetScrollPos(getWnd(),bar,pos,flag);
	}
	INT getScrollPos(INT bar) const
	{
		return GetScrollPos(getWnd(),bar);
	}
	bool getScrollInfo(INT bar,LPSCROLLINFO info) const
	{
		info->cbSize = sizeof(SCROLLINFO);
		info->fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		return GetScrollInfo(getWnd(),bar,info) != FALSE;
	}
	bool setScrollInfo(INT bar,LPSCROLLINFO info,bool flag=true) const
	{
		return SetScrollInfo(getWnd(),bar,info,flag) != FALSE;
	}

	bool getWindowRect(LPRECT rect) const;
	bool getClientRect(LPRECT rect) const;
	bool moveWindow(int x,int y,int width,int height,bool repaint=true);
	void setClientSize(int width,int height,bool repaint=true);
	static DWORD message(bool wait=true);
	HWND getParent()const;
	HWND getOwner()const;

	void setParent(HWND hwnd);
	void setQuit(bool flag);
	void addMasterMessage(UINT message,ClassProc& classProc,INT priority=0);
	void addMessage(UINT message,ClassProc& classProc,INT priority=0);
	void addNotify(UINT message,ClassProc& classProc,INT priority=0);
	void addEvent(UINT message,ClassProc& classProc,INT priority=0);
	void setChildStyle(CHILD_STYLE style);

	void setChildPriority(INT value);
	CHILD_STYLE getChildStyle()const;
	INT getChildPriority()const;
	DWORD getStyle()const;
	DWORD getStyleEx()const;
	void setPos(INT x,INT y);
	INT getClientWidth() const;
	INT getClientHeight() const;
	INT getWindowWidth() const;
	INT getWindowHeight() const;
	void setWindowWidth(INT width);
	void setWindowHeight(INT height);
	void setWindowSize(INT width,INT height);
	void setStyle(DWORD style);
	void setStyleEx(DWORD style);

	HDC getDC() const;
	INT releaseDC(HDC hdc) const;

	INT fillRect(const RECT* rect,COLORREF color) const;
	void setTitle(LPCSTR text);
	void setTitle(LPCWSTR text);
	void getTitle(String& dest);
	void getTitle(WString& dest);
	void enableWindow(bool flag);
	bool isWindowEnabled() const;
	virtual void recalcLayout(RECT& rect);
	void recalcLayout();
	bool setLayeredWindowAttributes(COLORREF crKey,BYTE bAlpha,DWORD dwFlags);
	bool isWindowVisible() const;
	bool isIconic() const;
	bool isZoomed() const;
	bool setForeground();
	bool updateWindow()const;
	static bool callDefaultProc(Message* message);

	void setFocus()const;
	INT getPosX()const;
	INT getPosY()const;
	void setPosX(INT value);
	void setPosY(INT value);
	bool doModal(HWND hwnd=NULL);
	void setModalResult(bool value);
	void setMargin(INT left,INT right,INT top,INT bottom);
	void setPadding(INT left,INT right,INT top,INT bottom);

	static Window* getWindow(HWND hwnd);

	void setFullScreen(bool flag=true);
	bool isFullScreen() const;
	bool invalidate();

	INT printf(LPCSTR format, ...);
	INT printf(LPCWSTR format, ...);

	DWORD getThreadID()const;
	void setScrollVertLine(DWORD value)
	{
		m_scrollVertLine = value;
	}
	void setScrollHorzLine(DWORD value)
	{
		m_scrollHorzLine = value;
	}
protected:
	virtual void onWindowProc(Message* message);
	virtual void onCustomProc(Message* message);
	virtual void onDefaultProc(Message* message);

	void _addChild(HWND hwnd);
	void _delChild(HWND hwnd);
	void _addChild(Window* window);
	void _delChild(Window* window);

	bool _createWindow(LPCREATESTRUCTW cs);
	HWND m_hwnd;
	WString m_windowName;
	WString m_windowTitle;
	WString m_className;
	DWORD m_defaultStyle;
	DWORD m_defaultStyleEx;
	SIZE m_windowSize;
	POINT m_windowPosition;
	WNDPROC m_defaultProc;
	HWND m_defaultParent;
	COLORREF m_backColor;
	bool m_quit;
	bool m_modalResult;
	RECT m_keepRect;
	DWORD m_keepStyle;
	DWORD m_keepStyleEx;
	bool m_fullScreen;


	std::map<UINT,std::multimap<INT,ClassProc> > m_messageProc;
	std::map<UINT,std::multimap<INT,ClassProc> > m_notifyProc;
	std::map<UINT,std::multimap<INT,ClassProc> > m_eventProc;

	std::set<Window*> m_childWindow;

	CHILD_STYLE m_childStyle;
	INT m_childPriority;
	HWND m_hwndNextFocus;
	RECT m_marginRect;
	RECT m_paddingRect;
	bool m_create;
	HCURSOR m_cursor;
	DWORD m_threadID;
	static WindowManage g_manage;
	DWORD m_scrollVertLine;
	DWORD m_scrollHorzLine;
};


}}
#include "aflWindowCtrl.h"


