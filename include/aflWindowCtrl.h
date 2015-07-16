#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLCTRL


#include "aflWindow.h"
#include <time.h>
namespace AFL{ namespace WINDOWS{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WSplit
// 分割ウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum SPLIT_STYLE
{
	SPLIT_DEFAULT,
	SPLIT_VERT,		//横分け
	SPLIT_HORIZ		//縦分け
};
enum SPLIT_BASE
{
	SPLIT_FIRST,
	SPLIT_SECOND
};
class WSplit : public Window
{
public:
	WSplit();
	virtual bool createChildWindow(HWND hwnd=NULL);
	bool create(HWND parent=NULL,SPLIT_STYLE style=SPLIT_DEFAULT);
	void setBorderWidth(INT width);
	void recalcLayout(RECT& rect);
	void update();
	Window* getChild(INT index);

	void setSplitSize(INT size,SPLIT_BASE base=SPLIT_FIRST);
	bool addChild(INT index,Window* window,CHILD_STYLE style=CHILD_AUTO);
	void setSplitStyle(SPLIT_STYLE style);
protected:
	bool updateSplit();
	void onEracebkgnd(Message* message);
	void onSplitMove(Message* message);
	void onSplitSize(Message* message);
	void onSplitPaint(Message* message);
	void onSetCursor(Message* message);
	void onHitTest(Message* message);
	void onMoving(Message* message);

	Window m_splitBar;
	Window m_client1;
	Window m_client2;
	SPLIT_STYLE m_splitStyle;
	SPLIT_BASE m_splitBase;
	HCURSOR m_cursor;
	INT m_borderWidth;
	INT m_splitSize;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowStatic
// スタティックテキスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowStatic : public Window
{
public:
	WindowStatic();
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowStatus
// ステータスウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowStatus : public Window
{
public:
	WindowStatus();
	void setText(INT index,LPCSTR text);
	void setText(INT index,LPCWSTR text);
	void setParts(INT count,const INT* widths);
	bool createChildWindow(HWND hwnd);
	bool create(HWND parent,UINT id=0);
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowEdit
// エディットコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowEdit : public Window
{
public:
	WindowEdit();
	void setPassword(bool flag);
	void setReadonry(bool flag=true);
	void setMultiLine(bool flag);
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WText
// テキストウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WText : public Window
{
public:
	WText();

	INT printf(LPCSTR format, ...);
	INT printf(LPCWSTR format, ...);
	void out(LPCSTR text);
	void out(LPCWSTR text);
	void clear() const;

	void setTextColor(COLORREF color);
	void getText(std::string& dest);
	void getText(std::wstring& dest);
	void setText(LPCSTR value);
	void setText(LPCWSTR value);
	void setMultiLine(bool flag);
	void setPassword(bool flag);
	void setReadonly(bool flag);
	HWND getCtrl()const;
protected:
	void onCreate(Message* message);
	void onSize(Message* message);
	HWND m_edit;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowTree
// ツリーウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowTree : public Window
{
public:
	WindowTree();
	HTREEITEM insertItem(HTREEITEM parent,LPCSTR text,bool expand=true);
	HTREEITEM insertItem(HTREEITEM parent,LPCWSTR text,bool expand=true);
	bool delItem(HTREEITEM item=TVI_ROOT);
	bool delItemChildren(HTREEITEM parent);
	void setItemStat(HTREEITEM item,UINT stat,UINT statMask);
	bool sort(HTREEITEM item)
	{
		return sendMessage(TVM_SORTCHILDREN,(WPARAM)0,(LPARAM)item) != 0;
	}
	INT getChildCount(HTREEITEM parent)
	{
		INT i;
		HTREEITEM item = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)parent);
		for(i=0;item;i++)
		{
			item = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item);
		}
		return i;
	}
	bool setChildren(HTREEITEM item,bool flag)
	{
		TVITEM tv;
		ZeroMemory(&tv,sizeof(tv));
		tv.hItem = item;
		tv.mask = TVIF_CHILDREN;
		tv.cChildren = flag;
		return sendMessage(TVM_SETITEM,(WPARAM)0,(LPARAM)&tv) != 0;

	}
	bool getItemText(HTREEITEM item,WString& text)
	{
		WCHAR buff[2048];
		TVITEMW tv;
		ZeroMemory(&tv,sizeof(tv));
		tv.hItem = item;
		tv.mask = TVIF_TEXT;
		tv.pszText = buff;
		tv.cchTextMax = 1024;


		if(sendMessage(TVM_GETITEM,(WPARAM)0,(LPARAM)&tv) == 0)
			return false;

		text = tv.pszText;
		return true;
	}

	HTREEITEM getItemParent(HTREEITEM item)
	{
		return (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)item);
	}
	HTREEITEM getItemChild(HTREEITEM item)
	{
		return (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)item);
	}
	HTREEITEM findItem(HTREEITEM parent,LPCWSTR name)
	{
		HTREEITEM item = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)parent);
		while(item)
		{
			WString itemText;
			getItemText(item,itemText);
			if(itemText == name)
				return item;
			item = (HTREEITEM)sendMessage(TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item);
		}
		return 0;
	}
	bool openItem(HTREEITEM item,bool flag=true)
	{
		return sendMessage(TVM_EXPAND,(WPARAM)flag?TVE_EXPAND:TVE_COLLAPSE,(LPARAM)item) != 0;

	}
protected:


};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowList
// リストウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct SORTPARAM
{
	INT index1,index2;
	INT cmp;
};
class WindowList : public Window
{
public:
	struct ITEMPARAM
	{
		ITEMPARAM();
		COLORREF text;
		COLORREF back;
		HFONT font;
	};
	static const int WM_ITEM_UPDATE = WM_APP + 100;


