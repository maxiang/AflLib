#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DWINDOW

#include "aflWindow.h"
#include "aflDirect3DUnitCustom.h"


namespace AFL{namespace DIRECT3D{

using namespace AFL::WINDOWS;

enum HIT_TEST
{
	HIT_NONE,
	HIT_THROUGH,
	HIT_CLIENT,
	HIT_FRAME_TOP_LEFT,
	HIT_FRAME_TOP_RIGHT,
	HIT_FRAME_BOTTOM_LEFT,
	HIT_FRAME_BOTTOM_RIGHT,
	HIT_FRAME_LEFT,
	HIT_FRAME_RIGHT,
	HIT_FRAME_TOP,
	HIT_FRAME_BOTTOM,
	HIT_CAPTION,
	HIT_OHTER
};
enum WND_MSG
{
	WND_ALL,
	WND_CLOSE,
	WND_NORMAL,
	WND_MAX,
	WND_MIN,
	WND_MOVE,
	WND_SIZE,
	WND_ACTIVE,
	WND_CHAR,
	WND_KEYDOWN,
	WND_KEYUP,
	WND_MOUSE_DOWN,
	WND_MOUSE_UP,
	WND_MOUSE_DRAG,
	WND_MOUSE_WHEEL,
	WND_MOUSE_CLICK,
	WND_MOUSE_LDBLCLICK,
	WND_MOUSE_HOVER,
	WND_MOUSE_OUT,
	WND_MOUSE_MOVE,
	WND_SCROLL,

	WND_TREE_OPEN,
	WND_TREE_SELECT,

	WND_VALUE,

