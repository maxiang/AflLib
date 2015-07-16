#include <windows.h>
#include "aflDirect3DWindow.h"

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
using namespace Gdiplus;



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWndSplit
// DirectX - スプリットウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWndSplit::UnitWndSplit()
{
	setBarType(BAR_HORI);
	m_splitBorder = 5;
	m_borderColor1 = 0xfeffffff;
	m_borderColor2 = 0xfe444444;
	m_borderClient = 0xeebbbbbb;
}
bool UnitWndSplit::setSize(FLOAT width,FLOAT height,bool flag)
{
	UnitWnd::setSize(width,height,flag);

	FLOAT x = width - m_splitBorder;
	FLOAT y = height - m_splitBorder;

	AFL::DIRECT3D::VectorObject vo;
	if(m_barType == BAR_HORI)
	{
		vo.drawLine(0,0,(FLOAT)x+m_splitBorder,0,m_borderColor1);
		vo.drawLine(0,1,(FLOAT)x+m_splitBorder,1,m_borderClient,(FLOAT)m_splitBorder-2);
		vo.drawLine(0,(FLOAT)m_splitBorder-1,(FLOAT)x+m_splitBorder,(FLOAT)m_splitBorder-1,m_borderColor2);

		m_width = width;
		m_height = m_splitBorder;
	}
	else
	{
		vo.drawLine(0,0,0,(FLOAT)y+m_splitBorder,m_borderColor1);
		vo.drawLine(1,0,1,(FLOAT)y+m_splitBorder,m_borderClient,(FLOAT)m_splitBorder-2);
		vo.drawLine((FLOAT)m_splitBorder-1,0,(FLOAT)m_splitBorder-1,(FLOAT)y+m_splitBorder,m_borderColor2);

		m_width = m_splitBorder;
		m_height = height;
	}
	create(&vo);
	return true;
}
HIT_TEST UnitWndSplit::hitTest(FLOAT x,FLOAT y)
{
	if(UnitWnd::hitTest(x,y) != HIT_NONE)
		return HIT_CAPTION;
	return HIT_NONE;
}
FLOAT UnitWndSplit::getBarSize() const
{
	return m_splitBorder;
}
void UnitWndSplit::setBarType(BARTYPE type)
{
	m_barType = type;
	if(type == BAR_VERT)
		m_cursor = (HCURSOR)LoadImage(0,IDC_SIZEWE,IMAGE_CURSOR,0,0,LR_SHARED);
	else
		m_cursor = (HCURSOR)LoadImage(0,IDC_SIZENS,IMAGE_CURSOR,0,0,LR_SHARED);
}
BARTYPE UnitWndSplit::getBarType() const
{
	return m_barType;
}
void UnitWndSplit::onMove(FLOAT x,FLOAT y)
{
	if(m_parentWnd)
	{
		Rect2DF rect;
		m_parentWnd->getClientRect(&rect);

		if(m_barType == BAR_HORI)
		{
			x = rect.left;
			if(y < rect.top)
				y = rect.top;
			FLOAT height = rect.bottom - rect.top;
			if(y > height - m_splitBorder)
				y = height - m_splitBorder;
		}
		else
		{
			y = rect.top;
			if(x < rect.left)
				x = rect.left;
			FLOAT width = rect.right - rect.left;
			if(x > width - m_splitBorder)
				x = width - m_splitBorder;
		}
	}
	UnitWnd::onMove(x,y);
}

UnitWindowSplit::UnitWindowSplit()
{
	UnitWnd::addChild(&m_child[0]);
	UnitWnd::addChild(&m_child[1]);
	m_areaSize = 200;
	setChildStyle(CHILD_CLIENT);
	setSplitType(SPLIT_WE);
	UnitWnd::addChild(&m_split);
	m_split.setPosW(10000);
	m_split.addEvent(WND_MOUSE_DRAG,CLASSPROC(this,UnitWindowSplit,onBarDrag));
}

void UnitWindowSplit::addChild(UnitWnd* unit,INT index)
{
	if(index < 0 || index >= 2)
		return;
	m_child[index].addChild(unit);
}
void UnitWindowSplit::delChild(UnitWnd* unit)
{
	m_child[0].delChild(unit);
	m_child[1].delChild(unit);
}

bool UnitWindowSplit::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	Rect2DF rect;

	UnitWnd::getClientRect(&rect);

	getClientRect(&rect,0);
	getClientRect(&rect,1);
	return UnitVector::onRender(world,x,y,z);
}
void UnitWindowSplit::getClientRect(Rect2DF* rect,INT index)
{
	FLOAT width = getWindowWidth();
	FLOAT height = getWindowHeight();
	if(m_split.getBarType() == BAR_VERT)
	{
		if(index == 0)
		{
			rect->left = 0;
			rect->top = 0;
			rect->right = m_split.getPosX();
			rect->bottom = height;
		}
		else
		{
			rect->left = m_split.getPosX()+m_split.getBarSize();
			rect->top = 0;
			rect->right = width;
			rect->bottom = height;
		}
	}
	else
	{
		if(index == 0)
		{
			rect->left = 0;
			rect->top = 0;
			rect->right = width;
			rect->bottom = m_split.getPosY();
		}
		else
		{
			rect->left = 0;
			rect->top = m_split.getPosY()+m_split.getBarSize();
			rect->right = width;
			rect->bottom = height;
		}
	}
}
void UnitWindowSplit::setSplitSize(FLOAT value)
{
	m_areaSize = value;
}
void UnitWindowSplit::recalcLayout2(Rect2DF* rect,bool flag)
{
	FLOAT width = rect->right - rect->left;
	FLOAT height = rect->bottom - rect->top;
	m_split.setSize(width,height);
	if(m_splitType == SPLIT_SN)
		m_split.setPos(0,height-m_areaSize-m_split.getBarSize());
	else if(m_splitType == SPLIT_NS)
		m_split.setPos(0,m_areaSize);
	else if(m_splitType == SPLIT_EW)
		m_split.setPos(width-m_areaSize-m_split.getBarSize(),0);
	else if(m_splitType == SPLIT_WE)
		m_split.setPos(m_areaSize,0);
	if(m_split.getBarType() == BAR_VERT)
	{
		m_child[0].setPosX(0);
		m_child[0].setPosY(0);
		m_child[0].setSize(m_split.getPosX(),height,flag);
		m_child[1].setPosX(m_split.getPosX()+m_split.getBarSize());
		m_child[1].setPosY(0);
		m_child[1].setSize(width-m_split.getPosX()-m_split.getBarSize(),height,flag);
	}
	else
	{
		m_child[0].setPosX(0);
		m_child[0].setPosY(0);
		m_child[0].setSize(width,m_split.getPosY(),flag);
		m_child[1].setPosX(0);
		m_child[1].setPosY(m_split.getPosY()+m_split.getBarSize());
		m_child[1].setSize(width,height-m_split.getPosY()-m_split.getBarSize(),flag);
	}
}

void UnitWindowSplit::setSplitType(SPLIT_TYPE type)
{
	m_splitType = type;
	if(type == SPLIT_SN || type == SPLIT_NS)
		m_split.setBarType(BAR_HORI);
	else
		m_split.setBarType(BAR_VERT);
	m_split.setPos(m_areaSize,m_areaSize);
	//UnitWnd::recalcLayout();
}
void UnitWindowSplit::onBarDrag(UnitWndMessage* m)
{
	if(m_splitType == SPLIT_NS)
		m_areaSize = m_split.getPosY();
	else if(m_splitType == SPLIT_SN)
		m_areaSize = getWindowHeight() - m_split.getBarSize() - m_split.getPosY();
	else if(m_splitType == SPLIT_WE)
		m_areaSize = m_split.getPosX();
	else if(m_splitType == SPLIT_EW)
		m_areaSize = getWindowWidth() - m_split.getBarSize() - m_split.getPosX();
	UnitWnd::recalcLayout();
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowPanel
// DirectX - ウインドウ用パネルパーツ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowPanel::UnitWindowPanel()
{
	setChildStyle(CHILD_TOP);
	m_borderColor1 = 0xfeffffff;
	m_borderColor2 = 0xfe444444;
	m_borderClient = 0xa0bbbbbb;
	setBackColor(m_borderClient);
	setPadding(1,1,1,1);
	setSize(20,20);
}
bool UnitWindowPanel::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		FLOAT x1 = 0;
		FLOAT y1 = 0;
		FLOAT x2 = (FLOAT)m_width-1;
		FLOAT y2 = (FLOAT)m_height-1;
		AFL::DIRECT3D::VectorObject vo;
		vo.drawLine(x1,y1,x2,y1,m_borderColor1);
		vo.drawLine(x1,y1+1,x1,y2,m_borderColor1);
		vo.drawLine(x1,y2,x2,y2,m_borderColor2);
		vo.drawLine(x2,y1,x2,y2,m_borderColor2);
		vo.drawBox(x1+1,y1+1,(FLOAT)m_width-2,(FLOAT)m_height-2,getBackColor());
		create(&vo);
	}
	return UnitWindow::onRender(world,x,y,z);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowCampus
// DirectX - イメージウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowCampus::UnitWindowCampus()
{
	addEvent(WND_SIZE,CLASSPROC(this,UnitWindowCampus,onSize));
	m_gdip.setViewClip(true);
	m_gdip.setChainClip(true);
	add(&m_gdip);
}
Gdiplus::Graphics* UnitWindowCampus::getGraphics()
{
	return m_gdip.getGraphics();
}
UnitGdip* UnitWindowCampus::getGdip()
{
	return &m_gdip;
}
INT UnitWindowCampus::getImageWidth() const
{
	return m_gdip.getImageWidth();
}
INT UnitWindowCampus::getImageHeight() const
{
	return m_gdip.getImageHeight();
}

