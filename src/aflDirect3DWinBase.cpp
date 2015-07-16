#include <windows.h>
#include "aflDirect3DWinBase.h"

#ifdef _MSC_VER
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
namespace AFL{namespace DIRECT3D{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWndMessage
// DirectX - ウインドウメッセージクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWndMessage::UnitWndMessage()
{
	m_wnd = NULL;
	m_defaltProc = true;
}
void UnitWndMessage::setDefaultProc(bool flag)
{
	m_defaltProc = flag;
}
bool UnitWndMessage::isDefaultProc() const
{
	return m_defaltProc;
}
int UnitWndMessage::getX() const
{
	return (INT)getParamFloat(0);
}
int UnitWndMessage::getY() const
{
	return (INT)getParamFloat(1);
}
void UnitWndMessage::setWnd(UnitWnd* wnd)
{
	m_wnd = wnd;
}
UnitWnd* UnitWndMessage::getWnd() const
{
	return m_wnd;
}
void UnitWndMessage::setMessage(DWORD msg)
{
	m_msg = msg;
}
DWORD UnitWndMessage::getMessage()
{
	return m_msg;
}
void UnitWndMessage::setParamInt(int index,int data)
{
	m_paramInt[index] = data;
}
void UnitWndMessage::setParamFloat(int index,float data)
{
	m_paramFloat[index] = data;
}
int UnitWndMessage::getParamInt(int index) const
{
	return m_paramInt[index];
}
float UnitWndMessage::getParamFloat(int index) const
{
	return m_paramFloat[index];
}
void UnitWndMessage::setParamAdr(int index,LPVOID data)
{
	m_paramAdr[index] = data;
}
LPVOID UnitWndMessage::getParamAdr(int index) const
{
	return m_paramAdr[index];
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowManager
// DirectX - ウインドウ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowManager::UnitWindowManager()
{
	m_threadID = 0;
	m_mainWindow = NULL;
	m_unitChild = NEW UnitWnd();
	m_unitChild->setChainClip(false);
	m_unitChild->setViewClip(false);
	m_unitChild->setSize(640,480);
	m_unitChild->setManager(this);
	Unit::add(m_unitChild);

	m_unitActive = NULL;
	m_unitMouseHover = NULL;
	m_unitMouseDown = NULL;

	m_downLeft = false;
	m_downRight = false;

	m_moveWnd = NULL;

	m_cursorNS = (HCURSOR)LoadImage(0,IDC_SIZENS,IMAGE_CURSOR,0,0,LR_SHARED);
	m_cursorWE = (HCURSOR)LoadImage(0,IDC_SIZEWE,IMAGE_CURSOR,0,0,LR_SHARED);

	m_cursorNWSE = (HCURSOR)LoadImage(0,IDC_SIZENWSE,IMAGE_CURSOR,0,0,LR_SHARED);
	m_cursorNESW = (HCURSOR)LoadImage(0,IDC_SIZENESW,IMAGE_CURSOR,0,0,LR_SHARED);

	m_cursor = (HCURSOR)LoadImage(0,IDC_ARROW,IMAGE_CURSOR,0,0,LR_SHARED);
	m_cursorKeep = m_cursor;

}
UnitWindowManager::~UnitWindowManager()
{
	delete m_unitChild;
}
void UnitWindowManager::add(UnitWnd* unit)
{
	m_unitChild->addChild(unit);
}
void UnitWindowManager::init(Window* window)
{
	m_mainWindow = window;
	window->addMessage(0,CLASSPROC(this,UnitWindowManager,onMessage));
	m_unitChild->setSize((FLOAT)window->getClientWidth(),(FLOAT)window->getClientHeight());
}
void UnitWindowManager::callEvent(UnitWndMessage* m)
{
	UnitWnd* unit = m->getWnd();
	if(unit && m_threadID == GetCurrentThreadId())
	{
		std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
		itMsg = unit->m_eventProc.find(m->getMessage());
		if(itMsg !=   unit->m_eventProc.end())
		{
			std::multimap<INT,ClassProc>::iterator it;
			for(it=itMsg->second.begin();it!=itMsg->second.end();++it)
			{
				it->second.call(m);
				if(!m->isDefaultProc())
					break;
			}
		}
	}
	else
	{
		lock();
		switch(m->getMessage())
		{
		case WND_MOUSE_MOVE:
		case WND_MOUSE_DRAG:
			{
				std::list<UnitWndMessage>::iterator it;
				for(it=m_listMessage.begin();it!=m_listMessage.end();)
				{
					if(it->getMessage() == m->getMessage() && it->getWnd() == m->getWnd())
						it = m_listMessage.erase(it);
					else
						++it;
				}
			}
			break;
		}
		m_listMessage.push_back(*m);
		unlock();
	}
}
void UnitWindowManager::sort() const
{
	FLOAT w = 0;
	m_unitChild->sort(w);
}
FLOAT UnitWindowManager::getWindowWidth() const
{
	return m_unitChild->getWindowWidth();
}
FLOAT UnitWindowManager::getWindowHeight() const
{
	return m_unitChild->getWindowHeight();
}
UnitWnd* UnitWindowManager::getHoverWindow() const
{
	if(m_unitChild == m_unitMouseHover)
		return NULL;
	return m_unitMouseHover;
}
UnitWnd* UnitWindowManager::getMoveWindow() const
{
	if(m_unitChild == m_moveWnd)
		return NULL;
	return m_moveWnd;
}
bool UnitWindowManager::isBusy() const
{
	if(getMoveWindow() || getHoverWindow())
		return true;
	return false;
}
void UnitWindowManager::lock()
{
	m_critical.lock();
}
void UnitWindowManager::unlock()
{
	m_critical.unlock();
}
Window* UnitWindowManager::getMasterWindow() const
{
	return m_mainWindow;
}
void UnitWindowManager::setActive(UnitWnd* unit)
{
	//メッセージ送信
	UnitWndMessage m;
	m.setMessage(WND_ACTIVE);

	if(m_unitActive != unit)
	{
		//旧アクティブの無効化
		UnitWnd* oldActive;
		for(oldActive = m_unitActive;oldActive && oldActive->isActive();oldActive=oldActive->getParent())
		{
			if(unit && (unit==oldActive || unit->isParents(oldActive)))
				break;
			m.setDefaultProc(false);
			m.setParamInt(0,false);
			m.setWnd(oldActive);
			oldActive->m_active = false;
			oldActive->callEvent(&m);
		}
		//新アクティブ
		UnitWnd* newActive;
		std::list<UnitWnd*> units;
		for(newActive=unit;newActive && !newActive->isActive();newActive=newActive->getParent())
		{
			units.push_front(newActive);
		}

		std::list<UnitWnd*>::iterator it;
		for(it=units.begin();it!=units.end();++it)
		{
			m.setDefaultProc(false);
			m.setParamInt(0,true);
			m.setWnd(*it);
			(*it)->m_active = true;
			(*it)->callEvent(&m);
		}
		m_unitActive = unit;
	}

}
bool UnitWindowManager::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	sort();
	return true;
}

void UnitWindowManager::onAction(World* world,LPVOID value)
{
	m_threadID = GetCurrentThreadId();
	//メッセージの実行
	while(!m_listMessage.empty())
	{
		lock();
		UnitWndMessage msg = m_listMessage.front();
		m_listMessage.pop_front();
		unlock();
		UnitWnd* unit = msg.getWnd();
		if(unit)
		{
			std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
			itMsg = unit->m_eventProc.find(msg.getMessage());
			if(itMsg !=   unit->m_eventProc.end())
			{
				std::multimap<INT,ClassProc>::iterator it;
				for(it=itMsg->second.begin();it!=itMsg->second.end();++it)
				{
					it->second.call(&msg);
				}
			}
		}
		else
		{
			switch(msg.getMessage())
			{
			case WND_ACTIVE:
				unit  = (UnitWnd*)msg.getParamAdr(0);
				unit->setActive(true);
				unit->setForeground();
				break;
			case WND_SIZE:
				m_unitChild->setSize((FLOAT)msg.getParamFloat(0),(FLOAT)msg.getParamFloat(1));
				break;
			case WND_MOUSE_MOVE:
				{
					FLOAT x = (FLOAT)msg.getParamFloat(0);
					FLOAT y = (FLOAT)msg.getParamFloat(1);
					unit = getWindow(x,y);
					onMove(unit,x,y);
					bool buttonLeft = GetAsyncKeyState(VK_LBUTTON) < 0;
					bool buttonRight = GetAsyncKeyState(VK_RBUTTON) < 0;
					if(buttonLeft || buttonRight)
					{
						if(m_unitMouseDown)
						{
							if(m_movePoint.x != x || m_movePoint.y != y)
							{
								UnitWndMessage m;
								m.setMessage(WND_MOUSE_DRAG);
								m.setParamFloat(0,(FLOAT)x);
								m.setParamFloat(1,(FLOAT)y);
								m_unitMouseDown->callEvent(&m);
							}
						}
					}
					m_movePoint.x = (INT)x;
					m_movePoint.y = (INT)y;
				}
				break;
			}
		}
	}
	Unit::onAction(world,value);
}

void UnitWindowManager::onMessage(Message* m)
{
	switch(m->getMsg())
	{
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			UnitWnd* unit = m_unitMouseHover;
			if(m_unitMouseDown)
			{
				INT x = m->getX();
				INT y = m->getY();
				UnitWndMessage mm;
				mm.setMessage(WND_MOUSE_UP);
				mm.setParamInt(0,m->getMsg() != WM_LBUTTONDOWN);
				mm.setParamFloat(0,(FLOAT)x);
				mm.setParamFloat(1,(FLOAT)y);
				unit->callEvent(&mm);
			}

			if(unit && unit == m_unitMouseDown)
			{
				INT x = m->getX();
				INT y = m->getY();
				UnitWndMessage mm;
				mm.setMessage(WND_MOUSE_CLICK);
				mm.setParamInt(0,m->getMsg() != WM_LBUTTONDOWN);
				mm.setParamFloat(0,(FLOAT)x);
				mm.setParamFloat(1,(FLOAT)y);
				unit->callEvent(&mm);
			}
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			bool buttonLeft = m->getMsg() == WM_LBUTTONDOWN;
			FLOAT x = (FLOAT)m->getX();
			FLOAT y = (FLOAT)m->getY();
			UnitWnd* unit = m_unitMouseHover;

			if(unit && !m_unitMouseDown)
			{
				UnitWndMessage msgActive;
				msgActive.setMessage(WND_ACTIVE);
				msgActive.setParamAdr(0,unit);
				callEvent(&msgActive);

				m_unitMouseDown = unit;
				UnitWndMessage m;
				m.setMessage(WND_MOUSE_DOWN);
				m.setParamInt(0,buttonLeft?0:1);
				m.setParamFloat(0,(FLOAT)x);
				m.setParamFloat(1,(FLOAT)y);
				unit->callEvent(&m);
				m_downPoint.x = x;
				m_downPoint.y = y;
				unit->getWindowRect(&m_rectPoint);
			}
			if(!m_downLeft)
			{
				if(m_downHit == HIT_NONE)
				{
					m_downLeft = true;
					if(unit)
					{
						m_moveWnd = unit;
						m_downHit = unit->hitTest(x,y);
					}
				}
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			UnitWnd* unit = m_unitMouseHover;
			if(unit)
			{
				if(unit->getWindowStat() == WINDOW_MIN)
				{
					unit->setWindowStat(WINDOW_NORMAL);
				}
				else
				{
					UnitWndMessage m;
					m.setMessage(WND_MOUSE_LDBLCLICK);
					unit->callEvent(&m);
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			UnitWndMessage msg;
			msg.setMessage(WND_MOUSE_MOVE);
			msg.setParamFloat(0,(FLOAT)m->getX());
			msg.setParamFloat(1,(FLOAT)m->getY());
			callEvent(&msg);
		}
		break;
	case WM_MOVE:
		{
			UnitWndMessage msg;
			msg.setMessage(WND_MOVE);
			msg.setParamFloat(0,(FLOAT)m->getX());
			msg.setParamFloat(1,(FLOAT)m->getY());
			callEvent(&msg);
		}
		break;
	case WM_SIZE:
		{
			UnitWndMessage msg;
			msg.setMessage(WND_SIZE);
			msg.setParamFloat(0,(FLOAT)m->getX());
			msg.setParamFloat(1,(FLOAT)m->getY());
			callEvent(&msg);
		}
		break;
	case WM_CHAR:
		if(m_unitActive)
		{
			UnitWndMessage wm;
			wm.setMessage(WND_CHAR);
			wm.setParamInt(0,m->getChar());
			m_unitActive->callEvent(&wm);
		}
		break;
	case WM_KEYDOWN:
		if(m_unitActive)
		{
			UnitWndMessage wm;
			wm.setMessage(WND_KEYDOWN);
			wm.setParamInt(0,m->getChar());
			m_unitActive->callEvent(&wm);
		}
		break;
	case WM_MOUSEWHEEL:
		if(m_unitMouseHover)
		{
			UnitWndMessage msg;
			msg.setMessage(WND_MOUSE_WHEEL);
			msg.setParamInt(0,0);
			msg.setParamInt(1,GET_WHEEL_DELTA_WPARAM(m->getWParam()));
			m_unitMouseHover->callEvent(&msg);
		}
		break;
	}
}

void UnitWindowManager::proc(HWND hWnd)
{
	RECT rect;
	POINT point;
	::GetClientRect(hWnd,&rect);
	GetCursorPos(&point);
	ScreenToClient(hWnd,&point);
	if(point.x < rect.left || point.y < rect.top || point.x >= rect.right || point.y >= rect.bottom)
	{
		//マウス範囲外
		if(m_unitMouseHover)
		{
			UnitWndMessage m;
			m.setMessage(WND_MOUSE_OUT);
			m_unitMouseHover->callEvent(&m);
			m_unitMouseHover = NULL;
		}
		return;
	}

	bool buttonLeft = GetAsyncKeyState(VK_LBUTTON) < 0;
	bool buttonRight = GetAsyncKeyState(VK_RBUTTON) < 0;
	if(!buttonLeft && !buttonRight)
	{
		FLOAT x = (FLOAT)point.x;
		FLOAT y = (FLOAT)point.y;
		if(m_unitMouseDown)
		{
			UnitWndMessage m;
			m.setMessage(WND_MOUSE_UP);
			m.setParamInt(0,0);
			m_unitMouseDown->callEvent(&m);
			m_unitMouseDown = NULL;
		}
		m_downHit = HIT_NONE;
		m_moveWnd = NULL;
		m_downLeft = false;
	}
}


UnitWnd* UnitWindowManager::getWindow(FLOAT x,FLOAT y)
{
	return m_unitChild->getWindow(x,y);
}
void UnitWindowManager::recalcLayout()
{
	m_unitChild->recalcLayout(true);
//	FLOAT w = 0;
//	m_unitChild->sort(w);
}

void UnitWindowManager::onMove(UnitWnd* unit,FLOAT x,FLOAT y)
{
	FLOAT moveX = m_movePoint.x - x;
	FLOAT moveY = m_movePoint.y - y;
	FLOAT moveX2 = m_downPoint.x - x;
	FLOAT moveY2 = m_downPoint.y - y;


	if(m_unitMouseHover != unit)
	{
		if(m_unitMouseHover)
		{
			UnitWndMessage m;
			m.setParamFloat(0,x);
			m.setParamFloat(1,y);
			m.setMessage(WND_MOUSE_OUT);
			m_unitMouseHover->callEvent(&m);
		}
		if(unit)
		{
			UnitWndMessage m;
			m.setParamFloat(0,x);
			m.setParamFloat(1,y);
			m.setMessage(WND_MOUSE_HOVER);
			unit->callEvent(&m);
		}
		m_unitMouseHover = unit;
	}

	HIT_TEST hit = m_downHit;
	if(!m_moveWnd || m_moveWnd->isStatic())
		hit = HIT_NONE;

	switch(hit)
	{
	case HIT_CAPTION:
		m_moveWnd->onMove(m_moveWnd->getPosX() - m_movePoint.x + x,
			m_moveWnd->getPosY() - m_movePoint.y + y);
		break;

	case HIT_FRAME_TOP_LEFT:
		if(moveX || moveY)
		{
			FLOAT width = m_rectPoint.right-m_rectPoint.left + moveX2;
			FLOAT height = m_rectPoint.bottom-m_rectPoint.top + moveY2;
			m_moveWnd->setPosX(m_rectPoint.left - moveX2);
			m_moveWnd->setPosY(m_rectPoint.top - moveY2);
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_TOP_RIGHT:
		if(moveX || moveY)
		{
			FLOAT width = m_moveWnd->getWindowWidth() - moveX;
			FLOAT height = m_moveWnd->getWindowHeight() + moveY;
			m_moveWnd->setPosY(m_moveWnd->getPosY() - moveY);
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_BOTTOM_LEFT:
		if(moveX || moveY)
		{
			FLOAT width = m_moveWnd->getWindowWidth() + moveX;
			FLOAT height = m_moveWnd->getWindowHeight() - moveY;
			m_moveWnd->setPosX(m_moveWnd->getPosX() - moveX);
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_BOTTOM_RIGHT:
		if(moveX || moveY)
		{
			FLOAT width = m_moveWnd->getWindowWidth() - moveX;
			FLOAT height = m_moveWnd->getWindowHeight() - moveY;
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_LEFT:
		if(moveX)
		{
			FLOAT width = m_moveWnd->getWindowWidth() + moveX;
			FLOAT height = m_moveWnd->getWindowHeight();
			m_moveWnd->setPosX(m_moveWnd->getPosX() - moveX);
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_RIGHT:
		if(moveX)
		{
			FLOAT width = m_moveWnd->getWindowWidth() - moveX;
			FLOAT height = m_moveWnd->getWindowHeight();
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_TOP:
		if(moveY)
		{
			FLOAT width = m_moveWnd->getWindowWidth();
			FLOAT height = m_moveWnd->getWindowHeight() + moveY;
			m_moveWnd->setPosY(m_moveWnd->getPosY() - moveY);
			m_moveWnd->setSize(width,height);
		}
		break;
	case HIT_FRAME_BOTTOM:
		if(moveY)
		{
			FLOAT width = m_moveWnd->getWindowWidth();
			FLOAT height = m_moveWnd->getWindowHeight() - moveY;
			m_moveWnd->setSize(width,height);
		}
		break;
	default:
		{

			HCURSOR cursor;
			if(unit)
			{

				HIT_TEST hit = unit->hitTest(x,y);
				switch(hit)
				{
				case HIT_FRAME_TOP_LEFT:
				case HIT_FRAME_BOTTOM_RIGHT:
					if(m_cursorKeep != m_cursorNWSE)
					{
						m_mainWindow->setCursor(m_cursorNWSE);
						m_cursorKeep = m_cursorNWSE;
					}
					break;
				case HIT_FRAME_TOP_RIGHT:
				case HIT_FRAME_BOTTOM_LEFT:
					if(m_cursorKeep != m_cursorNESW)
					{
						m_mainWindow->setCursor(m_cursorNESW);
						m_cursorKeep = m_cursorNESW;
					}
					break;
				case HIT_FRAME_LEFT:
				case HIT_FRAME_RIGHT:
					if(m_cursorKeep != m_cursorWE)
					{
						m_mainWindow->setCursor(m_cursorWE);
						m_cursorKeep = m_cursorWE;
					}
					break;
				case HIT_FRAME_TOP:
				case HIT_FRAME_BOTTOM:
					if(m_cursorKeep != m_cursorNS)
					{
						m_mainWindow->setCursor(m_cursorNS);
						m_cursorKeep = m_cursorNS;
					}
					break;
				default:
					{
						UnitWndMessage m;
						m.setParamFloat(0,x);
						m.setParamFloat(1,y);
						m.setWnd(unit);
						m.setMessage(WND_MOUSE_MOVE);
						unit->callEvent(&m);
						if(m_unitMouseDown && m_unitMouseDown != unit)
						{
							m.setWnd(m_unitMouseDown);
							m_unitMouseDown->callEvent(&m);
						}
					}

					cursor = unit->getCursor();
					if(!cursor)
						cursor = m_cursor;
					if(m_cursorKeep != cursor)
					{
						m_mainWindow->setCursor(cursor);
						m_cursorKeep = cursor;
					}
					break;
				}
			}
			else
			{
				if(m_cursorKeep != m_cursor)
				{
					m_mainWindow->setCursor(m_cursor);
					m_cursorKeep = m_cursor;
				}

			}
			break;
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWnd
// DirectX - ウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWnd::UnitWnd()
{
	m_manager = NULL;
	m_parentWnd = NULL;
	m_height = 100;
	m_width = 100;
	m_childStyle = CHILD_NORMAL;
	m_windowStat = WINDOW_NORMAL;

	
	m_marginRect.bottom = 0;
	m_marginRect.top = 0;
	m_marginRect.left = 0;
	m_marginRect.right = 0;
	m_paddingRect.bottom = 0;
	m_paddingRect.top = 0;
	m_paddingRect.left = 0;
	m_paddingRect.right = 0;
	m_windowX = 0;
	m_windowY = 0;
	m_targetWidth = 0.0f;
	m_targetHeight = 0.0f;
	m_statPosX = 0.0f;
	m_statPosY = 0.0f;
	m_statScaleX = 0.0f;
	m_statScaleY = 0.0f;
	m_minPosX = -1000;
	m_minPosY = -1000;
	m_minWidth = 8;
	m_minHeight = 8;
	m_childPriority = 0;
	m_cursor = NULL;
	m_backColor = 0;
	m_active = false;
	m_static = false;
	setChainW(false);
	setChainClip(true);
	setViewClip(true);
	setTextureFilter(D3DTEXF_LINEAR);
	addEvent(WND_CLOSE,CLASSPROC(this,UnitWindow,onMessage));
	addEvent(WND_MAX,CLASSPROC(this,UnitWindow,onMessage));
	addEvent(WND_MIN,CLASSPROC(this,UnitWindow,onMessage));

}
UnitWnd::~UnitWnd()
{
	std::list<UnitWnd*>::iterator it;
	while(m_windowChilds.size())
	{
		delChild(m_windowChilds.front());
	}
	if(m_parentWnd)
		m_parentWnd->delChild(this);
}
void UnitWnd::setWindowStat(WINDOW_STAT s)
{
	const float count = 8.0f;
	if(s == m_windowStat)
		return;
	if(m_windowStat == WINDOW_NORMAL || m_windowStat == WINDOW_THROUGH)
	{
		m_normalPosX = getPosX();
		m_normalPosY = getPosY();
		m_normalWidth = getWindowWidth();
		m_normalHeight = getWindowHeight();
	}
	else if(m_windowStat == WINDOW_MAX)
	{
		m_targetPosX = (FLOAT)m_normalPosX;
		m_targetPosY = (FLOAT)m_normalPosY;
		m_statPosX = (m_targetPosX-getPosX()) / count;
		m_statPosY = (m_targetPosY-getPosY()) / count;

		setScaleX((FLOAT)getWindowWidth() / m_normalWidth);
		setScaleY((FLOAT)getWindowHeight() / m_normalHeight);
		m_targetScaleX = 1.0f;
		m_targetScaleY = 1.0f;
		m_statScaleX = (m_targetScaleX - getScaleX()) / count;
		m_statScaleY = (m_targetScaleY - getScaleY()) / count;

		setSize(m_normalWidth,m_normalHeight);
	}
	else if(m_windowStat == WINDOW_MIN)
	{
		m_minPosX = getPosX();
		m_minPosY = getPosY();
		m_statPosX = (m_normalPosX-getPosX()) / count;
		m_statPosY = (m_normalPosY-getPosY()) / count;


		m_targetScaleX = 1.0f;
		m_targetScaleY = 1.0f;
		m_statScaleX = (m_targetScaleX - getScaleX()) / count;
		m_statScaleY = (m_targetScaleY - getScaleY()) / count;
		m_targetPosX = (FLOAT)m_normalPosX;
		m_targetPosY = (FLOAT)m_normalPosY;
		m_statPosX = (m_normalPosX-getPosX()) / count;
		m_statPosY = (m_normalPosY-getPosY()) / count;
	}
	if(s == WINDOW_MAX)
	{
		m_targetPosX = 0.0f;
		m_targetPosY = 0.0f;
		m_statPosX = (m_targetPosX-getPosX()) / count;
		m_statPosY = (m_targetPosY-getPosY()) / count;
		UnitWnd* unit = getParent();
		if(unit)
		{
			Rect2DF rect;
			unit->getClientRect(&rect);
			FLOAT width = (FLOAT)rect.right - rect.left;
			FLOAT height = (FLOAT)rect.bottom - rect.top;
			setScaleX(getWindowWidth() / width);
			setScaleY(getWindowHeight() / height);
			setSize((FLOAT)rect.right-rect.left,(FLOAT)rect.bottom-rect.top);

			m_targetScaleX = 1.0f;
			m_targetScaleY = 1.0f;
			m_statScaleX = (m_targetScaleX - getScaleX()) / count;
			m_statScaleY = (m_targetScaleY - getScaleY()) / count;
		}
	}
	else if(s == WINDOW_MIN)
	{
		if(m_minPosX >= 0 && m_minPosX < m_parentWnd->getWindowWidth() &&
			m_minPosY >=0 && m_minPosY < m_parentWnd->getWindowHeight())
		{
			m_targetPosX = m_minPosX;
			m_targetPosY = m_minPosY;
			m_statPosX = (m_targetPosX-getPosX()) / count;
			m_statPosY = (m_targetPosY-getPosY()) / count;
		}
		else
		{
			m_statPosX = 0.0f;
			m_statPosY = 0.0f;
			m_minPosX = getPosX();
			m_minPosY = getPosY();
		}
		m_targetScaleX = 32.0f/getWindowWidth();
		m_targetScaleY = 32.0f/getWindowHeight();
		m_statScaleX = (m_targetScaleX-getScaleX()) / count;
		m_statScaleY = (m_targetScaleY-getScaleY()) / count;

	}
	m_windowStat = s;
}
WINDOW_STAT UnitWnd::getWindowStat() const
{
	return m_windowStat;
}
void UnitWnd::onAction(World* world,LPVOID value)
{
	if(isVisible())
	{
		if(m_statPosX || m_statPosY)
		{
			FLOAT posX = getPosX();
			FLOAT posY = getPosY();
			if(abs(m_targetPosX - posX) < abs(m_statPosX))
			{
				posX = m_targetPosX;
				m_statPosX = 0.0f;
			}
			else
				posX += m_statPosX;
			if(abs(m_targetPosY - posY) < abs(m_statPosY))
			{
				posY = m_targetPosY;
				m_statPosY = 0.0f;
			}
			else
				posY += m_statPosY;
			setPos(posX,posY);
		}
		if(m_statScaleX || m_statScaleY)
		{
			FLOAT scaleX = getScaleX();
			FLOAT scaleY = getScaleY();
			if(abs(m_targetScaleX - scaleX) < abs(m_statScaleX))
			{
				scaleX = m_targetScaleX;
				m_statScaleX = 0.0f;
			}
			else
				scaleX += m_statScaleX;
			if(abs(m_targetScaleY - scaleY) < abs(m_statScaleY))
			{
				scaleY = m_targetScaleY;
				m_statScaleY = 0.0f;
			}
			else
				scaleY += m_statScaleY;
			setScaleX(scaleX);
			setScaleY(scaleY);
			setCenter(getWindowWidth()/2.0f,getWindowHeight()/2.0f);
		}
	}
	UnitVector::onAction(world,value);
}
DWORD UnitWnd::onMessage(UnitWndMessage* m)
{
	if(m->getMessage() == WND_CLOSE)
	{
		setVisible(false);
	}
	else if(m->getMessage() == WND_NORMAL)
	{
		setWindowStat(WINDOW_NORMAL);
	}
	else if(m->getMessage() == WND_MAX)
	{
		setWindowStat(WINDOW_MAX);
	}
	else if(m->getMessage() == WND_MIN)
	{
		setWindowStat(WINDOW_MIN);
	}

	return 0;
}
INT UnitWnd::getLevel() const
{
	INT i;
	const UnitWnd* unit = this;
	for(i=0;unit=unit->m_parentWnd;i++);
	return i;
}
bool UnitWnd::isParent(UnitWnd* wnd) const
{
	return wnd == m_parentWnd;
}
bool UnitWnd::isParents(UnitWnd* wnd) const
{
	const UnitWnd* unit = this;
	while(unit = unit->m_parentWnd)
	{
		if(unit == wnd)
			return true;
	}
	return false;
}
void UnitWnd::setVisible(bool flag)
{
	if(flag != isVisible())
	{
		//アクティブだった場合、親に委譲
		if(!flag && isActive() && getParent())
		{
			getParent()->setActive(true);
		}
		if(getParent())
			getParent()->recalcLayout(true);
		else
			recalcLayout(true);
		UnitVector::setVisible(flag);
	}
}
void UnitWnd::setActive(bool flag)
{
	if(flag)
	{
		UnitWindowManager* manager = getManager();
		if(manager)
			manager->setActive(this);
	}
}
bool UnitWnd::isActive() const
{
	return m_active;
}
UnitWnd* UnitWnd::getParent() const
{
	return m_parentWnd;
}
void UnitWnd::setMinSize(FLOAT width,FLOAT height)
{
	m_minWidth = width;
	m_minHeight = height;
}

class Less : public std::less<Unit*> {
public:
	bool operator()(const UnitWnd* a, const UnitWnd* b) const
	{ 
		FLOAT wa = a->getPosW();
		FLOAT wb = b->getPosW();

		if(a->getWindowStat() != WINDOW_MIN && b->getWindowStat() == WINDOW_MIN)
			return true;
		if(a->getWindowStat() == WINDOW_MIN && b->getWindowStat() != WINDOW_MIN)
			return false;

		if(a->getSortStyle() == SORT_CONTROL)
			wa += 15000;
		else if(a->getSortStyle() == SORT_BOTTOM)
			wa -= 10000;
		else if(a->getSortStyle() == SORT_TOP)
			wa += 10000;

		if(b->getSortStyle() == SORT_CONTROL)
			wb += 15000;
		else if(b->getSortStyle() == SORT_BOTTOM)
			wb -= 10000;
		else if(b->getSortStyle() == SORT_TOP)
			wb += 10000;


		if(a->getChildStyle() == CHILD_NORMAL)
			wa += 1000;
		if(b->getChildStyle() == CHILD_NORMAL)
			wb += 1000;
		return wa < wb;
	}
};

void UnitWnd::sort(FLOAT& w)
{
	std::list<UnitWnd*>::iterator it;
	for(it=m_windowChilds.begin();it!=m_windowChilds.end();++it)
	{
		w += 1;
		(*it)->setPosW(w);
		(*it)->sort(w);
	}
}
void UnitWnd::setForeground()
{
	UnitWnd* unit = this;
	unit->setPosW(1000);
	while(unit = unit->getParent())
	{
		unit->setPosW(1000);
		unit->m_windowChilds.sort(Less());		
	}
}
bool UnitWnd::moveWindow(FLOAT x,FLOAT y,FLOAT width,FLOAT height,bool repaint)
{
	setPosX(x);
	setPosY(y);
	if(width < 0)
		width = 0;
	if(height < 0)
		height = 0;
	setSize(width,height,repaint);
	return true;
}


bool UnitWnd::setSize(FLOAT width,FLOAT height,bool flag)
{
	resetRenderFlag();

	if(width < m_minWidth)
		width = m_minWidth;
	if(height < m_minHeight)
		height = m_minHeight;
	m_width = width;
	m_height = height;

	RECT rect = {0,0,(INT)width,(INT)height};
	setViewClip(&rect);

	UnitWndMessage m;
	m.setMessage(WND_SIZE);
	callEvent(&m);

	recalcLayout(flag);
	return true;
}
void UnitWnd::getWindowRect(Rect2DF* rect)
{
	rect->left = getPosX();
	rect->top = getPosY();
	rect->right = getPosX() + m_width;
	rect->bottom = getPosY() + m_height;
}

void UnitWnd::getClientRect(Rect2DF* rect)
{
	rect->top = 0;
	rect->left = 0;
	rect->bottom = m_height;
	rect->right = m_width;
}
FLOAT UnitWnd::getWindowHeight() const
{
	return m_height;
}
FLOAT UnitWnd::getWindowWidth() const
{
	return m_width;
}
UnitWnd* UnitWnd::getWindow(FLOAT x,FLOAT y,bool flag)
{
	if(!isVisible())
		return NULL;
	if(!isChainClip())
		flag = true;

	HIT_TEST hit = hitTest(x,y);
	if(hit==HIT_NONE)
		flag = false;

	if(getWindowStat() != WINDOW_MIN)
	{
		std::list<UnitWnd*>::reverse_iterator it;
		for(it=m_windowChilds.rbegin();it!=m_windowChilds.rend();++it)
		{
			UnitWnd* unitWnd = (*it)->getWindow(x,y,flag);
			if(unitWnd)
				return unitWnd;
		}
	}
	if(hit == HIT_NONE || hit == HIT_THROUGH)
		return NULL;
	return this;
}
bool UnitWnd::isHit(FLOAT x,FLOAT y)
{
	if(isVisible())
	{
		x -= getAbsX();
		y -= getAbsY();

		if(getWindowStat() == WINDOW_MIN)
		{
			x /= getScaleX();
			y /= getScaleY();
		}

		if(x >= 0 && y >= 0 && x < m_width && y < m_height)
			return true;
	}
	return false;
}
HIT_TEST UnitWnd::hitTest(FLOAT x,FLOAT y)
{
	if(!isHit(x,y))
		return HIT_NONE;
	if(getWindowStat() == WINDOW_MIN)
		return HIT_CAPTION;
	return HIT_CLIENT;
}


void UnitWnd::add(Unit* unit)
{
	unit->setViewClip(true);
	unit->setChainClip(true);
	UnitVector::add(unit);
}

void UnitWnd::addChild(UnitWnd* unit)
{
	if(unit->m_parentWnd)
		unit->m_parentWnd->delChild(unit);
	unit->m_parentWnd = this;
	UnitVector::add(unit);
	if(m_windowChilds.size() && unit->getChildPriority() == 0)
		unit->setChildPriority(m_windowChilds.back()->getChildPriority()+1);
	m_windowChilds.push_back(unit);
}
void UnitWnd::delChild(UnitWnd* unit)
{
	m_windowChilds.remove(unit);
	del(unit);
	unit->m_parentWnd = NULL;
}
void UnitWnd::setMargin(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2)
{
	m_marginRect.left = x1;
	m_marginRect.right = x2;
	m_marginRect.top = y1;
	m_marginRect.bottom = y2;
}

void UnitWnd::setPadding(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2)
{
	m_paddingRect.left = x1;
	m_paddingRect.right = x2;
	m_paddingRect.top = y1;
	m_paddingRect.bottom = y2;
}
void UnitWnd::setManager(UnitWindowManager* manager)
{
	m_manager = manager;
}
UnitWindowManager* UnitWnd::getManager() const
{
	if(m_manager)
		return m_manager;
	if(getParent())
		return getParent()->getManager();
	return NULL;
}
Window* UnitWnd::getMasterWindow() const
{
	UnitWindowManager* manager = getManager();
	if(manager)
		return manager->getMasterWindow();
	return NULL;
}
void UnitWnd::recalcLayout(bool flag)
{
	resetRenderFlag();

	Rect2DF rect;
	getClientRect(&rect);
	recalcLayout2(&rect,flag);
}
void UnitWnd::recalcLayout2(Rect2DF* rect,bool flag)
{
	INT vertCount = 0;
	INT horizCount = 0;
	FLOAT vSize = 0;
	FLOAT hSize = 0;

	m_windowChilds.sort(Less());

	std::multimap<INT,UnitWnd*> stat;
	std::multimap<INT,UnitWnd*> rayout;
	std::list<UnitWnd*>::iterator it;
	for(it=m_windowChilds.begin();it!=m_windowChilds.end();++it)
	{
		UnitWnd* window = *it;
		INT priority = window->getChildPriority();
		CHILD_STYLE childStyle = window->getChildStyle();
		if(childStyle == CHILD_NORMAL)
		{
			stat.insert(std::pair<INT,UnitWnd*>(priority,window));
			continue;
		}
		if(childStyle == CHILD_LEFT || childStyle ==  CHILD_RIGHT)
			priority += 1000;
		else if(childStyle == CHILD_TOP || childStyle ==  CHILD_BOTTOM)
			priority += 2000;
		else
		{
			if(childStyle == CHILD_EQUARL_VERT)
				vertCount++;
			else if(childStyle == CHILD_EQUARL_HORIZ)
				horizCount++;
			priority += 3000;
		}
		rayout.insert(std::pair<INT,UnitWnd*>(priority,window));
	}

	rect->left += m_paddingRect.left;
	rect->right -= m_paddingRect.right;
	rect->top += m_paddingRect.top;
	rect->bottom -= m_paddingRect.bottom;

	if(stat.size())
	{
		std::multimap<INT,UnitWnd*>::iterator itRayout;
		for(itRayout=stat.begin();itRayout!=stat.end();++itRayout)
		{
			UnitWnd* window = itRayout->second;
			CHILD_STYLE childStyle = window->getChildStyle();
			//非表示ウインドウを無視
			if(!flag && !window->isVisible())
				continue;
			WINDOW_STAT windowStat = window->getWindowStat();
			if(windowStat == WINDOW_MAX)
			{
				window->moveWindow(rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,flag);
			}
			else if(flag)
			{
				window->recalcLayout(flag);
			}
		}
	}


	if(rayout.size())
	{
		std::multimap<INT,UnitWnd*>::iterator itRayout;
		for(itRayout=rayout.begin();itRayout!=rayout.end();++itRayout)
		{
			//範囲判定
			if(rect->top > rect->bottom || rect->left > rect->right)
				break;
			UnitWnd* window = itRayout->second;
			CHILD_STYLE childStyle = window->getChildStyle();
			//非表示ウインドウを無視
			if(!window->isVisible() &&
				!(flag && (childStyle == CHILD_AUTO || childStyle == CHILD_CLIENT || childStyle == CHILD_NORMAL)))
				continue;
			FLOAT size,size2;
			Rect2DF clientRect;
			Rect2DF marginRect;
			WINDOW_STAT stat = window->getWindowStat();
			marginRect = window->m_marginRect;
			window->getWindowRect(&clientRect);
			switch(childStyle)
			{
			case CHILD_TOP:
				size = clientRect.bottom - clientRect.top;
				window->moveWindow(rect->left,rect->top,rect->right-rect->left,size,flag);
				if(stat != WINDOW_THROUGH)
					rect->top += size;
				break;
			case CHILD_BOTTOM:
				size = clientRect.bottom - clientRect.top;
				window->moveWindow(rect->left,rect->bottom-size,rect->right-rect->left,size,flag);
				if(stat != WINDOW_THROUGH)
					rect->bottom -= size;
				break;
			case CHILD_LEFT:
				size = clientRect.right - clientRect.left;
				window->moveWindow(rect->left+marginRect.left,rect->top,size,rect->bottom-rect->top,flag);
				size += marginRect.left + marginRect.right;
				if(stat != WINDOW_THROUGH)
					rect->left += size;
				break;
			case CHILD_RIGHT:
				size = clientRect.right - clientRect.left;
				window->moveWindow(rect->right-size - marginRect.right,rect->top,size,rect->bottom-rect->top,flag);
				size += marginRect.left + marginRect.right;
				if(stat != WINDOW_THROUGH)
					rect->right -= size;
				break;
			case CHILD_CLIENT:
				window->moveWindow(rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,flag);
				break;
			case CHILD_EQUARL_VERT:
				if(vSize == 0)
					vSize = (rect->bottom - rect->top) / vertCount;
				window->moveWindow(rect->left,rect->top,rect->right-rect->left,vSize,flag);
				if(stat != WINDOW_THROUGH)
					rect->top += vSize;
				break;
			case CHILD_EQUARL_HORIZ:
				if(hSize == 0)
					hSize = (rect->right - rect->left) / horizCount;
				window->moveWindow(rect->left,rect->top,hSize,rect->bottom-rect->top,flag);
				if(stat != WINDOW_THROUGH)
					rect->left += hSize;
				break;
			case CHILD_CENTER:
				size = ((rect->right-rect->left) - (clientRect.right - clientRect.left)) / 2;
				size2 = ((rect->bottom-rect->top) - (clientRect.bottom - clientRect.top)) / 2;
				window->setPos(size,size2);
				break;
			}
		}
	}
}
void UnitWnd::addEvent(UINT message,ClassProc& classProc,INT priority)
{
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
	itMsg = m_eventProc.find(message);
	if(itMsg ==  m_eventProc.end())
	{
		m_eventProc[message].insert(std::pair<INT,ClassProc>(priority,classProc));
	}
	else
	{
		itMsg->second.insert(std::pair<INT,ClassProc>(priority,classProc));
	}
}
void UnitWnd::delEvent(UINT message,ClassProc& classProc)
{
	std::map<UINT,std::multimap<INT,ClassProc> >::iterator itMsg;
	itMsg = m_eventProc.find(message);
	if(itMsg !=  m_eventProc.end())
	{
		std::multimap<INT,ClassProc>::iterator it;
			for(it=itMsg->second.begin();it!=itMsg->second.end();++it)
		{
			if(it->second == classProc)
			{
				itMsg->second.erase(it++);
				it--;
			}
		}
	}
}
void UnitWnd::callEvent(UnitWndMessage* m)
{
	if(!m->getWnd())
		m->setWnd(this);
	
	if(getManager())
	{
		getManager()->callEvent(m);
	}
}
INT UnitWnd::getChildPriority() const
{
	return m_childPriority;
}
CHILD_STYLE UnitWnd::getChildStyle() const
{
	return m_childStyle;
}
void UnitWnd::setChildStyle(CHILD_STYLE style)
{
	m_childStyle=style;
}
DWORD UnitWnd::getBackColor() const
{
	return m_backColor;
}
void UnitWnd::setBackColor(DWORD color)
{
	m_backColor = color;
	resetRenderFlag();
}

HCURSOR UnitWnd::getCursor() const
{
	return m_cursor;
}
void UnitWnd::setCursor(HCURSOR cursor)
{
	m_cursor = cursor;
}
void UnitWnd::setChildPriority(INT value)
{
	m_childPriority = value;
}

bool UnitWnd::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	Rect2DF rect;
	if(!isRenderFlag())
	{
		if(m_backColor != 0)
		{
			getClientRect(&rect);
			DIRECT3D::VectorObject vo;
			vo.drawBox((FLOAT)rect.left,(FLOAT)rect.top,(FLOAT)rect.right,(FLOAT)rect.bottom,m_backColor);
			create(&vo);
		}
		onPaint();
	}
	return UnitVector::onRender(world,x,y,z);
}
void UnitWnd::onMove(FLOAT x,FLOAT y)
{
	if(getWindowStat() != WINDOW_MAX)
	{
		setPosX(x);
		setPosY(y);
		UnitWndMessage msg;
		msg.setMessage(WND_MOVE);
		callEvent(&msg);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowThrough
// DirectX - 透過ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
HIT_TEST UnitWindowThrough::hitTest(FLOAT x,FLOAT y)
{
	if(isHit(x,y))
		return HIT_THROUGH;
	return HIT_NONE;
}




//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitButton
// DirectX - ボタンコントロールクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitButton::UnitButton()
{
	m_unitText.setPos(1,1,0,1);
	add(&m_unitText);
	m_hover = false;
	m_click = false;
	addEvent(WND_MOUSE_HOVER,CLASSPROC(this,UnitButton,onMouseHover));
	addEvent(WND_MOUSE_OUT,CLASSPROC(this,UnitButton,onMouseOut));
	addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitButton,onMouseDown));
	addEvent(WND_MOUSE_UP,CLASSPROC(this,UnitButton,onMouseUp));
}
bool UnitButton::setSize(FLOAT width,FLOAT height,bool flag)
{
	UnitWnd::setSize(width,height,flag);

	m_unitText.setPos((width-m_unitText.getImageWidth())/2,(height-m_unitText.getImageHeight())/2);
	return true;
}
void UnitButton::onMouseHover(UnitWndMessage* m)
{
	m_hover = true;
	resetRenderFlag();
}
void UnitButton::onMouseOut(UnitWndMessage* m)
{
	m_hover = false;
	resetRenderFlag();
}

bool UnitButton::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		DWORD colorBorder1 = 0xffffffff;
		DWORD colorBorder2 = 0xff444444;
		DWORD color2 = 0xffcccccc;

		if(m_click)
		{
			colorBorder1 = 0xff444444;
			colorBorder2 = 0xffffffff;
		}
		else if(m_hover)
		{
			color2 = 0xffdddddd;
		}
		FLOAT width = (FLOAT)getWindowWidth();
		FLOAT height = (FLOAT)getWindowHeight();
		AFL::DIRECT3D::VectorObject vo;
		vo.drawLine(0,0,width-1,0,colorBorder1);
		vo.drawLine(0,0,0,height-1,colorBorder1);
		vo.drawLine(width-1,0,width-1,height-1,colorBorder2);
		vo.drawLine(0,height-1,width-1,height-1,colorBorder2);
		vo.drawBox(1,1,width-2,height-2,color2);

		vo.add(&m_vo);
		create(&vo);

		if(m_text.length())
		{

			m_unitText.setVisible(true);
			m_unitText.createText(m_text,m_font,0,-2,(INT)width-2,false,::D3DFMT_A1R5G5B5);
			m_unitText.setPosX(floor((width-m_unitText.getImageWidth())*0.5f+0.5f));

		}
		else
			m_unitText.setVisible(false);
	}
	return UnitVector::onRender(world,x,y,z);
}

void UnitButton::setClick(bool flag)
{
	m_click = flag;
	setSize(getWindowWidth(),getWindowHeight());
}
DWORD UnitButton::onMouseDown(UnitWndMessage* m)
{
	setClick(true);
	return 0;
}
DWORD UnitButton::onMouseUp(UnitWndMessage* m)
{
	setClick(false);
	return 0;
}
void UnitButton::setVector(VectorObject* vo)
{
	m_vo = *vo;
	resetRenderFlag();
}
void UnitButton::setText(LPCWSTR text)
{
	if(m_text != text)
	{
		m_text = text;
		resetRenderFlag();
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitPushButton
// DirectX - プッシュボタンコントロールクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitPushButton::UnitPushButton()
{
	m_unitText.setPos(1,1,0,1);
	add(&m_unitText);
	m_hover = false;
	m_click = false;
	addEvent(WND_MOUSE_HOVER,CLASSPROC(this,UnitPushButton,onMouseHover));
	addEvent(WND_MOUSE_OUT,CLASSPROC(this,UnitPushButton,onMouseOut));
	addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitPushButton,onMouseDown));
}
bool UnitPushButton::setSize(FLOAT width,FLOAT height,bool flag)
{
	UnitWnd::setSize(width,height,flag);

	m_unitText.setPos((width-m_unitText.getImageWidth())/2,(height-m_unitText.getImageHeight())/2);
	return true;
}
void UnitPushButton::onMouseHover(UnitWndMessage* m)
{
	m_hover = true;
	resetRenderFlag();
}
void UnitPushButton::onMouseOut(UnitWndMessage* m)
{
	m_hover = false;
	resetRenderFlag();
}

bool UnitPushButton::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		DWORD colorBorder1 = 0xffffffff;
		DWORD colorBorder2 = 0xff444444;
		DWORD color2 = 0xcccccccc;

		if(m_click)
		{
			colorBorder1 = 0xff444444;
			colorBorder2 = 0xffffffff;
		}
		else if(m_hover)
		{
			color2 = 0xffdddddd;
		}
		FLOAT width = (FLOAT)getWindowWidth();
		FLOAT height = (FLOAT)getWindowHeight();
		AFL::DIRECT3D::VectorObject vo;
		vo.drawLine(0,0,width-1,0,colorBorder1);
		vo.drawLine(0,0,0,height-1,colorBorder1);
		vo.drawLine(width-1,0,width-1,height-1,colorBorder2);
		vo.drawLine(0,height-1,width-1,height-1,colorBorder2);
		vo.drawBox(1,1,width-2,height-2,color2);

		vo.add(&m_vo);
		create(&vo);

		if(m_text.length())
		{

			m_unitText.setVisible(true);
			m_unitText.createText(m_text,m_font,0,-2,(INT)width-2,false,::D3DFMT_A1R5G5B5);
			m_unitText.setPos(floor((width-m_unitText.getImageWidth())*0.5f+0.5f),(height-m_unitText.getImageHeight())/2+0.5f);
		}
		else
			m_unitText.setVisible(false);
	}
	return UnitVector::onRender(world,x,y,z);
}

void UnitPushButton::setClick(bool flag)
{
	m_click = flag;
	setSize(getWindowWidth(),getWindowHeight());
}
DWORD UnitPushButton::onMouseDown(UnitWndMessage* m)
{
	setClick(!m_click);
	return 0;
}

void UnitPushButton::setVector(VectorObject* vo)
{
	m_vo = *vo;
	resetRenderFlag();
}
void UnitPushButton::setText(LPCWSTR text)
{
	if(m_text != text)
	{
		m_text = text;
		resetRenderFlag();
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowCaption
// DirectX - ウインドウタイトルクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowCaption::UnitWindowCaption()
{
	m_size = 18;

	m_buttonClose.setChildStyle(CHILD_AUTO);
	m_buttonClose.setPosW(100.1f);
	addChild(&m_buttonClose);

	m_buttonMax.setChildStyle(CHILD_AUTO);
	m_buttonMax.setPosW(100.1f);
	addChild(&m_buttonMax);

	m_buttonMin.setChildStyle(CHILD_AUTO);
	m_buttonMin.setPosW(100.1f);
	addChild(&m_buttonMin);

	add(&m_text);
	m_text.setPosX(3.0f);
	m_text.setPosY(1.0f);
	m_titleDraw = true;

	FLOAT size = m_size-4;

	AFL::DIRECT3D::VectorObject voClose;
	voClose.drawLine(3.0f,3.0f,size-4.0f,size-4.0f,0xff000000,1);
	voClose.drawLine(size-4.0f,3.0f,3.0f,size-4.0f,0xff000000,1);
	m_buttonClose.setVector(&voClose);
	m_buttonClose.setSize(size,size);
	m_buttonClose.setPosY(2);


	AFL::DIRECT3D::VectorObject voMax;
	voMax.drawLineBox(3,3,size-6.0f,size-6.0f,0xff000000,1);
	m_buttonMax.setVector(&voMax);
	m_buttonMax.setSize(size,size);
	m_buttonMax.setPosY(2);

	AFL::DIRECT3D::VectorObject voMin;
	voMin.drawLine(3.0f,size-3.0f,size-3.0f,size-3.0f,0xff000000,1);
	m_buttonMin.setVector(&voMin);
	m_buttonMin.setSize(size,size);
	m_buttonMin.setPosY(2);

}
void UnitWindowCaption::setMaxImage(bool flag)
{
	FLOAT size = m_size-4.0f;
	AFL::DIRECT3D::VectorObject voMax;
	if(flag)
		voMax.drawLineBox(3.0f,3.0f,size-6.0f,size-6.0f,0xff000000,1);
	else
	{
		voMax.drawLineBox(3.0f,3.0f,size-8.0f,size-8.0f,0xff000000,1);
		voMax.drawLineBox(5.0f,5.0f,size-8.0f,size-8.0f,0xff000000,1);
	}
	m_buttonMax.setVector(&voMax);
}

bool UnitWindowCaption::setSize(FLOAT width,FLOAT height,bool flag)
{
	height = m_size;
	UnitWnd::setSize(width,height,flag);

	DWORD color1 = 0xccbbbbbb;
	DWORD color2 = 0xbbff3333;
	AFL::DIRECT3D::VectorObject vo;
	vo.drawLineBox(0,0,(FLOAT)width,(FLOAT)height,color1);
	vo.drawBox(1,1,(FLOAT)width-2,(FLOAT)height-2,color2);
	m_titleDraw = true;
	create(&vo);

	FLOAT size = m_size-3;
	m_buttonClose.setPosX(width-size-1);
	m_buttonMax.setPosX(width-size*2-1);
	m_buttonMin.setPosX(width-size*3-1);
	return true;
}
void UnitWindowCaption::setTitle(LPCSTR title)
{
	setTitle(UCS2(title));
}
void UnitWindowCaption::setTitle(LPCWSTR title)
{
	if(m_title != title)
	{
		m_titleDraw = true;
		m_title = title;
	}
}
bool UnitWindowCaption::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		FLOAT width = getWindowWidth();
		if(m_titleDraw)
		{
			m_titleDraw = false;
			m_text.createText(m_title.c_str(),(INT)m_size-6,0xffffff,-2,(INT)width,false,::D3DFMT_A1R5G5B5);
		}
	}
	return UnitWnd::onRender(world,x,y,z);
}
UnitButton* UnitWindowCaption::getCloseButton()
{
	return &m_buttonClose;
}
UnitButton* UnitWindowCaption::getMaxButton()
{
	return &m_buttonMax;
}
UnitButton* UnitWindowCaption::getMinButton()
{
	return &m_buttonMin;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindow
// DirectX - ウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindow::UnitWindow()
{
	addEvent(WND_SIZE,CLASSPROC(this,UnitWindow,onSize));

	m_scrollVert.setChildStyle(CHILD_AUTO);
	m_scrollHori.setChildStyle(CHILD_AUTO);
	UnitWnd::addChild(&m_scrollVert);
	UnitWnd::addChild(&m_scrollHori);
	m_scrollVert.setSortStyle(SORT_CONTROL);
	m_scrollHori.setSortStyle(SORT_CONTROL);
	m_scrollVert.setBarType(BAR_VERT);
	m_scrollHori.setBarType(BAR_HORI);
	m_scrollVert.setVisible(false);
	m_scrollHori.setVisible(false);

	m_scrollBox.setVisible(false);
	m_scrollBox.setChildStyle(CHILD_AUTO);
	m_scrollBox.setSize(16,16);
	m_scrollBox.setBackColor(0xdddddddd);
	UnitWnd::addChild(&m_scrollBox);

	addEvent(WND_MOUSE_WHEEL,CLASSPROC(this,UnitWindow,onMouseWheel));

	m_caption.setChildStyle(CHILD_AUTO);
	m_caption.setSize(10,64);
	m_caption.setVisible(false);

	UnitWnd::addChild(&m_caption);
	UnitWnd::addChild(&m_client);
	m_client.setChildStyle(CHILD_CLIENT);
	m_frameStyle = FRAME_THICK;
	m_frameBorder = 0;
	m_borderColor1 = 0xfeffffff;
	m_borderColor2 = 0xfe444444;
	m_borderClient = 0xeebbbbbb;

	m_caption.getCloseButton()->addEvent(WND_MOUSE_CLICK,CLASSPROC(this,UnitWindow,onButtonClick));
	m_caption.getMaxButton()->addEvent(WND_MOUSE_CLICK,CLASSPROC(this,UnitWindow,onButtonClick));
	m_caption.getMinButton()->addEvent(WND_MOUSE_CLICK,CLASSPROC(this,UnitWindow,onButtonClick));

	m_frameStyle = FRAME_SIMPLE;

	m_scrollVert.addEvent(WND_SCROLL,CLASSPROC(this,UnitWindow,onScroll));
	m_scrollHori.addEvent(WND_SCROLL,CLASSPROC(this,UnitWindow,onScroll));

	m_compScroll = true;
	m_clientScroll = false;

}
UnitWindow::~UnitWindow()
{
	m_scrollType = SCROLL_MANUAL;
}

DWORD UnitWindow::onScroll(UnitWndMessage* m)
{
	UnitWndMessage msg;
	msg.setMessage(WND_SCROLL);
	callEvent(&msg);

	if(m_clientScroll)
	{
		m_client.setPos(-getScrollValue(BAR_HORI),-getScrollValue(BAR_VERT));
	}

	resetRenderFlag();
	return 0;

}
void UnitWindow::setScrollBar(BARTYPE type,bool flag)
{
	if(type == BAR_VERT)
		m_scrollVert.setVisible(flag);
	else
		m_scrollHori.setVisible(flag);
	resetRenderFlag();
	UnitWnd::recalcLayout();
}	

UnitScroll* UnitWindow::getScrollBar(BARTYPE type)
{
	if(type == BAR_VERT)
		return &m_scrollVert;
	return &m_scrollHori;
}
FLOAT UnitWindow::getScrollValue(BARTYPE type) const
{
	if(type == BAR_VERT)
		return m_scrollVert.getScrollValue();
	return m_scrollHori.getScrollValue();
}
void UnitWindow::setScrollValue(BARTYPE type,FLOAT value,bool flag)
{
	resetRenderFlag();
	if(type == BAR_VERT)
		return m_scrollVert.setScrollValue(value,flag);
	return m_scrollHori.setScrollValue(value,flag);
}
void UnitWindow::setScrollRange(BARTYPE type,FLOAT value)
{
	m_compScroll = true;
	resetRenderFlag();
	if(type == BAR_VERT)
		m_scrollVert.setScrollRange(value);
	else
		m_scrollHori.setScrollRange(value);
}
FLOAT UnitWindow::getScrollRange(BARTYPE type) const
{
	if(type == BAR_VERT)
		return m_scrollVert.getScrollRange();
	return m_scrollHori.getScrollRange();
}

void UnitWindow::setAutoScroll()
{
	Rect2DF rect;
	getInnerRect(&rect);

	if(m_scrollType == SCROLL_AUTO)
	{
		//スクロールバーの表示調整
		INT rangeX = (INT)m_scrollHori.getScrollRange();
		INT rangeY = (INT)m_scrollVert.getScrollRange();
		m_scrollHori.setVisible(false);
		m_scrollVert.setVisible(false);
		FLOAT width = rect.right - rect.left;
		FLOAT height = rect.bottom - rect.top;
		INT i;
		for(i=0;i<2;i++)
		{
			if(!m_scrollHori.isVisible() && rangeX > width)
			{
				m_scrollHori.setVisible(true);
				height -= (INT)m_scrollHori.getBarWeight();
			}
			if(!m_scrollVert.isVisible() && rangeY > height)
			{
				m_scrollVert.setVisible(true);
				width -= (INT)m_scrollVert.getBarWeight();
			}
		}
	}

	bool both = m_scrollVert.isVisible() && m_scrollHori.isVisible();
	if(both)
	{
		FLOAT width = rect.right - rect.left;
		FLOAT height = rect.bottom - rect.top;
		FLOAT weight = m_scrollVert.getBarWeight();
		m_scrollVert.setPos(rect.right-weight,rect.top);
		m_scrollVert.setSize(weight,height-weight);
		m_scrollHori.setPos(rect.left,rect.bottom-weight);
		m_scrollHori.setSize(width-weight,weight);
		rect.right -= m_scrollVert.getBarWeight();
		rect.bottom -= m_scrollHori.getBarWeight();
		m_scrollBox.setPos(rect.right,rect.bottom);
		m_scrollBox.setVisible(true);
	}
	else
	{
		FLOAT width = rect.right - rect.left;
		FLOAT height = rect.bottom - rect.top;
		FLOAT weight = m_scrollVert.getBarWeight();
		m_scrollVert.setPos(rect.right-weight,rect.top);
		m_scrollVert.setSize(weight,height);
		if(m_scrollVert.isVisible())
			rect.right -= m_scrollVert.getBarWeight();

		weight = m_scrollHori.getBarWeight();
		m_scrollHori.setPos(rect.left,rect.bottom-weight);
		m_scrollHori.setSize(width,weight);
		if(m_scrollHori.isVisible())
			rect.bottom -= m_scrollHori.getBarWeight();
		m_scrollBox.setVisible(false);
	}
}

void UnitWindow::recalcLayout2(Rect2DF* rect1,bool flag)
{
	if(m_caption.isVisible())
	{
		Rect2DF rect;
		getInnerRect(&rect);
		FLOAT width = rect.right - rect.left;
		FLOAT height = rect.bottom - rect.top;
		m_caption.moveWindow(rect.left,rect.top,width,height);
	}
	UnitWnd::recalcLayout2(rect1,flag);
}
void UnitWindow::getClientRect(Rect2DF* rect)
{
	UnitWnd::getClientRect(rect);

	//ボーダー設定
	if(m_frameBorder>=1.0f)
	{
		rect->top += m_frameBorder;
		rect->left += m_frameBorder;
		rect->bottom -= m_frameBorder;
		rect->right -= m_frameBorder;
	}
	//キャプション設定
	if(m_caption.isVisible())
		rect->top += m_caption.getWindowHeight();

	//スクロールバー
	if(m_scrollVert.isVisible())
	{
		rect->right -= (INT)m_scrollVert.getBarWeight();
	}
	if(m_scrollHori.isVisible())
	{
		rect->bottom -= (INT)m_scrollHori.getBarWeight();
	}
}

void UnitWindow::onSize(UnitWndMessage* m)
{
	setAutoScroll();
	m_compScroll = true;
}
bool UnitWindow::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		if(m_frameBorder < 1.0f)
		{
			AFL::DIRECT3D::VectorObject vo;
			create(&vo);
		}
		else if(m_frameBorder < 2.0f)
		{
			FLOAT x1 = 0;
			FLOAT y1 = 0;
			FLOAT x2 = (FLOAT)m_width-m_frameBorder;
			FLOAT y2 = (FLOAT)m_height-m_frameBorder;
			AFL::DIRECT3D::VectorObject vo;
			vo.drawLine(x1,y1,x2,y1,m_borderColor2);
			vo.drawLine(x1,y1,x1,y2,m_borderColor2);
			vo.drawLine(x1,y2,x2,y2,m_borderColor2);
			vo.drawLine(x2,y1,x2,y2,m_borderColor2);
			create(&vo);
		}
		else
		{
			FLOAT x = (FLOAT)m_width - m_frameBorder;
			FLOAT y = (FLOAT)m_height - m_frameBorder;

			VectorObject vo;
			vo.drawLine(0,0,x+m_frameBorder-1,0,m_borderColor1);
			vo.drawLine(0,1,0,y+m_frameBorder-1,m_borderColor1);
			vo.drawLine(1,1,1+x,1,  m_borderClient,(FLOAT)m_frameBorder-2);
			vo.drawLine(1,1,1,  1+y,m_borderClient,(FLOAT)m_frameBorder-2);
			vo.drawLine((FLOAT)m_frameBorder-1,(FLOAT)m_frameBorder-1,x,(FLOAT)m_frameBorder-1,m_borderColor2);
			vo.drawLine((FLOAT)m_frameBorder-1,(FLOAT)m_frameBorder-1,(FLOAT)m_frameBorder-1,y,m_borderColor2);

			vo.drawLine((FLOAT)m_frameBorder-1,y,x,y,m_borderColor1);
			vo.drawLine(x,(FLOAT)m_frameBorder-1,x,y,m_borderColor1);
			vo.drawLine(1,y+1,x+1,y+1,m_borderClient,(FLOAT)m_frameBorder-2);
			vo.drawLine(x+1,1,x+1,y+1,m_borderClient,(FLOAT)m_frameBorder-2);
			vo.drawLine(0,y+m_frameBorder-1,x+m_frameBorder-1,y+m_frameBorder-1,m_borderColor2);
			vo.drawLine(x+m_frameBorder-1,0,x+m_frameBorder-1,y+m_frameBorder-1,m_borderColor2);
			create(&vo);
		}

	}
	return UnitWnd::onRender(world,x,y,z);
}

DWORD UnitWindow::onButtonClick(UnitWndMessage* m)
{
	UnitWnd* unit = m->getWnd();
	if(unit == m_caption.getCloseButton())
	{
		UnitWndMessage m2;
		m2.setDefaultProc(false);
		m2.setMessage(WND_CLOSE);
		callEvent(&m2);
	}
	else if(unit == m_caption.getMaxButton())
	{


		if(getWindowStat() == WINDOW_MAX)
		{
			m_caption.setMaxImage(true);
			m_frameBorder = m_frameBorderKeep;
			setWindowStat(WINDOW_NORMAL);

			UnitWndMessage m2;
			m2.setDefaultProc(false);
			m2.setMessage(WND_NORMAL);
			callEvent(&m2);
		}
		else
		{
			m_caption.setMaxImage(false);
			m_frameBorderKeep = m_frameBorder;
			m_frameBorder = 0;
			setWindowStat(WINDOW_MAX);

			UnitWndMessage m2;
			m2.setDefaultProc(false);
			m2.setMessage(WND_MAX);
			callEvent(&m2);
		}

	}
	else if(unit == m_caption.getMinButton())
	{
		UnitWndMessage m2;
		m2.setDefaultProc(false);
		m2.setMessage(WND_MIN);
		callEvent(&m2);
	}
	return 0;
}

FLOAT UnitWindow::getBorderSize() const
{
	return m_frameBorder;
}
void UnitWindow::setScrollSize(INT x,INT y)
{
	setScrollRange(BAR_HORI,(FLOAT)x);
	setScrollRange(BAR_VERT,(FLOAT)y);
}
SCROLL_TYPE UnitWindow::getScrollType() const
{
	return m_scrollType;
}

void UnitWindow::setScrollType(SCROLL_TYPE type)
{
	m_scrollType = type;
}

void UnitWindow::setBorderSize(FLOAT size)
{
	m_frameBorder = size;
}
void UnitWindow::setBorderStyle(FRAME_STYLE style)
{
	m_frameStyle = style;
}

void UnitWindow::getInnerRect(Rect2DF* rect)
{
	if(m_frameBorder>=1.0f)
	{
		rect->top = m_frameBorder;
		rect->left = m_frameBorder;
		rect->bottom = m_height - m_frameBorder;
		rect->right = m_width - m_frameBorder;
	}
	else
	{
		rect->top = 0;
		rect->left = 0;
		rect->bottom = m_height;
		rect->right = m_width;
	}
}
void UnitWindow::setTitle(LPCSTR title)
{
	setTitle(UCS2(title));
}
void UnitWindow::setTitle(LPCWSTR title)
{
	m_caption.setTitle(title);
}
void UnitWindow::setCaptionVisible(bool flag)
{
	m_caption.setVisible(flag);
	recalcLayout();
}


HIT_TEST UnitWindow::hitTest(FLOAT x,FLOAT y)
{
	if(!isHit(x,y))
		return HIT_NONE;
	if(getWindowStat() == WINDOW_MIN)
		return HIT_CAPTION;

	const static int margin = 8;

	if(m_caption.isVisible() && m_caption.isHit(x,y))
		return HIT_CAPTION;
	if(m_scrollVert.isVisible() && m_scrollVert.isVisible() && m_scrollBox.isHit(x,y))
	{
		if(m_frameStyle != FRAME_SIMPLE)
			return HIT_FRAME_BOTTOM_RIGHT;
		return HIT_OHTER;
	}
	x -= getAbsX();
	y -= getAbsY();
	Rect2DF rect;
	getInnerRect(&rect);

	if(x >= rect.left && y>= rect.top && x < rect.right && y < rect.bottom)
		return HIT_CLIENT;
	if(m_frameStyle != FRAME_SIMPLE)
	{
		if(x < rect.left+margin)
		{
			if(y < rect.top+margin)
				return HIT_FRAME_TOP_LEFT;
			if(y > rect.bottom-margin)
				return HIT_FRAME_BOTTOM_LEFT;
		}
		if(x >= rect.right-margin)
		{
			if(y < rect.top+margin)
				return HIT_FRAME_TOP_RIGHT;
			if(y > rect.bottom-margin)
				return HIT_FRAME_BOTTOM_RIGHT;
		}

		if(x < rect.left)
			return HIT_FRAME_LEFT;
		if(x >= rect.right)
			return HIT_FRAME_RIGHT;
		if(y < rect.top)
			return HIT_FRAME_TOP;
		return HIT_FRAME_BOTTOM;
	}
	return HIT_NONE;
}
void UnitWindow::addChild(UnitWnd* unit)
{
	m_client.addChild(unit);
}
void UnitWindow::delChild(UnitWnd* unit)
{
	m_client.delChild(unit);
}
DWORD UnitWindow::onMouseWheel(UnitWndMessage* m)
{
	m_scrollVert.setScrollValue(m_scrollVert.getScrollValue()-(m_scrollVert.getButtonValue()*m->getParamInt(1)/120));
	resetRenderFlag();
	return 0;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindow
// DirectX - ウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowFrame::UnitWindowFrame()
{
	setMinSize(32,32);
	m_caption.setVisible(true);
	setBorderSize(4);
	m_frameStyle = FRAME_EDGE;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitScroll
// DirectX - スクロールバー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitScroll::UnitScroll()
{
	m_scrollValue = 0.0f;
	m_targetValue = 0.0f;
	m_buttonValue = 48.0f;
	m_barAreaSize = 100;
	m_scrollRange = 0;
	m_barPosition = 0;
	m_barSize = 0;
	m_barWeight = 16;

	m_barFrame.setChildStyle(CHILD_CLIENT);
	m_barFrame.setBackColor(0xeeeeeeee);
	setSize(m_barWeight,m_barWeight);

	addChild(&m_barFrame);
	addChild(&m_button2);
	addChild(&m_button1);
	m_barFrame.addChild(&m_bar);
	setBarType(BAR_HORI);

	m_button1.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitScroll,onClickUp));
	m_button2.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitScroll,onClickDown));
	m_bar.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitScroll,onBarDown));
	m_bar.addEvent(WND_MOUSE_DRAG,CLASSPROC(this,UnitScroll,onBarDrag));
}

UnitScroll::~UnitScroll()
{
}
void UnitScroll::setBarType(BARTYPE type)
{
	m_barType = type;
	if(m_barType == BAR_VERT)
	{
		m_button1.setChildStyle(CHILD_TOP);
		m_button2.setChildStyle(CHILD_BOTTOM);
	}
	else
	{
		m_button1.setChildStyle(CHILD_LEFT);
		m_button2.setChildStyle(CHILD_RIGHT);
	}
	m_button1.setSize(m_barWeight,m_barWeight);
	m_button2.setSize(m_barWeight,m_barWeight);
	setScrollValue(m_scrollValue);
}
void UnitScroll::setBarPosition(FLOAT value)
{
	if(m_barType == BAR_VERT)
	{
		m_bar.setPosX(0);
		m_bar.setPosY(value);
	}
	else
	{
		m_bar.setPosY(0);
		m_bar.setPosX(value);
	}
	m_barPosition = value;
}
bool UnitScroll::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		DIRECT3D::VectorObject vo1,vo2;

		FLOAT width = getWindowWidth();
		FLOAT height = getWindowHeight();
		if(m_barType == BAR_VERT)
		{
			m_barAreaSize = height - m_barWeight * 2;
			if(height < m_scrollRange)
				m_barSize =  m_barAreaSize * height / m_scrollRange;
			else
				m_barSize =  m_barAreaSize;
			if(m_barSize < 2)
				m_barSize = 2;
			setButtonSize(&m_bar,m_barWeight,m_barSize);
			vo1.drawTriangle(m_barWeight/2,m_barWeight/3, m_barWeight/5,m_barWeight*2/3, m_barWeight*4/5,m_barWeight*2/3,0xff000000);
			vo2.drawTriangle(m_barWeight/2,m_barWeight*2/3, m_barWeight/5,m_barWeight/3, m_barWeight*4/5,m_barWeight/3,0xff000000);
		}
		else
		{
			m_barAreaSize = width - m_barWeight * 2;
			if(width < m_scrollRange)
				m_barSize =  m_barAreaSize * width / m_scrollRange;
			else
				m_barSize =  m_barAreaSize;
			if(m_barSize < 2)
				m_barSize = 2;
			setButtonSize(&m_bar,m_barSize,m_barWeight);
			vo1.drawTriangle(m_barWeight/3,m_barWeight/2, m_barWeight*2/3,m_barWeight/5, m_barWeight*2/3,m_barWeight*4/5,0xff000000);
			vo2.drawTriangle(m_barWeight*2/3,m_barWeight/2, m_barWeight/3,m_barWeight/5, m_barWeight/3,m_barWeight*4/5,0xff000000);
		}

		m_button1.setVector(&vo1);
		m_button2.setVector(&vo2);
		if(m_scrollRange != 0)
			setBarPosition(m_scrollValue*(m_barAreaSize)/m_scrollRange);
	}
	return UnitWnd::onRender(world,x,y,z);
}

void UnitScroll::setButtonSize(UnitWnd* unit,FLOAT width,FLOAT height)
{
	unit->setSize(width,height);
	setScrollValue(m_targetValue);
	const DWORD colorBorder1 = 0xffffffff;
	const DWORD colorBorder2 = 0xff333333;
	const DWORD color2 = 0xffcccccc;

	AFL::DIRECT3D::VectorObject vo;
	vo.drawBox(1,1,width-2,height-2,color2);
	vo.drawLine(0,0,width-1,0,colorBorder1);
	vo.drawLine(0,0,0,height-1,colorBorder1);
	vo.drawLine(width-1,0,width-1,height-1,colorBorder2);
	vo.drawLine(0,height-1,width-1,height-1,colorBorder2);
	unit->create(&vo);

}
void UnitScroll::onClickUp(UnitWndMessage* m)
{
	setScrollValue(getScrollValue()-m_buttonValue);
	m->setDefaultProc(false);
}
void UnitScroll::onClickDown(UnitWndMessage* m)
{
	setScrollValue(getScrollValue()+m_buttonValue);
	m->setDefaultProc(false);
}
void UnitScroll::onBarDown(UnitWndMessage* m)
{
	m_barDragStartX = m->getParamFloat(0);
	m_barDragStartY = m->getParamFloat(1);
	m_barBasePosition = getBarPosition();
	m->setDefaultProc(false);
}
void UnitScroll::onBarUp(UnitWndMessage* m)
{
	m->setDefaultProc(false);
}
void UnitScroll::onBarDrag(UnitWndMessage* m)
{
	FLOAT p;
	if(m_barType == BAR_VERT)
	{
		p = m_barBasePosition - m_barDragStartY + m->getParamFloat(1);
	}
	else
	{
		p = m_barBasePosition - m_barDragStartX + m->getParamFloat(0);
	}
	setScrollValue(m_scrollRange * p / m_barAreaSize);
}

FLOAT UnitScroll::getBarPosition() const
{
	return m_barPosition;
}
FLOAT UnitScroll::getBarWeight() const
{
	return m_barWeight;
}
void UnitScroll::setScrollRange(FLOAT range)
{
	m_scrollRange = range;
	resetRenderFlag();
}
FLOAT UnitScroll::getScrollRange() const
{
	return m_scrollRange;
}


void UnitScroll::setScrollValue(FLOAT value,bool flag)
{
	if(m_barType == BAR_VERT)
	{
		if(value > m_scrollRange-getWindowHeight())
			value = m_scrollRange-getWindowHeight();
	}
	else
	{
		if(value > m_scrollRange-getWindowWidth())
			value = m_scrollRange-getWindowWidth();
	}
	if(value < 0)
		value = 0;

	m_targetValue = value;
	if(!flag)
		m_scrollValue = value;
}
FLOAT UnitScroll::getScrollValue() const
{
	return m_scrollValue;
}
void UnitScroll::setButtonValue(FLOAT value)
{
	m_buttonValue = value;
}
FLOAT UnitScroll::getButtonValue() const
{
	return m_buttonValue;
}
void UnitScroll::onAction(World* world,LPVOID value)
{
	if(m_scrollValue != m_targetValue)
	{
		FLOAT speed = (m_targetValue - m_scrollValue);
		FLOAT aSpeed = abs(speed);
		if(aSpeed > 5)
			speed /= 2.0f;
		
		if(aSpeed < 1)
			m_scrollValue = m_targetValue;
		else
		{
			m_scrollValue += speed;
		}

		UnitWndMessage m;
		m.setMessage(WND_SCROLL);
		callEvent(&m);
		resetRenderFlag();
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSlider
// DirectX - スライダークラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitSlider::UnitSlider()
{
	m_lineBold = 6.0f;
	m_barBold = 11.0f;
	m_slideCount = 5;
	m_slideValue = 0;
	m_bar.addEvent(WND_MOUSE_UP,CLASSPROC(this,UnitSlider,onBarUp));
	m_bar.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitSlider,onBarDown));
	m_bar.addEvent(WND_MOUSE_DRAG,CLASSPROC(this,UnitSlider,onBarDrag));
	addChild(&m_bar);
}
UnitSlider::~UnitSlider()
{
}
INT UnitSlider::getSliderPos() const
{
	return m_slideValue;
}
void UnitSlider::setSliderPos(INT pos)
{
	FLOAT width = getWindowWidth() - m_barBold;
	m_bar.setPosX(pos *width / (m_slideCount-1));
}


void UnitSlider::onBarDown(UnitWndMessage* m)
{
	m_barDragStartX = m->getParamFloat(0);
	m_barDragStartY = m->getParamFloat(1);
	m_barBasePosition = m_bar.getPosX();
	m->setDefaultProc(false);
}
void UnitSlider::onBarUp(UnitWndMessage* m)
{
	setSliderPos(getSliderPos());

	m->setDefaultProc(false);
}

void UnitSlider::onBarDrag(UnitWndMessage* m)
{
	FLOAT p;
	p = m_barBasePosition - m_barDragStartX + m->getParamFloat(0);
	if(p < 0)
		p = 0;
	else if(p > getWindowWidth() - m_barBold)
		p = getWindowWidth() - m_barBold;
	m_bar.setPosX(p);

	FLOAT width = getWindowWidth() - m_barBold;
	INT value = (INT)(m_bar.getPosX() / width * (m_slideCount-1) + 0.5f);

	if(m_slideValue != value)
	{
		m_slideValue = value;
		UnitWndMessage m;
		m.setMessage(WND_VALUE);
		callEvent(&m);
	}
}


bool UnitSlider::setSize(FLOAT width,FLOAT height,bool flag)
{
	m_bar.setPosY(2.0f);
	m_bar.setSize(m_barBold,height-4);
	return UnitWnd::setSize(width,height,flag);
}
bool UnitSlider::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		DWORD colorBorder1 = 0xff444444;
		DWORD colorBorder2 = 0xffffffff;
		DWORD color2 = 0xffcccccc;

		FLOAT bold = m_lineBold;
		FLOAT width = (FLOAT)getWindowWidth()-m_barBold;
		FLOAT height = (FLOAT)getWindowHeight();
		FLOAT x = m_barBold / 2;
		FLOAT y = (height - bold) / 2;

		AFL::DIRECT3D::VectorObject vo;

		if(m_backColor)
			vo.drawBox(0,0,getWindowWidth(),getWindowHeight(),m_backColor);

		vo.drawLine(x,y,x+width,y,colorBorder1);
		//vo.drawLine(x,y,x,y+bold-1,colorBorder1);
		//vo.drawLine(x+width,y,x+width,y+bold-1,colorBorder2);
		vo.drawLine(x,y+bold-1,x+width,y+bold-1,colorBorder2);
		vo.drawBox(x+1,y+1,width-2,bold-2,color2);

		FLOAT s = width / (m_slideCount-1);
		INT i;
		for(i=0;i<m_slideCount;i++)
			vo.drawLine(x+i*s,2,x+i*s,height-1-2,0x88444444);

		//vo.add(&m_vo);
		create(&vo);
	}
	return UnitVector::onRender(world,x,y,z);
}
}}
