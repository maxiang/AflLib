#include <windows.h>
#include <Richedit.h>
#include "aflWindowCtrl.h"

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
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 

namespace AFL { namespace WINDOWS{

//WindowRicheView WindowDebug::m_window;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowRiche
// リッチテキスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowRiche::WindowRiche()
{
	setStyleEx(WS_EX_CLIENTEDGE);	
	setStyle(WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |WS_CLIPCHILDREN);
	LoadLibraryW(L"RICHED20.DLL");
	setClassName(RICHEDIT_CLASSW);
}
void WindowRiche::out(LPCSTR text)
{
	SendMessageA(getWnd(),EM_REPLACESEL ,0,(LPARAM)text);
	SendMessage(getWnd(),EM_SCROLLCARET ,0,0);
}
void WindowRiche::out(LPVOID data,INT size)
{
	String text;
	text.assign((LPCSTR)data,size);

	SendMessageA(getWnd(),EM_REPLACESEL ,0,(LPARAM)text.c_str());
	SendMessage(getWnd(),EM_SCROLLCARET ,0,0);
}
void WindowRiche::out(LPCWSTR text)
{
	SendMessageW(getWnd(),EM_REPLACESEL ,0,(LPARAM)text);
	SendMessage(getWnd(),EM_SCROLLCARET ,0,0);
}
INT WindowRiche::printf(LPCSTR format, ...)
{
	std::string dest;

	va_list param_list;
	va_start(param_list, format);

	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	out(dest.c_str());
	return ret;
}
INT WindowRiche::printf(LPCWSTR format, ...)
{
	std::wstring dest;

	va_list param_list;
	va_start(param_list, format);

	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	out(dest.c_str());
	return ret;
}
void WindowRiche::setReadonly(bool flag)
{
	sendMessage(EM_SETREADONLY,flag,0);
}
void WindowRiche::setTextLimit(INT value) const
{
	sendMessage(EM_EXLIMITTEXT,0,(LPARAM)value);
}
void WindowRiche::setSel(INT start,INT end) const
{
	sendMessage(EM_SETSEL,(WPARAM)start,(LPARAM)end);
}
void WindowRiche::setCaret() const
{
	sendMessage(EM_SCROLLCARET);
}

void WindowRiche::getSel(INT* start,INT* end) const
{
	sendMessage(EM_GETSEL,(WPARAM)start,(LPARAM)end);
}
void WindowRiche::setReadonly(bool flag) const
{
	sendMessage(EM_SETREADONLY,(WPARAM)flag);
}
 
void WindowRiche::getSelText(WString& dest) const
{
	INT length = getSelLength()+1;
	WCHAR* buff = new WCHAR[length];
	sendMessage(EM_GETSELTEXT,NULL,(LPARAM)buff);
	dest = buff;
	delete[] buff;
}
INT WindowRiche::getSelLength() const
{
	INT start,end;
	getSel(&start,&end);
	INT length = end - start;
	return length;
}
void WindowRiche::setFont(LPCWSTR name)
{
	DWORD d = (DWORD)sendMessage(EM_GETLANGOPTIONS, 0, 0);
	sendMessage(EM_SETLANGOPTIONS, 0, d & (~IMF_DUALFONT));

	CHARFORMATW cfm;
	cfm.cbSize = sizeof(CHARFORMATW);
	sendMessage(EM_GETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cfm);
	lstrcpyW(cfm.szFaceName, name);
	sendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cfm);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowRiche
// リッチテキストビュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowRicheView::WindowRicheView()
{
	addMessage(WM_CREATE,CLASSPROC(this,WindowRicheView,onCreate));
	m_ctrl.setChildStyle(CHILD_CLIENT);
}
void WindowRicheView::out(LPCSTR text)
{
	m_ctrl.out(text);
}
void WindowRicheView::out(LPCWSTR text)
{
	m_ctrl.out(text);
}
WindowRiche* WindowRicheView::getCtrl()
{
	return &m_ctrl;
}
void WindowRicheView::onCreate(Message* message)
{
	m_ctrl.createChildWindow(message->getWnd());
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WSplit
// 分割ウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// WSplit::WSplit()
// -----  動作  -----
// コンストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
WSplit::WSplit()
{
	addMessage(WM_ERASEBKGND,CLASSPROC(this,WSplit,onEracebkgnd));
	m_splitBar.addMessage(WM_ERASEBKGND,CLASSPROC(this,WSplit,onEracebkgnd));
	m_splitBar.addMessage(WM_PAINT,CLASSPROC(this,WSplit,onSplitPaint));
	m_splitBar.addMessage(WM_MOVE,CLASSPROC(this,WSplit,onSplitMove));
	m_splitBar.addMessage(WM_SIZE,CLASSPROC(this,WSplit,onSplitSize));
	m_splitBar.addMessage(WM_NCHITTEST,CLASSPROC(this,WSplit,onHitTest));
	m_splitBar.addMessage(WM_MOVING,CLASSPROC(this,WSplit,onMoving));
	m_splitBar.addMessage(WM_SETCURSOR,CLASSPROC(this,WSplit,onSetCursor));
	m_borderWidth = 7;				//分割バーのボーダーサイズ
	m_splitSize = 150;				//分割バーの初期位置
	m_splitBase = SPLIT_FIRST;		//分割基準位置
	m_splitStyle = SPLIT_VERT;		//縦、横のスタイル設定(デフォルト縦)
}
//-----------------------------------------------
// void WSplit::onEracebkgnd(Message* message)
// -----  動作  -----
// 背景消去の無効化
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WSplit::onEracebkgnd(Message* message)
{
	message->setDefault(false);
}
//-----------------------------------------------
// void WSplit::onMoving(Message* message)
// -----  動作  -----
// 移動時の処理
// -----  引数  -----
// message メッセージ
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WSplit::onMoving(Message* message)
{
	Window* window = Window::getWindow(message->getWnd());
	if(window)
	{
		POINT parentPoint = {0,0};
		RECT parentRect;
		RECT clientRect;
		clientToScreen(&parentPoint);
		getClientRect(&clientRect);
		parentRect.left = clientRect.left + parentPoint.x;
		parentRect.right = clientRect.right + parentPoint.x;
		parentRect.bottom = clientRect.bottom + parentPoint.y;
		parentRect.top = clientRect.top + parentPoint.y;

		LPRECT moveRect = (LPRECT)message->getLParam();

		if(m_splitStyle == SPLIT_VERT)
		{
			moveRect->top = parentRect.top;
			moveRect->bottom = parentRect.bottom;

			if(moveRect->left < parentRect.left)
				moveRect->left = parentRect.left;
			if(moveRect->left+m_borderWidth > parentRect.right)
				moveRect->left = parentRect.right - m_borderWidth;
			moveRect->right = moveRect->left + m_borderWidth;
			
			if(m_splitBase == SPLIT_FIRST)
			{
				m_splitSize = moveRect->left - parentPoint.x;
			}
			else
			{
				m_splitSize = parentRect.right - moveRect->right + m_borderWidth;
			}
		}
		else
		{
			moveRect->left = parentRect.left;
			moveRect->right = parentRect.right;
			if(moveRect->top < parentRect.top)
				moveRect->top = parentRect.top;
			if(moveRect->top+m_borderWidth > parentRect.bottom)
				moveRect->top = parentRect.bottom - m_borderWidth;
			moveRect->bottom = moveRect->top + m_borderWidth;

			if(m_splitBase == SPLIT_FIRST)
			{
				m_splitSize = moveRect->top - parentPoint.y;
			}
			else
				m_splitSize = parentRect.bottom - moveRect->bottom + m_borderWidth;

		}
	}
}
//-----------------------------------------------
// void WSplit::update()
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::update()
{
	RECT rect;
	getClientRect(&rect);
	INT width = rect.right - rect.left;
	INT height = rect.bottom - rect.top;

	INT x,y;
	if(m_splitStyle == SPLIT_VERT)
	{
		if(m_splitBase == SPLIT_FIRST)
			x = m_splitSize;
		else
			x = width - m_splitSize;
		m_client1.moveWindow(0,0,x,height);
		m_client2.moveWindow(x+m_borderWidth,0,width-x-m_borderWidth,height);
	}
	else
	{
		if(m_splitBase == SPLIT_FIRST)
			y = m_splitSize;
		else
			y = height - m_splitSize;
		m_client1.moveWindow(0,0,width,y);
		m_client2.moveWindow(0,y+m_borderWidth,width,height-y-m_borderWidth);
	}
}
//-----------------------------------------------
// void WSplit::onSplitMove(Message* message)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::onSplitMove(Message* message)
{
	callDefaultProc(message);
	update();
}
//-----------------------------------------------
// void WSplit::onSplitSize(Message* message)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::onSplitSize(Message* message)
{
	callDefaultProc(message);
	update();
}
//-----------------------------------------------
// void WSplit::onSplitPaint(Message* message)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::onSplitPaint(Message* message)
{
	RECT rect;
	m_splitBar.getClientRect(&rect);

	HDC hdc = m_splitBar.getDC();
	HPEN hPen1 = CreatePen(PS_SOLID,0,RGB(230,230,230));
	HPEN hPen2 = CreatePen(PS_SOLID,0,RGB(0,0,0));
	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(200,200,200));

	HPEN hOld = (HPEN)SelectObject(hdc,(HGDIOBJ)hPen1);
	MoveToEx(hdc,0,rect.bottom,NULL);
	LineTo(hdc,0,0);
	LineTo(hdc,rect.right,0);
	
	SelectObject(hdc,(HGDIOBJ)hPen2);
	MoveToEx(hdc,0,rect.bottom-1,NULL);
	LineTo(hdc,rect.right-1,rect.bottom-1);
	LineTo(hdc,rect.right-1,0);

	rect.top += 1;
	rect.left += 1;
	rect.bottom -= 1;
	rect.right -= 1;
	FillRect(hdc,&rect,hBrush);

	SelectObject(hdc,(HGDIOBJ)hOld);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
	DeleteObject(hBrush);

	m_splitBar.releaseDC(hdc);
}
//-----------------------------------------------
// void WSplit::onSetCursor(Message* message)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::onSetCursor(Message* message)
{
	setCursor(m_cursor);
	message->setDefault(false);
}
//-----------------------------------------------
// void WSplit::onHitTest(Message* message)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::onHitTest(Message* message)
{
	message->setDefault(false);
	message->setResult(HTCAPTION);
}
//-----------------------------------------------
// bool WSplit::createChildWindow(HWND hwnd)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool WSplit::createChildWindow(HWND hwnd)
{
	return create(hwnd);
}

//-----------------------------------------------
// bool WSplit::create(HWND parent,SPLIT_STYLE style)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool WSplit::create(HWND parent,SPLIT_STYLE style)
{
	if(style != SPLIT_DEFAULT)
		m_splitStyle = style;
	setChildStyle(CHILD_CLIENT);
	if(!Window::createWindow(NULL,NULL,WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,parent))
		return false;

	//分割用子ウインドウの作成
	m_client1.createWindow(NULL,NULL,WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,m_hwnd);
	m_client2.createWindow(NULL,NULL,WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE,m_hwnd);
	
	updateSplit();
	return true;
}
//-----------------------------------------------
// bool WSplit::updateSplit()
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool WSplit::updateSplit()
{
	if(!isWindow())
		return false;
	RECT rect;
	getClientRect(&rect);

	POINT point;
	if(m_splitBase == SPLIT_FIRST)
	{
		point.x = m_splitSize;
		point.y = m_splitSize;
	}
	else
	{
		point.x = rect.right - m_splitSize;
		point.y = rect.bottom - m_splitSize;
	}
	m_splitBar.setPos(point.x,point.y);
	m_splitBar.createWindow(NULL,NULL,WS_CHILD|WS_VISIBLE,m_hwnd);

	if(m_splitStyle == SPLIT_VERT)
	{
		m_cursor = (HCURSOR)LoadImage(0,IDC_SIZEWE,IMAGE_CURSOR,0,0,LR_SHARED);
	}
	else
	{
		m_cursor = (HCURSOR)LoadImage(0,IDC_SIZENS,IMAGE_CURSOR,0,0,LR_SHARED);
	}
	return true;
}
//-----------------------------------------------
// void WSplit::setBorderWidth(INT width)
// -----  動作  -----
// 分割バーの幅を設定
// -----  引数  -----
// width 幅
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WSplit::setBorderWidth(INT width)
{
	m_borderWidth = width;
}

//-----------------------------------------------
// void WSplit::recalcLayout(RECT& rect)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WSplit::recalcLayout(RECT& rect)
{
	INT width = rect.right - rect.left;
	INT height = rect.bottom - rect.top;
	if(m_splitBase == SPLIT_FIRST)
	{
		if(m_splitStyle == SPLIT_VERT)
			m_splitBar.moveWindow(m_splitSize,0,m_borderWidth,height);
		else
			m_splitBar.moveWindow(0,m_splitSize,width,m_borderWidth);
	}
	else
	{
		if(m_splitStyle == SPLIT_VERT)
			m_splitBar.moveWindow(width-m_splitSize,0,m_borderWidth,height);
		else
			m_splitBar.moveWindow(0,height-m_splitSize,width,m_borderWidth);
	}
	update();
}
//-----------------------------------------------
// Window* WSplit::getChild(INT index)
// -----  動作  -----
// 子ウインドウを持つ中間ウインドウの取得
// -----  引数  -----
// 0:左もしくは上 1:右もしくは下
// ----- 戻り値 -----
// 中間ウインドウのポインタ
//-----------------------------------------------
Window* WSplit::getChild(INT index)
{
	if(index == 0)
		return &m_client1;
	else
		return &m_client2;
}


//-----------------------------------------------
// void WSplit::setSplitSize(INT size,SPLIT_BASE base)
// -----  動作  -----
// スプリットバーの位置を設定
// -----  引数  -----
// size		サイズ
// base 	SPLIT_FIRST  左もしくは上を固定サイズとする   
//			SPLIT_SECOND 右もしくは下を固定サイズとする
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WSplit::setSplitSize(INT size,SPLIT_BASE base)
{
	m_splitSize = size;
	m_splitBase = base;
	updateSplit();

	Window::recalcLayout();
}
//-----------------------------------------------
// bool WSplit::addChild(INT index,Window* window,CHILD_STYLE style)
// -----  動作  -----
// 子ウインドウの追加
// -----  引数  -----
// index  0:左もしくは上 1:右もしくは左
// window ウインドウクラスのポインタ
// style  配置スタイル
// ----- 戻り値 -----
// true:正常 false:失敗
//-----------------------------------------------
bool WSplit::addChild(INT index,Window* window,CHILD_STYLE style)
{
	if(index == 0)
		return m_client1.addChild(window,style);
	else if(index == 1)
		return m_client2.addChild(window,style);
	return false;
}
//-----------------------------------------------
// void WSplit::setSplitStyle(SPLIT_STYLE style)
// -----  動作  -----
// スプリットバーの状態を設定する
// -----  引数  -----
// style	SPLIT_DEFAULT	縦分け
//			SPLIT_VERT		横分け
//			SPLIT_HORIZ	縦分け
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WSplit::setSplitStyle(SPLIT_STYLE style)
{
	m_splitStyle = style;
	updateSplit();
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowStatic
// スタティックテキスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowStatic::WindowStatic()
{
	setWindowSize(64,16);
	setClassName(L"static");
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowStatus
// ステータスウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowStatus::WindowStatus()
{
	setWindowHeight(32);
}
void WindowStatus::setText(INT index,LPCSTR text)
{
	sendMessage(SB_SETTEXTA, (WPARAM)index, (LPARAM)text);
}
void WindowStatus::setText(INT index,LPCWSTR text)
{
	sendMessage(SB_SETTEXTW, (WPARAM)index, (LPARAM)text);
}
void WindowStatus::setParts(INT count,const INT* widths)
{
	if(count==0 || (count && widths))
		sendMessage(SB_SETPARTS,(WPARAM)count,(LPARAM)widths);
}

bool WindowStatus::createChildWindow(HWND hwnd)
{
	return create(hwnd,0);
}
bool WindowStatus::create(HWND parent,UINT id)
{
	return createWindow(STATUSCLASSNAMEW, L"", WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,parent,id);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowEdit
// エディットコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowEdit::WindowEdit()
{
	setStyle(WS_CHILD | WS_VISIBLE |  ES_AUTOHSCROLL | WS_CLIPCHILDREN);
	setStyleEx(WS_EX_CLIENTEDGE);
	setClassName(L"EDIT");
}
void WindowEdit::setPassword(bool flag)
{
	if(flag)
		sendMessage(EM_SETPASSWORDCHAR,'*',0);
}
void WindowEdit::setReadonry(bool flag)
{
	sendMessage(EM_SETREADONLY,flag,0);
}
void WindowEdit::setMultiLine(bool flag)
{
	LONG_PTR l = getWindowLong(GWL_STYLE);
	if(flag)
		setWindowLong(GWL_STYLE,l | ES_MULTILINE|ES_AUTOVSCROLL );
	else
		setWindowLong(GWL_STYLE,l & (~(ES_MULTILINE|ES_AUTOVSCROLL)));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WText
// テキストウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// WText::WText()
// -----  動作  -----
// コンストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
WText::WText()
{
	addMessage(WM_CREATE,CLASSPROC(this,WText,onCreate));
	addMessage(WM_SIZE,CLASSPROC(this,WText,onSize),1);
}
//-----------------------------------------------
// void WText::onCreate(Message* message)
// -----  動作  -----
// ウインドウ作成時の処理
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WText::onCreate(Message* message)
{
	HWND hwnd = message->getWnd();
	m_edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", 
		WS_CHILD | WS_VISIBLE |  ES_AUTOVSCROLL|WS_CLIPCHILDREN|WS_TABSTOP|ES_MULTILINE | WS_VSCROLL,
		0, 0, 0, 0, //とりあえず幅、高さ０のウィンドウを作る
		hwnd, (HMENU)1, NULL, NULL); 
	SendMessage(m_edit,EM_SETBKGNDCOLOR,0,0);
	setTextColor(0xffffff);

}
//-----------------------------------------------
// void WText::onSize(Message* message)
// -----  動作  -----
// ウインドウサイズ変更時の処理
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void WText::onSize(Message* message)
{
	MoveWindow(m_edit, 0, 0, message->getLParamLow(), message->getLParamHi(), TRUE);
}
//-----------------------------------------------
// void WText::out(LPCSTR text)
// -----  動作  -----
// テキストの出力
// -----  引数  -----
// text 文字列
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::out(LPCSTR text)
{
	SendMessage(m_edit,EM_REPLACESEL ,0,(LPARAM)text);
	SendMessage(m_edit,EM_SCROLLCARET ,0,0);
}
//-----------------------------------------------
// void WText::out(LPCWSTR text)
// -----  動作  -----
// テキストの出力
// -----  引数  -----
// text 文字列
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::out(LPCWSTR text)
{
	SendMessageW(m_edit,EM_REPLACESEL ,0,(LPARAM)text);
	SendMessage(m_edit,EM_SCROLLCARET ,0,0);
}
//-----------------------------------------------
// void WText::setPassword(bool flag)
// -----  動作  -----
// パスワードマスクの設定
// -----  引数  -----
// flag true:マスク有り false:マスク無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setPassword(bool flag)
{
	if(flag)
		sendMessage(EM_SETPASSWORDCHAR,'*',0);
}
//-----------------------------------------------
// void WText::clear() const
// -----  動作  -----
// テキストのクリア
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::clear() const
{
	::SetWindowTextW(m_edit,L"");
}
//-----------------------------------------------
// INT WText::printf(LPCWSTR format, ...)
// -----  動作  -----
// フォーマットを指定してテキストの出力
// -----  引数  -----
// format フォーマット内容
// ----- 戻り値 -----
// 出力文字数
//-----------------------------------------------
INT WText::printf(LPCWSTR format, ...)
{
	WString dest;

	va_list param_list;
	va_start(param_list, format);
	INT ret = dest.vprintf(format,param_list);
	va_end(param_list);
	out(dest.c_str());
	return ret;
}
//-----------------------------------------------
// INT WText::printf(LPCSTR format, ...)
// -----  動作  -----
// フォーマットを指定してテキストの出力
// -----  引数  -----
// format フォーマット内容
// ----- 戻り値 -----
// 出力文字数
//-----------------------------------------------
INT WText::printf(LPCSTR format, ...)
{
	std::string dest;

	va_list param_list;
	va_start(param_list, format);

	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	out(dest.c_str());
	return ret;
}
//-----------------------------------------------
// void WText::setTextColor(COLORREF color)
// -----  動作  -----
// テキスト色の設定
// -----  引数  -----
// color 色
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setTextColor(COLORREF color)
{
	CHARFORMAT cf;
	ZeroMemory(&cf,sizeof(cf));
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR|CFM_FACE;
	cf.dwEffects = 0;
	cf.crTextColor = color;
	cf.bPitchAndFamily = 8;
	SendMessage(m_edit,EM_SETCHARFORMAT ,SCF_DEFAULT,(LPARAM)&cf);
}
//-----------------------------------------------
// void WText::setMultiLine(bool flag)
// -----  動作  -----
// 複数行を利用可能にするか設定
// -----  引数  -----
// flag true:複数行 false:単行
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setMultiLine(bool flag)
{
	D_PTR l = GetWindowLongPtr(m_edit,GWL_STYLE);
	if(flag)
		SetWindowLongPtr(m_edit,GWL_STYLE,l | ES_MULTILINE|ES_AUTOVSCROLL );
	else
		SetWindowLongPtr(m_edit,GWL_STYLE,l & (~(ES_MULTILINE|ES_AUTOVSCROLL)));
}
//-----------------------------------------------
// void WText::getText(std::string& dest)
// -----  動作  -----
// テキスト内容の読み出し
// -----  引数  -----
// dest 格納用文字列領域
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::getText(std::string& dest)
{
	if(m_edit)
	{
		INT length = GetWindowTextLength(m_edit)+1;
		LPSTR s = new CHAR[length];
		if(s)
		{
			::GetWindowTextA(m_edit,s,length);
			dest = s;
			delete[] s;
		}
	}
}
//-----------------------------------------------
// void WText::getText(std::wstring& dest)
// -----  動作  -----
// テキスト内容の読み出し
// -----  引数  -----
// dest 格納用文字列領域
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::getText(std::wstring& dest)
{
	if(m_edit)
	{
		INT length = GetWindowTextLengthW(m_edit)+1;
		LPWSTR s = new WCHAR[length];
		if(s)
		{
			::GetWindowTextW(m_edit,s,length);
			dest = s;
			delete[] s;
		}
	}
}
//-----------------------------------------------
// void WText::setText(LPCSTR value)
// -----  動作  -----
// テキストの設定
// -----  引数  -----
// value　テキスト内容
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setText(LPCSTR value)
{
	SetWindowTextA(m_edit,value);
}
//-----------------------------------------------
// void WText::setText(LPCWSTR value)
// -----  動作  -----
// テキストの設定
// -----  引数  -----
// value　テキスト内容
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setText(LPCWSTR value)
{
	SetWindowTextW(m_edit,value);
}
//-----------------------------------------------
// void WText::setReadonly(bool flag)
// -----  動作  -----
// 読み取り専用設定
// -----  引数  -----
// flag true:読み取り専用 false:書き込み可
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void WText::setReadonly(bool flag)
{
	SendMessage(m_edit,EM_SETREADONLY,flag,0);
}
//-----------------------------------------------
// HWND WText::getCtrl() const
// -----  動作  -----
// エディットコントロールの取得
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// エディットコントロールのウインドウハンドル
//-----------------------------------------------
HWND WText::getCtrl() const
{
	return m_edit;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowList
// リストコントロールラッパー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowList::WindowList()
{
	setClassName(WC_LISTVIEWW);
	setStyle(WS_CHILD | WS_VISIBLE | LVS_REPORT |LVS_SHOWSELALWAYS);
	addMessage(WM_SIZE,CLASSPROC2(this,WindowList,onSize,Message*),1);
	addNotify(NM_CUSTOMDRAW,CLASSPROC2(this,WindowList,onCustom,Message*));
}
void WindowList::setItemText( int item, int subItem, LPCSTR text) const
{
	if(text)
	{
		LV_ITEMA lvItem;
		lvItem.iSubItem = subItem;
		lvItem.pszText = (LPSTR)text;
		sendMessage(LVM_SETITEMTEXTA,(WPARAM)(item),(LPARAM)&lvItem);
	}
}
void WindowList::setItemText( int item, int subItem, LPCWSTR text) const
{
	if(text)
	{
		LV_ITEMW lvItem;
		lvItem.iSubItem = subItem;
		lvItem.pszText = (LPWSTR)text;
		sendMessage(LVM_SETITEMTEXTW,(WPARAM)(item),(LPARAM)&lvItem);
	}
}
bool WindowList::getItemText(int item,int subItem,LPSTR text,int length) const
{
	LV_ITEMA lvItem;
	lvItem.iSubItem = subItem;
	lvItem.cchTextMax = length;
	lvItem.pszText = text;
	return sendMessage(LVM_GETITEMTEXTA,(WPARAM)(item),(LPARAM)&lvItem) > 0;
}
bool WindowList::getItemText(int item,int subItem,LPWSTR text,int length) const
{
	LV_ITEMW lvItem;
	lvItem.iSubItem = subItem;
	lvItem.cchTextMax = length;
	lvItem.pszText = text;
	return sendMessage(LVM_GETITEMTEXTW,(WPARAM)(item),(LPARAM)&lvItem) > 0;
}
bool WindowList::getItemText(INT item,INT subItem,String& value) const
{
	CHAR buff[2048];
	if(!getItemText(item,subItem,buff,2048))
		return false;
	value = buff;
	return true;
}
bool WindowList::getItemText(INT item,INT subItem,WString& value) const
{
	WCHAR buff[2048];
	if(!getItemText(item,subItem,buff,2048))
		return false;
	value = buff;
	return true;
}
int WindowList::getStringWidth(LPCSTR lpszString) const
{
	return (INT)sendMessage(LVM_GETSTRINGWIDTHA,0,(LPARAM)lpszString);
}
int WindowList::getStringWidth(LPCWSTR lpszString) const
{
	return (INT)sendMessage(LVM_GETSTRINGWIDTHW,0,(LPARAM)lpszString);
}
bool WindowList::getSubItemRect( int iItem, int iSubItem, int nArea, LPRECT pRect) const
{
	return ListView_GetSubItemRect(m_hwnd,iItem,iSubItem,nArea,pRect)!=0;
}
DWORD WindowList::setExtendedStyle( DWORD dwNewStyle ) const
{
	return ListView_SetExtendedListViewStyle(m_hwnd,dwNewStyle);
}
bool WindowList::deleteItem( int nItem ) const
{
	return ListView_DeleteItem(m_hwnd,nItem)!=0;
}
bool WindowList::deleteAllItems() const
{
	return ListView_DeleteAllItems(m_hwnd)!=0;
}

int WindowList::findItem( LVFINDINFOA* pFindInfo, int nStart) const
{
	return (int)sendMessage(LVM_FINDITEMA,(WPARAM)nStart,(LPARAM)pFindInfo);
}
int WindowList::findItem( LVFINDINFOW* pFindInfo, int nStart) const
{
	return (int)sendMessage(LVM_FINDITEMW,(WPARAM)nStart,(LPARAM)pFindInfo);
}
int WindowList::getNextItem(INT index) const
{
	return ListView_GetNextItem(m_hwnd,index,LVNI_ALL);
}
bool WindowList::sortItems(PFNLVCOMPARE pfnCompare,DWORD dwData) const
{
	return ListView_SortItems(m_hwnd,pfnCompare,dwData)!=0;
}
int CALLBACK WindowList::FNLVCOMPARE(LPARAM param1, LPARAM param2, LPARAM param3)
{
	WindowList* list = (WindowList*)param3;
	SORTPARAM p = {(INT)param1,(INT)param2};
	return list->m_sortProc.call(&p);
}

bool WindowList::sort() const
{
	return ListView_SortItems(m_hwnd,FNLVCOMPARE,this)!=0;
}

UINT WindowList::getItemStat(int item,UINT mask) const
{
	return ListView_GetItemState(getWnd(),item,mask)!=0;
}
LPARAM WindowList::getItemParam(INT item) const
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.lParam = NULL;
	ListView_GetItem(getWnd(),&lvi);
	return lvi.lParam;
}
int WindowList::setItemParam(INT item,LPARAM data) const
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)data;
	return ListView_SetItem(getWnd(),&lvi);
}
int WindowList::hitTest(LVHITTESTINFO* pHitTestInfo ) const
{
	return (int)sendMessage(LVM_HITTEST,0,(LPARAM)pHitTestInfo);
}
INT WindowList::hitTest(INT x,INT y) const
{
	LVHITTESTINFO lvhit;
	lvhit.flags = LVHT_ABOVE|LVHT_TOLEFT;
	lvhit.pt.x = x;
	lvhit.pt.y = y;
	return ListView_HitTest(getWnd(),&lvhit);
}
int WindowList::hitTest(LPPOINT pPoint) const
{
	LVHITTESTINFO hitTest;
	hitTest.flags = LVHT_ABOVE|LVHT_TOLEFT;
	hitTest.pt = *pPoint;
	return ListView_HitTest(m_hwnd,&hitTest);
}
int WindowList::hitTestSub(LVHITTESTINFO* pHitTestInfo ) const
{
	return ListView_SubItemHitTest(m_hwnd,pHitTestInfo);
}

bool WindowList::ensureVisible( int nItem, bool bPartialOK) const
{
	return ListView_EnsureVisible(m_hwnd,nItem,bPartialOK)!=0;
}
bool WindowList::scroll(int nX,int nY) const
{
	return ListView_Scroll(m_hwnd,nX,nY)!=0;
}
bool WindowList::redrawItems( int nFirst, int nLast ) const
{
	return ListView_RedrawItems(m_hwnd,nFirst,nLast)!=0;
}
bool WindowList::update( int nItem ) const
{
	return ListView_Update(m_hwnd,nItem)!=0;
}
bool WindowList::arrange( UINT nCode ) const
{
	return ListView_Arrange(m_hwnd,nCode)!=0;
}
bool WindowList::getEditText(WString& dest) const
{
	HWND hwnd = (HWND)ListView_GetEditControl(getWnd());;
	if(!hwnd)
		return false;

	WCHAR buff[1024];
	GetWindowTextW(hwnd,buff,sizeof(buff));
	dest = buff;
	return true;
}
HWND WindowList::editLabel(int nItem ) const
{
	return ListView_EditLabel(m_hwnd,nItem);
}
bool WindowList::deleteColumn( int nCol ) const
{
	if(nCol==-1)
	{
		while(deleteColumn(0));
		return true;
	}
	return ListView_DeleteColumn(getWnd(),nCol)!=0;
}

HIMAGELIST WindowList::createDragImage( int nItem, LPPOINT lpPoint ) const
{
	return ListView_CreateDragImage(m_hwnd,nItem,lpPoint);
}
INT WindowList::getColumnCount()const
{
	return Header_GetItemCount(ListView_GetHeader(getWnd()));
}
int WindowList::getColumnWidth(int nCol) const 
{
	return ListView_GetColumnWidth(m_hwnd,nCol);
}
bool WindowList::setColumnWidth(int nCol,int nWidth) const
{
	return ListView_SetColumnWidth(m_hwnd,nCol,nWidth)!=0;
}

INT WindowList::getItemCount() const
{
	return ListView_GetItemCount(m_hwnd);
}
void WindowList::selectItem(INT index) const
{
	if(index >= 0)
	{
		ListView_SetItemState(getWnd(),index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED); 
	}
	else
	{
		//全ての選択を解除
		INT i;
		INT count = getItemCount();
		for(i=0;i<count;i++)
		{
			ListView_SetItemState(getWnd(),i,0,LVIS_SELECTED|LVIS_FOCUSED); 
		}
	}
}
INT WindowList::getSelectItemCount() const
{
	INT i;
	INT index = -1;
	for(i=0;(index = getSelectItem(index)) != -1;i++);
	return i;
}
INT WindowList::getSelectItem(INT index) const
{
	return ListView_GetNextItem(getWnd(),index,LVNI_SELECTED);
}
//--------------------------------
//アイテムの挿入
//--------------------------------
int WindowList::insertItem(int nItem, LPCSTR lpszItem ) const
{
	return insertItem(nItem,UCS2(lpszItem));
}
int WindowList::insertItem(int nItem, LPCWSTR lpszItem ) const
{
	LV_ITEMW lvItem;
	lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
	lvItem.pszText = (LPWSTR)lpszItem;
	lvItem.iImage = 0;
	if(nItem < 0)
		lvItem.iItem = getItemCount();
	else
		lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	return (int)sendMessage(LVM_INSERTITEMW,0,(LPARAM)&lvItem);
}
int WindowList::insertItem(LPCWSTR text) const
{
	return insertItem(-1,text);
}
int WindowList::insertItem(LPCSTR text) const
{
	return insertItem(-1,text);
}

//--------------------------------
//カラムの挿入
//--------------------------------
int WindowList::insertColumn(int nCol,LPCSTR lpszColumnHeading,int nFormat,int nWidth,int nSubItem) const
{
	return insertColumn(nCol,UCS2(lpszColumnHeading),nFormat,nWidth,nSubItem);
}
int WindowList::insertColumn(int nCol,LPCWSTR lpszColumnHeading,int nFormat,int nWidth,int nSubItem) const
{
	LV_COLUMNW lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = nFormat;
	lvColumn.cx = nWidth;
	lvColumn.pszText = (LPWSTR)lpszColumnHeading;
	lvColumn.iSubItem = nSubItem;

	if(nCol == -1)
		nCol = getColumnCount();

	if(nWidth < 0)
		lvColumn.cx = getStringWidth(lpszColumnHeading)+16;
	return (int)sendMessage(LVM_INSERTCOLUMNW, (WPARAM)nCol, (LPARAM)&lvColumn);
}
int WindowList::insertColumn(LPCSTR lpszColumnHeading) const
{
	return insertColumn(-1,lpszColumnHeading);
}
int WindowList::insertColumn(LPCSTR lpszColumnHeading,INT width) const
{
	return insertColumn(-1,lpszColumnHeading,LVCFMT_LEFT,width);
}
int WindowList::insertColumn(LPCWSTR lpszColumnHeading) const
{
	return insertColumn(-1,lpszColumnHeading);
}
int WindowList::insertColumn(LPCWSTR lpszColumnHeading,INT width) const
{
	return insertColumn(-1,lpszColumnHeading,LVCFMT_LEFT,width);
}
int WindowList::findItem(LPARAM param) const
{
	LVFINDINFOA info;
	ZeroMemory(&info,sizeof(info));
	info.flags = LVFI_PARAM;
	info.lParam = param;
	return (int)sendMessage(LVM_FINDITEMA,(WPARAM)-1,(LPARAM)&info);
}

int WindowList::findItem(LPCSTR itemName,int nStart) const
{
	LVFINDINFOA info;
	ZeroMemory(&info,sizeof(info));
	info.lParam = LVFI_STRING;
	info.psz = itemName;
	return findItem(&info,nStart);
}
DWORD WindowList::onSize(Message* message)
{
	setColumnWidth();
	return 0;
}
void WindowList::createItems(INT count)const
{
	INT i;
	deleteAllItems();
	for(i=0;i<count;i++)
		insertItem(i,L"");
}
void WindowList::setColumnWidth() const
{
	INT count = getColumnCount();
	if(count)
	{
		INT i;
		RECT rect;
		getClientRect(&rect);
		for(i=0;i<count-1;i++)
			rect.right -= getColumnWidth(i);
		if(rect.right < 0)
			rect.right = 0;

		if(rect.right)
			setColumnWidth(count-1,rect.right);
	}
}
COLORREF WindowList::getTextColor()
{
	return ListView_GetTextColor(getWnd());
}
void WindowList::setTextColor(COLORREF color)
{
	ListView_SetTextColor(getWnd(),color);
}
COLORREF WindowList::getBackColor()
{
	return ListView_GetBkColor(getWnd());
}
void WindowList::setBackColor(COLORREF color)
{
	ListView_SetTextBkColor(getWnd(),color);
	ListView_SetBkColor(getWnd(),color);
}

DWORD WindowList::onCustom(Message* m)
{
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)m->getLParam();


	if(cd->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		m->setResult(CDRF_NOTIFYSUBITEMDRAW);
		m->setDefault(false);
	}
	else if(cd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		m->setResult(CDRF_NOTIFYSUBITEMDRAW);
		m->setDefault(false);
	}
	else if(cd->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT|CDDS_SUBITEM))
	{
		INT item = (INT)cd->nmcd.lItemlParam;//dwItemSpec;
		INT subItem = (INT)cd->iSubItem;
		std::map<std::pair<INT,INT>,ITEMPARAM>::iterator it;
		it = m_itemParam.find(std::pair<INT,INT>(item,subItem));

		cd->clrText =  getTextColor();
		cd->clrTextBk =  getBackColor();
		if(it != m_itemParam.end())
		{
			if(it->second.text != -1)
				cd->clrText = it->second.text;
			if(it->second.back != -1)
				cd->clrTextBk = it->second.text;
			//if(it->second.font)
			//	cd->clrTextBk = it->second.text;
			m->setResult(CDRF_NOTIFYITEMDRAW);
			m->setDefault(false);
		}

	}