void UnitWindowCampus::onScroll(UnitWndMessage* m)
{
	resetRenderFlag();
}
void UnitWindowCampus::onSize(UnitWndMessage* m)
{
	resetRenderFlag();
	Rect2DF rect;
	getClientRect(&rect);
	m_gdip.setPos((FLOAT)rect.left,(FLOAT)rect.top);

	FLOAT width = rect.right - rect.left;
	FLOAT height = rect.bottom - rect.top;
	if(width > 0 && height > 0)
		m_gdip.setSize((INT)width,(INT)height);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowText
// DirectX - テキストコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowText::UnitWindowText()
{
	m_textColor = 0xff000000;
}
void UnitWindowText::setText(LPCWSTR text)
{
	if(m_text != text)
	{
		m_text = text;
		resetRenderFlag();
	}
}

void UnitWindowText::onPaint()
{
	UnitGdip* g = getGdip();
	if(g)
	{
		g->clear(0);
		g->drawString(m_text,m_font,m_textColor);
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowEdit
// DirectX - テキストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

UnitWindowEdit::UnitWindowEdit()
{
	addEvent(WND_SIZE,CLASSPROC(this,UnitWindowEdit,onSize));

	setScrollType(SCROLL_AUTO);

	m_selectStart = 0;
	m_selectEnd = 0;
	m_select = false;
	m_multiLine = true;
	m_pointFlag = false;
	m_fepPoint = 0;
	m_cursolTime = 0;
	m_textPoint = 0;
	add(&m_cursol);
	m_cursol.setPosW(0.1f);

	//カーソルの作成
	VectorObject vo;
	vo.drawLine(0,0,0,(FLOAT)m_textArea.getFontHeight(),0x88000000);
	m_cursol.create(&vo);
	m_cursol.setViewClip(false);

	m_menu.setVisible(false);
	m_menu.addItem(L"切り取り");
	m_menu.addItem(L"コピー");
	m_menu.addItem(L"貼り付け");
	m_menu.addEvent(WND_LIST_ITEM_CLICK,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
	addChild(&m_menu);
	m_menu.setChainClip(false);

	//メッセージコールバックの設定
	addEvent(WND_CHAR,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
	addEvent(WND_KEYDOWN,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
	addEvent(WND_MOUSE_UP,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
	addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
	addEvent(WND_MOUSE_MOVE,CLASSPROC(this,UnitWindowEdit,onUnitMessage));
}

void UnitWindowEdit::onMessage(Message* m)
{
	if(!isActive())
		return;
	switch(m->getMsg())
	{
	case WM_IME_STARTCOMPOSITION:
		//IME有効時にテキストを描画させない
		m_fepPoint = m_textPoint;
		m->setDefault(false);
		break;
	case WM_IME_COMPOSITION:
		//IMEテキスト変更時の描画処理
		{
			WString work;
			m_fep.getString(work);
			if(work != m_fepString)
			{
				if(m_fepString.length())
					m_textArea.eraseText(m_fepPoint,(INT)m_fepString.length());

				if(work.length())
				{
					m_textArea.insertText(m_fepPoint,work);
				}
				m_fepString = work;
				resetRenderFlag();
			}
		}
		m_fepCursol =  m_fep.getCursorPos();
		if(m_textPoint != m_fepPoint + m_fepCursol)
		{
			m_textPoint = m_fepPoint + m_fepCursol;
			setCursolPos();
		}
		break;
	case WM_IME_ENDCOMPOSITION:
		m_textPoint = m_fepPoint;
		break;
	case WM_IME_NOTIFY:
		//無視するメッセージ
		switch(m->getWParam())
		{
			case IMN_OPENSTATUSWINDOW:
			case IMN_CLOSESTATUSWINDOW:
		//	case IMN_OPENCANDIDATE:
		//	case IMN_CHANGECANDIDATE:
		//	case IMN_CLOSECANDIDATE:
				m->setDefault(false);
		}
		break;
	}
}
bool UnitWindowEdit::delSelect()
{
	if(isSelect())
	{
		m_textArea.eraseText(getSelectStart(),getSelectLength());
		m_selectStart = getSelectStart(); 
		m_selectEnd = m_selectStart;
		m_textPoint = m_selectStart;
		resetRenderFlag();
		return true;
	}
	return false;
}
void UnitWindowEdit::copy()
{
	WString text;
	text.append(m_textArea.getString(),getSelectStart(),getSelectLength());
	ClipBoard::setString(text);
}
void UnitWindowEdit::paste()
{
	WString text;
	delSelect();
	ClipBoard::getString(text);
	m_textArea.insertText(m_textPoint,text);
	m_textPoint += (INT)text.length();
	resetRenderFlag();
}
void UnitWindowEdit::cut()
{
	WString text;
	text.append(m_textArea.getString(),getSelectStart(),getSelectLength());
	ClipBoard::setString(text);
	delSelect();
	resetRenderFlag();
}
void UnitWindowEdit::onUnitMessage(UnitWndMessage* m)
{
	WCHAR c = m->getParamInt(0);
	m->setDefaultProc(false);
	if(m->getWnd() == &m_menu && m->getMessage()==WND_LIST_ITEM_CLICK)
	{
		INT index = m->getParamInt(0);
		switch(index)
		{
		case 0:
			cut();
			break;
		case 1:
			copy();
			break;
		case 2:
			paste();
			break;
		}
	}
	else if(m->getMessage() == WND_CHAR)
	{
		if(GetAsyncKeyState(VK_CONTROL) < 0)
		{
			if(c == 0x16)
			{
				//CTRL+V 
				paste();
			}
			else if((c == 0x18 || c == 0x3) && isSelect())
			{
				//CTRL+C CTRL+X
				if(c == 0x18) 
					cut();
				else
					copy();
			}
		}
		else
		{
			if(c == 0x1b)
			{
				setSelect();
				setActive(false);
			}
			else if(c == '\b')
			{
				if(!delSelect() && m_textPoint > 0)
				{
					m_textPoint--;
					m_textArea.eraseText(m_textPoint,1);
				}
			}
			else if(c == '\r')
			{
				if(m_multiLine)
				{
					delSelect();
					m_textArea.insertText(m_textPoint,L'\n');
					++m_textPoint;
					resetRenderFlag();
				}
			}
			else
			{
				delSelect();
				m_textArea.insertText(m_textPoint,c);
				++m_textPoint;
			}
		}
		m_pointFlag = true;
		resetRenderFlag();
	}
	else if(m->getMessage() == WND_KEYDOWN)
	{
		DWORD key = m->getParamInt(0);
		if(key == VK_LEFT)
			setPointLeft();
		else if(key == VK_RIGHT)
			setPointRight();
		else if(key == VK_UP)
			setPointUp();
		else if(key == VK_DOWN)
			setPointDown();
		else if(key == VK_DELETE)
		{
			if(!delSelect())
				m_textArea.eraseText(m_textPoint,1);
			resetRenderFlag();
		}
		m_cursol.setVisible(true);
		m_cursolTime = 0;
		m_pointFlag = true;
	}
	else if(m->getMessage() == WND_MOUSE_MOVE)
	{
		if(m_select)
		{
			FLOAT x = m->getParamFloat(0) - getAbsX()+getScrollValue(BAR_HORI);
			FLOAT y = m->getParamFloat(1) - getAbsY()+getScrollValue(BAR_VERT);
			INT point = m_textArea.getPoint((INT)x,(INT)y);
			if(m_selectEnd != point)
			{
				m_selectEnd = point;
				resetRenderFlag();
			}
		}
	}
	else if(m->getMessage() == WND_MOUSE_UP)
	{
		m_select = false;
	}
	else if(m->getMessage() == WND_MOUSE_DOWN)
	{
		FLOAT x = m->getParamFloat(0) - getAbsX();
		FLOAT y = m->getParamFloat(1) - getAbsY();
		if(m->getParamInt(0) == 0)
		{
			x += (INT)getScrollValue(BAR_HORI);
			y += (INT)getScrollValue(BAR_VERT);
			INT point = m_textArea.getPoint((INT)x,(INT)y);
			m_textPoint = point;
			POINT p;
			m_textArea.getPoint(&p,m_textPoint);
			m_cursol.setPos((FLOAT)p.x-getScrollValue(BAR_HORI),(FLOAT)p.y-getScrollValue(BAR_VERT));
			m_cursolTime = 0;
			m_cursol.setVisible(true);

			if(m_selectStart != m_selectEnd)
				resetRenderFlag();
			m_selectStart = point;
			m_selectEnd = point;
			m_select = true;
		}
		else
		{
			m_menu.setSelectItem(-1,false);
			m_menu.setPos(x,y);
			m_menu.setVisible(true);
			m_menu.setActive(true);
		}

	}

}
void UnitWindowEdit::setMultiLine(bool flag)
{
	m_multiLine = flag;
	resetRenderFlag();
}
void UnitWindowEdit::setSelect(INT start,INT end)
{
	if(start==-1)
	{
		m_selectStart = m_textPoint;
		m_selectEnd = m_textPoint;
	}
	else
	{
		m_selectStart = start;
		if(end == -1)
			m_selectEnd = m_textArea.getLength();
		else
			m_selectEnd = end;
	}

}

INT UnitWindowEdit::getSelectStart() const
{
	if(m_selectStart > m_selectEnd)
		return m_selectEnd;
	return m_selectStart;
}
INT UnitWindowEdit::getSelectEnd() const
{
	if(m_selectStart > m_selectEnd)
		return m_selectStart;
	return m_selectEnd;
}
INT UnitWindowEdit::getSelectLength() const
{
	if(m_selectStart > m_selectEnd)
		return m_selectStart - m_selectEnd;
	return m_selectEnd - m_selectStart;
}
bool UnitWindowEdit::isSelect() const
{
	return m_selectStart != m_selectEnd;
}
LPCWSTR UnitWindowEdit::getText() const
{
	return m_textArea.getString();
}
void UnitWindowEdit::setMask(bool flag)
{
	m_textArea.setMask(flag);
	resetRenderFlag();
}
bool UnitWindowEdit::isMask() const
{
	return m_textArea.isMask();
}
void UnitWindowEdit::setCursolPos()
{
	POINT p;
	m_textArea.getPoint(&p,m_textPoint);
	m_cursol.setPos((FLOAT)p.x,(FLOAT)p.y);
}
void UnitWindowEdit::setText(LPCWSTR text)
{
	m_pointFlag = true;
	m_textArea.setText(text);
	m_textPoint = (INT)wcslen(text);
	setScrollValue(BAR_VERT,0,false);
	setScrollValue(BAR_HORI,0,false);
	resetRenderFlag();
}
void UnitWindowEdit::addText(LPCWSTR text)
{
	m_pointFlag = true;
	m_textArea.insertText(m_textPoint,text);
	m_textPoint += (INT)wcslen(text);
	resetRenderFlag();
}
void UnitWindowEdit::onSize(UnitWndMessage* m)
{
	resetRenderFlag();
}
void UnitWindowEdit::onAction(World* world,LPVOID value)
{
	if(isActive())
	{
		m_cursolTime++;
		if(m_cursolTime > 14)
		{
			m_cursolTime = 0;
			m_cursol.setVisible(!m_cursol.isVisible());
		}
	}
	else
	{
		m_cursol.setVisible(false);
	}
	UnitWindow::onAction(world,value);
}

void UnitWindowEdit::onPaint()
{
	if(!m_fep.getWindow())
	{
		Window* window = getMasterWindow();
		if(window)
		{
			m_fep.setWindow(window->getWnd());
			window->addMessage(WM_IME_STARTCOMPOSITION,CLASSPROC(this,UnitWindowEdit,onMessage));
			window->addMessage(WM_IME_COMPOSITION,CLASSPROC(this,UnitWindowEdit,onMessage));
			window->addMessage(WM_IME_ENDCOMPOSITION,CLASSPROC(this,UnitWindowEdit,onMessage));
			window->addMessage(WM_IME_NOTIFY,CLASSPROC(this,UnitWindowEdit,onMessage));
		}
	}
	if(!isRenderFlag())
	{
		//GDI+の取得
		Gdiplus::Graphics* g = getGraphics();
		if(!g)
			return;


		//テキスト位置の再計算
		if(m_multiLine)
		{
			setScrollRange(BAR_HORI,(FLOAT)getImageWidth());
			m_textArea.compute(getImageWidth());
		}
		else
		{
			setScrollRange(BAR_HORI,10000.0f);
			m_textArea.compute(10000);
		}
		//スクロールレンジの調整
		INT areaHeight = m_textArea.getAreaHeight();
		setScrollRange(BAR_VERT,(FLOAT)areaHeight);
		if(m_multiLine)
			setScrollSize(0,areaHeight);

		//文字数からカーソルの位置の取得
		POINT p;
		m_textArea.getPoint(&p,m_textPoint);

		//クライアントサイズの取得
		Rect2DF rect;
		getClientRect(&rect);
		FLOAT width = rect.right - rect.left;
		FLOAT height = rect.bottom - rect.top;
		INT fontHeight = m_textArea.getFontHeight();
		//カーソル移動位置へスクロール
		if(m_pointFlag)
		{
			m_pointFlag = false;
			if((FLOAT)p.y - getScrollValue(BAR_VERT)-fontHeight  < 0)
				setScrollValue(BAR_VERT,(FLOAT)p.y-fontHeight);
			else if(p.y - getScrollValue(BAR_VERT) + fontHeight > height)
				setScrollValue(BAR_VERT,(FLOAT)p.y - height + fontHeight);

			if((FLOAT)p.x - getScrollValue(BAR_HORI)-fontHeight  < 0)
				setScrollValue(BAR_HORI,(FLOAT)p.x - fontHeight);
			else if((FLOAT)p.x - getScrollValue(BAR_HORI) + fontHeight/2 > width)
				setScrollValue(BAR_HORI,(FLOAT)p.x - width + fontHeight/2);
		}
		//カーソル位置の補正
		FLOAT x = p.x-getScrollValue(BAR_HORI);
		FLOAT y = p.y-getScrollValue(BAR_VERT);
		if(x < 0)
			x = 0;
		else if(x > width)
			x = width;
		if(y < 0)
			y = 0;
		else if(y > height-fontHeight)
			y = height-fontHeight;
		m_cursol.setPos((FLOAT)x,(FLOAT)y);

		INT dibHeight = getImageHeight();

		HDC hDC = CreateCompatibleDC(NULL);
		HFONT oldFont = (HFONT)SelectObject(hDC,m_textArea.getFont());
		Gdiplus::Font font(hDC);
		INT spaceWidth = m_textArea.getCharWidth(hDC,L' ');

		Gdiplus::StringFormat stringFormat(StringFormat::GenericTypographic());

		g->Clear(Gdiplus::Color(m_backColor));
		std::list<WINDOWS::TextArea::AREA>::iterator it;
		FLOAT posX = -getScrollValue(BAR_HORI);
		y = -getScrollValue(BAR_VERT);
		LPCWSTR src = m_textArea.getString();
		for(it=m_textArea.getArea().begin();it!=m_textArea.getArea().end();++it)
		{
			if(y >= dibHeight)
				break;
			if(y+fontHeight > 0)
			{
				INT i;
				LPCWSTR text = src + it->point;
				INT length = it->length;
				x = posX;
				WCHAR c = L'*';
				for(i=0;i<length;i++)
				{
					INT pt = it->point + i;
					INT width;

					if(!isMask())
						c = text[i];
					if(c == L'\t')
					{

						width = spaceWidth*4;
						width = width - ((INT)x % width);
					}
					else
						width = m_textArea.getCharWidth(hDC,c);

					if(pt >= m_fepPoint && pt < m_fepPoint+(INT)m_fepString.length())
					{
						//FEP文字列の表示
						g->DrawString(&c,1,&font,PointF((FLOAT)x,(FLOAT)y),
							&stringFormat,&SolidBrush(Color(0xffcc0000)));
					}
					else
					{
						//選択領域の背景色
						if(pt>=getSelectStart() && pt<getSelectEnd())
							getGdip()->drawBox((INT)x,(INT)y,(INT)width,fontHeight,0xDD88FF88);
						//文字列描画
						g->DrawString(&c,1,&font,PointF((FLOAT)x,(FLOAT)y),
							&stringFormat,&SolidBrush(Color(0xff000000)));
					}
					x += width;

				}

			}
			y += m_textArea.getFontHeight();
		}
		SelectObject(hDC,oldFont);
		DeleteDC(hDC);
	}
}
void UnitWindowEdit::setPoint(INT point)
{
	m_textPoint = point; 
}
void UnitWindowEdit::setPointUp()
{
	POINT p;
	m_textArea.getPoint(&p,m_textPoint);
	p.y -= m_textArea.getFontHeight();
	m_textPoint = m_textArea.getPoint(p.x,p.y);
	setCursolPos();
	resetRenderFlag();
}
void UnitWindowEdit::setPointDown()
{
	POINT p;
	m_textArea.getPoint(&p,m_textPoint);
	p.y += m_textArea.getFontHeight();
	m_textPoint = m_textArea.getPoint(p.x,p.y);
	setCursolPos();
	resetRenderFlag();
}
void UnitWindowEdit::setPointLeft()
{
	if(m_textPoint > 0)
		m_textPoint--;
	setCursolPos();
	resetRenderFlag();
}
void UnitWindowEdit::setPointRight()
{
	if(m_textPoint < m_textArea.getLength())
		m_textPoint++;
	setCursolPos();
	resetRenderFlag();
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitTextBox
// DirectX - テキストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitTextBox::UnitTextBox()
{
	setScrollType(SCROLL_MANUAL);
	//setScrollBar(BAR_VERT,false);
	setMultiLine(false);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowList
// DirectX - リストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowList::UnitWindowList()
{
	setScrollSize(0,0);
	setCursor((HCURSOR)LoadImage(0,IDC_SIZEWE,IMAGE_CURSOR,0,0,LR_SHARED));
	m_headerFont.setBold(600);
	m_headerFont.createFont();

	m_header.height = m_headerFont.getSize()+4;
	m_headerColor1 = 0xf0eeeeee;
	m_headerColor2 = 0xf0333333;
	m_headerClient = 0xe0bbbbbb;
	m_headerClientSelect = 0xe0cccccc;

	m_itemBackColor = 0xccffffff;
	m_itemTextColor = 0xff000000;


	setScrollType(SCROLL_AUTO);
	m_sortIndex = -1;
	m_sortOrder = true;
	m_sort = false;
	m_sortEnable = true;
	m_moveColumnIndex = -1;
	m_scrollColumnIndex = -1;

	m_itemPadding.left = 2;
	m_itemPadding.top = 1;
	m_itemPadding.right = 2;
	m_itemPadding.bottom = 1;

	addEvent(WND_MOUSE_UP,CLASSPROC(this,UnitWindowList,onMouseUp));
	addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowList,onMouseDown));
	addEvent(WND_MOUSE_DRAG,CLASSPROC(this,UnitWindowList,onMouseDrag));
	addEvent(WND_MOUSE_MOVE,CLASSPROC(this,UnitWindowList,onMouseMove));
	addEvent(WND_MOUSE_OUT,CLASSPROC(this,UnitWindowList,onMouseOut));
	addEvent(WND_MOUSE_LDBLCLICK,CLASSPROC(this,UnitWindowList,onMouseLDBClick));

	m_headerVisible = true;
}
LPCWSTR UnitWindowList::getHeaderText(INT item) const
{
	return m_header.items[item].name;
}

LPCWSTR UnitWindowList::getItemText(INT item,INT subItem) const
{
	if(item < (INT)m_items.size())
	{
		if(subItem < (INT)m_items[item]->items.size())
			return m_items[item]->items[subItem].name;
		if(subItem < getCols())
			return L"";
	}
	return NULL;
}
INT UnitWindowList::getItemHeight(INT index) const
{
	if(index < (INT)m_items.size())
		return m_items[index]->height;
	return 0;
}
void UnitWindowList::setColumnWidth(INT index,INT width)
{
	if(index < (INT)m_header.items.size())
		m_header.items[index].width = width;
	getScrollBar(BAR_HORI)->setScrollRange((FLOAT)getItemsWidth());
	//setScrollSize((INT)getScrollRange(BAR_HORI),(INT)getScrollRange(BAR_VERT));
}
INT UnitWindowList::getColumnWidth(INT index) const
{
	if(index < (INT)m_header.items.size())
		return m_header.items[index].width;
	return 0;
}
INT UnitWindowList::getColumnType(INT index) const
{
	if(index < (INT)m_header.items.size())
		return m_header.items[index].type;
	return 0;
}
INT UnitWindowList::getItemTextWidth(INT item,INT subItem)
{
	LPCWSTR text = getItemText(item,subItem);
	if(!text)
		return 0;
	SIZE size; 
	m_itemFont.getFontSize(&size,text);
	return size.cx;
}

INT UnitWindowList::getColumnTextWidth(INT index)
{
	INT rows = getRows();
	INT maxWidth = 0;
	INT i;
	for(i=0;i<rows;i++)
	{
		INT width = getItemTextWidth(i,index);
		if(width > maxWidth)
			maxWidth = width;
	}
	return maxWidth+8;
}

INT UnitWindowList::getItemsWidth(INT index)
{
	INT width = 0;
	INT cols = getCols();
	INT i;
	for(i=index;i<cols;i++)
	{
		width += getColumnWidth(i);
	}
	return width;
}
INT UnitWindowList::getItemsHeight()
{
	INT height = 0;
	INT rows = getRows();
	INT i;
	for(i=0;i<rows;i++)
	{
		height += getItemHeight(i);
	}
	return height;
}
INT UnitWindowList::addHeader(LPCSTR name,INT size,INT type)
{
	return addHeader(UCS2(name),size,type);
}

INT UnitWindowList::addHeader(LPCWSTR name,INT size,INT type)
{
	HITEM item;
	item.type = type;
	item.name = name;
	if(size < 0)
	{
		SIZE size;
		m_headerFont.getFontSize(&size,name);
		item.width = size.cx+6;
	}
	else
	{
		item.width = size;
	}
	m_header.items.push_back(item);

	if(m_scrollColumnIndex >= 0)
		setScrollRange(BAR_HORI,(FLOAT)getItemsWidth(m_scrollColumnIndex));
	else
		setScrollRange(BAR_HORI,(FLOAT)getItemsWidth(0));

	resetRenderFlag();
	return (INT)m_header.items.size() - 1;
}
INT UnitWindowList::addItem(LPCWSTR text,LPVOID value)
{
	if(getCols() == 0)
		return -1;
	ITEMS* items = new ITEMS;
	items->height = 16+3;
	items->select = false;
	items->value = value;
	items->items.resize(getCols());
	items->items[0].name = text;
	m_items.push_back(items);

	INT headerHeight = 0;
	if(m_headerVisible)
		headerHeight = m_header.height;

	setScrollRange(BAR_VERT,(FLOAT)getItemsHeight()+headerHeight);
	resetRenderFlag();
	return (INT)m_items.size()-1;
}
bool UnitWindowList::delItem(INT index)
{
	if(index <0 || index >= (INT)m_items.size())
		return false;
	std::vector<AFL::SP<ITEMS> >::iterator it = m_items.begin();
	it += index;
	m_items.erase(it);
	resetRenderFlag();
	return true;
}

LPVOID UnitWindowList::getItemValue(INT index) const
{
	if(index <0 || index >= (INT)m_items.size())
		return NULL;
	return m_items[index]->value;

}
bool UnitWindowList::setItemValue(INT index,LPVOID value)
{
	if(index <0 || index >= (INT)m_items.size())
		return false;
	m_items[index]->value = value;
	return true;
}
INT UnitWindowList::findItemIndex(LPVOID value) const
{
	INT i;
	for(i=0;i<(INT)m_items.size();i++)
	{
		if(m_items[i]->value == value)
			return i;
	}
	return -1;
}

bool UnitWindowList::setItemText(INT item,INT subItem,LPCWSTR text)
{
	if(item >= (INT)m_items.size())
		return false;
	if(subItem >= (INT)m_items[item]->items.size())
	{
		m_items[item]->items.resize(subItem+1);
	}
	if(m_items[item]->items[subItem].name != text)
	{
		m_items[item]->items[subItem].name = text;
		if(m_sortIndex == subItem)
			m_sort = true;
		resetRenderFlag();
	}
	return true;
}
INT UnitWindowList::getRows() const
{
	return (INT)m_items.size();
}
INT UnitWindowList::getCols() const
{
	return (INT)m_header.items.size();
}
void UnitWindowList::drawBox(INT x,INT y,INT width,INT height,bool select)
{
	Gdiplus::Graphics* g = getGraphics();

	getGdip()->drawLine(x, y, x+width-1, y,m_headerColor1);
	getGdip()->drawLine(x, y, x+0, y+height-1,m_headerColor1);
	getGdip()->drawLine(x+width-1, y, x+width-1, y+height-1,m_headerColor2);
	getGdip()->drawLine(x, y+height-1, x+width-1, y+height-1,m_headerColor2);
	if(select)
		getGdip()->drawBox(x+1, y+1, width-2, height-2,m_headerClientSelect);
	else
		getGdip()->drawBox(x+1, y+1, width-2, height-2,m_headerClient);
}
void UnitWindowList::clearItem()
{
	m_items.clear();
	resetRenderFlag();
}
void UnitWindowList::setItemPadding(RECT* rect)
{
	m_itemPadding = *rect;
}
void UnitWindowList::getItemPadding(RECT* rect) const
{
	*rect = m_itemPadding;
}
bool UnitWindowList::getItemRect(INT item,INT subItem,RECT* rect) const
{
	INT i;
	INT cols = getCols();
	INT rows = getRows();
	if(item >= rows || subItem >= cols)
		return false;

	INT width = getImageWidth();
	INT height = getImageHeight();

	//スクロール位置
	INT posY = (INT)getScrollValue(BAR_VERT);

	INT headerHeight = 0;
	if(m_headerVisible)
		headerHeight = m_header.height;

	//ヘッダサイズ分の修正
	INT y = headerHeight-posY;

	for(i = 0;i < item && y < height;i++)
	{
		//アイテムサイズの取得
		y += getItemHeight(i);
	}
	if(i != item)
		return false;
	//Y確定
	rect->top = y;
	rect->bottom = y+getItemHeight(i);

	//横スクロール
	INT staticWidth = 0;
	INT posX = (INT)getScrollValue(BAR_HORI);
	INT x = 0;
	for(i=0;i<subItem;i++)
	{
		//スクロール処理
		if(i == m_scrollColumnIndex+1)
			x -= posX;
		if(i<=m_scrollColumnIndex)
			staticWidth += getColumnWidth(i);
		x += getColumnWidth(i);
	}
	if(i != subItem)
		return false;
	rect->right = x + getColumnWidth(i);
	if(i < m_scrollColumnIndex+1)
		rect->left = x;
	else if(x < staticWidth)
		rect->left = staticWidth;
	else
		rect->left = x;

	if(rect->left >= rect->right)
		return false;

	//補正
	if(rect->left < 0)
		rect->left = 0;
	if(rect->top < headerHeight)
		rect->top = m_header.height;
	if(rect->right > width)
		rect->right = width;
	if(rect->bottom > height)
		rect->bottom = height;

	return true;
}
void UnitWindowList::setHeaderVisible(bool flag)
{
	m_headerVisible = flag;
	resetRenderFlag();
}
bool UnitWindowList::isHeaderVisible() const
{
	return m_headerVisible;
}
void UnitWindowList::setItemBackColor(DWORD color)
{
	m_itemBackColor = color;
}
void UnitWindowList::setItemTextColor(DWORD color)
{
	m_itemTextColor = color;
}
DWORD UnitWindowList::getItemTextColor(INT index,INT subItem) const
{
	ITEM* item = getItem(index,subItem);
	if(!item)
		return 0;
	if(item->flag & 1)
		return item->color;
	return m_itemTextColor;
}
bool UnitWindowList::setItemTextColor(INT index,INT subItem,DWORD color)
{
	ITEM* item = getItem(index,subItem);
	if(!item)
		return false;
	item->color = color;
	item->flag |= 1;
	resetRenderFlag();
	return true;
}
UnitWindowList::ITEM* UnitWindowList::getItem(INT item,INT subItem) const
{
	if(item >= (INT)m_items.size())
		return false;
	ITEMS* items = m_items[item].get();
	if(subItem >= (INT)items->items.size())
		return false;
	return &items->items[subItem];
}

void UnitWindowList::setLastWidth()
{
	INT cols = getCols();
	if(cols)
	{
		INT width = (INT)getWindowWidth();
		INT i;
		for(i=0;i<cols;i++)
			width -= getColumnWidth(i);
		if(width > 0)
			setColumnWidth(cols-1,getColumnWidth(cols-1)+width);
	}
}
void UnitWindowList::onPaint()
{
	if(m_compScroll)
	{
		setAutoScroll();
		m_compScroll = false;
	}
	if(!isRenderFlag())
	{
		sortList();

		Gdiplus::Graphics* g = getGraphics();
		if(!g)
			return;

		INT headerHeight = 0;
		if(m_headerVisible)
			headerHeight = m_header.height;


		INT i,j;
		INT width = getImageWidth();
		INT height = getImageHeight();
		g->Clear(Gdiplus::Color(m_itemBackColor));

		HDC hDC = CreateCompatibleDC(NULL);
		HFONT oldFont = (HFONT)SelectObject(hDC,m_itemFont);
		
		Gdiplus::Font font(hDC);
		Color color(m_itemTextColor);
		SolidBrush sb(color);

		INT x,y;
		INT posX = (INT)getScrollValue(BAR_HORI);

		INT clipX = 0;
		INT limitX = 0;
		INT cols = getCols();
		INT rows = getRows();
		//アイテムの表示
		INT posY = (INT)getScrollValue(BAR_VERT);
		y = headerHeight-posY;
		StringFormat stringFormat(StringFormat::GenericTypographic());
		for(i = 0;i < rows && y < height;i++)
		{
			INT columnHeight = getItemHeight(i);
			ITEMS* items = m_items[i].get();

			//範囲外のアイテムを省く
			if(y+getItemHeight(i) < headerHeight)
			{
				y += getItemHeight(i);
				continue;
			}
			g->ResetClip();
			if(items->select)
			{
				if(m_hoverItem == i)
					getGdip()->drawBox(0,y+1,width,getItemHeight(i)-1,0xeeaadddd);
				else
					getGdip()->drawBox(0,y+1,width,getItemHeight(i)-1,0xeeaaaaff);
			}
			else if(m_hoverItem == i)
			{
				getGdip()->drawBox(0,y+1,width,getItemHeight(i)-1,0xeeffffaa);
			}
			std::vector<ITEM>::iterator it;
			x = 0;
			limitX = 0;
			for(j=0;j<cols;j++)
			{
				INT type = getColumnType(j);
				INT columnWidth = getColumnWidth(j);

				INT clipWidth = columnWidth;
				//スクロール処理
				if(j == m_scrollColumnIndex + 1)
				{
					limitX = x;
					x -= posX;
				}
				if(x < limitX)
				{
					clipX = limitX;
					clipWidth -= limitX - x;
				}
				else
					clipX = x;
				g->SetClip(Gdiplus::Rect(clipX,y,clipWidth-1,columnHeight));

				if(type == 1)
					stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
				else
					stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);

				sb.SetColor(Color(getItemTextColor(i,j)));
				g->DrawString(getItemText(i,j),(INT)wcslen(getItemText(i,j)),&font,
					Gdiplus::RectF((FLOAT)x+m_itemPadding.left,(FLOAT)y+m_itemPadding.top,
					(FLOAT)columnWidth-m_itemPadding.right,(FLOAT)columnHeight-m_itemPadding.bottom),
					&stringFormat,
					&sb);

				x += columnWidth;
			}
			y += columnHeight;
			g->ResetClip();
			getGdip()->drawLine(0,y,width,y,0xff888888);
		}

		//サブアイテム
		limitX = 0;
		x = 0;
		stringFormat.SetAlignment(StringAlignmentCenter);

		SelectObject(hDC,m_headerFont);
		Gdiplus::Font font2(hDC);
		for(i=0;i<cols;i++)
		{
			INT columnWidth = getColumnWidth(i);
			if(i == m_scrollColumnIndex+1)
			{
				limitX = x;
				x -= posX;
			}
			INT clipX = x;
			if(clipX < limitX)
				clipX = limitX;
			//ヘッダの表示
			if(m_headerVisible)
			{
				g->SetClip(Gdiplus::Rect(clipX,0,columnWidth,headerHeight));
				if(m_hoverItem == -1 && m_hoverSubItem == i)
					drawBox(x,0,columnWidth,headerHeight,true);
				else
					drawBox(x,0,columnWidth,headerHeight,false);
				g->DrawString(getHeaderText(i),(INT)wcslen(getHeaderText(i)),&font2,
					RectF((FLOAT)x,1,(FLOAT)columnWidth,(FLOAT)headerHeight),&stringFormat,&sb);
				g->ResetClip();
			}
			x += columnWidth;
			if(x >= limitX)
				g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(0xff888888)),x,headerHeight,x,height);

		}

		if(x < width)
		{
			drawBox(x,0,width-x,headerHeight,false);
		}



		SelectObject(hDC,oldFont);
		DeleteDC(hDC);
	}
}
void UnitWindowList::onMouseMove(UnitWndMessage* m)
{
	FLOAT x = m->getParamFloat(0) - getAbsX();
	FLOAT y = m->getParamFloat(1) - getAbsY();
	if(getColumnIndex((INT)x,(INT)y) >= 0)
		setCursor((HCURSOR)LoadImage(0,IDC_SIZEWE,IMAGE_CURSOR,0,0,LR_SHARED));
	else
		setCursor();

	INT headerHeight = 0;
	if(m_headerVisible)
		headerHeight = m_header.height;


	INT i;
	INT hoverItem = -1;
	bool sub = true;
	if(y >= headerHeight)
	{
		//該当アイテムの取得
		INT posY = headerHeight - (INT)getScrollValue(BAR_VERT);
		INT rows = getRows();
		for(i = 0;i < rows && y > posY;i++)
			posY += getItemHeight(i);
		if(y <= posY)
			hoverItem = i-1;
		else
			sub = false;
	}

	//該当サブアイテムの取得
	INT posX = 0;
	INT hoverSubItem = -1;
	if(m_scrollColumnIndex == -1)
		posX = -(INT)getScrollValue(BAR_HORI);

	if(sub)
	{
		INT cols = getCols();
		for(i=0;i<cols;i++)
		{
			posX += getColumnWidth(i);
			if(x < posX)
			{
				hoverSubItem = i;
				break;
			}
			if(i == m_scrollColumnIndex)
				posX -= (INT)getScrollValue(BAR_HORI);
		}
	}
	if(m_hoverItem != hoverItem || m_hoverSubItem != hoverSubItem)
	{
		m_hoverItem = hoverItem;
		m_hoverSubItem = hoverSubItem;
		resetRenderFlag();

		UnitWndMessage msg;
		msg.setMessage(WND_LIST_ITEM_HOVER);
		msg.setParamInt(0,hoverItem);
		msg.setParamInt(1,hoverSubItem);
		callEvent(&msg);
	}


}
INT UnitWindowList::getColumnIndex(INT x,INT y) const
{
	INT headerHeight = 0;
	if(m_headerVisible)
		headerHeight = m_header.height;

	if(y >= 0 && y < headerHeight)
	{
		INT i;
		INT w = 0;
		for(i=0;i<getCols();i++)
		{
			if(i == m_scrollColumnIndex+1)
				w -= (INT)getScrollValue(BAR_HORI);
			w += getColumnWidth(i);
			if(x >= w-4 && x <= w+2)
				return i;
		}
	}
	return -1;
}
void UnitWindowList::setSortEnable(bool enable)
{
	m_sortEnable = enable;
}

void UnitWindowList::onMouseUp(UnitWndMessage* m)
{
	m_moveColumnIndex = -1;
}
void UnitWindowList::onMouseDown(UnitWndMessage* m)
{
	INT button = m->getParamInt(0);
	INT x = m->getX() - (INT)getAbsX();
	INT y = m->getY() - (INT)getAbsY();
	m_moveColumnIndex = getColumnIndex(x,y);
	if(m_moveColumnIndex >= 0)
		m_moveColumnX = m->getX() - getColumnWidth(m_moveColumnIndex);

	if(m_sortEnable && m_hoverItem == -1 && m_hoverSubItem > -1)
	{
		sort(m_hoverSubItem,!m_sortOrder);
	}
	INT headerHeight = 0;
	if(m_headerVisible)
		headerHeight = m_header.height;
	//アイテムの選択処理
	if(y >= headerHeight)
	{
		UnitWndMessage m;
		m.setMessage(WND_LIST_ITEM_CLICK);
		if(m_hoverItem >= 0)
		{
			m.setParamInt(0,m_hoverItem);
			m.setParamInt(1,m_hoverSubItem);
			callEvent(&m);

			if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				setSelectItem(m_hoverItem,!isSelectItem(m_hoverItem));
			}
			else if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				if(m_hoverItem > -1)
				{
					INT index = getSelectItem();
					if(index == -1)
						setSelectItem(m_hoverItem,!isSelectItem(m_hoverItem));
					else
					{
						INT s,e;
						if(index < m_hoverItem)
						{
							s = index;
							e = m_hoverItem;
						}
						else
						{
							s = m_hoverItem;
							e = index;
						}
						INT i;
						for(i=s;i<=e;i++)
							setSelectItem(i,true);
					}
				}

			}
			else
			{
				if(button == 0 || !isSelectItem(m_hoverItem))
				{
					setSelectItem(-1,false);
					setSelectItem(m_hoverItem,true);
				}
			}
		}
	}
}
void UnitWindowList::onMouseDrag(UnitWndMessage* m)
{
	if(m_moveColumnIndex >= 0)
	{
		INT width = m->getX() - m_moveColumnX;
		if(width > 0)
		{
			setColumnWidth(m_moveColumnIndex,width);
			resetRenderFlag();
		}
	}
}
void UnitWindowList::sortList()
{
	if(m_sort)
	{
		INT i,j;
		INT index = m_sortIndex;
		bool order = m_sortOrder;
		const INT count = (INT)m_items.size();
		bool text = m_header.items[index].type == 0;
		bool flag = true;
		for(j=0;j<count-1 && flag;j++)
		{
			flag = false;
			for(i=0;i<count-1-j;i++)
			{
				bool sort;
				if(text)
					sort = m_items[i+1]->items[index].name > m_items[i]->items[index].name;
				else
					sort = _wtof(m_items[i+1]->items[index].name.c_str()) > _wtof(m_items[i]->items[index].name.c_str());
				if(sort^order)
				{
					AFL::SP<ITEMS> item = m_items[i];
					m_items[i] =  m_items[i+1];
					m_items[i+1] = item;
					flag = true;
				}
			}
		}
		m_sort = false;
	}
}

void UnitWindowList::sort(INT index,bool order)
{
	m_sortIndex = index;
	m_sortOrder = order;
	m_sort = true;
	resetRenderFlag();
}

bool UnitWindowList::isSelectItem(INT index) const
{
	if(index >= 0 && index < (INT)m_items.size())
		return m_items[index]->select;
	return false;
}
INT UnitWindowList::getSelectItem() const
{
	INT i = 0;
	std::vector<AFL::SP<ITEMS> >::const_iterator it;
	for(it=m_items.begin();it!=m_items.end();++it)
	{
		if((*it)->select)
			return i;
		i++;
	}
	return -1;
}
void UnitWindowList::setSelectItem(INT index,bool flag)
{
	if(index < 0)
	{
		std::vector<AFL::SP<ITEMS> >::iterator it;
		for(it=m_items.begin();it!=m_items.end();++it)
		{
			(*it)->select = flag;
		}
	}
	else
	{
		if(index < (INT)m_items.size())
		{
			m_items[index]->select = flag;
		}
	}
	resetRenderFlag();
}
void UnitWindowList::onMouseOut(UnitWndMessage* m)
{
	if(m_hoverItem != -1)
	{
		m_hoverItem = -1;
		resetRenderFlag();
	}
	if(m_hoverSubItem != -1)
	{
		m_hoverSubItem = -1;
	}
}

void UnitWindowList::onMouseLDBClick(UnitWndMessage* m1)
{
	UnitWndMessage m;
	m.setMessage(WND_LIST_ITEM_DBCLICK);
	if(m_hoverItem >= 0)
	{
		m.setParamInt(0,m_hoverItem);
		m.setParamInt(1,m_hoverSubItem);
		callEvent(&m);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitPulldown
// DirectX - プルダウンメニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

UnitPulldown::UnitPulldown()
{
	addEvent(WND_SIZE,CLASSPROC(this,UnitPulldown,onSize));

	setSize(100,18);
	m_unitButton.setChildStyle(CHILD_RIGHT);
	addChild(&m_unitButton);
	m_unitText.setChildStyle(CHILD_CLIENT);
	m_unitText.setBackColor(0xffffffff);
	addChild(&m_unitText);
	addChild(&m_unitList);
	m_unitList.setScrollType(SCROLL_AUTO);
	m_unitList.setVisible(false);
	m_unitList.setHeaderVisible(false);
	m_unitList.addHeader("",100);
	m_unitList.setChainClip(false);
	m_unitList.setBorderSize(1);
	m_selectValue = -1;
	m_unitButton.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitPulldown,onUnitMessage));
	m_unitList.addEvent(WND_ACTIVE,CLASSPROC(this,UnitPulldown,onUnitMessage));
	m_unitList.addEvent(WND_LIST_ITEM_CLICK,CLASSPROC(this,UnitPulldown,onUnitMessage));
}
void UnitPulldown::clearItem()
{
	m_unitList.clearItem();
}
void UnitPulldown::addItem(LPCWSTR text,INT value)
{
	INT index = m_unitList.addItem(text);
	m_unitList.setItemValue(index,(LPVOID)value);
	if(m_selectValue == -1)
	{
		m_selectValue = value;
		m_unitText.setText(text);
	}
}
INT UnitPulldown::getSelectValue() const
{
	return m_selectValue;
}
void UnitPulldown::setSelectValue(INT value)
{
	LPCWSTR text = getItemText(value);
	if(text)
	{
		m_selectValue = value;
		m_unitText.setText(text);
	}
	else
		m_selectValue = -1;
	INT index =	m_unitList.findItemIndex((LPVOID)value);
	m_unitList.setSelectItem(-1,false);
	if(index >= 0)
		m_unitList.setSelectItem(index);
}
LPCWSTR UnitPulldown::getItemText(INT value) const
{
	INT index = m_unitList.findItemIndex((LPVOID)value);
	if(index >= 0)
		return m_unitList.getItemText(index,0);
	return NULL;
}
LPCWSTR UnitPulldown::getSelectText() const
{
	INT index = m_unitList.findItemIndex((LPVOID)m_selectValue);
	if(index >= 0)
		return m_unitList.getItemText(index,0);
	return NULL;
}
void UnitPulldown::onSize(UnitWndMessage* m)
{
	Rect2DF rect;
	getClientRect(&rect);
	FLOAT width = rect.right - rect.left;
	FLOAT height = rect.bottom - rect.top;
	m_unitButton.setSize(height,height);

	DIRECT3D::VectorObject vo;
	FLOAT weight = (FLOAT)height;

	vo.drawTriangle(weight/2,weight*2/3, weight/5,weight/3, weight*4/5,weight/3,0xff000000);
	m_unitButton.setVector(&vo);

	recalcLayout();
}

void UnitPulldown::onUnitMessage(UnitWndMessage* m)
{
	if(m->getWnd() == &m_unitList)
	{
		if(m->getMessage() == WND_ACTIVE)
		{
			if(m->getParamInt(0) == 0)
				m_unitList.setVisible(false);
		}
		else if(m->getMessage() == WND_LIST_ITEM_CLICK)
		{
			setSelectValue((INT)m_unitList.getItemValue(m->getParamInt(0)));
			m_unitList.setVisible(false);
		}
	}
	else
	{
		m_unitList.setPos(0,getWindowHeight());
		INT height = m_unitList.getItemsHeight()+2;
		INT width = (INT)getWindowWidth()-2;//m_unitList.getColumnTextWidth(0);
		if(height > 200)
		{
			height = 200;
			width -= 16;
		}
		m_unitList.setColumnWidth(0,width);
		m_unitList.setSize(getWindowWidth(),(FLOAT)height);
		if(m_unitList.isVisible())
		{
			m_unitList.setVisible(false);
		}
		else
		{
			m_unitList.setVisible(true);
			//m_unitList.setActive(true);
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitNumeric
// DirectX - 数値入力用コントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitNumeric::UnitNumeric()
{
	setBackColor(0xddffffff);
	m_unitText.setChildStyle(CHILD_CLIENT);
	m_unitPanel.setChildStyle(CHILD_RIGHT);
	m_unitButtonUP.setChildStyle(CHILD_EQUARL_VERT);
	m_unitButtonDOWN.setChildStyle(CHILD_EQUARL_VERT);

	m_unitPanel.setSize(10,10);
	addChild(&m_unitText);
	addChild(&m_unitPanel);
	m_unitPanel.addChild(&m_unitButtonDOWN);
	m_unitPanel.addChild(&m_unitButtonUP);
	m_unitButtonDOWN.setChildPriority(0);
	m_unitButtonUP.setChildPriority(1);
	m_unitButtonDOWN.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitNumeric,onMessage));
	m_unitButtonDOWN.addEvent(WND_MOUSE_LDBLCLICK,CLASSPROC(this,UnitNumeric,onMessage));
	m_unitButtonUP.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitNumeric,onMessage));
	m_unitButtonUP.addEvent(WND_MOUSE_LDBLCLICK,CLASSPROC(this,UnitNumeric,onMessage));
	m_unitText.addEvent(WND_CHAR,CLASSPROC(this,UnitNumeric,onMessage));
}
void UnitNumeric::setValue(INT value)
{
	WString work;
	work.printf(L"%d",value);
	m_unitText.setText(work);
}
INT UnitNumeric::getValue() const
{
	return _wtoi(m_unitText.getText());
}
LPCWSTR UnitNumeric::getText() const
{
	return m_unitText.getText();
}

void UnitNumeric::onMessage(UnitWndMessage* m)
{
	INT num = getValue();
	if(m->getWnd() == &m_unitButtonUP)
	{
		num++;
		setValue(num);
		WString work;
		work.printf(L"%d",num);
		m_unitText.setText(work);
	}
	else if(m->getWnd() == &m_unitButtonDOWN)
	{
		num--;
		setValue(num);
	}
	else if(m->getWnd() == &m_unitText)
	{
		m->setWnd(this);
		callEvent(m);
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMenu
// DirectX - メニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitMenu::UnitMenu()
{
	setItemBackColor(0xf0dddddd);
	setHeaderVisible(false);
	setBorderSize(1);
	addHeader();
	addEvent(WND_LIST_ITEM_CLICK,CLASSPROC(this,UnitMenu,onMessage));
	addEvent(WND_ACTIVE,CLASSPROC(this,UnitMenu,onMessage));

}
void UnitMenu::onMessage(UnitWndMessage* m)
{
	if(m->getMessage() == WND_ACTIVE && m->getParamInt(0)==0)
		setVisible(false);
	else if(m->getMessage() == WND_LIST_ITEM_CLICK)
		setVisible(false);
}
void UnitMenu::onPaint()
{
	INT height = getItemsHeight();
	INT width = getColumnTextWidth(0);
	setSize((FLOAT)width+8,(FLOAT)height+2);
	setColumnWidth(0,width+6);
	UnitWindowList::onPaint();
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitTabItem
// DirectX - タブ用アイテム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

UnitTabItem::UnitTabItem()
{
	m_value = NULL;
	m_select = false;
	m_unitText.setPos(4,2);
	add(&m_unitText);
}
void UnitTabItem::setText(LPCWSTR text,WINDOWS::Font* font)
{
	SIZE size;
	font->getFontSize(&size,text);
	setSize((FLOAT)size.cx+16,(FLOAT)size.cy);
	m_unitText.setSize(size.cx+8,size.cy);

	m_font = font;
	m_text = text;
	resetRenderFlag();
}
void UnitTabItem::setSelect(bool flag)
{
	if(m_select != flag)
	{
		m_select = flag;
		resetRenderFlag();
	}
}
bool UnitTabItem::isSelect() const
{
	return m_select;
}
bool UnitTabItem::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		FLOAT width = (FLOAT)getWindowWidth();
		FLOAT height = (FLOAT)getWindowHeight();
		if(m_text.length())
		{

			m_unitText.setVisible(true);
			m_unitText.drawString(m_text,*m_font,0xff000000);
		}
		else
			m_unitText.setVisible(false);

		DWORD colorBorder1 = 0xffffffff;
		DWORD colorBorder2 = 0xff444444;

		AFL::DIRECT3D::VectorObject vo;
		if(m_select)
		{
			vo.drawLine(0,0,width-1,0,colorBorder1);
			vo.drawLine(0,0,0,height-1,colorBorder1);
			vo.drawLine(width-1,0,width-1,height-1,colorBorder2);
			vo.drawBox(1,1,width-2,height-2,getBackColor());
			m_unitText.setPos(4,2);
		}
		else
		{
			vo.drawLine(0,1,width-1,0,colorBorder1);
			vo.drawLine(0,1,0,height-1,colorBorder1);
			vo.drawLine(width-1,1,width-1,height-1,colorBorder2);
			vo.drawLine(0,height-1,width-1,height-1,colorBorder2);
			vo.drawBox(1,2,width-2,height-3,getBackColor());
			m_unitText.setPos(4,3);
		}
		create(&vo);
	}
	return UnitVector::onRender(world,x,y,z);
}
void UnitTabItem::setValue(LPVOID value)
{
	m_value = value;
}
LPVOID UnitTabItem::getValue() const
{
	return m_value;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowTab
// DirectX - タブコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

UnitWindowTab::UnitWindowTab()
{
	addEvent(WND_SIZE,CLASSPROC(this,UnitWindowTab,onSize));

	m_selectItem = NULL;
	setSize(20,20);
	m_backColor = 0x77aaaaaa;
	m_selectColor = 0x77dddddd;
	
	m_panel.setSize(20,10);
	m_panel.setChildStyle(CHILD_CLIENT);
	m_panel.setBackColor(0xddcccccc);
	addChild(&m_panel);
	m_panel.addChild(&m_panel2);
	m_left.setSize(16,16);
	m_right.setSize(16,16);
	m_left.setChildStyle(CHILD_LEFT);
	m_right.setChildStyle(CHILD_RIGHT);
	m_left.setVisible(false);
	m_right.setVisible(false);
	addChild(&m_left);
	addChild(&m_right);

	m_left.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowTab,onMessage));
	m_right.addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowTab,onMessage));
	setBackColor(0xccffffff);

	VectorObject vo1,vo2;
	FLOAT m_barWeight = 16;
	vo1.drawTriangle(m_barWeight/3,m_barWeight/2, m_barWeight*2/3,m_barWeight/5, m_barWeight*2/3,m_barWeight*4/5,0xff000000);
	vo2.drawTriangle(m_barWeight*2/3,m_barWeight/2, m_barWeight/3,m_barWeight/5, m_barWeight/3,m_barWeight*4/5,0xff000000);
	m_left.setVector(&vo1);
	m_right.setVector(&vo2);

	m_panel.addEvent(WND_SCROLL,CLASSPROC(this,UnitWindowTab,onScroll));
}
UnitWindowTab::~UnitWindowTab()
{
	clearItem();
}