	WND_USER = 10000
};
enum BARTYPE
{
	BAR_NONE,
	BAR_VERT, //垂直線
	BAR_HORI  //水平線
};
enum SPLIT_TYPE
{
	SPLIT_NS,
	SPLIT_WE,
	SPLIT_SN,
	SPLIT_EW
};
class UnitWnd;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWndMessage
// DirectX - ウインドウメッセージクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class UnitWndMessage
{
public:
	UnitWndMessage();
	void setWnd(UnitWnd* wnd);
	UnitWnd* getWnd() const;
	void setMessage(DWORD msg);
	DWORD getMessage();
	void setParamInt(int index,int data);
	void setParamFloat(int index,float data);
	int getParamInt(int index) const;
	float getParamFloat(int index) const;
	void setParamAdr(int index,LPVOID data);
	LPVOID getParamAdr(int index) const;
	int getX() const;
	int getY() const;
	void setDefaultProc(bool flag);
	bool isDefaultProc() const;
protected:
	DWORD m_msg;
	UnitWnd* m_wnd;
	float m_paramFloat[10];
	int m_paramInt[10];
	LPVOID m_paramAdr[10];
	bool m_defaltProc;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowManager
// DirectX - ウインドウ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowManager : public Unit
{
public:
	UnitWindowManager();
	~UnitWindowManager();
	void init(Window* window);
	void add(UnitWnd* unit);
	void proc(HWND hWnd);
	UnitWnd* getWindow(FLOAT x,FLOAT y);
	void recalcLayout();
	void lock();
	void unlock();
	Window* getMasterWindow() const;
	void setActive(UnitWnd* unit);
	void callEvent(UnitWndMessage* m);
	void sort() const;
	FLOAT getWindowWidth() const;
	FLOAT getWindowHeight() const;
	UnitWnd* getHoverWindow() const;
	UnitWnd* getMoveWindow() const;
	bool isBusy() const;
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void onAction(World* world,LPVOID value);
	void onMessage(Message* m);
	void onMove(UnitWnd* unit,FLOAT x,FLOAT y);
	bool m_downLeft;
	bool m_downRight;
	Window* m_mainWindow;
	UnitWnd* m_moveWnd;
	POINT m_movePoint;
	Point2D m_downPoint;
	Rect2DF m_rectPoint;
	DWORD m_threadID;

	HCURSOR m_cursorNS;
	HCURSOR m_cursorWE;
	HCURSOR m_cursorNWSE;
	HCURSOR m_cursorNESW;
	HCURSOR m_cursor;
	HCURSOR m_cursorKeep;

	HIT_TEST m_downHit;

	UnitWnd* m_unitMouseDown;
	UnitWnd* m_unitMouseHover;

	UnitWnd* m_unitActive;

	UnitWnd* m_unitChild;
	Critical m_critical;
	std::list<UnitWndMessage> m_listMessage;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWnd
// DirectX - ウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum FRAME_STYLE
{
	FRAME_THICK,
	FRAME_EDGE,
	FRAME_SIMPLE
};
enum WINDOW_STAT
{
	WINDOW_NORMAL,
	WINDOW_MAX,
	WINDOW_MIN,
	WINDOW_THROUGH
};
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
enum SORT_STYLE
{
	SORT_NOMAL,
	SORT_TOP,
	SORT_BOTTOM,
	SORT_CONTROL
};
class UnitScroll;
class UnitWindowCaption;
class UnitWnd : public UnitVector
{
	friend class UnitWindowManager;
public:

	UnitWnd();
	virtual ~UnitWnd();
	void setWindowStat(WINDOW_STAT s);
	WINDOW_STAT getWindowStat() const;
	virtual FLOAT getScrollValue(BARTYPE type) const
	{
		return 0;
	}
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	virtual void getWindowRect(Rect2DF* rect);
	virtual void getClientRect(Rect2DF* rect);
	virtual FLOAT getWindowHeight() const;
	virtual FLOAT getWindowWidth() const;
	virtual bool isHit(FLOAT x,FLOAT y);
	virtual UnitWnd* getWindow(FLOAT x,FLOAT y,bool flag=true);
	virtual HIT_TEST hitTest(FLOAT x,FLOAT y);
	bool moveWindow(FLOAT x,FLOAT y,FLOAT width,FLOAT height,bool repaint=false);

	void add(Unit* unit);
	void addChild(UnitWnd* unit);
	void delChild(UnitWnd* unit);
	INT getChildPriority()const;
	CHILD_STYLE getChildStyle()const;
	void setChildStyle(CHILD_STYLE style);
	DWORD getBackColor()const;
	void setBackColor(DWORD color);
	void addEvent(UINT message,ClassProc& classProc,INT priority=0);
	void delEvent(UINT message,ClassProc& classProc);
	void callEvent(UnitWndMessage* m);
	HCURSOR getCursor()const;
	void setCursor(HCURSOR cursor=NULL);
	void setChildPriority(INT value);

	INT getLevel() const;

	void setForeground();
	bool isParent(UnitWnd* wnd) const;
	bool isParents(UnitWnd* wnd) const;
	void setVisible(bool flag=true);
	void setActive(bool flag=true);
	bool isActive() const;
	void setStatic(bool flag)
	{
		m_static = flag;
	}
	bool isStatic() const
	{
		return m_static;
	}
	UnitWnd* getParent() const;
	void setMinSize(FLOAT width,FLOAT height);
	virtual void recalcLayout(bool flag=false);
	void setPadding(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2);
	void setMargin(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2);
	void setManager(UnitWindowManager* manager);
	UnitWindowManager* getManager() const;
	Window* getMasterWindow() const;
	SORT_STYLE getSortStyle() const
	{
		return m_sortStyle;
	}
	void setSortStyle(SORT_STYLE sortStyle)
	{
		m_sortStyle = sortStyle;
	}
	void setPosX(FLOAT value)
	{
		setPos(value,getPosY());
	}
	void setPosY(FLOAT value)
	{
		setPos(getPosX(),value);
	}
	void setPos(FLOAT x,FLOAT y)
	{
		m_windowX = x;
		m_windowY = y;
		UnitVector::setPos((FLOAT)x,(FLOAT)y);

	}


protected:

	DWORD onMessage(UnitWndMessage* m);
	void sort(FLOAT& w);
	virtual void onAction(World* world,LPVOID value);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void onPaint(){}
	virtual void onMove(FLOAT x,FLOAT y);
	virtual void recalcLayout2(Rect2DF* rect,bool flag=false);

	std::map<UINT,std::multimap<INT,ClassProc> > m_eventProc;
	std::list<UnitWnd*> m_windowChilds;
	UnitWnd* m_parentWnd;
	UnitWindowManager* m_manager;

	SORT_STYLE m_sortStyle;
	DWORD m_backColor;
	FLOAT m_windowX;
	FLOAT m_windowY;
	FLOAT m_height;
	FLOAT m_width;
	FLOAT m_minWidth;
	FLOAT m_minHeight;
	INT m_childPriority;
	Rect3DF m_paddingRect;
	Rect2DF m_marginRect;
	CHILD_STYLE m_childStyle;
	HCURSOR m_cursor;
	bool m_active;
	bool m_static;

	WINDOW_STAT m_windowStat;
	FLOAT m_minPosX;
	FLOAT m_minPosY;
	FLOAT m_normalPosX;
	FLOAT m_normalPosY;
	FLOAT m_normalWidth;
	FLOAT m_normalHeight;

	FLOAT m_targetScaleX;
	FLOAT m_targetScaleY;
	FLOAT m_targetPosX;
	FLOAT m_targetPosY;
	FLOAT m_targetWidth;
	FLOAT m_targetHeight;
	FLOAT m_statScaleX;
	FLOAT m_statScaleY;
	FLOAT m_statPosX;
	FLOAT m_statPosY;
	FLOAT m_statWidth;
	FLOAT m_statHeight;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowThrough
// DirectX - 透過ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowThrough : public UnitWnd
{
public:
	virtual HIT_TEST hitTest(FLOAT x,FLOAT y);
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitButton
// DirectX - ボタンコントロールクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitButton : public UnitWnd
{
public:
	UnitButton();
	virtual ~UnitButton(){}
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	void setClick(bool flag);
	DWORD onMouseDown(UnitWndMessage* m);
	DWORD onMouseUp(UnitWndMessage* m);
	void setVector(VectorObject* vo);
	void setText(LPCWSTR text);
protected:
	WString m_text;
	UnitGdip m_unitText;
	WINDOWS::Font m_font;
	void onMouseHover(UnitWndMessage* m);
	void onMouseOut(UnitWndMessage* m);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	VectorObject m_vo;
	bool m_click;
	bool m_hover;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitPushButton
// DirectX - プッシュボタンコントロールクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitPushButton : public UnitWnd
{
public:
	UnitPushButton();
	virtual ~UnitPushButton(){}
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	void setClick(bool flag);
	DWORD onMouseDown(UnitWndMessage* m);
	void setVector(VectorObject* vo);
	void setText(LPCWSTR text);
protected:
	WString m_text;
	UnitGdip m_unitText;
	WINDOWS::Font m_font;
	void onMouseHover(UnitWndMessage* m);
	void onMouseOut(UnitWndMessage* m);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	VectorObject m_vo;
	bool m_click;
	bool m_hover;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowCaption
// DirectX - ウインドウタイトルクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowCaption : public UnitWindowThrough
{
public:
	UnitWindowCaption();
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	void setTitle(LPCSTR title);
	void setTitle(LPCWSTR title);
	void setMaxImage(bool flag);

	UnitButton* getCloseButton();
	UnitButton* getMaxButton();
	UnitButton* getMinButton();
protected:
	DWORD onButtonClick(UnitWndMessage* m);
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	UnitButton m_buttonClose;
	UnitButton m_buttonMax;
	UnitButton m_buttonMin;
	UnitSprite m_text;
	FLOAT m_size;
	WString m_title;
	WString m_title2;
	bool m_titleDraw;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitScroll
// DirectX - スクロールバー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class UnitScroll : public UnitWnd
{
public:
	UnitScroll();
	virtual ~UnitScroll();
	void setBarType(BARTYPE type);
	FLOAT getBarWeight() const;
	void setScrollRange(FLOAT range);
	FLOAT getScrollRange() const;
	void setScrollValue(FLOAT value,bool flag=true);
	FLOAT getScrollValue() const;
	void setButtonValue(FLOAT value);
	FLOAT getButtonValue() const;

protected:
	void onBarUp(UnitWndMessage* m);
	void onBarDown(UnitWndMessage* m);
	void onBarDrag(UnitWndMessage* m);
	virtual void onAction(World* world,LPVOID value);
	void setBarPosition(FLOAT value);
	FLOAT getBarPosition()const;
	void setButtonSize(UnitWnd* unit,FLOAT width,FLOAT height);
	void onClickUp(UnitWndMessage* m);
	void onClickDown(UnitWndMessage* m);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);

	BARTYPE m_barType;
	FLOAT m_targetValue;
	FLOAT m_barWeight;
	FLOAT m_barSize;
	FLOAT m_barAreaSize;
	FLOAT m_buttonValue;
	FLOAT m_barPosition;
	FLOAT m_scrollValue;
	FLOAT m_scrollRange;

	FLOAT m_barDragStartX;
	FLOAT m_barDragStartY;
	FLOAT m_barBasePosition;

	UnitWnd m_barFrame;
	UnitWnd m_bar;
	UnitButton m_button1;
	UnitButton m_button2;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSlider
// DirectX - スライダークラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitSlider : public UnitWnd
{
public:
	UnitSlider();
	virtual ~UnitSlider();
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	void setSlideCount(INT count)
	{
		m_slideCount = count;
	}
	INT getSliderPos() const;
	void setSliderPos(INT pos);
protected:
	void onBarDown(UnitWndMessage* m);
	void onBarUp(UnitWndMessage* m);
	void onBarDrag(UnitWndMessage* m);

	INT m_slideValue;
	FLOAT m_barBold;
	FLOAT m_lineBold;
	INT m_slideCount;
	UnitButton m_bar;
	FLOAT m_barBasePosition;
	FLOAT m_barDragStartY;
	FLOAT m_barDragStartX;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindow
// DirectX - ウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum SCROLL_TYPE
{
	SCROLL_MANUAL,
	SCROLL_AUTO
};
class UnitWindow : public UnitWnd
{
public:
	UnitWindow();
	virtual ~UnitWindow();
	void setScrollBar(BARTYPE type,bool flag=true);
	UnitScroll* getScrollBar(BARTYPE type);
	virtual FLOAT getScrollValue(BARTYPE type) const;
	void setScrollValue(BARTYPE type,FLOAT value,bool flag=true);
	void setScrollRange(BARTYPE type,FLOAT value);
	FLOAT getScrollRange(BARTYPE type) const;
	void setScrollSize(INT x,INT y);
	void setScrollType(SCROLL_TYPE type);
	SCROLL_TYPE getScrollType() const;

	virtual void getClientRect(Rect2DF* rect);
	FLOAT getBorderSize() const;
	void setBorderSize(FLOAT size);
	void setBorderStyle(FRAME_STYLE style);
	void setCaptionVisible(bool flag);
	void setTitle(LPCSTR title);
	void setTitle(LPCWSTR title);
	void getInnerRect(Rect2DF* rect);


	virtual HIT_TEST hitTest(FLOAT x,FLOAT y);
	virtual void addChild(UnitWnd* unit);
	virtual void delChild(UnitWnd* unit);
	void setClientScroll(bool flag=true)
	{
		m_clientScroll = flag;
	}
	UnitWindowCaption* getCaptionUnit()
	{
		return &m_caption;
	}
protected:
	void setAutoScroll();
	
	void onSize(UnitWndMessage* m);
	DWORD onScroll(UnitWndMessage* m);
	DWORD onButtonClick(UnitWndMessage* m);
	DWORD onMouseWheel(UnitWndMessage* m);
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);

	virtual void recalcLayout2(Rect2DF* rect,bool flag);
	bool m_clientScroll;
	bool m_compScroll;
	UnitScroll m_scrollVert;
	UnitScroll m_scrollHori;
	UnitWindowThrough m_scrollBox;
	UnitWindowThrough m_client;
	UnitWindowCaption m_caption;
	FRAME_STYLE m_frameStyle;
	FLOAT m_frameBorder;
	FLOAT m_frameBorderKeep;
	DWORD m_borderColor1;
	DWORD m_borderColor2;
	DWORD m_borderClient;
	SCROLL_TYPE m_scrollType;
};
class UnitWindowFrame : public UnitWindow
{
public:
	UnitWindowFrame();
};





}}

#define __INC_AFLDIRECT3DWINDOW
#endif