	std::map<std::pair<INT,INT>,ITEMPARAM> m_itemParam;
	WindowList();
	void setFont()
	{
		m_font.setSize(12);
		m_font.setFontName(L"ＭＳ ゴシック");
		sendMessage(WM_SETFONT,(WPARAM)(HFONT)m_font,1);
	}
	void setItemText( int item, int subItem, LPCSTR text)const;
	void setItemText( int item, int subItem, LPCWSTR text)const;
	bool getItemText(int item,int subItem,LPSTR text,int length) const;
	bool getItemText(int item,int subItem,LPWSTR text,int length) const;
	bool getItemText(INT item,INT subItem,String& value) const;
	bool getItemText(INT item,INT subItem,WString& value) const;
	int getStringWidth(LPCSTR text)const;
	int getStringWidth(LPCWSTR text)const;
	INT getColumnCount()const;

	bool getSubItemRect( int iItem, int iSubItem, int nArea, LPRECT pRect)const;
	DWORD setExtendedStyle( DWORD dwNewStyle )const;
	int insertItem(int nItem, LPCSTR lpszItem )const;
	int insertItem(int nItem, LPCWSTR lpszItem )const;
	int insertItem(LPCWSTR text) const;
	int insertItem(LPCSTR text) const;


	bool deleteItem( int nItem )const;
	bool deleteAllItems()const;
	void createItems(INT count)const;
	void setColumnWidth() const;
	int findItem( LVFINDINFOA* pFindInfo, int nStart = -1 )const;
	int findItem( LVFINDINFOW* pFindInfo, int nStart = -1 )const;
	int getNextItem(INT index = -1)const;
	int findItem(LPCSTR itemName,int nStart = -1)const;
	int findItem(LPARAM param)const;
	bool sortItems(PFNLVCOMPARE pfnCompare,DWORD dwData)const;
	int hitTest(LVHITTESTINFO* pHitTestInfo )const;
	int hitTest(LPPOINT pPoint)const;
	int hitTestSub(LVHITTESTINFO* pHitTestInfo )const;
	INT hitTest(INT x,INT y) const;