void UnitWindowTab::onScroll(UnitWndMessage* m)
{
	m_panel2.setPosX(-m_panel.getScrollValue(BAR_HORI));
}

void UnitWindowTab::clearItem()
{
	std::list<UnitTabItem*>::iterator it;
	for(it=m_item.begin();it!=m_item.end();++it)
	{
		delete *it;
	}
	m_item.clear();
}
INT UnitWindowTab::getItemsWidth() const
{
	INT width = 0;
	std::list<UnitTabItem*>::const_iterator it;
	for(it=m_item.begin();it!=m_item.end();++it)
	{
		width += (INT)(*it)->getWindowWidth();
	}
	return width;
}
UnitTabItem* UnitWindowTab::addItem(LPCWSTR name,LPVOID value)
{
	UnitTabItem* item = new UnitTabItem;
	item->setChildPriority((INT)m_item.size());
	item->setValue(value);
	item->setChildStyle(CHILD_LEFT);
	item->addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowTab,onMessage));
	item->setText(name,&m_font);
	m_item.push_back(item);
	m_panel2.addChild(item);

	INT w = getItemsWidth();
	m_panel.setScrollSize(w,0);
	m_panel2.setSize((FLOAT)w,getWindowHeight());

	if(m_item.size() == 1)
	{
		setSelect(item);
	}
	else
	{
		item->setSelect(false);
		item->setBackColor(m_backColor);
	}
	setScroll();
	return item;
}
void UnitWindowTab::delItem(UnitTabItem* item)
{
	m_item.remove(item);
	delete item;
}
void UnitWindowTab::setSelect(UnitTabItem* item)
{
	if(m_selectItem != item)
	{
		std::list<UnitTabItem*>::iterator it;
		for(it=m_item.begin();it!=m_item.end();++it)
		{
			(*it)->setSelect(false);
			(*it)->setBackColor(m_backColor);
		}		
		if(item)
		{
			item->setSelect(true);
			item->setBackColor(m_selectColor);
		}
		m_selectItem = item;

		UnitWndMessage m;
		m.setMessage(WND_TAB_CHANGE);
		callEvent(&m);
	}
}
void UnitWindowTab::setSelectValue(LPVOID value)
{
	std::list<UnitTabItem*>::iterator it;
	for(it=m_item.begin();it!=m_item.end();++it)
	{
		if((*it)->getValue() == value)
		{
			setSelect(*it);
			break;		
		}
	}
}
UnitTabItem* UnitWindowTab::getSelectItem() const
{
	return m_selectItem;
}
void UnitWindowTab::onMessage(UnitWndMessage* m)
{
	m->setDefaultProc(false);
	if(m->getWnd() == &m_left)
	{
		m_panel.setScrollValue(BAR_HORI,m_panel.getScrollValue(BAR_HORI)-20.0f);
	}
	else if(m->getWnd() == &m_right)
	{
		m_panel.setScrollValue(BAR_HORI,m_panel.getScrollValue(BAR_HORI)+20.0f);
	}
	else
		setSelect((UnitTabItem*)m->getWnd());
}