	return 0;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowVList
// 仮想リスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowVList::WindowVList()
{
	setClassName(WC_LISTVIEW);
	setStyle(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA);
	setExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	addMessage(WM_CREATE,CLASSPROC(this,WindowVList,onCreate));
	addMessage(WM_KEYDOWN,CLASSPROC(this,WindowVList,onKeydown));
	addMessage(WM_SIZE,CLASSPROC(this,WindowVList,onSize));
	addNotify(LVN_GETDISPINFOA,CLASSPROC(this,WindowVList,onDispInfo));
	addNotify(LVN_GETDISPINFOW,CLASSPROC(this,WindowVList,onDispInfoW));
}



bool WindowVList::deleteColumn( int nCol)
{
	if(nCol==-1)
	{
		while(deleteColumn(0));
		return true;
	}
	return ListView_DeleteColumn(getWnd(),nCol)!=0;
}

void WindowVList::createColumn(INT count)
{
	deleteColumn();
	INT i;
	for(i=0;i<count;i++)
		insertColumn(-1,L"",LVCFMT_LEFT,64);
}
void WindowVList::setColumnWidth()const
{
	INT count = getColumnCount();
	if(count)
	{
		INT i;
		RECT rect;
		getClientRect(&rect);
		for(i=0;i<count-1;i++)
			rect.right -= getColumnWidth(i);
		if(rect.right < 0)
			rect.right = 0;

		setColumnWidth(count-1,rect.right);
	}
}
DWORD WindowVList::setExtendedStyle( DWORD dwNewStyle )
{
	m_extendedStyle = dwNewStyle;
	return ListView_SetExtendedListViewStyle(getWnd(),dwNewStyle);
}
int WindowVList::getStringWidth(LPCSTR lpszString)
{
	return (INT)SendMessage(getWnd(),LVM_GETSTRINGWIDTHA,0,(LPARAM)lpszString);
}
int WindowVList::getStringWidth(LPCWSTR lpszString)
{
	return (INT)SendMessage(getWnd(),LVM_GETSTRINGWIDTHW,0,(LPARAM)lpszString);
}
INT WindowVList::getItemCount()const
{
	return ListView_GetItemCount(getWnd());
}
INT WindowVList::getColumnCount()const
{
	return Header_GetItemCount(ListView_GetHeader(getWnd()));
}
void WindowVList::setItemCount(INT count) const
{
	ListView_SetItemCount( getWnd(),count);
}
void WindowVList::setDisplay(ClassProc& proc)
{
	m_callDisplay = proc;
}
//--------------------------------
//カラムの挿入
//--------------------------------
int WindowVList::insertColumn(LPCSTR lpszColumnHeading)
{
	return insertColumn(-1,lpszColumnHeading);
}
int WindowVList::insertColumn(LPCSTR lpszColumnHeading,INT width)
{
	return insertColumn(-1,lpszColumnHeading,LVCFMT_LEFT,width);
}
int WindowVList::insertColumn(int nCol,LPCSTR lpszColumnHeading,int nFormat,int nWidth,int nSubItem)
{
	LV_COLUMNA lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = nFormat;
	lvColumn.cx = nWidth;
	lvColumn.pszText = (LPSTR)lpszColumnHeading;
	lvColumn.iSubItem = nSubItem;
	
	if(nWidth == -10)
		lvColumn.cx = getStringWidth(lpszColumnHeading)+16;
	if(nCol == -1)
		nCol = getColumnCount();
	return (INT)SendMessage(getWnd(),LVM_INSERTCOLUMNA ,nCol,(LPARAM) &lvColumn);
}
int WindowVList::insertColumn(int nCol,LPCWSTR lpszColumnHeading,int nFormat,int nWidth,int nSubItem)
{
	LV_COLUMNW lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = nFormat;
	lvColumn.cx = nWidth;
	lvColumn.pszText = (LPWSTR)lpszColumnHeading;
	lvColumn.iSubItem = nSubItem;
	
	if(nWidth == -10)
		lvColumn.cx = getStringWidth(lpszColumnHeading)+16;
	if(nCol == -1)
		nCol = getColumnCount();
	return (INT)SendMessage(getWnd(),LVM_INSERTCOLUMNW ,nCol,(LPARAM) &lvColumn);
}
void WindowVList::onDispInfo(Message* m)
{
	LV_DISPINFOA* dispInfo = (LV_DISPINFOA*)m->getLParam();
	if(!(dispInfo->item.mask & LVIF_TEXT))
		return;
	VItem item;
	item.item = dispInfo->item.iItem;
	item.subItem = dispInfo->item.iSubItem;

	if(m_callDisplay.isAddress())
		m_callDisplay.call(&item);
	INT length = (int)item.value.length() + 1;
	if(length >= dispInfo->item.cchTextMax)
		length = dispInfo->item.cchTextMax-1;

	strncpy(dispInfo->item.pszText,item.value.c_str(),length);
}
void WindowVList::onCreate(Message* message)
{
	setExtendedStyle(m_extendedStyle);
}
void WindowVList::onKeydown(Message* message)
{
	if(message->getWParam() == 'A' && GetKeyState(VK_CONTROL))
	{
		selectItem();
	}
}
void WindowVList::onDispInfoW(Message* m)
{
	LV_DISPINFOW* dispInfo = (LV_DISPINFOW*)m->getLParam();
	if(!(dispInfo->item.mask & LVIF_TEXT))
		return;
	VItemW item;
	item.item = dispInfo->item.iItem;
	item.subItem = dispInfo->item.iSubItem;

	if(m_callDisplay.isAddress())
		m_callDisplay.call(&item);

	wcsncpy(dispInfo->item.pszText,item.value.c_str(),dispInfo->item.cchTextMax);
}
void WindowVList::updateItem(INT index) const
{
	ListView_Update(getWnd(),index);
}
void WindowVList::redrawItem(INT index1,INT index2)
{
	if(index2 == -2)
		index2 = index1;
	if(index1 == -1)
	{
		index1 = 0;
		index2 = getItemCount();
	}
	ListView_RedrawItems(getWnd(),index1,index2);
}
bool WindowVList::setColumnWidth(int nCol,int nWidth) const
{
	return ListView_SetColumnWidth(getWnd(),nCol,nWidth)!=0;
}

int WindowVList::getColumnWidth(int nCol) const
{
	return ListView_GetColumnWidth(getWnd(),nCol);
}
COLORREF WindowVList::getTextColor()
{
	return ListView_GetTextColor(getWnd());
}
void WindowVList::setTextColor(COLORREF color)
{
	ListView_SetTextColor(getWnd(),color);
}
COLORREF WindowVList::getBackColor()
{
	return ListView_GetBkColor(getWnd());
}
void WindowVList::setBackColor(COLORREF color)
{
	ListView_SetTextBkColor(getWnd(),color);
	ListView_SetBkColor(getWnd(),color);
}
INT WindowVList::getSelectItem(INT index) const
{
	return ListView_GetNextItem(getWnd(),index,LVNI_SELECTED);
}
INT WindowVList::hitTestSub(LVHITTESTINFO* pHitTestInfo )
{
	return ListView_SubItemHitTest(getWnd(),pHitTestInfo);
}
INT WindowVList::hitTest(INT x,INT y)
{
	LVHITTESTINFO lvhit;
	lvhit.flags = LVHT_ABOVE|LVHT_TOLEFT;
	lvhit.pt.x = x;
	lvhit.pt.y = y;
	return ListView_HitTest(getWnd(),&lvhit);
}
void WindowVList::selectItem(INT index,bool flag)
{
	if(index >= 0)
	{
		if(flag)
		{
			ListView_SetItemState(getWnd(),index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
		}
		else
		{
			ListView_SetItemState(getWnd(),index,0,LVIS_SELECTED|LVIS_FOCUSED);
		}
	}
	else
	{
		//全ての選択を解除
		INT i;
		INT count = getItemCount();
		for(i=0;i<count;i++)
		{
			if(flag)
			{
				ListView_SetItemState(getWnd(),i,LVIS_SELECTED,LVIS_SELECTED);
			}
			else
			{
				ListView_SetItemState(getWnd(),i,0,LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
	}
}
int WindowVList::getNextItem(INT index)
{
	return ListView_GetNextItem(getWnd(),index,0);
}
UINT WindowVList::getItemStat(int item,UINT mask)
{
	 return ListView_GetItemState(getWnd(),item,mask);
}
void WindowVList::setUnicode(bool flag) const
{
	ListView_SetUnicodeFormat(getWnd(),flag);
}
DWORD WindowVList::onSize(Message* message)
{
	setColumnWidth();
	return 0;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowPanel
// パネル
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowPanel::WindowPanel()
{
	setWindowSize(32,32);
	setStyleEx(WS_EX_WINDOWEDGE);
	setBackColor(GetSysColor(COLOR_BTNFACE));
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowTab
// タブウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowTab::WindowTab()
{
	setStyle(WS_CHILD | WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
	setClassName(WC_TABCONTROLW);
	addMessage(WM_CREATE,CLASSPROC(this,WindowTab,onCreate));
	addNotify(TCN_SELCHANGE,CLASSPROC(this,WindowTab,onSelchanges));
}
void WindowTab::getAjustRect(LPRECT rect)
{
	sendMessage(TCM_ADJUSTRECT , (WPARAM)false, (LPARAM)rect);
}
void WindowTab::recalcLayout(RECT& rect)
{
	//クライアント表示領域の取得
	getAjustRect(&rect);
	Window::recalcLayout(rect);
}
INT WindowTab::insertItem(INT index,LPCSTR text,Window& window)
{
	if(!window.isWindow())
		window.createChildWindow(getWnd());
	return insertItem(index,text,window.getWnd());
}
INT WindowTab::insertItem(INT index,LPCSTR text,HWND hwnd)
{
	TC_ITEMA item;
	item.mask = TCIF_TEXT|TCIF_PARAM;
	item.pszText = (LPSTR)text;
	item.lParam = (LPARAM)hwnd;
	index = (INT)sendMessage(TCM_INSERTITEMA, (WPARAM)index, (LPARAM)&item);
	if(index == -1)
		return index;
	showTab();
	Window::recalcLayout();
	return index;
}

void WindowTab::showTab()
{
	INT index = TabCtrl_GetCurSel(getWnd());
	HWND select = (HWND)getParam(index);
	INT count = getItemCount();
	INT i;
	//選択タブを表示
	for(i=0;i<count;i++)
	{
		HWND hwnd = (HWND)getParam(i);
		if(hwnd == select)
		{
			ShowWindow(hwnd,SW_SHOW);
			SetFocus(hwnd);
		}
		else
			ShowWindow(hwnd,SW_HIDE);
	}
	Window::recalcLayout();
}
void WindowTab::onSelchanges(LPNMHDR nmHDR)
{
	showTab();
}
void WindowTab::onCreate(Message* message)
{

}
bool WindowTab::selectItem(INT index)
{
	return sendMessage(TCM_SETCURFOCUS,index,0) != 0;
}
INT WindowTab::getCurSel() 
{
	return (INT)sendMessage(TCM_GETCURSEL,0,0);
}
INT WindowTab::getItemCount()
{
	return (INT)sendMessage(TCM_GETITEMCOUNT,0,0);
}
void WindowTab::setParam(INT index,LPVOID data)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = (LPARAM)data;
	sendMessage(TCM_SETITEM,index,(LPARAM)&item);
}
LPARAM WindowTab::getParam(INT index)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = 0;
	sendMessage(TCM_GETITEM,index,(LPARAM)&item);
	return item.lParam;
}
bool WindowTab::setItemText(INT index,LPCSTR value)
{
	TCITEMA item;
	item.mask = TCIF_TEXT;
	item.pszText = (LPSTR)value;
	return sendMessage(TCM_SETITEMA,index,(LPARAM)&item) != 0;
}
bool WindowTab::getName(INT index,std::string& desc)
{
	TCITEMA item;
	item.mask = TCIF_TEXT;
	item.pszText = NULL;
	if(!sendMessage(TCM_GETITEMCOUNT,index,(LPARAM)&item))
		return NULL;
	item.pszText = new CHAR[item.cchTextMax];
	sendMessage(TCM_GETITEMA,index,(LPARAM)&item);
	desc = item.pszText;
	delete item.pszText;
	return true;

}
bool WindowTab::deleteItem(HWND wndDel)
{
	INT count = getItemCount();
	INT i;
	//選択タブを表示
	for(i=0;i<count;i++)
	{
		HWND hwnd = (HWND)getParam(i);
		if(hwnd == wndDel)
		{
			deleteItem(i);
			showTab();
		}

	}
	return true;
}
bool WindowTab::deleteItem(INT index)
{
	if(index == -1)
	{
		while(sendMessage(TCM_DELETEITEM,0,0));
		return true;
	}
	return sendMessage(TCM_DELETEITEM,index,0) != 0;
}
void WindowTab::setTabSize(INT width,INT height)
{
	LOGFONTW logFont;
	ZeroMemory(&logFont,sizeof(logFont));
	logFont.lfHeight = height-3;
	wcscpy(logFont.lfFaceName,L"MS UI Gothic");
	HFONT newFont = ::CreateFontIndirectW(&logFont);
	sendMessage(WM_SETFONT,(WPARAM)newFont,true);
	
	TabCtrl_SetItemSize(getWnd(),width,height);

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WButton
// ボタン
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// WButton::WButton()
// -----  動作  -----
// ボタンクラスのコンストラクタ
// -----  引数  -----
// 無し
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
WButton::WButton()
{
	setWindowSize(64,32);
	setStyle(WS_TABSTOP|BS_PUSHBUTTON);
	setClassName(L"BUTTON");
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowCombo
// コンボ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowCombo::WindowCombo()
{
	addEvent(CBN_DROPDOWN,CLASSPROC(this,WindowCombo,onDropDown));
	addMessage(WM_CREATE,CLASSPROC(this,WindowCombo,onCreate));
	setStyle(WS_POPUP|WS_VISIBLE|CBS_HASSTRINGS|CBS_DROPDOWN);
	setClassName(L"COMBOBOX");
}
bool WindowCombo::insertItem(LPCWSTR value)
{
	return sendMessage(CB_INSERTSTRING, -1, (LPARAM)value) != CB_ERR;
}
bool WindowCombo::insertItem(LPCSTR value)
{
	return insertItem(UCS2(value));
}
bool WindowCombo::insertItem(INT index,LPCWSTR value)
{
	return sendMessage(CB_INSERTSTRING, index, (LPARAM)value) != CB_ERR;
}
bool WindowCombo::insertItem(INT index,LPCSTR value)
{
	return insertItem(index,UCS2(value));
}
bool WindowCombo::clear()
{
	return sendMessage(CB_RESETCONTENT) != CB_ERR;
}
bool WindowCombo::deleteItem(INT index)
{
	return sendMessage(CB_DELETESTRING , index,0) != CB_ERR;
}
bool WindowCombo::getItem(INT index,WString& value)
{
	INT length = (INT)sendMessage(CB_GETLBTEXTLEN , index);
	if(length >= 0)
	{
		WCHAR* buff = new WCHAR[length+1];
		sendMessage(CB_GETLBTEXT  , index,(LPARAM)buff);
		value = buff;
		delete[] buff;
		return true;
	}
	return false;
}
bool WindowCombo::getItem(INT index,String& value)
{
	INT length = (INT)sendMessage(CB_GETLBTEXTLEN , index);
	if(length >= 0)
	{
		WCHAR* buff = new WCHAR[length+1];
		sendMessage(CB_GETLBTEXT  , index,(LPARAM)buff);
		value = buff;
		delete[] buff;
		return true;
	}
	return false;
}
bool WindowCombo::getItem(WString& value)
{
	INT index = getSelectItemIndex();
	INT length = (INT)sendMessage(CB_GETLBTEXTLEN , index);
	if(length >= 0)
	{
		WCHAR* buff = new WCHAR[length+1];
		sendMessage(CB_GETLBTEXT  , index,(LPARAM)buff);
		value = buff;
		delete[] buff;
		return true;
	}
	return false;
}
bool WindowCombo::getItem(String& value)
{
	INT index = getSelectItemIndex();
	INT length = (INT)sendMessage(CB_GETLBTEXTLEN , index);
	if(length >= 0)
	{
		WCHAR* buff = new WCHAR[length+1];
		sendMessage(CB_GETLBTEXT  , index,(LPARAM)buff);
		value = buff;
		delete[] buff;
		return true;
	}
	return false;
}
INT WindowCombo::findItem(LPCSTR value)
{
	return (INT)sendMessage(CB_FINDSTRING , -1, (LPARAM)value);
}
bool WindowCombo::selectItem(INT index)
{
	return sendMessage(CB_SETCURSEL, index, 0) != CB_ERR;
}
INT WindowCombo::getItemCount()
{
	return (INT)sendMessage(CB_GETCOUNT);
}
HWND WindowCombo::getEdit()
{
	HWND hwnd = GetWindow(getWnd(),GW_CHILD);
	while(hwnd)
	{
		CHAR buff[512];
		GetClassNameA(hwnd,buff,512);
		if(strcmp(buff,"Edit") == 0)
			return hwnd;
		hwnd = GetWindow(hwnd,GW_HWNDNEXT);
	}
	return NULL;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowListEdit
// 編集可能リストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

WindowListEdit::WindowListEdit()
{
	m_item = -1;
	addMessage(WM_LBUTTONDOWN,CLASSPROC(this,WindowListEdit,onLButtonDown));
	m_edit.addMessage(WM_CHAR,CLASSPROC(this,WindowListEdit,onEditChar));
	m_edit.addMessage(WM_KILLFOCUS,CLASSPROC(this,WindowListEdit,onEditKillFocus));
}
void WindowListEdit::setEditColumn(INT index)
{
	m_editIndex.insert(index);
}
void WindowListEdit::setValue()
{
	if(m_edit.isWindow())
	{
		String value;
		m_edit.getTitle(value);
		setItemText(m_item,m_itemSub,value.c_str());
	}
}
void WindowListEdit::onEditChar(Message* m)
{
	INT code = (INT)m->getWParam();
	if(code == VK_RETURN)
	{
		onEditKillFocus(NULL);
	}
	else if(code == VK_ESCAPE)
	{
		m_item = -1;
		onEditKillFocus(NULL);
	}
}
void WindowListEdit::onEditKillFocus(Message* m)
{
	if(m_edit.isWindow())
	{
		m_edit.showWindow(SW_HIDE);
		if(m_item != -1)
		{
			String value;
			m_edit.getTitle(value);

			CHAR data[1024];
			getItemText(m_item,m_itemSub,data,sizeof(data));
			if(value != data)
			{
				setItemText(m_item,m_itemSub,value.c_str());
				sendMessage(WM_ITEM_UPDATE,m_item,m_itemSub);
			}
		}
	}
}
void WindowListEdit::onLButtonDown(Message* m)
{
	m->setDefault(false);
	SetFocus(NULL);

	INT x = m->getX();
	INT y = m->getY();


	LVHITTESTINFO lvhit;
	lvhit.flags = LVHT_ABOVE|LVHT_TOLEFT;
	lvhit.pt.x = x;
	lvhit.pt.y = y;
	INT item = hitTestSub(&lvhit);
	INT itemSub = lvhit.iSubItem;

	if(item > -1 && m_editIndex.size())
	{
		if(m_editIndex.find(itemSub) == m_editIndex.end())
		{
			itemSub = *m_editIndex.begin();
		}
		onEditItem(item,itemSub);
		m_item = item;
		m_itemSub = itemSub;
	}
}
void WindowListEdit::onEditItem(INT item,INT itemSub)
{
	RECT rect;
	if(!m_edit.isWindow())
	{
		m_edit.createWindow(L"EDIT",L"",WS_CHILD |WS_VISIBLE|ES_AUTOHSCROLL ,getWnd());
		LRESULT hfont = sendMessage(WM_GETFONT,0,0);
		m_edit.sendMessage(WM_SETFONT,hfont,true);
	}
	getSubItemRect(item,itemSub,LVIR_LABEL,&rect);
	if(itemSub == 0)
		rect.left += 2;
	else
		rect.left += 6;

	
	CHAR buff[1024];
	getItemText(item,itemSub,buff,sizeof(buff));
	m_edit.setTitle(buff);
	m_edit.moveWindow(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
	m_edit.showWindow();
	SetFocus(m_edit.getWnd());

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(*this,&p);
	m_edit.sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x-rect.left,p.y-rect.top));
	m_edit.sendMessage(WM_LBUTTONUP,0,MAKELONG(p.x-rect.left,p.y-rect.top));
	m_edit.sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x-rect.left,p.y-rect.top));
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowTree
// ツリーウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WindowTree::WindowTree()
{
	setClassName(WC_TREEVIEW);
	setStyle(WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);


}

bool WindowTree::delItemChildren(HTREEITEM parent)
{
	HTREEITEM item = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)parent);
	while(item)
	{
		HTREEITEM itemNext = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item);
		delItem(item);
		item = itemNext;
	}
	return true;
}
bool WindowTree::delItem(HTREEITEM item)
{
	return sendMessage(TVM_DELETEITEM, 0, (LPARAM)item) != 0;
}

HTREEITEM WindowTree::insertItem(HTREEITEM parent,LPCSTR text,bool expand)
{
	return insertItem(parent,UCS2(text),expand);
}
HTREEITEM WindowTree::insertItem(HTREEITEM parent,LPCWSTR text,bool expand)
{
	TV_INSERTSTRUCTW tv;
	ZeroMemory(&tv,sizeof(tv));
	tv.hParent = parent;
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_STATE;
	tv.item.pszText = (LPWSTR)text;
	tv.item.stateMask = TVIS_EXPANDED;
	if(expand)
	{
		tv.item.state = TVIS_EXPANDED;
	}

	return (HTREEITEM)sendMessage(TVM_INSERTITEMW, 0, (LPARAM)&tv);
}
void WindowTree::setItemStat(HTREEITEM item,UINT stat,UINT statMask)
{
	TreeView_SetItemState(*this,item,stat,statMask);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Menu
// メニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Menu::Menu()
{
	m_menu = CreatePopupMenu();
}
Menu::~Menu()
{
	DestroyMenu(m_menu);
}
Menu::operator HMENU() const
{
	return m_menu;
}
bool Menu::append(UINT id) const
{
	return AppendMenuA(m_menu,MF_SEPARATOR,id,NULL) != FALSE;
}
bool Menu::append(UINT id,LPCSTR text) const
{
	return AppendMenuA(m_menu,MF_STRING,id,text) != FALSE;
}
bool Menu::append(UINT id,LPCWSTR text) const
{
	return AppendMenuW(m_menu,MF_STRING,id,text) != FALSE;
}
void Menu::show(HWND hwnd)
{
	POINT point;
	GetCursorPos(&point);
	show(point.x,point.y,hwnd);
}
void Menu::show(INT x,INT y,HWND hwnd)
{
	POINT p = {x,y};
	if(getCount())
		TrackPopupMenu(m_menu,0,p.x,p.y,0,hwnd,NULL);
}
INT Menu::getCount() const
{
	return GetMenuItemCount(m_menu);
}
void Menu::clear()
{
	//メニューの作り直し
	DestroyMenu(m_menu);
	m_menu = CreatePopupMenu();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TaskIcon
// タスクアイコン制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
TaskIcon::TaskIcon()
{
	ZeroMemory(&m_icn,sizeof(NOTIFYICONDATA));
	m_icn.cbSize = sizeof(NOTIFYICONDATA);
}
TaskIcon::~TaskIcon()
{
	hide();
}
bool TaskIcon::create(HWND hWnd,HICON hIcon,LPCSTR title,UINT code,UINT msg)
{
	WString name;
	name = title;
	m_icn.hWnd = hWnd;
	m_icn.uID = code;
	m_icn.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_icn.uCallbackMessage = msg;
	m_icn.hIcon = hIcon;
	wcscpy(m_icn.szTip,name);
	return true;
}
void TaskIcon::show()
{
	Shell_NotifyIconW( NIM_ADD, &m_icn);
}
void TaskIcon::hide()
{
	Shell_NotifyIconW( NIM_DELETE, &m_icn);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PropertyView
// プロパティ表示用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void PropertyView::setCallback(INT item,INT subItem,ClassProc& classProc)
{
	m_control[item][subItem] = classProc;
}
ClassProc* PropertyView::getCallback(INT item,INT subItem)
{
	std::map<INT,std::map<INT,ClassProc> >::iterator it1;
	it1 = m_control.find(item);
	if(it1 == m_control.end())
	{
		it1 = m_control.find(-1);
		if(it1 == m_control.end())
			return NULL;
	}
	std::map<INT,ClassProc>::iterator it2;
	it2 = it1->second.find(subItem);
	if(it2 == it1->second.end())
	{
		it2 = it1->second.find(-1);
		if(it2 == it1->second.end())
			return NULL;
	}
	ClassProc* classProc = &it2->second;
	return classProc;
}
INT PropertyView::getClickItem()
{
	return m_item;
}
INT PropertyView::getClickSubItem()
{
	return m_subItem;
}
void PropertyView::addMenu(LPCSTR name,ClassProc& classProc,bool selected)
{
	m_menuList.push_back(PropertyMenu(name,classProc,selected));
}
void PropertyView::addMenu(LPCWSTR name,ClassProc& classProc,bool selected)
{
	m_menuList.push_back(PropertyMenu(name,classProc,selected));
}
void PropertyView::edit(INT item,INT subItem)
{
	postMessage(WM_PROPERTY_EDITSELECT,item,subItem);
}
void PropertyView::closeEdit()
{
	if(m_editWindow)
	{
		m_editWindow->closeWindow();
		delete m_editWindow;
		m_editWindow = NULL;
	}
}
PropertyMenu::PropertyMenu(LPCWSTR name,ClassProc& classProc,bool select)
{
	m_name = name;
	m_classProc = classProc;
	m_select = select;
}
PropertyMenu::PropertyMenu(LPCSTR name,ClassProc& classProc,bool select)
{
	m_name = name;
	m_classProc = classProc;
	m_select = select;
}
LPCWSTR PropertyMenu::getName()const
{
	return m_name;
}
bool PropertyMenu::isSelect()const
{
	return m_select;
}
ClassProc& PropertyMenu::getClassProc()
{
	return m_classProc;
}



PropertyData::PropertyData()
{
	//初期カラム項目
	m_columnName.push_back(std::pair<WString,INT>(L"Name",128));
	m_columnName.push_back(std::pair<WString,INT>(L"Value",100));
}
void PropertyData::set(INT index,ClassProc& classProc)
{
	WString work;
	work.printf(L"%d",index);
	m_property[work][0] = classProc;
	m_propertyList.push_back(work);
}
void PropertyData::set(LPCSTR name,ClassProc& classProc)
{
	m_property[name][0] = classProc;
	m_propertyList.push_back(name);
}
void PropertyData::set(LPCWSTR name,ClassProc& classProc)
{
	m_property[name][0] = classProc;
	m_propertyList.push_back(name);
}
void PropertyData::set(INT index1,INT index2,ClassProc& classProc)
{
	WString name;
	name.printf(L"%d",index1);
	if(m_property.find(name) == m_property.end())
		m_propertyList.push_back(name);
	m_property[name][index2] = classProc;
}
void PropertyData::set(LPCSTR name,INT index,ClassProc& classProc)
{
	if(m_property.find(name) == m_property.end())
		m_propertyList.push_back(name);
	m_property[name][index] = classProc;
}
void PropertyData::set(LPCWSTR name,INT index,ClassProc& classProc)
{
	if(m_property.find(name) == m_property.end())
		m_propertyList.push_back(name);
	m_property[name][index] = classProc;
}
std::list<WString>& PropertyData::List()
{
	return m_propertyList;
}
std::map<WString,std::map<INT,ClassProc> >& PropertyData::Property()
{
	return m_property;
}
std::list<std::pair<WString,INT> >& PropertyData::Column()
{
	return m_columnName;
}

void PropertyData::clearData()
{
	m_propertyList.clear();
	m_property.clear();
}
void PropertyData::addMenu(LPCSTR name,ClassProc& classProc,bool select)
{
	m_menu.push_back(PropertyMenu(name,classProc,select));
}

void PropertyData::addMenu(LPCWSTR name,ClassProc& classProc,bool select)
{
	m_menu.push_back(PropertyMenu(name,classProc,select));
}
PropertyView::PropertyView()
{
	m_directClick = false;
	m_editWindow = NULL;
	m_item = -1;
	m_subItem = -1;
	m_editItem = -1;
	m_editSubItem = -1;
	//リストコントロールのマウスダウン
	addMessage(WM_CREATE,CLASSPROC(this,PropertyView,onCreate));
	addMessage(WM_LBUTTONDOWN,CLASSPROC(this,PropertyView,onLButtonDown));
	addMessage(WM_RBUTTONDOWN,CLASSPROC(this,PropertyView,onRButtonDown));
	addMessage(WM_APP,CLASSPROC(this,PropertyView,onEdit));
	addMessage(WM_APP+1,CLASSPROC(this,PropertyView,onSelect));
	addMessage(WM_PROPERTY_EDITSELECT,CLASSPROC(this,PropertyView,onEditSelect));
	//メニューの通知
	addMessage(WM_COMMAND,CLASSPROC(this,PropertyView,onCommand));

	setTitle("プロパティ");
	//setStyleEx(WS_EX_TOOLWINDOW);

}
PropertyView::~PropertyView()
{
	if(m_editWindow)
		delete m_editWindow;
}

void PropertyView::setProperty(PropertyData* propertyData)
{
	if(!getWnd())
	{
		createWindow();
	}
	m_control.clear();
	//既存のカラムの削除
	while(getColumnCount())
		deleteColumn(0);
	deleteAllItems();
	//プロパティ用ヘッダの作成
	INT i=0;
	std::list<std::pair<WString,INT> >::iterator it;
	for(it = propertyData->Column().begin();it != propertyData->Column().end();++it)
	{
		insertColumn(i++,it->first,0,it->second);
	}
	//最後尾カラムの幅調整
	setColumnWidth();

	std::list<WString>& list = propertyData->List();
	std::list<WString>::iterator itList;
	std::map<WString,std::map<INT,ClassProc> >& pro = propertyData->Property();
	std::map<WString,std::map<INT,ClassProc> >::iterator itPro;

	for(itList=list.begin();itList!=list.end();itList++)
	{
		INT index = insertItem(-1,*itList);
		
		std::map<INT,ClassProc>::iterator itProc;
		for(itProc = pro[*itList].begin();itProc != pro[*itList].end();++itProc)
		{
			PropertyMessage pm;
			pm.stat = PROPERTY_GET;
			pm.item = index;
			setCallback(index,itProc->first+1,itProc->second);
		}
	}
	m_menuList = propertyData->m_menu;
	update();
}

void PropertyView::update()
{
//	deleteAllItems();
	if(m_editWindow)
	{
		m_editWindow->closeWindow();
		delete m_editWindow;
		m_editWindow = NULL;
	}
	INT itemCount = getItemCount();
	INT subItemCount = getColumnCount();
	INT i,j;
	for(j=0;j<itemCount;j++)
	{
		for(i=0;i<subItemCount;i++)
		{
			ClassProc* classProc = getCallback(j,i);
			if(classProc)
			{
				RECT rect={0,0,0,0};
				CHAR buff[1024];
				getItemText(j,i,buff,sizeof(buff));
				PropertyMessage pm = {getWnd(),buff,rect,j,i,PROPERTY_GET,NULL};
				if(classProc->call(&pm))
				{
					setItemText(j,i,pm.value);
				}
			}
		}
	}

}
void PropertyView::onCreate(Message* m)
{
	setExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
}
void PropertyView::onCommand(Message* m)
{
/*	//メニューがクリックされたときの処理
	INT index = (INT)m->getWParam();
	//メニューの登録されたクラスプロシージャの呼び出し
	m_menuList[index].getClassProc().call(&m_item);
*/
}
void PropertyView::onRButtonDown(Message* m)
{
	callDefaultProc(m);
	

	LVHITTESTINFO lvhit;
	lvhit.flags = LVHT_ABOVE|LVHT_TOLEFT;
	lvhit.pt.x = m->getX();
	lvhit.pt.y = m->getY();
	INT item = hitTest(&lvhit);
	m_item = item;
	//メニューの登録
	m_menu.clear();
	std::vector<PropertyMenu>::iterator itMenu;
	INT i;
	INT count = (INT)m_menuList.size();
	for(i=0;i<count;i++)
	{
		bool select = m_menuList[i].isSelect();
		if(!select || item >= 0)
		{
			m_menu.append(i,m_menuList[i].getName());
		}
	}

	//メニュー表示
	POINT point;
	GetCursorPos(&point);
	m_menu.show(point.x,point.y,getWnd());
	m->setDefault(false);
}
void PropertyView::onLButtonDown(Message* m)
{

	INT x = m->getX();
	INT y = m->getY();


	LVHITTESTINFO lvhit;
	lvhit.flags = LVHT_ABOVE|LVHT_TOLEFT;
	lvhit.pt.x = x;
	lvhit.pt.y = y;
	INT item = hitTestSub(&lvhit);
	if(item >= 0 && (m_directClick || item == m_item))
	{
		m->setDefault(false);
		edit(item,lvhit.iSubItem);
		//WindowDebug::DEBUG("EDIT %d\n",item);

	}
	else
	{
		if(m_editWindow)
		{
			m_editWindow->sendMessage(WM_KILLFOCUS);
			m_editWindow->closeWindow();
			delete m_editWindow;
			m_editWindow = NULL;
		}
	}
	m_item = item;
}
void PropertyView::onEditSelect(Message* m)
{
	INT item = (INT)m->getWParam();
	INT subItem = (INT)m->getLParam();

	RECT rect;
	getSubItemRect(item,subItem,LVIR_LABEL,&rect);
	if(subItem > 0)
		rect.left += 6;
	else
		rect.left += 2;
	rect.bottom--;
	onSelect(item,subItem,&rect);
}
void PropertyView::onSelect(INT item,INT subItem,RECT* rect)
{
	if(m_editWindow)
	{
		m_editWindow->closeWindow();
		delete m_editWindow;
		m_editWindow = NULL;
		m_editItem = -1;
		m_editSubItem = -1;
	}
	ClassProc* classProc = getCallback(item,subItem);
	if(classProc)
	{

		CHAR buff[1024];
		if(getItemText(item,subItem,buff,sizeof(buff)))
		{
			PropertyMessage pm = {*this,buff,*rect,item,subItem,PROPERTY_CREATE,NULL};
			classProc->call(&pm);
			m_editWindow = (Window*)pm.param;
			//m_editWindow->addMessage(WM_KILLFOCUS,)
			m_editItem = item;
			m_editSubItem = subItem;
		}
	}
	if(m_editWindow)
		m_editWindow->setFocus();
	m_item = item;
	m_subItem = subItem;
}
void PropertyView::onEdit(Message* m)
{

	if(m_editWindow)
	{
		if(m->getWParam()!=1)
		{
			ClassProc* classProc = getCallback(m_editItem,m_editSubItem);
			if(classProc)
			{
				RECT rect={0,0,0,0};
				WCHAR buff[1024];
				getItemText(m_editItem,m_editSubItem,buff,sizeof(buff));
				PropertyMessage pm = {*this,buff,rect,m_editItem,m_editSubItem,PROPERTY_SET,m_editWindow};
				if(classProc->call(&pm))
				{
					setItemText(m_editItem,m_editSubItem,pm.value);
				}
				//WindowDebug::DEBUG("SET %d\n",m_editItem);
			}
		}
		if(!m_directClick)
			m_editWindow->showWindow(SW_HIDE);
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputCombo
// コンボボックス入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputCombo::InputCombo()
{
	addEvent(CBN_SELCHANGE,CLASSPROC(this,InputCombo,onEditChange));
	addEvent(CBN_KILLFOCUS,CLASSPROC(this,InputCombo,onKillFocus));
}
void InputCombo::add(LPCWSTR value)
{
	m_value.push_back(value);
}
void InputCombo::add(LPCSTR value)
{
	m_value.push_back(value);
}
bool InputCombo::create(HWND hWnd,String& value,RECT& rect)
{
	WString work = value;
	bool f = create(hWnd,work,rect);
	value = work;
	return f;
}
bool InputCombo::create(HWND hWnd,WString& value,RECT& rect)
{
	if(!createWindow(L"COMBOBOX",L"",WS_CHILD |WS_CLIPSIBLINGS|WS_VISIBLE|CBS_HASSTRINGS|CBS_DROPDOWNLIST,hWnd))
		return false;
	HFONT font = (HFONT)SendMessage(hWnd,WM_GETFONT,0,0);
	sendMessage(WM_SETFONT,(WPARAM)font,true);

	INT i = 0;
	INT index = -1;
	std::list<WString>::iterator it;
	for(it=m_value.begin();it!=m_value.end();++it)
	{
		sendMessage(CB_INSERTSTRING, -1, (LPARAM)it->c_str());
		if(*it == value)
			index = i;
		i++;
	}
	if(index > -1)
		sendMessage(CB_SETCURSEL, index, 0);

	moveWindow(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top+120);
	showWindow();
	setFocus();
	sendMessage(CB_SHOWDROPDOWN, true, 0);


	m_quit = false;
	m_ret = true;

	return m_ret;
}
void InputCombo::onEditChange(Message* m)
{
	SendMessage(getParent(),WM_APP,0,0);
	PostMessage(getParent(),WM_APP,1,0);
	SetFocus(getParent());
}
void InputCombo::onKillFocus(Message* m)
{
	
/*	if(isWindow())
	{
		SendMessage(getParent(),WM_APP,0,0);
	}
*/
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFile
// ファイル入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool InputFile::doModal(HWND hwnd,String& fileName)
{
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH];

	strncpy(szFileName,fileName,MAX_PATH);
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "ALL Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "db";
	ofn.lpstrTitle = "データベースファイル";
	ofn.lpstrInitialDir = ".\\";
	if(GetOpenFileNameA(&ofn))
	{
		fileName = szFileName;
		return true;
	}
	return false;
}
bool InputFile::doModal(HWND hwnd,std::wstring& fileName)
{
	OPENFILENAMEW ofn;
	WCHAR szFileName[MAX_PATH];

	wcsncpy(szFileName,fileName.c_str(),MAX_PATH);
	memset(&ofn, 0, sizeof(OPENFILENAMEW)); ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"ALL Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"db";
	ofn.lpstrTitle = L"データベースファイル";
	if(GetOpenFileNameW(&ofn))
	{
		fileName = szFileName;
		return true;
	}
	return false;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFont
// フォント入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputFont::InputFont()
{
	setSubmit(CLASSPROC(this,InputFont,submit));
}
void InputFont::setLogFont(LPLOGFONTW logfont)
{
	m_logfont = *logfont;
}
void InputFont::getLogFont(LPLOGFONTW logfont)
{
	*logfont = m_logfont;
}

bool InputFont::submit(Message* m)
{
	CHOOSEFONTW CHF;
	ZeroMemory(&CHF,sizeof(CHOOSEFONTW));
	CHF.lStructSize = sizeof(CHOOSEFONTW); 
	CHF.hwndOwner = m->getWnd();
	CHF.lpLogFont = &m_logfont;
	CHF.Flags =CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT|CF_NOSCRIPTSEL|CF_FORCEFONTEXIST;
	if(ChooseFontW(&CHF)!=0)
	{
		m_text.setTitle(m_logfont.lfFaceName);
		return true;
	}
	return false;

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputList
// リスト入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputList::InputList()
{
	setSubmit(CLASSPROC(this,InputList,submit));
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputColor
// カラー入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputColor::InputColor()
{
	setSubmit(CLASSPROC(this,InputColor,submit));
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFileText
// ファイル名入力
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputFileText::InputFileText()
{
	setSubmit(CLASSPROC(this,InputFileText,submit));
}
bool InputFileText::submit(Message* m)
{
	String value;
	m_text.getTitle(value);
	if(InputFile::doModal(m->getWnd(),value))
	{
		m_text.setTitle(value);
		return true;
	}
	return false;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputList
// カラー入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool InputList::submit(Message* m)
{
	String value;
	m_text.getTitle(value);

	Window window;
	window.setClassName(WC_LISTVIEW);
	//window.setParent(getWnd());
	window.doModal(getWnd());
	return false;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputColor
// カラー入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool InputColor::submit(Message* m)
{
	String value;
	m_text.getTitle(value);

	static DWORD custColor[16];
	CHOOSECOLOR CHC;
	ZeroMemory(&CHC,sizeof(CHOOSECOLOR));
	CHC.lStructSize = sizeof(CHOOSECOLOR); 
	CHC.hwndOwner = m->getWnd();
	CHC.Flags = CC_RGBINIT|CC_FULLOPEN;
	CHC.lpCustColors = custColor;
	sscanf(value,"%06X",&CHC.rgbResult);

	if(ChooseColor(&CHC))
	{
		value.printf("%06X",CHC.rgbResult);
		m_text.setTitle(value);
		return true;
	}
	return false;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputText
// テキスト入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputText::InputText()
{
	m_pass = false;
	addMessage(WM_CHAR,CLASSPROC(this,InputText,onEditChar));
	addMessage(WM_KILLFOCUS,CLASSPROC(this,InputText,onEditKillFocus));
	m_lastCode = 0;
}
void InputText::setPassword(bool flag)
{
	m_pass = flag;
}
bool InputText::create(HWND hWnd,LPCWSTR value,RECT& rect)
{
	//エディットコントロールの作成
	if(!createWindow(L"EDIT",L"",WS_CHILD |WS_CLIPSIBLINGS|WS_VISIBLE|ES_AUTOHSCROLL,hWnd))
		return false;
	//フォントを親ウインドウに合わせる
	HFONT font = (HFONT)SendMessage(hWnd,WM_GETFONT,0,0);
	sendMessage(WM_SETFONT,(WPARAM)font,true);
	//パスワードの場合は入力をマスク
	if(m_pass)
		sendMessage(EM_SETPASSWORDCHAR,L'*',0);

	//初期値設定
	setTitle(value);
	//位置を合わせる
	moveWindow(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
	showWindow();
	setFocus();

	//親ウインドウのクリック位置に合わせてテキストをクリック
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(*this,&p);
	sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x,p.y));
	sendMessage(WM_LBUTTONUP,0,MAKELONG(p.x,p.y));
	sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x,p.y));
	sendMessage(WM_LBUTTONUP,0,MAKELONG(p.x,p.y));

	m_ret = true;
	return true;
}
void InputText::onEditChar(Message* m)
{
	INT code = (INT)m->getWParam();
	m_lastCode = code;
	if(code == VK_RETURN || code == VK_TAB)
	{
		PostMessage(getParent(),WM_APP,0,code);
	}
	else if(code == VK_ESCAPE)
	{
		m_ret = false;
		PostMessage(getParent(),WM_APP,1,code);
	}
}
void InputText::onEditKillFocus(Message* m)
{
	if(m_ret)
	{
		m_ret = false;
		SendMessage(getParent(),WM_APP,0,0);
	}
}


InputButtonText::InputButtonText()
{
	m_ret = false;
	m_text.setClassName(L"EDIT");
	m_text.addMessage(WM_KILLFOCUS ,CLASSPROC(this,InputButtonText,onKillFocus));
	m_text.addMessage(WM_CHAR,CLASSPROC(this,InputButtonText,onEditChar));
	//m_button.addMessage(WM_KILLFOCUS ,CLASSPROC(this,InputButtonText,onKillFocus));
	m_button.addEvent(0,CLASSPROC(this,InputButtonText,onSubmit));
}
void InputButtonText::onSubmit(Message* m)
{
	if(m_proc.isAddress())
	{
		m_ret = false;
		if(m_proc.call(m))
			SendMessage(getParent(),WM_APP,0,0);
		else
			PostMessage(getParent(),WM_APP,1,0);
		m_ret = true;
	}
}
void InputButtonText::onEditChar(Message* m)
{
	INT code = (INT)m->getWParam();
	if(code == VK_RETURN)
	{
		SendMessage(getParent(),WM_APP,0,0);
	}
	else if(code == VK_ESCAPE)
	{
		m_ret = false;
		PostMessage(getParent(),WM_APP,1,0);
	}
}
void InputButtonText::onKillFocus(Message* m)
{
	HWND hwnd = (HWND)m->getWParam();
	if(m_button.getWnd() != hwnd && m_text.getWnd() != hwnd)
	{
	//	if(m_ret)
	//		SendMessage(getParent(),WM_APP,0,0);
	}
}

bool InputButtonText::create(HWND hWnd,WString& value,RECT& rect)
{
	//エディットコントロールの作成
	moveWindow(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
	createWindow(NULL,NULL,WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL|WS_CLIPCHILDREN,hWnd);
	addChild(&m_button,CHILD_RIGHT);
	addChild(&m_text,CHILD_CLIENT);
	m_button.setWindowWidth(12);

	m_text.setTitle(value);
	recalcLayout();
	m_ret = true;

	m_text.setFocus();
	//フォントを親ウインドウに合わせる
	HFONT font = (HFONT)SendMessage(hWnd,WM_GETFONT,0,0);
	m_text.sendMessage(WM_SETFONT,(WPARAM)font,true);
	//親ウインドウのクリック位置に合わせてテキストをクリック
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_text,&p);
	if(value.length())
	{
		m_text.sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x,p.y));
		m_text.sendMessage(WM_LBUTTONUP,0,MAKELONG(p.x,p.y));
		m_text.sendMessage(WM_LBUTTONDOWN,0,MAKELONG(p.x,p.y));
	}
	return m_ret;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowDebug
// デバッグ用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
INT WindowDebug::Debug(LPCSTR format, ...)
{
	if(!m_window.isWindow())
	{
		//m_window.createWindow();
		m_window.setTitle("DEBUG");
		m_window.setWindowSize(480,320);
		m_window.showWindow();
	}
	std::string dest;

	time_t t;
	time(&t);
	tm lt = *localtime(&t);
	strprintf(dest,"%02d/%02d %02d:%02d %02d\n",lt.tm_mon+1,lt.tm_mday,lt.tm_hour,lt.tm_min,lt.tm_sec);
	m_window.out(dest.c_str());


	va_list param_list;
	va_start(param_list, format);

	INT ret = strprintf(dest,format,param_list);
	va_end(param_list);

	m_window.out(dest.c_str());
	return ret;

}
*/
}}