	UINT getItemStat(int item,UINT mask)const;
	LPARAM getItemParam(INT item) const;
	int setItemParam(INT item,LPARAM data)const;

	bool ensureVisible( int nItem, bool bPartialOK)const;
	bool scroll(int nX,int nY)const;
	bool redrawItems( int nFirst, int nLast )const;
	bool update( int nItem )const;
	bool arrange( UINT nCode )const;
	
	bool getEditText(WString& dest) const;
	HWND editLabel(int nItem )const;
	int insertColumn(int nCol,LPCSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth = -10,int nSubItem = -1 ) const;
	int insertColumn(LPCSTR lpszColumnHeading)  const;
	int insertColumn(LPCSTR lpszColumnHeading,INT width) const;
	int insertColumn(int nCol,LPCWSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth=-10,int nSubItem=-1) const;
	int insertColumn(LPCWSTR lpszColumnHeading) const;
	int insertColumn(LPCWSTR lpszColumnHeading,INT width) const;

	bool deleteColumn( int nCol )const;

	HIMAGELIST createDragImage( int nItem, LPPOINT lpPoint )const;
	
	int getColumnWidth(int nCol) const;
	bool setColumnWidth(int nCol,int nWidth) const;

	INT getItemCount() const;
	void selectItem(INT index)const;
	INT getSelectItem(INT index=-1) const;
	INT getSelectItemCount() const;

	bool setImageList(HIMAGELIST imageList,INT type=LVSIL_SMALL)
	{
		return ListView_SetImageList(*this,imageList, type) != NULL;
	}
	bool setItemImage(INT index,INT iImage)
	{
		LVITEMW item;
		ZeroMemory(&item,sizeof(item));
		item.mask = LVIF_IMAGE;
		item.iItem = index;
		item.iSubItem = 0;
		item.iImage = iImage;
		sendMessage(LVM_SETITEM,0,(LPARAM)&item);
		return true;
	}

	bool setItemImage(INT index,LPCSTR path)
	{
		return setItemImage(index,UCS2(path));
	}
	bool setItemImage(INT index,LPCWSTR path)
	{
		SHFILEINFOW info;
		SHGetFileInfoW(path,0,&info,sizeof(SHFILEINFOW),SHGFI_ICON|SHGFI_USEFILEATTRIBUTES );
		LVITEMW item;
		ZeroMemory(&item,sizeof(item));
		item.iItem = index;
		item.iSubItem = 0;
		item.mask = LVIF_IMAGE;
		item.iImage = info.iIcon;
		sendMessage(LVM_SETITEM,0,(LPARAM)&item);
		return true;
	}
	COLORREF getTextColor();
	void setTextColor(COLORREF color);
	COLORREF getBackColor();
	void setBackColor(COLORREF color);
	bool sort() const;

	void setSort(ClassProc& proc)
	{
		m_sortProc = proc;
	}
protected:
	static int CALLBACK FNLVCOMPARE(LPARAM param1, LPARAM param2, LPARAM param3);

	ClassProc m_sortProc;
	DWORD onSize(Message* message);
	DWORD onCustom(Message* m);
	Font m_font;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowList
// リストウインドウクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
class WindowList : public Window
{
public:

	std::map<std::pair<INT,INT>,ITEMPARAM> m_itemParam;
	WindowList();

	void setTextColor(INT item,INT subItem,COLORREF color);
	void setBackColor(INT item,INT subItem,COLORREF color);

	int getStringWidth(LPCSTR lpszString)const;
	int getStringWidth(LPCWSTR lpszString)const;
	void setItemText( int nItem, int nSubItem, LPCWSTR lpcszText);

	void setItemText( int item, int nSubItem, LPCSTR lpcszText);