void UnitWindowTab::onSize(UnitWndMessage* m)
{
	m_panel2.setSize(m_panel2.getWindowWidth(),getWindowHeight());
	setScroll();
}
void UnitWindowTab::setScroll()
{
	Rect2DF rect;
	getClientRect(&rect);
	bool flag = m_left.isVisible();
	bool flag2 = false;
	FLOAT width = rect.right - rect.left;
	if(m_panel2.getWindowWidth() > width)
	{
		flag2 = true;
	}
	if(flag != flag2)
	{
		m_left.setVisible(flag2);
		m_right.setVisible(flag2);
		m_panel.setScrollValue(BAR_HORI,0);
		m_panel2.setPosX(0);
		recalcLayout(true);
	}

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TreeItem
// DirectX - ツリーアイテム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TreeItem::TreeItem(UnitWindowTree* tree)
{
	m_parent = NULL;
	m_opened = false;
	m_value = NULL;
	m_tree = tree;

}
void TreeItem::setText(LPCWSTR text)
{
	m_text = text;
}
LPCWSTR TreeItem::getText() const
{
	return m_text;
}
bool TreeItem::isOpen() const
{
	return m_opened;
}
void TreeItem::setOpen(bool flag)
{
	if(m_opened != flag)
	{
		m_opened = flag;
		UnitWndMessage msg;
		msg.setMessage(WND_TREE_OPEN);
		msg.setParamAdr(0,this);
		msg.setParamInt(0,flag);
		m_tree->callEvent(&msg);
		m_tree->resetRenderFlag();
	}
}
LPVOID TreeItem::getValue() const
{
	return m_value;
}
void TreeItem::setValue(LPVOID value)
{
	m_value = value;
}
TreeItem* TreeItem::addItem(LPCWSTR text)
{
	TreeItem* item = new TreeItem(m_tree);
	if(text)
		item->setText(text);
	item->m_parent = this;
	m_childs.push_back(item);
	return item;
}
void TreeItem::delItem(TreeItem* item)
{
	std::list<SP<TreeItem> >::iterator it;
	for(it=m_childs.begin();it!=m_childs.end();++it)
	{
		if(it->get() == item)
		{
			m_childs.erase(it);
			break;
		}
	}
}
INT TreeItem::getChildCount() const
{
	return (INT)m_childs.size();
}
TreeItem* TreeItem::getOpenItem(INT& count)
{
	if(count == 0)
		return this;
	--count;
	if(isOpen())
	{
		std::list<SP<TreeItem> >::iterator it;
		for(it=m_childs.begin();it!=m_childs.end();++it)
		{
			TreeItem* item = (*it)->getOpenItem(count);
			if(item)
				return item;
		}
	}
	return NULL;
}
INT TreeItem::getLevel() const
{
	INT i;
	const TreeItem* item = this;
	for(i=0;item = item->getParentItem();i++);
	return i;
}
TreeItem* TreeItem::getParentItem() const
{
	return m_parent;
}
void TreeItem::clearItem()
{
	m_childs.clear();
}
TreeItem* TreeItem::findItemValue(LPVOID value)
{
	std::list<SP<TreeItem> >::iterator it;
	for(it=m_childs.begin();it!=m_childs.end();++it)
	{
		if((*it)->getValue() == value)
			return it->get();
	}
	return NULL;
}
TreeItem* TreeItem::findItemText(LPCWSTR value)
{
	WString findText = value;
	findText.toUpper();
	std::list<SP<TreeItem> >::iterator it;
	for(it=m_childs.begin();it!=m_childs.end();++it)
	{
		WString data = (*it)->getText();
		data.toUpper();
		if(data == findText)
			return it->get();
	}
	return NULL;
}
bool TreeItem::isItemParent(TreeItem* item)
{
	TreeItem* parent = this;
	while(parent = parent->getParentItem())
	{
		if(parent == item)
			return true;
	}
	return false;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowTree
// DirectX - ツリービュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWindowTree::UnitWindowTree()
{
	m_rootItem = new TreeItem(this);
	setScrollType(SCROLL_AUTO);
	m_selectItem = NULL;
	m_hoverItem = NULL;
	addEvent(WND_MOUSE_DOWN,CLASSPROC(this,UnitWindowTree,onMouseDown));
	addEvent(WND_MOUSE_MOVE,CLASSPROC(this,UnitWindowTree,onMouseMove));
	addEvent(WND_MOUSE_OUT,CLASSPROC(this,UnitWindowTree,onMouseOut));
	m_rootItem->setText(L"root");
	m_rootItem->addItem(L"ふぉっふぉっふぉ");
	m_rootItem->addItem(L"ぐはぁ");
}
UnitWindowTree::~UnitWindowTree()
{
	delete m_rootItem;
}
TreeItem* UnitWindowTree::getItem()
{
	return m_rootItem;
}
void UnitWindowTree::clearItem()
{
	m_rootItem->clearItem();
}
void UnitWindowTree::getItemSize(SIZE* size)
{
	size->cx = 0;
	size->cy = 0;
	size->cy = getItemSize(m_rootItem,0,0,size->cx);
}
void UnitWindowTree::setSelectItem(TreeItem* item)
{
	if(item != m_selectItem)
	{
		m_selectItem = item;
		TreeItem* parent = item;
		while(parent = parent->getParentItem())
		{
			parent->setOpen(true);
		}
		POINT point;
		getItemPos(item,&point);
		setScrollValue(BAR_VERT,(FLOAT)point.y);
		resetRenderFlag();
	}
}
bool UnitWindowTree::getItemPos(TreeItem* parentItem,TreeItem* item,POINT* point)
{
	if(parentItem == item)
		return true;
	point->y += 16;
	if(parentItem->isOpen())
	{
		point->x += 16;
		std::list<SP<TreeItem> >::iterator it;
		for(it=parentItem->m_childs.begin();it!=parentItem->m_childs.end();++it)
		{
			if(getItemPos(it->get(),item,point))
				return true;
		}
		point->x -= 16;
	}
	return false;
}
bool UnitWindowTree::getItemPos(TreeItem* item,POINT* point)
{
	point->x = 0;
	point->y = 0;
	return getItemPos(m_rootItem,item,point);
}
bool UnitWindowTree::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		SIZE size;
		getItemSize(&size);
		setScrollSize(size.cx,size.cy);
		setAutoScroll();

		INT width = getImageWidth();
		INT height = getImageHeight();

		getGdip()->clear(0xccffffff);
		TreeItem* item = m_rootItem;
		INT x = (INT)-getScrollValue(BAR_HORI);
		INT y = (INT)-getScrollValue(BAR_VERT);
		drawItem(item,x,y);
	}
	return true;
}
INT UnitWindowTree::getItemSize(TreeItem* item,INT x,INT y,LONG& width)
{
	SIZE fontSize;
	WINDOWS::Font font;
	font.getFontSize(&fontSize,item->getText());
	if(width < x + fontSize.cx+16)
		width = x + fontSize.cx+16;
	if(item->isOpen())
	{
		std::list<SP<TreeItem> >::iterator it;
		for(it=item->m_childs.begin();it!=item->m_childs.end();++it)
		{
			y = getItemSize(it->get(),x+16,y,width);
		}
	}
	y += 16;
	return y;
}

