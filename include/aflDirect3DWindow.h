#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DWINDOW

#include "aflDirect3DWinBase.h"
#include "aflImage.h"


namespace AFL{namespace DIRECT3D{


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWndSplit
// DirectX - スプリットウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWndSplit : public UnitWnd
{
public:
	UnitWndSplit();
	virtual bool setSize(FLOAT width,FLOAT height,bool flag=false);
	virtual HIT_TEST hitTest(FLOAT x,FLOAT y);
	FLOAT getBarSize() const;
	void setBarType(BARTYPE type);
	BARTYPE getBarType() const;
protected:
	virtual void onMove(FLOAT x,FLOAT y);
	BARTYPE m_barType;
	FLOAT m_splitBorder;
	DWORD m_borderColor1;
	DWORD m_borderColor2;
	DWORD m_borderClient;
};
class UnitWindowSplit : public UnitWnd
{
public:
	UnitWindowSplit();
	void addChild(UnitWnd* unit,INT index=0);
	void delChild(UnitWnd* unit);
	void setSplitType(SPLIT_TYPE type);
	void getClientRect(Rect2DF* rect,INT index);
	void setSplitSize(FLOAT value);
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void recalcLayout2(Rect2DF* rect,bool flag=false);

	void onBarDrag(UnitWndMessage* m);
	SPLIT_TYPE m_splitType;
	FLOAT m_areaSize;
	UnitWndSplit m_split;
	UnitWindowThrough m_child[2];
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowPanel
// DirectX - ウインドウ用パネルパーツ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowPanel : public UnitWindow
{
public:
	UnitWindowPanel();
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
protected:
	DWORD m_borderColor1;
	DWORD m_borderColor2;
	DWORD m_borderClient;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowCampus
// DirectX - イメージウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowCampus : public UnitWindow
{
public:
	UnitWindowCampus();
	Gdiplus::Graphics* getGraphics();
	UnitGdip* getGdip();
	INT getImageWidth()const;
	INT getImageHeight()const;
protected:
	void onScroll(UnitWndMessage* m);
	void onSize(UnitWndMessage* m);
	UnitGdip m_gdip;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowText
// DirectX - テキストコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowText : public UnitWindowCampus
{
public:
	UnitWindowText();
	void setText(LPCWSTR text);
protected:
	virtual void onPaint();
	WString m_text; 
	Font m_font;
	DWORD m_textColor;
};



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowList
// DirectX - リストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum 
{
	WND_LIST_ITEM_CLICK=100,
	WND_LIST_ITEM_HOVER,
	WND_LIST_ITEM_DBCLICK
};
class UnitWindowList : public UnitWindowCampus
{
	struct HITEM
	{
		INT type;
		INT width;
		WString name;
	};
	struct HITEMS
	{
		INT height;
		std::vector<HITEM> items;
	};
	struct ITEM
	{
		ITEM()
		{
			flag = 0;
		}
		WString name;
		DWORD color;
		DWORD bcolor;
		DWORD flag;
	};
	struct ITEMS
	{
		bool select;
		INT height;
		std::vector<ITEM> items;
		LPVOID value;
	};
public:
	UnitWindowList();
	LPCWSTR getHeaderText(INT item) const;
	LPCWSTR getItemText(INT item,INT subItem) const;
	INT getItemHeight(INT index) const;
	void setColumnWidth(INT index,INT width);
	INT getColumnType(INT index) const;
	INT getColumnWidth(INT index) const;
	INT getColumnTextWidth(INT index);
	INT getItemTextWidth(INT item,INT subItem);
	INT getItemsWidth(INT index=0);
	INT getItemsHeight();
	INT addHeader(LPCSTR name,INT size=-1,INT type=0);
	INT addHeader(LPCWSTR name=L"",INT size=-1,INT type=0);
	INT addItem(LPCWSTR text,LPVOID value=NULL);
	INT addItem(LPCWSTR text,INT value)
	{
		return addItem(text,(LPVOID)value);
	}
	bool delItem(INT index);
	LPVOID getItemValue(INT index)const;
	bool setItemValue(INT index,LPVOID value);
	bool setItemText(INT item,INT subItem,LPCWSTR text);
	INT findItemIndex(LPVOID value) const;
	INT getRows() const;
	INT getCols() const;
	void sort(INT index=0,bool order=true);
	bool isSelectItem(INT index) const;
	INT getSelectItem() const;
	void setSelectItem(INT index=-1,bool flag = true);
	void clearItem();
	void setItemPadding(RECT* rect);
	void getItemPadding(RECT* rect) const;
	bool getItemRect(INT item,INT subItem,RECT* rect) const;
	void setHeaderVisible(bool flag);
	bool isHeaderVisible() const;

	void setItemBackColor(DWORD color);
	void setItemTextColor(DWORD color);
	bool setItemTextColor(INT item,INT subItem,DWORD color);
	DWORD getItemTextColor(INT index,INT subItem) const;
	void setLastWidth();
	void setScrollColumn(INT index)
	{
		m_scrollColumnIndex = index;
	}
	void setSortEnable(bool enable);
	AFL::WINDOWS::Font* getItemFont(){return &m_itemFont;}
protected:
	void sortList();
	ITEM* getItem(INT item,INT subItem) const;
	void drawBox(INT x,INT y,INT width,INT height,bool select);
	virtual void onPaint();
	INT getColumnIndex(INT x,INT y) const;
	void onMouseMove(UnitWndMessage* m);
	void onMouseUp(UnitWndMessage* m);
	void onMouseDown(UnitWndMessage* m);
	void onMouseDrag(UnitWndMessage* m);
	void onMouseOut(UnitWndMessage* m);
	void onMouseLDBClick(UnitWndMessage* m);

	AFL::WINDOWS::Font m_headerFont;
	AFL::WINDOWS::Font m_itemFont;
	HITEMS m_header;
	std::vector<AFL::SP<ITEMS> > m_items;
	DWORD m_headerColor1;
	DWORD m_headerColor2;
	DWORD m_headerClient;
	DWORD m_headerClientSelect;
	DWORD m_itemBackColor;
	DWORD m_itemTextColor;

	INT m_moveColumnIndex;
	INT m_moveColumnX;
	INT m_scrollColumnIndex;
	INT m_hoverItem;
	INT m_hoverSubItem;
	RECT m_itemPadding;
	bool m_sort;
	bool m_sortEnable;
	INT m_sortIndex;
	bool m_sortOrder;
	bool m_headerVisible;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitPulldown
// DirectX - プルダウンメニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitPulldown : public UnitWnd
{
public:
	UnitPulldown();
	void clearItem();
	void addItem(LPCWSTR text,INT value);
	LPCWSTR getItemText(INT value) const;
	INT getSelectValue() const;
	void setSelectValue(INT value);
	LPCWSTR getSelectText() const;
protected:
	void onSize(UnitWndMessage* m);
	void onUnitMessage(UnitWndMessage* m);
	UnitWindowList m_unitList;
	UnitWindowText m_unitText;
	UnitButton m_unitButton;
	INT m_selectValue;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMenu
// DirectX - メニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitMenu : public UnitWindowList
{
public:
	UnitMenu();
	void show(FLOAT x,FLOAT y)
	{
		UnitWindowManager* manager = getManager();
		if(manager)
		{
			FLOAT width = manager->getWindowWidth();
			FLOAT height = manager->getWindowHeight();
			if(x+getWindowWidth() > width)
				x = width - getWindowWidth();
			if(y+getWindowHeight() > height)
				y = height - getWindowHeight();
			if(x < 0)
				x = 0;
			if(y < 0)
				y = 0;
		}

		setSelectItem(-1,false);
		setPosW(1000);
		setPos(x,y);
		setVisible(true);
		setActive(true);
	}
protected:
	void onMessage(UnitWndMessage* m);
	virtual void onPaint();

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowEdit
// DirectX - テキストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowEdit : public UnitWindowCampus
{
public:
	UnitWindowEdit();
	void setText(LPCWSTR text);
	void addText(LPCWSTR text);
	void setPoint(INT point);
	void setPointUp();
	void setPointDown();
	void setPointLeft();
	void setPointRight();
	bool delSelect();
	void setMultiLine(bool flag);
	INT getSelectStart() const;
	INT getSelectEnd() const;
	INT getSelectLength() const;
	bool isSelect() const;
	void setSelect(INT start=-1,INT end=-1);
	LPCWSTR getText() const;
	void setMask(bool flag);
	bool isMask() const;
	void copy();
	void cut();
	void paste();
protected:
	void setCursolPos();
	void onMessage(Message* m);
	void onUnitMessage(UnitWndMessage* m);

	virtual void onSize(UnitWndMessage* m);
	virtual void onPaint();
	virtual void onAction(World* world,LPVOID value);
	WINDOWS::TextArea m_textArea;
	bool m_resize;
	UnitVector m_cursol;
	INT m_textPoint;
	INT m_cursolTime;

	Fep m_fep;
	WString m_fepString;
	INT m_fepPoint;
	INT m_fepCursol;
	bool m_pointFlag;
	bool m_multiLine;
	INT m_selectStart;
	INT m_selectEnd;
	bool m_select;
	bool m_mask;
	UnitMenu m_menu;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowEdit
// DirectX - テキストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitTextBox : public UnitWindowEdit
{
public:
	UnitTextBox();
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitNumeric
// DirectX - 数値入力用コントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitNumeric : public UnitWnd
{
public:
	UnitNumeric();
	void setValue(INT value);
	INT getValue() const;
	LPCWSTR getText() const;
protected:
	void onMessage(UnitWndMessage* m);
	UnitTextBox m_unitText;
	UnitWnd m_unitPanel;
	UnitButton m_unitButtonUP;
	UnitButton m_unitButtonDOWN;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitTabItem
// DirectX - タブ用アイテム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitTabItem : public UnitWnd
{
public:
	UnitTabItem();
	void setText(LPCWSTR text,WINDOWS::Font* font);
	void setSelect(bool flag=true);
	bool isSelect() const;
	void setValue(LPVOID value);
	LPVOID getValue() const;
protected:
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	WString m_text;
	UnitGdip m_unitText;
	bool m_select;
	WINDOWS::Font* m_font;
	LPVOID m_value;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowTab
// DirectX - タブコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum 
{
	WND_TAB_CHANGE=200
};
class UnitWindowTab : public UnitWnd
{
public:
	UnitWindowTab();
	virtual ~UnitWindowTab();
	void clearItem();
	INT getItemsWidth() const;
	UnitTabItem* addItem(LPCWSTR name,LPVOID value=NULL);
	void delItem(UnitTabItem* item);
	void setSelectIndex(INT index)
	{
		std::list<UnitTabItem*>::iterator it = m_item.begin();
		std::advance(it, index);
		if(it != m_item.end())
			setSelect(*it);
		recalcLayout(true);
	}
	void setSelect(UnitTabItem* item);
	void setSelectValue(LPVOID value);
	UnitTabItem* getSelectItem() const;
	LPVOID getSelectValue() const
	{
		if(!m_selectItem)
			return NULL;
		return m_selectItem->getValue();
	}
protected:
	void setScroll();
	void onScroll(UnitWndMessage* m);
	void onMessage(UnitWndMessage* m);
	void onSize(UnitWndMessage* m);
	WINDOWS::Font m_font;
	UnitWindow m_panel;
	UnitWnd m_panel2;
	UnitButton m_left;
	UnitButton m_right;
	UnitTabItem* m_selectItem;
	std::list<UnitTabItem*> m_item;
	DWORD m_selectColor;
	DWORD m_backColor;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TreeItem
// DirectX - ツリーアイテム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowTree;
class TreeItem
{
	friend class UnitWindowTree;
public:
	TreeItem(UnitWindowTree* tree);
	void setText(LPCWSTR text);
	LPCWSTR getText() const;
	bool isOpen() const;
	void setOpen(bool flag=true);
	LPVOID getValue() const;
	void setValue(LPVOID value);
	TreeItem* addItem(LPCWSTR text=NULL);
	void delItem(TreeItem* item);
	INT getChildCount() const;
	TreeItem* getOpenItem(INT& count);
	INT getLevel() const;
	TreeItem* getParentItem() const;
	void clearItem();
	TreeItem* findItemValue(LPVOID value);
	TreeItem* findItemText(LPCWSTR value);
	bool isItemParent(TreeItem* item);
protected:
	WString m_text;
	LPVOID m_value;
	bool m_opened;
	TreeItem* m_parent;
	std::list<SP<TreeItem> > m_childs;
	UnitWindowTree* m_tree;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWindowTree
// DirectX - ツリービュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWindowTree : public UnitWindowCampus
{
public:
	UnitWindowTree();
	~UnitWindowTree();
	TreeItem* getItem();
	void clearItem();
	void getItemSize(SIZE* size);
	void setSelectItem(TreeItem* item);
protected:
	bool getItemPos(TreeItem* parentItem,TreeItem* item,POINT* point);
	bool getItemPos(TreeItem* item,POINT* point);
	INT getItemSize(TreeItem* item,INT x,INT y,LONG& width);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	INT drawItem(TreeItem* item,INT x,INT y);
	void onMouseDown(UnitWndMessage* m);
	void onMouseOut(UnitWndMessage* m);
	void onMouseMove(UnitWndMessage* m);
	TreeItem* getItem(INT y);

	
	TreeItem* m_rootItem;
	TreeItem* m_hoverItem;
	TreeItem* m_selectItem;
};

}}

#define __INC_AFLDIRECT3DWINDOW
#endif