	void createColumn(INT count);
	int insertColumn(int nCol,LPCSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth = -10,int nSubItem = -1 );
	int insertColumn(LPCSTR lpszColumnHeading);
	int insertColumn(LPCSTR lpszColumnHeading,INT width);
	int insertColumn(int nCol,LPCWSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth=-10,int nSubItem=-1);
	int insertColumn(LPCWSTR lpszColumnHeading);
	int insertColumn(LPCWSTR lpszColumnHeading,INT width);

	bool getItemText(int item,int subItem,LPSTR text,int length) const;
	bool getItemText(int item,int subItem,LPWSTR text,int length) const;
	bool getItemText(INT item,INT subItem,String& value) const;
	bool getItemText(INT item,INT subItem,WString& value) const;
	bool setItemImage(INT index,LPCWSTR path)
	{
		SHFILEINFO info;
		HIMAGELIST hList = (HIMAGELIST)SHGetFileInfo(TEXT("C:\\"), 0, &info, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		SHGetFileInfo(path,0,&info,sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_USEFILEATTRIBUTES );
		ListView_SetImageList(*getCtrl(),hList, LVSIL_SMALL);
		LVITEMW item;
		ZeroMemory(&item,sizeof(item));
		item.mask = LVIF_IMAGE;
		item.iImage = info.iIcon;
		item.stateMask = LVIS_STATEIMAGEMASK;
		item.state = LVIS_STATEIMAGEMASK;
		getCtrl()->sendMessage(LVM_SETITEMSTATE,index,(LPARAM)&item);
		return true;
	}


	UINT getItemStat(int item,UINT mask);


	bool getSubItemRect( int iItem, int iSubItem, int nArea, LPRECT pRect);
	DWORD setExtendedStyle( DWORD dwNewStyle );

	int insertItem(LPCWSTR text);
	int insertItem(LPCSTR text);
	LPARAM getItemParam(INT item) const;
	int setItemParam(INT item,LPARAM data);
	void createItems(INT count);
	int insertItem(int nItem, LPCSTR lpszItem );
	int insertItem(int nItem, LPCWSTR lpszItem );
	void selectItem(INT index=-1);
	INT getSelectItem(INT index=-1) const;
	bool deleteItem( int nItem );
	bool deleteAllItems();
	int findItem( LVFINDINFOA* pFindInfo, int nStart = -1 );
	int findItem( LVFINDINFOW* pFindInfo, int nStart = -1 );
	int getNextItem(INT index = -1);
	int findItem(LPCSTR itemName,int nStart = -1);
	bool sortItems(PFNLVCOMPARE pfnCompare,LPARAM data);
	int hitTest(LVHITTESTINFO* pHitTestInfo );
	int hitTest(LPPOINT pPoint);
	bool ensureVisible( int nItem, bool bPartialOK);
	bool scroll(int nX,int nY);
	bool redrawItems( int nFirst, int nLast );
	bool update( int nItem );
	bool arrange( UINT nCode );
	
	HWND editLabel(int nItem );


	bool deleteColumn( int nCol=-1 );

	HIMAGELIST createDragImage( int nItem, LPPOINT lpPoint );
	
	int getColumnWidth(int nCol) const ;
	bool setColumnWidth(int nCol,int nWidth) const ;
	bool setColumnText(int col,LPCSTR name) const;

	INT getItemCount()const;
	INT getColumnCount()const;
	void setColumnWidth()const;
	Window* getCtrl();
	INT hitTestSub(LVHITTESTINFO* pHitTestInfo );
	INT hitTest(INT x,INT y);
	COLORREF getTextColor();
	void setTextColor(COLORREF color);
	COLORREF getBackColor();
	void setBackColor(COLORREF color);
protected:
	DWORD onCreate(Message* message);
	DWORD onCustom(Message* m);

};
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowVList
// 仮想リスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct VItem
{
	INT item;
	INT subItem;
	String value;
};
struct VItemW
{
	INT item;
	INT subItem;
	WString value;
};
class WindowVList : public Window
{
public:
	WindowVList();
	int insertColumn(int nCol,LPCSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth = -10,int nSubItem = -1 );
	int insertColumn(LPCSTR lpszColumnHeading);
	int insertColumn(LPCSTR lpszColumnHeading,INT width);
	bool deleteColumn( int nCol=-1 );
	void createColumn(INT count);
	int insertColumn(int nCol,LPCWSTR lpszColumnHeading,int nFormat = LVCFMT_LEFT,int nWidth=-10,int nSubItem=-1);
	DWORD setExtendedStyle( DWORD dwNewStyle );
	int getStringWidth(LPCSTR lpszString);
	int getStringWidth(LPCWSTR lpszString);
	INT getItemCount()const;
	INT getColumnCount()const;
	void setItemCount(INT count) const;
	void setDisplay(ClassProc& proc);