INT UnitWindowTree::drawItem(TreeItem* item,INT x,INT y)
{
	WINDOWS::Font font;
	if(item->getChildCount()==0)
	{
		getGdip()->drawBox(x+7,y+7,2,2,0xff000000);
	}
	else
	{
		getGdip()->drawBoxLine(x+4,y+4,8,8,0xff000000);
		getGdip()->drawLine(x+6,y+8,x+2+8,y+8,0xff000000);
		if(!item->isOpen())
		{
			getGdip()->drawLine(x+8,y+6,x+8,y+2+8,0xff000000);
		}
	}

	if(item == m_hoverItem)
		font.setUnderline(true);
	if(item == m_selectItem)
	{
		SIZE size;
		font.getFontSize(&size,item->getText());
		getGdip()->drawBox(x+16,y,size.cx,16,0xff2222FF);
		getGdip()->drawString(x+16,y,item->getText(),font,0xffffffff);

	}
	else
		getGdip()->drawString(x+16,y,item->getText(),font,0xff000000);
	y += 16;

	if(item->isOpen())
	{
		std::list<SP<TreeItem> >::iterator it;
		for(it=item->m_childs.begin();it!=item->m_childs.end();++it)
		{
			y = drawItem(it->get(),x+16,y);
		}
	}
	return y;
}
void UnitWindowTree::onMouseDown(UnitWndMessage* m)
{
	if(m_hoverItem)
	{
		FLOAT x = m->getParamFloat(0) - getAbsX()+getScrollValue(BAR_HORI);
		FLOAT y = m->getParamFloat(1) - getAbsY()+getScrollValue(BAR_VERT);
		INT level = m_hoverItem->getLevel();
		if(x >= level*16 && x < (level+1)*16)
		{
			m_hoverItem->setOpen(!m_hoverItem->isOpen());
			resetRenderFlag();
		}
		else
		{
			if(m_selectItem != m_hoverItem)
			{
				UnitWndMessage msg;
				msg.setMessage(WND_TREE_SELECT);
				msg.setParamAdr(0,m_hoverItem);
				callEvent(&msg);

				m_selectItem = m_hoverItem;
				resetRenderFlag();
			}
		}
	}
}
void UnitWindowTree::onMouseOut(UnitWndMessage* m)
{
	if(m_hoverItem)
	{
		m_hoverItem = NULL;
		resetRenderFlag();
	}
}
void UnitWindowTree::onMouseMove(UnitWndMessage* m)
{
	FLOAT x = m->getParamFloat(0) - getAbsX()+getScrollValue(BAR_HORI);
	FLOAT y = m->getParamFloat(1) - getAbsY()+getScrollValue(BAR_VERT);
	TreeItem* item = getItem((INT)y);
	if(m_hoverItem != item)
	{
		m_hoverItem = item;
		resetRenderFlag();
	}

}

TreeItem* UnitWindowTree::getItem(INT y) 
{
	INT count = y/16;
	return m_rootItem->getOpenItem(count);
}


}}
