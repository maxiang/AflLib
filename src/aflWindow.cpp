#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <algorithm>
#include "aflWindow.h"

//----------------------------------------------------
//メモリリークテスト用
#if _MSC_VER && !defined(_WIN32_WCE) && _DEBUG
	//メモリリークテスト
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
#endif
//----------------------------------------------------

#ifdef _WIN64

	#undef GWL_WNDPROC
	#undef GWL_HINSTANCE
	#undef GWL_HWNDPARENT
	#undef GWL_USERDATA
	#define	GWL_USERDATA GWLP_USERDATA
	#define	GWL_WNDPROC  GWLP_WNDPROC
	#define	GWL_HWNDPARENT  GWLP_HWNDPARENT
#endif

#ifdef _WIN32_WCE
	#define GetWindowLongPtrW GetWindowLong
	#define SetWindowLongPtrW SetWindowLong
	#define GetClassLongPtrW GetClassLong
	#define SetClassLongPtrW SetClassLong
	#define SWP_NOCOPYBITS 0
#endif
namespace AFL{namespace WINDOWS{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Message
// ウインドウメッセージ引き渡し用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// Message::Message(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam)
// -----  動作  -----
// ウインドウメッセージ　コンストラクタ
// -----  引数  -----
// hwnd   ウインドウハンドル
// msg    メッセージ
// wParam Wパラメータ
// lParam Lパラメータ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
Message::Message(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_hwnd = hwnd;
	m_msg = msg;
	m_wParam = wParam;
	m_lParam = lParam;
	m_result = 0;
	m_default = true;
}
//-----------------------------------------------
// HWND Message::getWnd() const
// -----  動作  -----
// ウインドウハンドルを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウハンドル
//-----------------------------------------------
HWND Message::getWnd() const
{
	return m_hwnd;
}
//-----------------------------------------------
// UINT Message::getMsg() const
// -----  動作  -----
// ウインドウメッセージを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウメッセージ
//-----------------------------------------------
UINT Message::getMsg() const
{
	return m_msg;
}
//-----------------------------------------------
// WPARAM Message::getWParam() const
// -----  動作  -----
// Wパラメータを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Wパラメータ
//-----------------------------------------------
WPARAM Message::getWParam() const
{
	return m_wParam;
}
//-----------------------------------------------
// LPARAM Message::getLParam() const
// -----  動作  -----
// Lパラメータを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Lパラメータ
//-----------------------------------------------
LPARAM Message::getLParam() const
{
	return m_lParam;
}
//-----------------------------------------------
// WORD Message::getWParamLow() const
// -----  動作  -----
// Wパラメータの下位バイトを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Wパラメータの下位バイト
//-----------------------------------------------
WORD Message::getWParamLow() const
{
	return LOWORD(m_wParam);
}
//-----------------------------------------------
// WORD Message::getWParamHi() const
// -----  動作  -----
// Wパラメータの上位バイトを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Wパラメータの上位バイト
//-----------------------------------------------
WORD Message::getWParamHi() const
{
	return HIWORD(m_wParam);
}
//-----------------------------------------------
// WORD Message::getLParamLow() const
// -----  動作  -----
// Lパラメータの下位バイトを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Lパラメータの下位バイト
//-----------------------------------------------
WORD Message::getLParamLow() const
{
	return LOWORD(m_lParam);
}
//-----------------------------------------------
// WORD Message::getLParamHi() const
// -----  動作  -----
// Lパラメータの上位バイトを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Lパラメータの上位バイト
//-----------------------------------------------
WORD Message::getLParamHi() const
{
	return HIWORD(m_lParam);
}
//-----------------------------------------------
// LRESULT Message::getResult() const
// -----  動作  -----
// 戻り値を返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 戻り値
//-----------------------------------------------
LRESULT Message::getResult() const
{
	return m_result;
}
//-----------------------------------------------
// void Message::setWParam(WPARAM wParam)
// -----  動作  -----
// Wパラメータの設定
// -----  引数  -----
// wParam Wパラメータ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Message::setWParam(WPARAM wParam)
{
	m_wParam = wParam;
}
//-----------------------------------------------
// void Message::setLParam(LPARAM lParam)
// -----  動作  -----
// Lパラメータの設定
// -----  引数  -----
// lParam Lパラメータ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Message::setLParam(LPARAM lParam)
{
	m_lParam = lParam;
}
//-----------------------------------------------
// void Message::setResult(LRESULT result)
// -----  動作  -----
// 戻り値の設定
// -----  引数  -----
// result 戻り値
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Message::setResult(LRESULT result)
{
	m_result = result;
}
//-----------------------------------------------
// void Message::setDefault(bool flag)
// -----  動作  -----
// デフォルトプロシージャを呼び出すか設定
// -----  引数  -----
// flag true:呼び出す false:呼び出さない
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Message::setDefault(bool flag)
{
	m_default = flag;
}
//-----------------------------------------------
// bool Message::isDefaultProc() const
// -----  動作  -----
// デフォルトプロシージャを呼び出すか返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:呼び出す false:呼び出さない
//-----------------------------------------------
bool Message::isDefaultProc() const
{
	return m_default;
}
//-----------------------------------------------
// Window* Message::getWindow() const
// -----  動作  -----
// ウインドウクラスのポインタを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウクラスのポインタ
//-----------------------------------------------
Window* Message::getWindow() const
{
	return Window::getWindow(m_hwnd);
}
//-----------------------------------------------
// INT Message::getX() const
// -----  動作  -----
// パラメータからX座標を返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// X座標
//-----------------------------------------------
INT Message::getX() const
{
	return GET_X_LPARAM(m_lParam);
}
//-----------------------------------------------
// DWORD Message::getChar() const
// -----  動作  -----
// パラメータからキャラコードを取り出す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// X座標
//-----------------------------------------------
DWORD Message::getChar() const
{
	return (DWORD)m_wParam;
}
//-----------------------------------------------
// INT Message::getY() const
// -----  動作  -----
// パラメータからY座標を返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Y座標
//-----------------------------------------------
INT Message::getY() const
{
	return GET_Y_LPARAM (m_lParam);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Hook
// ウインドウイベントフッククラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
HookBase::HookBase()
{
	HINSTANCE hInst = GetModuleHandle(NULL) ;
	m_hHook = SetWindowsHookEx(WH_MOUSE ,hookProc,hInst,GetCurrentThreadId());
}
HookBase::~HookBase()
{
	UnhookWindowsHookEx(m_hHook);

}
void HookBase::addHook(ClassProc& classProc)
{
	m_classProc.push_back(classProc);
}
void HookBase::delHook(ClassProc& classProc)
{
	m_classProc.remove(classProc);
}

LRESULT CALLBACK HookBase::hookProc(int nCode, WPARAM wp, LPARAM lp)
{
	if(nCode < 0)
		return CallNextHookEx(m_hHook, nCode, wp, lp); 

	HookMessage hm = {nCode,wp,lp};
	std::list<ClassProc>::iterator it;
	for(it = m_classProc.begin();it != m_classProc.end();++it)
	{
		it->call(&hm);
	}
	return CallNextHookEx(m_hHook, nCode, wp, lp); 
}
HHOOK HookBase::m_hHook;
std::list<ClassProc> HookBase::m_classProc;
HookBase Hook::m_hookBase;
void Hook::addHook(ClassProc& classProc)
{
	m_hookBase.addHook(classProc);
}
void Hook::delHook(ClassProc& classProc)
{
	m_hookBase.delHook(classProc);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowManage
// ウインドウ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
static const WCHAR DEFAULT_CLASS_NAME[] = L"AFL WINDOW CLASS";
//-----------------------------------------------
// WindowManage::WindowManage()
// -----  動作  -----
// ウインドウ管理クラスコンストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
WindowManage::WindowManage()
{
	#if!defined(_WIN32_WCE)
		//ウインドウクラスの生成
		WNDCLASSEXW classWnd;
		classWnd.cbSize			= sizeof(WNDCLASSEXW);
		classWnd.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS; 
		classWnd.lpfnWndProc	= WindowManage::windowProc; 
		classWnd.hInstance		= GetModuleHandle(NULL);
		classWnd.cbClsExtra		= 0;
		classWnd.cbWndExtra		= 0;
		classWnd.hIcon			= LoadIcon(0,IDI_APPLICATION); 
		classWnd.hCursor		= LoadCursor( NULL, IDC_ARROW ); 
		classWnd.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		classWnd.lpszMenuName	= NULL; 
		classWnd.lpszClassName	= DEFAULT_CLASS_NAME; 
		classWnd.hIconSm		= NULL;
		m_atom = RegisterClassExW(&classWnd);
	#else
		WNDCLASSW classWnd;
		classWnd.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS; 
		classWnd.lpfnWndProc	= WindowManage::windowProc; 
		classWnd.hInstance		= GetModuleHandle(NULL);
		classWnd.cbClsExtra		= 0;
		classWnd.cbWndExtra		= 0;
		classWnd.hIcon			= NULL;//LoadIcon(0,IDI_APPLICATION); 
		classWnd.hCursor		= LoadCursor( NULL, IDC_ARROW ); 
		classWnd.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		classWnd.lpszMenuName	= NULL; 
		classWnd.lpszClassName	= DEFAULT_CLASS_NAME; 
		m_atom = RegisterClassW(&classWnd);
	#endif
}
void WindowManage::init()
{
	Window* windowMaker = NEW Window();
	windowMaker->createWindow();
	windowMaker->addMessage(WM_APP_CREATE,CLASSPROC(this,WindowManage,onCreate));
	windowMaker->addMessage(WM_APP_DESTROY,CLASSPROC(this,WindowManage,onDestory));
	m_windowMaker = windowMaker;
}
void WindowManage::destroyWindow(HWND hwnd)
{
	if(m_windowMaker && m_windowMaker->getWnd() != hwnd)
		m_windowMaker->sendMessage(WM_APP_DESTROY,(WPARAM)hwnd);
	else
		DestroyWindow(hwnd);
}

void WindowManage::onCreate(Message* m)
{
	LPCREATESTRUCTW cs = (LPCREATESTRUCTW)m->getWParam();
	HWND hwnd = ::CreateWindowExW(cs->dwExStyle,cs->lpszClass,cs->lpszName,
		cs->style,cs->x,cs->y,cs->cx,cs->cy,cs->hwndParent,cs->hMenu,cs->hInstance,cs->lpCreateParams);
	m->setResult((LRESULT)hwnd);
	m->setDefault(false);
}
void WindowManage::onDestory(Message* m)
{
	DestroyWindow((HWND)m->getWParam());
	//m->setResult((LRESULT)hwnd);
	m->setDefault(false);
}
INT WindowManage::getWindowCount(DWORD threadID) const
{
	INT count = 0;
	if(threadID==0)
		threadID = GetCurrentThreadId();
	std::map<HWND,Window*>::const_iterator it;
	for(it=m_windowMap.begin();it!=m_windowMap.end();++it)
	{
		if(it->second && it->second->getThreadID() == threadID)
			count++;
	}
	return count;
}
//-----------------------------------------------
// WindowManage::~WindowManage()
// -----  動作  -----
// ウインドウ管理クラスデストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
WindowManage::~WindowManage()
{
	if(m_windowMaker)
		delete m_windowMaker;
	UnregisterClassW(DEFAULT_CLASS_NAME,GetModuleHandle(NULL));
}
//-----------------------------------------------
// void WindowManage::addWindow(Window* window)
// -----  動作  -----
// 管理リストにウインドウを追加
// -----  引数  -----
// window ウインドウクラスのポインタ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WindowManage::addWindow(Window* window)
{
	m_critical.lock();
	m_windowList.insert(window);
	m_critical.unlock();
}
//-----------------------------------------------
// void WindowManage::delWindow(Window* window)
// -----  動作  -----
// 管理リストからウインドウを削除
// -----  引数  -----
// window ウインドウクラスのポインタ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WindowManage::delWindow(Window* window)
{
	if(this)
	{
		m_critical.lock();
		m_windowList.erase(window);
		m_critical.unlock();
	}
}
//-----------------------------------------------
// void  WindowManage::setHandle(Window* window)
// -----  動作  -----
// ウインドウハンドルを設定
// -----  引数  -----
// window ウインドウクラスのポインタ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void  WindowManage::setHandle(Window* window)
{
	if(!window)
		return;
	m_critical.lock();

	//既存データの削除
	std::map<HWND,Window*>::iterator it;
	for(it=m_windowMap.begin();it!=m_windowMap.end();++it)
	{
		if(it->second == window)
		{
			m_windowMap.erase(it);
			break;
		}
	}

	HWND hwnd = window->getWnd();
	if(hwnd)
		m_windowMap[hwnd] = window;
	m_critical.unlock();
}
//-----------------------------------------------
// Window* WindowManage::getWindow(HWND hwnd)
// -----  動作  -----
// ウインドウハンドルからウインドウクラスのポインタを返す
// -----  引数  -----
// hwnd ウインドウハンドル
// ----- 戻り値 -----
// ウインドウクラスのポインタ
//-----------------------------------------------
Window* WindowManage::getWindow(HWND hwnd)
{
	if(hwnd == NULL)
		return m_windowMaker;
	std::map<HWND,Window*>::iterator it;
	it = m_windowMap.find(hwnd);
	if(it == m_windowMap.end())
		return NULL;
	return it->second;
}
//-----------------------------------------------
// LRESULT CALLBACK WindowManage::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// -----  動作  -----
// ウインドウプロシージャ
// -----  引数  -----
// hwnd    ウインドウハンドル
// msg     メッセージ
// wParam  Wパラメータ
// lParam  Lパラメータ
// ----- 戻り値 -----
// 各メッセージの戻り値
//-----------------------------------------------
LRESULT CALLBACK WindowManage::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = NULL;


	#ifndef _WIN32_WCE
	if(msg == WM_NCCREATE)
	#else
	if(msg == WM_CREATE)
	#endif
	{
		//通常ウインドウ
		LPCREATESTRUCTW cs = (LPCREATESTRUCTW)lParam;
		window = (Window*)cs->lpCreateParams;
		if(window)
		{
			window->m_hwnd = hwnd;
			window->setWindowLong(GWL_USERDATA,reinterpret_cast<LONG_PTR>(window));
		}
	}
	else
	{
		//標準コントロール系
		window = (Window*)(LONG_PTR)GetWindowLongPtrW(hwnd,GWL_USERDATA);
	}
	//hWndに対応するクラスのポインタをコールバック
	if(window)
	{
		Message message(hwnd,msg,wParam,lParam);
		window->onWindowProc(&message);
		return message.getResult();
	}
	return DefWindowProcW(hwnd,msg,wParam,lParam);
}
//-----------------------------------------------
// DWORD WindowManage::message(bool wait) const
// -----  動作  -----
// メッセージループ
// -----  引数  -----
// wait true:終了条件を満たすまで待つ false:メッセージが無くなったら抜ける
// ----- 戻り値 -----
// Wパラメータ
//-----------------------------------------------
DWORD WindowManage::message(bool wait) const
{
	if(m_windowMap.size() == 0)
		return 0;
	MSG msg;
	if(wait)
	{
		while(GetMessageW(&msg,NULL,0,0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			if(getWindowCount() == 0 || (m_windowMaker && getWindowCount() == 1))
				break;
		}
	}
	else
	{
		while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if(msg.message == WM_QUIT || getWindowCount() == 0 || (m_windowMaker && getWindowCount() == 1))
			{
				PostQuitMessage((INT)msg.wParam);
				return 0;
			}
		}
		return 1;
	

	}
	return (DWORD)msg.wParam;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Window
// ウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowManage Window::g_manage;

//-----------------------------------------------
// Window::Window()
// -----  動作  -----
// ウインドウクラスのコンストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
Window::Window()
{
	//if(!g_manage.get())
	//	g_manage = NEW WindowManage;
	g_manage.addWindow(this);
	m_hwnd = NULL;
	m_defaultProc = NULL;
	m_defaultStyle = 0;
	m_defaultStyleEx = 0;
	m_className = DEFAULT_CLASS_NAME;
	m_windowSize.cx = CW_USEDEFAULT;
	m_windowSize.cy = CW_USEDEFAULT;
	m_windowPosition.x = CW_USEDEFAULT;
	m_windowPosition.y = CW_USEDEFAULT;
	m_quit = false;
	m_defaultParent = NULL;
	m_childStyle = CHILD_NORMAL;
	m_childPriority = 0;
	m_backColor = -2;
	m_hwndNextFocus = NULL;
	m_fullScreen = false;

	m_marginRect.left = 0;
	m_marginRect.right = 0;
	m_marginRect.top = 0;
	m_marginRect.bottom = 0;
	m_paddingRect.left = 0;
	m_paddingRect.right = 0;
	m_paddingRect.top = 0;
	m_paddingRect.bottom = 0;
	m_create = true;
	m_cursor = NULL;
	m_threadID = 0;

	m_scrollHorzLine = 1;
	m_scrollVertLine = 1;
}
//-----------------------------------------------
// Window::~Window()
// -----  動作  -----
// ウインドウクラスのデストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
Window::~Window()
{
	if(m_create && isWindow())
	{
		g_manage.destroyWindow(getWnd());
	}
	g_manage.delWindow(this);
}
void Window::init()
{
	g_manage.init();
}

//-----------------------------------------------
// bool Window::addChild(Window* window,CHILD_STYLE style)
// -----  動作  -----
// 子ウインドウの追加
// -----  引数  -----
// windows 追加する子ウインドウ
// style   チャイルドスタイル
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::addChild(Window* window,CHILD_STYLE style)
{
	window->setParent(getWnd());
	if(style != CHILD_AUTO)
		window->setChildStyle(style);
	if(!window->isWindow())
		return window->createChildWindow();
	window->setStyle((window->getStyle() & ~WS_POPUP) | WS_CHILD);
	return true;
}
//-----------------------------------------------
// void Window::addEvent(UINT message,ClassProc& classProc,INT priority)
// -----  動作  -----
// Eventのフック
// -----  引数  -----
// message イベント
// classProc コールバックプロシージャ
// priority  優先度(値の低い方が優先)
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::addEvent(UINT message,ClassProc& classProc,INT priority)
{
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
	itMsg = m_eventProc.find(message);
	if(itMsg ==  m_eventProc.end())
		m_eventProc[message].insert(std::pair<INT,ClassProc>(priority,classProc));
	else
		itMsg->second.insert(std::pair<INT,ClassProc>(priority,classProc));
}

//-----------------------------------------------
// Window::operator HWND()
// -----  動作  -----
// インスタンスのオペレータ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウハンドル
//-----------------------------------------------
Window::operator HWND()
{
	return getWnd();
}

//-----------------------------------------------
// HWND Window::getWnd() const
// -----  動作  -----
// ウインドウハンドルを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウハンドル
//-----------------------------------------------
HWND Window::getWnd() const
{
	return m_hwnd;
}
//-----------------------------------------------
// HWND Window::getParent() const
// -----  動作  -----
// 親ウインドウを返す
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 親ウインドウのハンドル
//-----------------------------------------------
HWND Window::getParent() const
{
	if(isWindow())
		return ::GetParent(m_hwnd);
	else
		return m_defaultParent;
}
HWND Window::getOwner() const
{
	if(isWindow())
		return (HWND)getWindowLong(GWL_HWNDPARENT);
	else
		return m_defaultParent;
}

//-----------------------------------------------
// void Window::setQuit(bool flag)
// -----  動作  -----
// ウインドウが消失した場合の処理
// -----  引数  -----
// true:メッセージループ終了 false:何もしない
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setQuit(bool flag)
{
	m_quit = flag;
}
//-----------------------------------------------
// void Window::setChildPriority(INT value)
// -----  動作  -----
// レイアウトの優先順位を設定
// -----  引数  -----
// 優先順位
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setChildPriority(INT value)
{
	m_childPriority = value;
}
//-----------------------------------------------
// Window::CHILD_STYLE Window::getChildStyle() const
// -----  動作  -----
// 子ウインドウ時のスタイルを取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// スタイル
//-----------------------------------------------
Window::CHILD_STYLE Window::getChildStyle() const
{
	return m_childStyle;
}
//-----------------------------------------------
// INT Window::getChildPriority() const
// -----  動作  -----
// ウインドウレイアウト時の優先順位を取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 優先度(低い方が優先)
//-----------------------------------------------
INT Window::getChildPriority() const
{
	return m_childPriority;
}
//-----------------------------------------------
// DWORD Window::getStyle() const
// -----  動作  -----
// ウインドウスタイルの取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウスタイル
//-----------------------------------------------
DWORD Window::getStyle() const
{
	return m_defaultStyle;
}
//-----------------------------------------------
// DWORD Window::getStyleEx() const
// -----  動作  -----
// 拡張ウインドウスタイルの取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 拡張ウインドウスタイル
//-----------------------------------------------
DWORD Window::getStyleEx() const
{
	return m_defaultStyleEx;
}
//-----------------------------------------------
// Window* Window::getWindow(HWND hwnd)
// -----  動作  -----
// ウインドウハンドルからウインドウクラスの取得
// -----  引数  -----
// hwnd ウインドウハンドル
// ----- 戻り値 -----
// ウインドウクラスのポインタ
//-----------------------------------------------
Window* Window::getWindow(HWND hwnd)
{
	return g_manage.getWindow(hwnd);
}
//-----------------------------------------------
// Window* Window::setFullScreen(bool flag)
// -----  動作  -----
// デスクトップ全体にウインドウを展開する
// -----  引数  -----
// flag True:フルスクリーン False:ノーマル
// ----- 戻り値 -----
// ウインドウクラスのポインタ
//-----------------------------------------------
void Window::setFullScreen(bool flag)
{
	if(flag != m_fullScreen)
	{
		m_fullScreen = flag;
		if(flag)
		{
			getWindowRect(&m_keepRect);
			m_keepStyle = getStyle();
			m_keepStyleEx = getStyleEx();
			setStyle(WS_POPUP| WS_CLIPCHILDREN|WS_VISIBLE);
			setStyleEx(WS_EX_TOPMOST);

			INT width = GetSystemMetrics(SM_CXSCREEN);
			INT height = GetSystemMetrics(SM_CYSCREEN);
			setPos(0,0);
			setWindowSize(width,height);
		}
		else
		{
			setStyle(m_keepStyle);
			setStyleEx(m_keepStyleEx);
			setWindowSize(m_keepRect.right-m_keepRect.left,
				m_keepRect.bottom-m_keepRect.top);
			setPos(m_keepRect.left,m_keepRect.top);
			//setWindowPos(NULL,0,0,0,0,SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE);
			showWindow();
			InvalidateRect(NULL,NULL,true);
		}
	}
}
//-----------------------------------------------
// bool Window::isFullScreen() const
// -----  動作  -----
// デスクトップ全体にウインドウを展開しているか
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// True:フルスクリーン False:ノーマル
//-----------------------------------------------
bool Window::isFullScreen() const
{
	return m_fullScreen;
}
//-----------------------------------------------
// bool Window::invalidate()
// -----  動作  -----
// 更新領域をクリアする
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
bool Window::invalidate()
{
	return InvalidateRect(m_hwnd,NULL,true)!=false;
}

//-----------------------------------------------
// void Window::setFocus() const
// -----  動作  -----
// ウインドウにフォーカスを与える
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setFocus() const
{
	SetFocus(getWnd());
}
//-----------------------------------------------
// INT Window::getPosX() const
// -----  動作  -----
// ウインドウのX座標を取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// X座標
//-----------------------------------------------
INT Window::getPosX() const
{
	if(!isWindow())
		return m_windowPosition.x;
	RECT rect;
	getWindowRect(&rect);
	return rect.left;
}
//-----------------------------------------------
// INT Window::getPosY() const
// -----  動作  -----
// ウインドウのY座標を取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// Y座標
//-----------------------------------------------
INT Window::getPosY() const
{
	if(!isWindow())
		return m_windowPosition.y;
	RECT rect;
	getWindowRect(&rect);
	return rect.top;
}
//-----------------------------------------------
// void Window::setPosX(INT value)
// -----  動作  -----
// ウインドウのX座標を設定
// -----  引数  -----
// value X座標
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setPosX(INT value)
{
	setPos(value,getPosY());			
}
//-----------------------------------------------
// void Window::setPosY(INT value)
// -----  動作  -----
// ウインドウのY座標を設定
// -----  引数  -----
// value Y座標
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setPosY(INT value)
{
	setPos(getPosX(),value);			
}
//-----------------------------------------------
// void Window::setModalResult(bool value)
// -----  動作  -----
// モーダル稼働時の戻り値を設定
// -----  引数  -----
// value 戻り値
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setModalResult(bool value)
{
	m_modalResult = value;
}
//-----------------------------------------------
// void Window::setMargin(INT left,INT right,INT top,INT bottom)
// -----  動作  -----
// ウインドウレイアウト時のマージン設定
// -----  引数  -----
// left   左マージン
// right  右マージン
// top    上マージン
// bottom 下マージン
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setMargin(INT left,INT right,INT top,INT bottom)
{
	m_marginRect.left = left;
	m_marginRect.right = right;
	m_marginRect.top = top;
	m_marginRect.bottom = bottom;
}
//-----------------------------------------------
// void Window::setPadding(INT left,INT right,INT top,INT bottom)
// -----  動作  -----
// ウインドウレイアウト時のパディング設定
// -----  引数  -----
// left   左マージン
// right  右マージン
// top    上マージン
// bottom 下マージン
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setPadding(INT left,INT right,INT top,INT bottom)
{
	m_paddingRect.left = left;
	m_paddingRect.right = right;
	m_paddingRect.top = top;
	m_paddingRect.bottom = bottom;
}

//-----------------------------------------------
// void Window::setClassName(LPCSTR name)
// -----  動作  -----
// クラス名の設定
// -----  引数  -----
// name クラス名
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setClassName(LPCSTR name)
{
	m_className = name;
}
//-----------------------------------------------
// void Window::setClassName(LPCWSTR name)
// -----  動作  -----
// クラス名の設定
// -----  引数  -----
// name クラス名
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setClassName(LPCWSTR name)
{
	m_className = name;
}

//-----------------------------------------------
// void Window::dragAcceptFiles(bool flag)
// -----  動作  -----
// ファイルのドラッグドロップの許可
// -----  引数  -----
// flag true:許可 false:不許可
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::dragAcceptFiles(bool flag)
{
	#ifndef _WIN32_WCE
		DragAcceptFiles(getWnd(),flag);
	#endif
}

//-----------------------------------------------
// bool Window::closeWindow()
// -----  動作  -----
// ウインドウを閉じる
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 成功/失敗
//-----------------------------------------------
bool Window::closeWindow()
{
	if(!isWindow())
		return false;
	g_manage.destroyWindow(getWnd());
	m_hwnd = NULL;
	setParent(NULL);
	return true;
}
//-----------------------------------------------
// void Window::setNextFocus(HWND hwnd)
// -----  動作  -----
// TABで切り替えるフォーカスの設定
// -----  引数  -----
// 次のフォーカスを設定するウインドウハンドル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setNextFocus(HWND hwnd)
{
	m_hwndNextFocus = hwnd;
}
//-----------------------------------------------
// HWND Window::getNextFocus() const
// -----  動作  -----
// 次のフォーカスを取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウハンドル
//-----------------------------------------------
HWND Window::getNextFocus() const
{
	return m_hwndNextFocus;
}
//-----------------------------------------------
// LONG_PTR Window::getClassLong(INT index) const
// -----  動作  -----
// ウインドウクラスデータの取得
// -----  引数  -----
// index データインデックス
// ----- 戻り値 -----
// 対応データ
//-----------------------------------------------
LONG_PTR Window::getClassLong(INT index) const
{
	return ::GetClassLongPtrW(m_hwnd,index);
}
//-----------------------------------------------
// LONG_PTR Window::setClassLong(INT index,LONG_PTR value) const
// -----  動作  -----
// ウインドウクラスデータの設定
// -----  引数  -----
// index データインデックス
// value データ
// ----- 戻り値 -----
// 変更時の戻り値
//-----------------------------------------------
LONG_PTR Window::setClassLong(INT index,LONG_PTR value) const
{
	return ::SetClassLongPtrW(m_hwnd,index,(D_PTR)value);
}

//-----------------------------------------------
// LONG_PTR Window::setWindowLong(INT index,LONG_PTR value) const
// -----  動作  -----
// ウインドウデータの設定
// -----  引数  -----
// index データインデックス
// value データ
// ----- 戻り値 -----
// 変更時の戻り値
//-----------------------------------------------
LONG_PTR Window::setWindowLong(INT index,LONG_PTR value) const
{
	return ::SetWindowLongPtrW(m_hwnd,index,(D_PTR)value);
}

//-----------------------------------------------
//LONG_PTR Window::getWindowLong(INT index) const
// -----  動作  -----
// ウインドウデータの取得
// -----  引数  -----
// index データインデックス
// ----- 戻り値 -----
// ウインドウデータ
//-----------------------------------------------
LONG_PTR Window::getWindowLong(INT index) const
{
	return ::GetWindowLongPtrW(m_hwnd,index);
}

//-----------------------------------------------
// void Window::setBackColor(COLORREF color)
// -----  動作  -----
// 背景色の設定
// -----  引数  -----
// color 色
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setBackColor(COLORREF color)
{
	m_backColor = color;
}
//-----------------------------------------------
// COLORREF Window::getBackColor() const
// -----  動作  -----
// 背景色を返す
// -----  引数  -----
// 背景色の取得
// ----- 戻り値 -----
// 色
//-----------------------------------------------
COLORREF Window::getBackColor() const
{
	return m_backColor;
}

//-----------------------------------------------
// HCURSOR Window::setCursor(HCURSOR hCursor) const
// -----  動作  -----
// カーソルの設定
// -----  引数  -----
// hCursor カーソルハンドル
// ----- 戻り値 -----
// 以前のカーソルハンドル
//-----------------------------------------------
HCURSOR Window::setCursor(HCURSOR hCursor)
{
	m_cursor = hCursor;
	return (HCURSOR)sendMessage(WM_APP_SETCURSOR,(WPARAM)hCursor);
	//return SetCursor(hCursor);
}
//-----------------------------------------------
// bool Window::postMessage(UINT msg,WPARAM wParam,LPARAM lParam)
// -----  動作  -----
// メッセージをポストする
// -----  引数  -----
// msg    メッセージ
// wParam Wパラメータ
// lParam Lパラメータ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::postMessage(UINT msg,WPARAM wParam,LPARAM lParam)
{
	return PostMessageW(m_hwnd,msg,wParam,lParam) != 0;
}
//-----------------------------------------------
// LRESULT Window::sendMessage(UINT msg,WPARAM wParam,LPARAM lParam) const
// -----  動作  -----
// メッセージの送信
// -----  引数  -----
// msg    メッセージ
// wParam Wパラメータ
// lParam Lパラメータ
// ----- 戻り値 -----
// 戻り値
//-----------------------------------------------
LRESULT Window::sendMessage(UINT msg,WPARAM wParam,LPARAM lParam) const
{
	return SendMessageW(m_hwnd,msg,wParam,lParam);
}
//-----------------------------------------------
// HWND Window::setActive()
// -----  動作  -----
// ウインドウをアクティブにする
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 以前アクティブだったウインドウ
//-----------------------------------------------
HWND Window::setActive()
{
	return ::SetActiveWindow(m_hwnd);
}

//-----------------------------------------------
// bool Window::clientToScreen(LPPOINT point)
// -----  動作  -----
// クライアント座標をスクリーン座標に変換
// -----  引数  -----
// point クライアント座標
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::clientToScreen(LPPOINT point)
{
	return ClientToScreen(m_hwnd,point)!=0;
}

//-----------------------------------------------
// bool Window::createChildWindow(HWND hwnd)
// -----  動作  -----
// 子ウインドウとして作成
// -----  引数  -----
// hwnd 親ウインドウのハンドル
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::createChildWindow(HWND hwnd)
{
	return createWindow(NULL,NULL,(getStyle()&(~WS_POPUP))|WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPCHILDREN,hwnd);
}


//-----------------------------------------------
// bool Window::createWindow(HWND hwnd)
// -----  動作  -----
// ウインドウの作成
// -----  引数  -----
// hwnd 親ウインドウ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::createWindow(HWND hwnd)
{
	if(!hwnd)
		return false;
	m_create = false;
	m_hwnd = hwnd;
	g_manage.setHandle(this);
	HWND parent = GetParent(hwnd);
	Window* window = getWindow(parent);
	if(window)
	{
		window->_addChild(m_hwnd);
	}

	WNDPROC winproc = (WNDPROC)getWindowLong(GWL_WNDPROC);
	if(winproc != (WNDPROC)WindowManage::windowProc)
	{
		m_defaultProc = winproc;
		setWindowLong(GWL_WNDPROC,(LONG_PTR)WindowManage::windowProc);
		setWindowLong(GWL_USERDATA,reinterpret_cast<LONG_PTR>(this));


		//メッセージメソッドのコールバック
		std::map<UINT,std::multimap<INT,ClassProc> >::iterator itProc;
		itProc = m_messageProc.find(WM_CREATE);
		if(itProc != m_messageProc.end())
		{
			std::multimap<INT,ClassProc>::iterator it;
			for(it=itProc->second.begin();it!=itProc->second.end();++it)
				it->second.call(message);	//ユーザ処理
		}
	}

	if(window)
	{
		window->recalcLayout();
	}

	return true;
}


//-----------------------------------------------
// bool Window::_createWindow(LPCREATESTRUCTW cs)
// -----  動作  -----
// ウインドウの作成
// -----  引数  -----
// cs ウインドウ作成用データ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::_createWindow(LPCREATESTRUCTW cs)
{
	m_defaultProc = NULL;
	if(g_manage.m_windowMaker)
	{
		m_hwnd = (HWND)g_manage.m_windowMaker->sendMessage(WM_APP_CREATE,(LPARAM)cs);
		m_threadID = g_manage.m_windowMaker->getThreadID();
	}	
	else
	{
		m_hwnd = ::CreateWindowExW(cs->dwExStyle,cs->lpszClass,cs->lpszName,
			cs->style,cs->x,cs->y,cs->cx,cs->cy,cs->hwndParent,cs->hMenu,cs->hInstance,cs->lpCreateParams);
		m_threadID = GetCurrentThreadId();
	}

	if(!m_hwnd)
		return false;
	m_create = true;
	g_manage.setHandle(this);

	Window* window = NULL;
	if(cs->hwndParent)
	{
		window = getWindow(cs->hwndParent);
		if(window)
		{
			window->_addChild(m_hwnd);
		}
	}

#if defined(_WIN32_WCE)
	WNDPROC winproc = (WNDPROC)(getWindowLong(GWL_WNDPROC)&0xfffff);
#else
	WNDPROC winproc = (WNDPROC)(getWindowLong(GWL_WNDPROC));
#endif
	if(winproc != (WNDPROC)WindowManage::windowProc)
	{
		m_defaultProc = winproc;
		setWindowLong(GWL_WNDPROC,(LONG_PTR)WindowManage::windowProc);
		setWindowLong(GWL_USERDATA,reinterpret_cast<LONG_PTR>(this));

		//メッセージメソッドのコールバック
		std::map<UINT,std::multimap<INT,ClassProc> >::iterator itProc;
		itProc = m_messageProc.find(WM_CREATE);
		if(itProc != m_messageProc.end())
		{
			std::multimap<INT,ClassProc>::iterator it;
			for(it=itProc->second.begin();it!=itProc->second.end();++it)
				it->second.call(message);	//ユーザ処理
		}
	}

	if(window)
	{
		window->recalcLayout();
	}

	return true;
}
//-----------------------------------------------
// bool Window::createWindow(LPCSTR className,LPCSTR windowTitle,DWORD style,HWND parentWnd,UINT id)
// -----  動作  -----
// ウインドウの作成
// -----  引数  -----
// className   クラス名
// windowTitle タイトル
// style       スタイル
// parentWnd   親ウインドウハンドル
// id          ID
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::createWindowA(LPCSTR className,LPCSTR windowTitle,DWORD style,HWND parentWnd,UINT id)
{
	return createWindow(UCS2(className),UCS2(windowTitle),style,parentWnd,id);
}

//-----------------------------------------------
// bool Window::createWindow(LPCWSTR className,LPCWSTR windowTitle,DWORD style,HWND parentWnd,UINT id)
// -----  動作  -----
// ウインドウの作成
// -----  引数  -----
// className   クラス名
// windowTitle タイトル
// style       スタイル
// parentWnd   親ウインドウハンドル
// id          ID
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::createWindow(LPCWSTR className,LPCWSTR windowTitle,DWORD style,HWND parentWnd,UINT id)
{
	if(isWindow())
		closeWindow();

	CREATESTRUCTW cs;
	ZeroMemory(&cs,sizeof(CREATESTRUCTW));

	//クラスの登録
	if(className)
		m_className = className;

	//ウインドウタイトルの設定
	if(windowTitle)
		m_windowTitle = windowTitle;
	//スタイルの設定
	if(style)
		m_defaultStyle = style;
	else if(m_defaultStyle == 0)
	{
		#ifndef _WIN32_WCE
			m_defaultStyle = WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN;
		#else
			m_defaultStyle = WS_VISIBLE;
		#endif
	}
	cs.style = m_defaultStyle;
	cs.dwExStyle = m_defaultStyleEx;
	cs.lpszClass = m_className.c_str();
	cs.lpszName = m_windowTitle.c_str();
	cs.x = m_windowPosition.x;
	cs.y = m_windowPosition.y;
	cs.cx = m_windowSize.cx;
	cs.cy = m_windowSize.cy;
	if(parentWnd)
		cs.hwndParent = parentWnd;
	else
		cs.hwndParent = m_defaultParent;
	cs.lpCreateParams = (LPVOID)this;
	
	return _createWindow(&cs);
}
//-----------------------------------------------
// bool Window::setWindowPos(HWND hwndAfter,int x,int y,int cx, int cy,UINT flag)
// -----  動作  -----
// ウインドウ位置の設定
// -----  引数  -----
// hwndAfter 前面ウインドウのハンドル
// x         X座標
// y         Y座標
// cx        幅
// cy        高さ
// flag      パラメータ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::setWindowPos(HWND hwndAfter,int x,int y,int cx, int cy,UINT flag)
{
	if(!(flag & SWP_NOMOVE))
	{
		m_windowPosition.x = x;
		m_windowPosition.y = y;
	}
	if(!(flag & SWP_NOSIZE))
	{
		m_windowSize.cx = cx;
		m_windowSize.cy = cy;
	}
	return SetWindowPos(m_hwnd,hwndAfter,x,y,cx,cy,flag) != 0;
}
//-----------------------------------------------
// bool Window::isWindow() const
// -----  動作  -----
// ウインドウがあるか確認
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:ある false:ない
//-----------------------------------------------
bool Window::isWindow() const
{
	if(!m_hwnd || !::IsWindow(m_hwnd))
		return false;
	return true;
}
//-----------------------------------------------
// bool Window::updateWindow() const
// -----  動作  -----
// ウインドウのアップデート
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::updateWindow() const
{
	return UpdateWindow(getWnd()) != false;
}

//-----------------------------------------------
// void Window::setChildStyle(CHILD_STYLE style)
// -----  動作  -----
// 子ウインドウ時のスタイル設定
// -----  引数  -----
// style スタイル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setChildStyle(CHILD_STYLE style)
{
	m_childStyle = style;
	Window* window = getWindow(getParent());
	if(window)
	{
		window->recalcLayout();
	}
}

//-----------------------------------------------
// void Window::setPos(INT x,INT y)
// -----  動作  -----
// 位置の設定
// -----  引数  -----
// X X座標
// Y Y座標
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setPos(INT x,INT y)
{
	m_windowPosition.x = x;
	m_windowPosition.y = y;
	if(getWnd())
	{
		setWindowPos(NULL,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
	}
}
//-----------------------------------------------
// INT Window::getClientWidth() const
// -----  動作  -----
// クライアント領域の幅を取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// クライアント領域の幅
//-----------------------------------------------
INT Window::getClientWidth() const
{
	if(!getWnd())
		return 0;
	RECT rect;
	getClientRect(&rect);
	return rect.right - rect.left;
}
//-----------------------------------------------
// INT Window::getClientHeight() const
// -----  動作  -----
// クライアント領域の高さを取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
//  クライアント領域の高さ
//-----------------------------------------------
INT Window::getClientHeight() const
{
	if(!getWnd())
		return 0;
	RECT rect;
	getClientRect(&rect);
	return rect.bottom - rect.top;
}
//-----------------------------------------------
// INT Window::getWindowWidth() const
// -----  動作  -----
// ウインドウの幅を取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウの幅
//-----------------------------------------------
INT Window::getWindowWidth() const
{
	return m_windowSize.cx;
}
//-----------------------------------------------
// INT Window::getWindowHeight() const
// -----  動作  -----
// ウインドウの高さを取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// ウインドウの高さ
//-----------------------------------------------
INT Window::getWindowHeight() const
{
	return m_windowSize.cy;
}
//-----------------------------------------------
// void Window::setWindowWidth(INT width)
// -----  動作  -----
// ウインドウの幅の設定
// -----  引数  -----
// width 幅
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setWindowWidth(INT width)
{
	setWindowSize(width,getWindowHeight());
}
//-----------------------------------------------
// bool Window::showWindow(int cmdShow)
// -----  動作  -----
// ウインドウの表示
// -----  引数  -----
// cmdShow 表示状態
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::showWindow(int cmdShow)
{
	if(!isWindow())
		createWindow();
	bool flag = ShowWindow(m_hwnd,cmdShow)!=0;
	Window* window = getWindow(getParent());
	if(window)
		window->recalcLayout();
	return flag;
}

//-----------------------------------------------
// void Window::setWindowHeight(INT height)
// -----  動作  -----
// ウインドウの高さ
// -----  引数  -----
// height 高さ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setWindowHeight(INT height)
{
	setWindowSize(getWindowWidth(),height);
}
//-----------------------------------------------
// void Window::setWindowSize(INT width,INT height)
// -----  動作  -----
// ウインドウサイズの設定
// -----  引数  -----
// width 幅
// height 高さ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setWindowSize(INT width,INT height)
{
	m_windowSize.cx = width;
	m_windowSize.cy = height;
	if(getWnd())
		setWindowPos(NULL,0,0,width,height,SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOCOPYBITS);

}
//-----------------------------------------------
// void Window::setStyle(DWORD style)
// -----  動作  -----
// ウインドウスタイルの設定
// -----  引数  -----
// style スタイル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setStyle(DWORD style)
{
	m_defaultStyle = style;
	if(isWindow())
	{
		setWindowLong(GWL_STYLE,style);
		setWindowPos(0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOSIZE|SWP_NOMOVE);
	}
}
//-----------------------------------------------
// void Window::setStyleEx(DWORD style)
// -----  動作  -----
// 拡張ウインドウスタイルの設定
// -----  引数  -----
// style 拡張スタイル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setStyleEx(DWORD style)
{
	m_defaultStyleEx = style;
	if(isWindow())
	{
		setWindowLong(GWL_EXSTYLE,style);
		setWindowPos(0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOSIZE|SWP_NOMOVE);
	}
	
}
//-----------------------------------------------
// void Window::setTitle(LPCSTR text)
// -----  動作  -----
// タイトルの設定
// -----  引数  -----
// text タイトル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setTitle(LPCSTR text)
{
	m_windowTitle = text;
	::SetWindowTextW(getWnd(),m_windowTitle.c_str());
}
//-----------------------------------------------
// void Window::setTitle(LPCWSTR text)
// -----  動作  -----
// タイトルの設定
// -----  引数  -----
// text タイトル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setTitle(LPCWSTR text)
{
	m_windowTitle = text;
	::SetWindowTextW(getWnd(),text);
}

//-----------------------------------------------
// void Window::enableWindow(bool flag)
// -----  動作  -----
// タイトルの取得
// -----  引数  -----
// flag true:有効 false:無効
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::enableWindow(bool flag)
{
	EnableWindow(getWnd(),flag);
}
//-----------------------------------------------
// bool Window::isWindowEnabled() const
// -----  動作  -----
// ウインドウの有効状態の取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:有効 false:無効
//-----------------------------------------------
bool Window::isWindowEnabled() const
{
	return IsWindowEnabled (getWnd()) != 0;
}

//-----------------------------------------------
// void Window::getTitle(String& dest)
// -----  動作  -----
// タイトルの取得
// -----  引数  -----
// dest タイトル格納用文字列クラス
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::getTitle(String& dest)
{
	if(isWindow())
	{
		INT length = GetWindowTextLengthW(getWnd())+1;
		LPWSTR s = new WCHAR[length];
		if(s)
		{
			::GetWindowTextW(getWnd(),s,length);
			m_windowTitle = s;
			delete[] s;
		}
	}
	dest = m_windowTitle;
}
//-----------------------------------------------
// void Window::getTitle(std::wstring& dest)
// -----  動作  -----
// タイトルの取得
// -----  引数  -----
// dest タイトル格納用文字列クラス
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::getTitle(WString& dest)
{
	if(isWindow())
	{
		INT length = GetWindowTextLengthW(getWnd())+1;
		LPWSTR s = new WCHAR[length];
		if(s)
		{
			::GetWindowTextW(getWnd(),s,length);
			m_windowTitle = s;
			delete[] s;
		}
	}
	dest = m_windowTitle;
}
//-----------------------------------------------
// bool Window::setLayeredWindowAttributes(COLORREF crKey,BYTE bAlpha,DWORD dwFlags)
// -----  動作  -----
// レイヤーウインドウモードの設定
// -----  引数  -----
// crKey   カラーキー
// bAlpha  α値
// dwFlags フラグ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::setLayeredWindowAttributes(COLORREF crKey,BYTE bAlpha,DWORD dwFlags)
{
	#if(_WIN32_WINNT > 0x0500)
		return ::SetLayeredWindowAttributes(getWnd(),crKey,bAlpha,dwFlags) != false;
	#else
		return false;
	#endif
}


//-----------------------------------------------
// bool Window::isWindowVisible() const
// -----  動作  -----
// ウインドウの表示状態の取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:表示 false:非表示
//-----------------------------------------------
bool Window::isWindowVisible() const
{
	return IsWindowVisible(m_hwnd) != false;
}
//-----------------------------------------------
// bool Window::isIconic() const
// -----  動作  -----
// アイコン状態か確認
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:アイコン false:通常
//-----------------------------------------------
bool Window::isIconic() const
{
	return IsIconic(m_hwnd) != false;
}
//-----------------------------------------------
// bool Window::isZoomed() const
// -----  動作  -----
// 最大化状態か確認
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:最大 false:通常
//-----------------------------------------------
bool Window::isZoomed() const
{
	return IsZoomed(m_hwnd) != false;
}
//-----------------------------------------------
// bool Window::setForeground()
// -----  動作  -----
// フォアグラウンドに設定
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::setForeground()
{
	#if _WIN32_WCE
		bool flag = ::SetForegroundWindow(getWnd()) != 0;
	#else
		DWORD dwTimeout;
		SystemParametersInfo (SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTimeout, 0);
		SystemParametersInfo (SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID) 0, 0);
		bool flag = ::SetForegroundWindow(getWnd()) != 0;
		SystemParametersInfo (SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)(DWORDLONG)dwTimeout, 0);
	#endif
	return flag;
}
//-----------------------------------------------
// bool Window::callDefaultProc(Message* message)
// -----  動作  -----
// デフォルトプロシージャの呼び出し
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::callDefaultProc(Message* message)
{
	Window* w = getWindow(message->getWnd());
	if(!w)
		return false;
	w->onDefaultProc(message);
	return true;
}

//-----------------------------------------------
// void Window::onWindowProc(Message* message)
// -----  動作  -----
// ウインドウプロシージャが呼び出された場合のデフォルト処理
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::onWindowProc(Message* message)
{
	if(message->getMsg() == WM_CREATE)
	{
		g_manage.setHandle(this);
	}

	//メッセージメソッドのコールバック
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itProc;
	std::multimap<INT,ClassProc>::iterator itP;

	//全メッセージ
	itProc = m_messageProc.find(0);
	if(itProc != m_messageProc.end())
	{
		for(itP=itProc->second.begin();itP!=itProc->second.end() && itP->first <= 0;++itP)
			itP->second.call(message);	//ユーザ処理
	}

	//特定メッセージのみ
	itProc = m_messageProc.find(message->getMsg());
	if(itProc != m_messageProc.end())
	{
		for(itP=itProc->second.begin();itP!=itProc->second.end() && itP->first <= 0;++itP)
			itP->second.call(message);	//ユーザ処理
	}

	//Notifyメソッドのコールバック
	if(message->getMsg() == WM_NOTIFY)
	{
		std::map<UINT,std::multimap<INT,ClassProc> >::iterator itProc;
		LPNMHDR nmHDR = (LPNMHDR)message->getLParam();
		Window* window = getWindow(nmHDR->hwndFrom);
		if(window)
		{
			itProc = window->m_notifyProc.find(nmHDR->code);
			if(itProc != window->m_notifyProc.end())
			{
				std::multimap<INT,ClassProc>::iterator it;
				for(it=itProc->second.begin();it!=itProc->second.end();++it)
					it->second.call((LPVOID)message);	//ユーザ処理
			}
		}
	}

	//コントロールのコールバック
	if(message->getMsg() == WM_COMMAND)
	{
		std::map<UINT,std::multimap<INT,ClassProc> >::iterator itProc;
		Window* window = getWindow((HWND)message->getLParam());
		if(window)
		{
			itProc = window->m_eventProc.find(message->getWParamHi());
			if(itProc != window->m_eventProc.end())
			{
				std::multimap<INT,ClassProc>::iterator it;
				for(it=itProc->second.begin();it!=itProc->second.end();++it)
					it->second.call((LPVOID)message);	//ユーザ処理
			}
		}
	}

	if(message->getMsg() == WM_SETCURSOR)
	{
		if(m_cursor)
		{
			if((message->getLParam() & 0xFFFF) == 1) 
			{
				message->setDefault(false);
				//SetCursor(m_cursor);
			}
		}
	}

	if(itProc != m_messageProc.end())
	{
		for(;itP!=itProc->second.end();++itP)
			itP->second.call(message);	//ユーザ処理
	}

	onCustomProc(message);				//AFL処理

	if(message->isDefaultProc())
	{
		onDefaultProc(message);
	}
}
//-----------------------------------------------
// void Window::onDefaultProc(Message* message)
// -----  動作  -----
// デフォルトプロシージャの処理
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::onDefaultProc(Message* message)
{
	//デフォルト
	LRESULT result;
	if(m_defaultProc)
	{
		result = CallWindowProcW(m_defaultProc,message->getWnd(),
			message->getMsg(),message->getWParam(),message->getLParam());
	}
	else
	{
		result = DefWindowProcW(m_hwnd,message->getMsg(),
			message->getWParam(),message->getLParam());
	}
	message->setDefault(false);
	message->setResult(result);
}
//-----------------------------------------------
// void Window::onCustomProc(Message* message)
// -----  動作  -----
// プロシージャの専用処理
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::onCustomProc(Message* message)
{
	switch(message->getMsg())
	{
		/*
	case WM_VSCROLL:
		{
			SCROLLINFO info;
			getScrollInfo(SB_VERT,&info);
			switch(message->getWParamLow())
			{
			case SB_THUMBTRACK:
				info.nPos = message->getWParamHi();
				setScrollInfo(SB_VERT,&info);
				break;
			case SB_LINEUP:
				info.nPos -= m_scrollVertLine;
				setScrollInfo(SB_VERT,&info);
				break;
			case SB_LINEDOWN:
				info.nPos += m_scrollVertLine;
				setScrollInfo(SB_VERT,&info);
				break;
			case SB_PAGEUP:
				info.nPos -= getClientHeight()/2;
				setScrollInfo(SB_VERT,&info);
				break;
			case SB_PAGEDOWN:
				info.nPos += getClientHeight()/2;
				setScrollInfo(SB_VERT,&info);
				break;
			}
		}
		InvalidateRect(getWnd(),NULL,true);
		break;
		*/
	case WM_APP_SETCURSOR:
		message->setResult((LRESULT)SetCursor((HCURSOR)message->getWParam()));
		break;

	case WM_ERASEBKGND:
		if(m_backColor == -2)
			break;
		message->setDefault(false);
		message->setResult(true);
		if(m_backColor != -1)
		{
			HDC hdc = (HDC)message->getWParam();
			HBRUSH hBrush = CreateSolidBrush(m_backColor);
			RECT rect;
			getClientRect(&rect);
			FillRect(hdc,&rect,hBrush);
			DeleteObject(hBrush);
		}
		break;
	case WM_CHAR:
		if(message->getWParam() == '\t' && m_hwndNextFocus)
		{
			SetFocus(m_hwndNextFocus);
			message->setDefault(false);
		}
		break;
	case WM_SHOWWINDOW:
		if(message->getWParam())
			recalcLayout();
		break;
	case WM_MOVE:
		RECT rect;
		getWindowRect(&rect);
		m_windowPosition.x = rect.left;
		m_windowPosition.y = rect.top;
		recalcLayout();
		break;
	case WM_SIZE:
		{
			RECT rect = {0,0,message->getLParamLow(),message->getLParamHi()};
			recalcLayout(rect);
			getWindowRect(&rect);
			m_windowSize.cx = rect.right - rect.left;
			m_windowSize.cy = rect.bottom - rect.top;
		}
		break;
	case WM_DESTROY:
		//メッセージループ終了
		if(m_quit)
			PostQuitMessage(0);
		m_hwnd = NULL;
		g_manage.setHandle(this);
		break;
	case WM_STYLECHANGED:
		if(message->getWParam() == GWL_STYLE)
		{
			LPSTYLESTRUCT style = (LPSTYLESTRUCT)message->getLParam();
			m_defaultStyle = style->styleNew;
		}
		else if(message->getWParam() == GWL_EXSTYLE)
		{
			LPSTYLESTRUCT style = (LPSTYLESTRUCT)message->getLParam();
			m_defaultStyleEx = style->styleNew;
		}
		break;
	case WM_WINDOWPOSCHANGED:
		m_defaultStyleEx = GetWindowLong(*this,GWL_EXSTYLE);
		break;
	}
}
//-----------------------------------------------
// bool Window::getWindowRect(LPRECT rect) const
// -----  動作  -----
// ウインドウの位置サイズの取得
// -----  引数  -----
// rect ウインドウ矩形格納用
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::getWindowRect(LPRECT rect) const
{
	if(!isWindow())
		return false;
	return ::GetWindowRect(m_hwnd,rect)!=0;
}
//-----------------------------------------------
// bool Window::getClientRect(LPRECT rect) const
// -----  動作  -----
// クライアントの一サイズの取得
// -----  引数  -----
// rect クライアント矩形格納用
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::getClientRect(LPRECT rect) const
{
	if(!isWindow())
		return false;
	return ::GetClientRect(m_hwnd,rect)!=0;
}

//-----------------------------------------------
// bool Window::moveWindow(int x,int y,int width,int height,bool repaint)
// -----  動作  -----
// ウインドウの位置、サイズの設定
// -----  引数  -----
// x       X座標
// y       Y座標
// width   幅
// height  高さ
// repaint 再描画
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Window::moveWindow(int x,int y,int width,int height,bool repaint)
{
	if(!isWindow())
	{
		m_windowPosition.x = x;
		m_windowPosition.y = y;
		m_windowSize.cx = width;
		m_windowSize.cy = height;
		return false;
	}
	return ::MoveWindow(m_hwnd,x,y,width,height,repaint)!=0;

}
//-----------------------------------------------
// void Window::setClientSize(int width,int height,bool repaint)
// -----  動作  -----
// クライアントサイズの設定
// -----  引数  -----
// width   幅
// height  高さ
// repaint 再描画
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setClientSize(int width,int height,bool repaint)
{
	if(isWindow())
	{
		int i;
		for (i = 0; i < 5; i++)
		{
			int windowWidth;
			int windowHeight;
			RECT rectWindow, rectClient;
			getWindowRect(&rectWindow);
			getClientRect(&rectClient);
			windowWidth = (rectWindow.right - rectWindow.left) -
				(rectClient.right - rectClient.left) + width;
			windowHeight = (rectWindow.bottom - rectWindow.top) -
				(rectClient.bottom - rectClient.top) + height;
			moveWindow(rectWindow.left, rectWindow.top,
				windowWidth, windowHeight, repaint);

			
			if (getClientWidth() == width && getClientHeight() == height)
				break;
		}
	}
}
//-----------------------------------------------
// void Window::setParent(HWND hwnd)
// -----  動作  -----
// 親ウインドウの設定
// -----  引数  -----
// hwnd 親ウインドウのハンドル
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::setParent(HWND hwnd)
{
	Window* window;
	if(m_defaultParent)
	{
		window = getWindow(m_defaultParent);
		if(window)
		{
			window->_delChild(this);
		}
	}

	if(hwnd)
	{
		Window* window = getWindow(hwnd);
		if(window)
		{
			window->_addChild(this);
		}
	}
	m_defaultParent = hwnd;
	if(isWindow() && m_defaultParent != hwnd)
	{
		::SetParent(m_hwnd,hwnd);
	}
}

//-----------------------------------------------
// void Window::addMessage(UINT message,ClassProc& classProc,INT priority)
// -----  動作  -----
// メッセージのフック
// -----  引数  -----
// messege   WM メッセージ
// classProc コールバックプロシージャ
// priority  優先度(値の低い方が優先)
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::addMessage(UINT message,ClassProc& classProc,INT priority)
{
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
	itMsg = m_messageProc.find(message);
	if(itMsg ==  m_messageProc.end())
		m_messageProc[message].insert(std::pair<INT,ClassProc>(priority,classProc));
	else
		itMsg->second.insert(std::pair<INT,ClassProc>(priority,classProc));
}
void Window::addMasterMessage(UINT message,ClassProc& classProc,INT priority)
{
	if(g_manage.m_windowMaker)
		g_manage.m_windowMaker->addMessage(message,classProc,priority);
}
LRESULT Window::sendMasterMessage(UINT msg,WPARAM wParam,LPARAM lParam) const
{
	if(g_manage.m_windowMaker)
		return g_manage.m_windowMaker->sendMessage(msg,wParam,lParam);
	return 0;
}

//-----------------------------------------------
// void Window::addNotify(UINT message,ClassProc& classProc,INT priority)
// -----  動作  -----
// Notifyイベントのフック
// -----  引数  -----
// message Noftyイベント
// classProc コールバックプロシージャ
// priority  優先度(値の低い方が優先)
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::addNotify(UINT message,ClassProc& classProc,INT priority)
{
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
	itMsg = m_notifyProc.find(message);
	if(itMsg ==  m_notifyProc.end())
		m_notifyProc[message].insert(std::pair<INT,ClassProc>(priority,classProc));
	else
		itMsg->second.insert(std::pair<INT,ClassProc>(priority,classProc));
}
//-----------------------------------------------
// void Window::_addChild(Window* window)
// -----  動作  -----
// 
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::_addChild(Window* window)
{
	if(window)
		m_childWindow.insert(window);
}
//-----------------------------------------------
// void Window::_delChild(Window* window)
// -----  動作  -----
// 
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::_delChild(Window* window)
{
	if(window)
		m_childWindow.erase(window);
}

//-----------------------------------------------
// void Window::_addChild(HWND hwnd)
// -----  動作  -----
// 
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::_addChild(HWND hwnd)
{
	Window* window = getWindow(hwnd);
	_addChild(window);
}
//-----------------------------------------------
// void Window::_delChild(HWND hwnd)
// -----  動作  -----
// 
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::_delChild(HWND hwnd)
{
	Window* window = getWindow(hwnd);
	_delChild(window);
}
//-----------------------------------------------
// HDC Window::getDC() const
// -----  動作  -----
// デバイスコンテキストの取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// デバイスコンテキスト
//-----------------------------------------------
HDC Window::getDC() const
{
	return GetDC(getWnd());
}
//-----------------------------------------------
// INT Window::releaseDC(HDC hdc) const
// -----  動作  -----
// デバイスコンテキストの開放
// -----  引数  -----
// hdc デバイスコンテキスト
// ----- 戻り値 -----
// コード
//-----------------------------------------------
INT Window::releaseDC(HDC hdc) const
{
	return ReleaseDC(getWnd(),hdc);
}

//-----------------------------------------------
// INT Window::fillRect(const RECT* rect,COLORREF color) const
// -----  動作  -----
// 矩形で塗りつぶす
// -----  引数  -----
// rect  範囲
// color 色
// ----- 戻り値 -----
// コード
//-----------------------------------------------
INT Window::fillRect(const RECT* rect,COLORREF color) const
{
	HDC hdc = getDC();
	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(color);
	INT ret = FillRect(hdc,rect,hBrush);
	DeleteObject(hBrush);
	releaseDC(hdc);
	return ret;
}
//-----------------------------------------------
// void Window::recalcLayout()
// -----  動作  -----
// 子ウインドウの位置を自動設定
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::recalcLayout()
{
	RECT rect;
	getClientRect(&rect);
	recalcLayout(rect);
}
//-----------------------------------------------
// void Window::recalcLayout(RECT& rect)
// -----  動作  -----
// 子ウインドウの位置を自動設定
// -----  引数  -----
// rect 設定範囲
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Window::recalcLayout(RECT& rect)
{
	INT vertCount = 0;
	INT horizCount = 0;
	INT vSize = 0;
	INT hSize = 0;

	rect.top += m_paddingRect.top;
	rect.left += m_paddingRect.left;
	rect.bottom -= m_paddingRect.bottom;
	rect.right -= m_paddingRect.right;


	std::multimap<INT,Window*> rayout;
	std::set<Window*>::iterator it;
	for(it=m_childWindow.begin();it!=m_childWindow.end();++it)
	{
		Window* window = *it;
		DWORD style = window->getStyle();
		if(style & WS_CHILD && !(style & WS_POPUP))
		{
			INT priority = window->getChildPriority();
			CHILD_STYLE childStyle = window->getChildStyle();
			if(childStyle == CHILD_NORMAL)
				continue;
			if(childStyle == CHILD_LEFT || childStyle ==  CHILD_RIGHT)
				priority += 100;
			else if(childStyle == CHILD_TOP || childStyle ==  CHILD_BOTTOM)
				priority += 200;
			else
			{
				if(childStyle == CHILD_EQUARL_VERT)
					vertCount++;
				else if(childStyle == CHILD_EQUARL_HORIZ)
					horizCount++;
				priority += 300;
			}
			rayout.insert(std::pair<INT,Window*>(priority,window));
		}
	}

	if(rayout.size())
	{
/*		rect.left += m_marginRect.left;
		rect.right -= m_marginRect.right;
		rect.top += m_marginRect.top;
		rect.bottom -= m_marginRect.bottom;
*/
		std::multimap<INT,Window*>::iterator itRayout;
		for(itRayout=rayout.begin();itRayout!=rayout.end();++itRayout)
		{
			Window* window = itRayout->second;
			//非表示ウインドウを無視
			if(!window->isWindowVisible())
				continue;
			CHILD_STYLE childStyle = window->getChildStyle();
			INT size,size2;
			RECT clientRect;
			window->getWindowRect(&clientRect);

			RECT rectMove;
			switch(childStyle)
			{
			case CHILD_TOP:
				size = clientRect.bottom - clientRect.top;
				rectMove.left = rect.left;
				rectMove.top = rect.top;
				rectMove.right = rect.right-rect.left;
				rectMove.bottom = size;
				rect.top += size;
				break;
			case CHILD_BOTTOM:
				size = clientRect.bottom - clientRect.top;
				rectMove.left = rect.left;
				rectMove.top = rect.bottom-size;
				rectMove.right = rect.right-rect.left;
				rectMove.bottom = size;
				rect.bottom -= size;
				break;

			case CHILD_LEFT:
				rect.left += window->m_marginRect.left;
				rect.right += window->m_marginRect.right;
				size = clientRect.right - clientRect.left;
				rectMove.left = rect.left;
				rectMove.top = rect.top;
				rectMove.right = size;
				rectMove.bottom = rect.bottom-rect.top;
				rect.left += size;
				break;
			case CHILD_RIGHT:
				rect.left += window->m_marginRect.left;
				rect.right += window->m_marginRect.right;
				size = clientRect.right - clientRect.left;
				rectMove.left = rect.right-size;
				rectMove.top = rect.top;
				rectMove.right = size;
				rectMove.bottom = rect.bottom-rect.top;
				rect.right -= size;
				break;
			case CHILD_CLIENT:
				rectMove.left = rect.left;
				rectMove.top = rect.top;
				rectMove.right = rect.right-rect.left;
				rectMove.bottom = rect.bottom-rect.top;
				break;
			case CHILD_EQUARL_VERT:
				if(vSize == 0)
					vSize = (rect.bottom - rect.top) / vertCount;
				window->moveWindow(rect.left,rect.top,rect.right-rect.left,vSize);
				rectMove.left = rect.left;
				rectMove.top = rect.top;
				rectMove.right = rect.right-rect.left;
				rectMove.bottom = vSize;
				rect.top += vSize;
				break;
			case CHILD_EQUARL_HORIZ:
				if(hSize == 0)
					hSize = (rect.right - rect.left) / horizCount;
				rectMove.left = rect.left;
				rectMove.top = rect.top;
				rectMove.right = hSize;
				rectMove.bottom = rect.bottom-rect.top;
				rect.left += hSize;
				break;
			case CHILD_CENTER:
				size = ((rect.right-rect.left) - (clientRect.right - clientRect.left)) / 2;
				size2 = ((rect.bottom-rect.top) - (clientRect.bottom - clientRect.top)) / 2;
				window->setPos(size,size2);
				rectMove.left = size;
				rectMove.top = size2;
				rectMove.right = rect.right-rect.left;
				rectMove.bottom = rect.bottom-rect.top;
				break;
				
			}
			window->moveWindow(rectMove.left,rectMove.top,rectMove.right,rectMove.bottom);

		}
	}
}
//-----------------------------------------------
// bool Window::doModal(HWND hwnd)
// -----  動作  -----
// モーダルウインドウの表示
// -----  引数  -----
// hwnd 親ウインドウのハンドル
// ----- 戻り値 -----
// true:OK false:CANSEL
//-----------------------------------------------
bool Window::doModal(HWND hwnd)
{
	m_modalResult = false;
	if(hwnd)
		setParent(hwnd);
	showWindow();
	if(isWindow())
	{
		if(!hwnd)
			hwnd = getParent();
		if(hwnd)
			EnableWindow(hwnd,false);
		while(isWindow())
		{
			MSG msg;
			while(PeekMessageW(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		if(hwnd)
			EnableWindow(hwnd,true);
	}
	return m_modalResult;
}
//-----------------------------------------------
// DWORD Window::message(bool wait)
// -----  動作  -----
// メッセージループ 
// -----  引数  -----
// wait ウインドウが閉じられるまで待つか
// ----- 戻り値 -----
// 終了時の戻り値
//-----------------------------------------------
DWORD Window::message(bool wait)
{
	return g_manage.message(wait);
}

//-----------------------------------------------
// INT Window::printf(LPCWSTR format, ...)
// -----  動作  -----
// フォーマットを指定してテキストの出力
// -----  引数  -----
// format フォーマット内容
// ----- 戻り値 -----
// 出力文字数
//-----------------------------------------------
INT Window::printf(LPCWSTR format, ...)
{
	WString dest;

	va_list param_list;
	va_start(param_list, format);
	INT ret = dest.vprintf(format,param_list);
	va_end(param_list);
	setTitle(dest.c_str());
	return ret;
}
//-----------------------------------------------
// INT Window::printf(LPCSTR format, ...)
// -----  動作  -----
// フォーマットを指定してテキストの出力
// -----  引数  -----
// format フォーマット内容
// ----- 戻り値 -----
// 出力文字数
//-----------------------------------------------
INT Window::printf(LPCSTR format, ...)
{
	std::string dest;

	va_list param_list;
	va_start(param_list, format);

	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	setTitle(dest.c_str());
	return ret;
}
DWORD Window::getThreadID() const
{
	if(this == (LPVOID)0xdddddddd)
		return 0;
	return m_threadID;
}


}}