	void setColumnWidth()const;
	void updateItem(INT index=-1) const;
	void redrawItem(INT index1=-1,INT index2=-2);
	bool setColumnWidth(int nCol,int nWidth) const;
	int getColumnWidth(int nCol) const;
	COLORREF getTextColor();
	void setTextColor(COLORREF color);
	COLORREF getBackColor();
	void setBackColor(COLORREF color);
	INT getSelectItem(INT index=-1) const;
	INT hitTestSub(LVHITTESTINFO* pHitTestInfo );
	INT hitTest(INT x,INT y);
	void selectItem(INT index=-1,bool flag=true);
	int getNextItem(INT index = -1);
	UINT getItemStat(int item,UINT mask);
	void setUnicode(bool flag) const;

protected:
	DWORD onSize(Message* message);

	void onCreate(Message* m);
	void onDispInfo(Message* m);
	void onDispInfoW(Message* m);
	void onKeydown(Message* m);

	ClassProc m_callDisplay;
	DWORD m_extendedStyle;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowPanel
// パネル
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowPanel : public Window
{
public:
	WindowPanel();
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WButton
// ボタンコントロール
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WButton : public Window
{
public:
	WButton();
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowCombo
// コンボボックス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowCombo : public Window
{
public:
	WindowCombo();
	bool insertItem(LPCWSTR value);
	bool insertItem(LPCSTR value);
	bool insertItem(INT index,LPCSTR value);
	bool insertItem(INT index,LPCWSTR value);
	bool clear();
	bool deleteItem(INT index);
	bool getItem(INT index,WString& value);
	bool getItem(INT index,String& value);
	INT findItem(LPCSTR value);
	bool selectItem(INT index);
	INT getSelectItemIndex()const
	{
		return (INT)sendMessage(CB_GETCURSEL);
	}
	bool getItem(WString& value);
	bool getItem(String& value);
	INT getItemCount();
	HWND getEdit();
	Window* getEditWindow()
	{
		return &m_text;
	}
protected:
	void onDropDown(Message* m)
	{
		setWindowHeight(400);
	}
	void onCreate(Message* m)
	{
		HWND hWnd = getEdit();
		m_text.createWindow(hWnd);
	}
	WindowEdit m_text;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowRiche
// リッチテキスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowRiche : public Window
{
public:
	WindowRiche();
	void out(LPCSTR text);
	void out(LPVOID text,INT size);
	void out(LPCWSTR text);
	INT printf(LPCSTR format, ...);
	INT printf(LPCWSTR format, ...);
	void setReadonly(bool flag=true);
	void setTextLimit(INT value) const;
	void setCaret() const;
	void setSel(INT start=-1,INT end=-1) const;
	void getSel(INT* start,INT* end) const;
	INT getSelLength()const;
	void getSelText(WString& dest) const;
	void setFont(LPCWSTR name);
	void setReadonly(bool flag) const;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowRiche
// リッチテキストビュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowRicheView : public Window
{
public:
	WindowRicheView();
	void out(LPCSTR text);
	void out(LPCWSTR text);
	WindowRiche* getCtrl();
protected:
	void onCreate(Message* message);
	WindowRiche m_ctrl;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowTab
// タブウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowTab : public Window
{
public:
	WindowTab();
	INT insertItem(INT index,LPCSTR text,HWND hwnd=NULL);
	INT insertItem(INT index,LPCSTR text,Window& window);
	bool selectItem(INT index);
	INT getCurSel() ;
	INT getItemCount();
	void setParam(INT index,LPVOID data);
	LPARAM getParam(INT index);
	bool setItemText(INT index,LPCSTR value);
	bool getName(INT index,std::string& desc);
	bool deleteItem(HWND wndDel);
	bool deleteItem(INT index=-1);
	void showTab();
	void setTabSize(INT width,INT height);
	void getAjustRect(LPRECT rect);
	void recalcLayout(RECT& rect);
protected:
	void onSelchanges(LPNMHDR nmHDR);
	void onCreate(Message* message);


};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowEditList
// 編集可能リストウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WindowListEdit : public WindowList
{
public:
	WindowListEdit();
	void setEditColumn(INT index);
protected:
	void setValue();
	void onEditChar(Message* m);
	void onEditKillFocus(Message* m);
	void onLButtonDown(Message* m);
	virtual void onEditItem(INT item,INT itemSub);

	INT m_item;
	INT m_itemSub;
	WText m_edit;
	std::set<INT> m_editIndex;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Menu
// メニュー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Menu
{
public:
	Menu();
	~Menu();
	operator HMENU()const;
	bool append(UINT id=0) const;
	bool append(UINT id,LPCSTR text) const;
	bool append(UINT id,LPCWSTR text) const;
	void show(HWND hwnd);
	void show(INT x,INT y,HWND hwnd);
	INT getCount() const;
	void clear();

protected:
	HMENU m_menu;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TaskIcon
// タスクアイコン制御
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TaskIcon
{
public:
	TaskIcon();
	~TaskIcon();
	bool create(HWND hWnd,HICON hIcon,LPCSTR title,UINT code=0,UINT msg=WM_APP);
	void show();
	void hide();
protected:
	NOTIFYICONDATAW  m_icn;
	std::string m_title;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PropertyView
// プロパティ表示用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PropertyMenu
{
public:
	PropertyMenu(LPCSTR name,ClassProc& classProc,bool select);
	PropertyMenu(LPCWSTR name,ClassProc& classProc,bool select);
	LPCWSTR getName()const;
	bool isSelect()const;
	ClassProc& getClassProc();
protected:
	WString m_name;
	bool m_select;
	ClassProc m_classProc;
};
class PropertyData
{
	friend class PropertyView;
public:
	PropertyData();
	void set(INT index,ClassProc& classProc);
	void set(LPCSTR name,ClassProc& classProc);
	void set(LPCSTR name,INT index,ClassProc& classProc);
	void set(LPCWSTR name,ClassProc& classProc);
	void set(LPCWSTR name,INT index,ClassProc& classProc);
	void set(INT index1,INT index2,ClassProc& classProc);

	std::list<WString>& List();
	std::map<WString,std::map<INT,ClassProc> >& Property();
	std::list<std::pair<WString,INT> >& Column();

	void clearData();
	void addMenu(LPCSTR name,ClassProc& classProc,bool select=false);
	void addMenu(LPCWSTR name,ClassProc& classProc,bool select=false);

protected:
	std::list<WString> m_propertyList;						//プロパティ順序保存用
	std::map<WString,std::map<INT,ClassProc> > m_property;	//アイテム名、インデックス、コールバックプロシージャ
	std::list<std::pair<WString,INT> > m_columnName;		//ヘッダ名,サイズ
	std::vector<PropertyMenu> m_menu;
};

enum PROPERTY_STAT
{
	PROPERTY_CREATE,
	PROPERTY_SET,
	PROPERTY_GET,
	PROPERTY_ACTION
};
struct PropertyMessage
{
	HWND hWnd;
	TString value;
	RECT rect;
	INT item;
	INT subItem;
	PROPERTY_STAT stat;
	Window* param;
};
#define WM_PROPERTY_EDITSELECT (WM_APP+5)
class PropertyView : public WindowList
{
public:
	PropertyView();
	~PropertyView();
	void setDirectClick(bool flag){m_directClick = flag;}
	void setProperty(PropertyData* propertyData);
	void update();

	void setCallback(INT item,INT subItem,ClassProc& classProc);
	ClassProc* getCallback(INT item,INT subItem);
	INT getClickItem();
	INT getClickSubItem();
	void addMenu(LPCSTR name,ClassProc& classProc,bool selected=false);
	void addMenu(LPCWSTR name,ClassProc& classProc,bool selected=false);
	void edit(INT item,INT subItem);
	void closeEdit();

protected:
	void onRButtonDown(Message* m);
	void onCommand(Message* m);
	void onCreate(Message* m);

	void onLButtonDown(Message* m);
	void onEditSelect(Message* m);
	void onSelect(INT item,INT subItem,RECT* rect);
	void onEdit(Message* m);

	Menu m_menu;
	std::vector<PropertyMenu> m_menuList;

	Window* m_editWindow;
	std::map<INT,std::map<INT,ClassProc > > m_control;
	INT m_item;
	INT m_subItem;
	INT m_editItem;
	INT m_editSubItem;
	bool m_directClick;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputCombo
// コンボボックス入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputCombo : public Window
{
public:
	InputCombo();
	void add(LPCSTR value);
	void add(LPCWSTR value);
	bool create(HWND hWnd,WString& value,RECT& rect);
	bool create(HWND hWnd,String& value,RECT& rect);
protected:
	void onEditChange(Message* m);
	void onKillFocus(Message* m);
	std::list<WString> m_value;
	bool m_quit;
	bool m_ret;
	bool m_editKill;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputButtonText
// ボタン付きテキスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputButtonText : public Window
{
public:
	InputButtonText();
	bool create(HWND hWnd,WString& value,RECT& rect);
	void setSubmit(ClassProc& proc){m_proc = proc;}
	Window* getTextWindow(){return &m_text;}
protected:
	void onEditChar(Message* m);
	void onKillFocus(Message* m);
	void onSubmit(Message* m);
	WButton m_button;
	Window m_text;
	bool m_ret;
	ClassProc m_proc;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFile
// ファイル入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputFile
{
public:
	static bool doModal(HWND hwnd,String& fileName);
	static bool doModal(HWND hwnd,std::wstring& fileName);
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFont
// フォント入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputFont : public InputButtonText
{
public:
	InputFont();
	bool submit(Message* m);
	void setLogFont(LPLOGFONTW logfont);
	void getLogFont(LPLOGFONTW logfont);
protected:
	LOGFONTW m_logfont;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputList
// リスト入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputList : public InputButtonText
{
public:
	InputList();
	bool submit(Message* m);
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputColor
// カラー入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputColor : public InputButtonText
{
public:
	InputColor();

	bool submit(Message* m);
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputFileText
// ファイル名入力
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputFileText : public InputButtonText
{
public:
	InputFileText();
protected:
	bool submit(Message* m);
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputText
// テキスト入力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputText : public WText
{
public:
	InputText();
	bool create(HWND hWnd,LPCWSTR value,RECT& rect);
	void setPassword(bool flag);
	INT getLastCode()const{return m_lastCode;}
protected:
	bool m_pass;
	void onEditChar(Message* m);
	void onEditKillFocus(Message* m);
	bool m_ret;
	INT m_lastCode;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowDebug
// デバッグ用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
class WindowDebug : public WindowRicheView
{
public:

	static INT WindowDebug::Debug(LPCSTR format, ...);
private:
	static WindowRicheView m_window;
};*/
}}
#define __INC_AFLCTRL
#endif
