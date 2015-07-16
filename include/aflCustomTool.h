#include "aflGraphics.h"
#include "aflMap.h"
#include "aflActiveTool.h"


enum
{
	ACTION_MANUAL,
	ACTION_ENEMY1
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteFade
// 画面フェード用スプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflSpriteFade : public AflMeshList,public AflActiveCounter
{
public:
	enum
	{
		FADE_IN=1,
		FADE_OUT
	};
	AflSpriteFade();
	bool create(FLOAT fWidth,FLOAT fHeight);
	void setFadeLevel(INT iLevel);
	INT getFadeLevel()const{return m_iLevel;}
	void fadeIn();
	void fadeOut();
	void fadeNormal();
	virtual void onCounterCallback(DWORD dwID);

protected:
	INT m_iMode;
	INT m_iLevel;
	AflMeshBuffer* m_pVertex;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteSelect
// キャラクター選択表示用スプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflSpriteSelect : public AflMeshList,public AflActiveCounter
{
public:
	AflSpriteSelect();
	void setTarget(AflMeshList* pTarget);
	AflMeshList* getTarget()const;
protected:
	virtual void onDraw();

	AflMeshList* m_ptargetSprite;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflChara
// キャラクター用カスタムクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflChara : public AflSprite,public AflActiveCounter
{
public:
	AflChara();
	~AflChara();
	void setTimeSpeed(INT dwData);
	void setActionMode(DWORD dwActionMode);
	DWORD getActionMode()const;
	void resetRoute();
	void setRoute(AflMapRoute* pmapRoute,INT iX,INT iY);
	void moveRoute(AflSpriteMap* pMap);
	bool isRoute()const{return m_ppointRoute != NULL;}
	INT getTargetX()const{return m_pointTarget.x;}
	INT getTargetY()const{return m_pointTarget.y;}
	void incImageCount(){m_dwImageCount++;}
	DWORD getImageCount()const{return m_dwImageCount;}


protected:
	POINT m_pointTarget;
	LPPOINT m_ppointRoute;
	INT m_iRouteCount;
	INT m_iNowPoint;
	DWORD m_dwActionMode;
	DWORD m_dwImageCount;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteWnd
// 自前描画ウインドウ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflSpriteWnd : public AflMeshList
{
public:

	enum
	{
		AFL_HIT_NO,
		AFL_HIT_TITLE,
		AFL_HIT_CLIENT,
		AFL_HIT_FRAME,
		AFL_HIT_SIZE,
		AFL_HIT_SCROLL1,
		AFL_HIT_SCROLL2,
		AFL_HIT_SCROLLBAR
	};
	enum
	{
		AFL_NOMAL,
		AFL_MOVING,
		AFL_SIZING,
		AFL_SCROLLBAR
	};
	enum
	{
		AFL_STYLE_TITLE = 1,
		AFL_STYLE_SIZE = 2

	};

	AflSpriteWnd();
	void setTitleText(LPCSTR pString);
	virtual bool createWindow(FLOAT fWidth,FLOAT fHeight);
	void crateFrameVertex(LPAFLVERTEX pVertex,FLOAT fX,FLOAT fY,FLOAT fWidth,FLOAT fHeight,D3DCOLOR colWindow1,D3DCOLOR colWindow2);
	bool isHit(LPAFLFRECT pRect,FLOAT fX,FLOAT fY);
	virtual INT hitTest(FLOAT fX,FLOAT fY);
	void clickEnd();
	void size(FLOAT fX,FLOAT fY);
	void move(FLOAT fX,FLOAT fY);
	bool isMoving()const{return m_iMode == AFL_MOVING;}

	void setWindowStyle(DWORD dwStyle)
	{
		m_dwWindowStyle = dwStyle;
	}
	void setFrameSize(FLOAT fWidth,FLOAT fHeight)
	{
		m_fFrameWidth = fWidth;
		m_fFrameHeight = fHeight;
	}

	FLOAT getWindowWidth()const{return m_rectWindow.getWidth();}
	FLOAT getWindowHeight()const{return m_rectWindow.getHeight();}

	virtual DWORD onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY);
protected:

	virtual void onWindowMove(FLOAT fX,FLOAT fY){}
	virtual void onWindowSize(FLOAT fWidth,FLOAT fHeight){}
	virtual void onScrollPos(INT iBar,DWORD dwPos){}
	
	INT m_iMode;
	AflMeshList	m_meshSizeLine;
	AflSprite m_spriteTitle;

	AFLFRECT m_rectWindow;
	AFLFRECT m_rectTitle;
	AFLFRECT m_rectClient;
	AFLFRECT m_rectSize;

	FLOAT m_fTitleHeight;
	FLOAT m_fFrameWidth;
	FLOAT m_fFrameHeight;

	FLOAT m_fSBWidth;
	FLOAT m_fSBHeight;

	FLOAT m_fClickPointBaseX;
	FLOAT m_fClickPointBaseY;
	FLOAT m_fClickPointX;
	FLOAT m_fClickPointY;

	D3DCOLOR m_colFrame1;
	D3DCOLOR m_colFrame2;

	AflMeshBuffer* m_pTitleVertex;
	AflMeshBuffer* m_pClientVertex;
	AflMeshBuffer* m_pSizeVertex;

	DWORD m_dwWindowStyle;

	std::string m_strTitleBack;
	std::string m_strTitle;
	bool m_bCreate;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteScroll
// スクロールバー表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflSpriteScroll : public AflSpriteWnd
{
public:

	AflSpriteScroll();
	virtual bool createWindow(FLOAT fWidth,FLOAT fHeight,DWORD dwStyle=-1);
	virtual INT hitTest(FLOAT fX,FLOAT fY);
	INT getScrollPos()const{return m_iScrollPos;}
	void AflSpriteScroll::setScrollPos();
	void setScrollPos(INT iPos);
	void setScrollMax(DWORD dwMax)
	{
		m_dwScrollMax = dwMax;
		m_bRedraw = true;

	}
	void setScrollPage(DWORD dwPage)
	{
		m_dwScrollPage = dwPage;
		m_bRedraw = true;
	}
	bool isLastPos() const;
	void setRedraw(bool bFlag=true){m_bRedraw = bFlag;}
	virtual DWORD onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY);
protected:
	void onDraw(D3DDevice* device,FLOAT fX,FLOAT fY,FLOAT fZ);

	
	FLOAT m_fButtonSize;
	DWORD m_dwButtonColor1;
	DWORD m_dwButtonColor2;
	DWORD m_dwBarColor1;
	DWORD m_dwBarColor2;

	INT m_iBasePos;
	INT m_iScrollPos;
	DWORD m_dwScrollMin;
	DWORD m_dwScrollMax;
	DWORD m_dwScrollPage;

	FLOAT m_fScrollClient;
	AflFRect m_rectScroll1;
	AflFRect m_rectScroll2;
	AflFRect m_rectScrollBar;

	AflMeshBuffer* m_pButtonVertex1;
	AflMeshBuffer* m_pButtonVertex2;
	AflMeshBuffer* m_pBarVertex1;


	bool m_bCreate;
	bool m_bRedraw;
	DWORD m_dwBarStyle;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteWindow
// スクロールバー表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflSpriteWindow : public AflSpriteWnd
{
public:
	AflSpriteWindow();

	void getClientRect(AflFRect* pRect)
	{
		pRect->fLeft = m_rectClient.fLeft;
		pRect->fTop = m_rectClient.fTop;
		pRect->fRight = m_rectClient.fRight - m_scrollVBar.getWindowWidth();
		pRect->fBottom = m_rectClient.fBottom - m_scrollHBar.getWindowHeight()-4;
	}
	void setScrollPos(INT iBar,DWORD dwPos);
	void setScrollBars(DWORD dwWidth,DWORD dwHeight)
	{
		m_scrollHBar.setScrollMax(dwWidth);
		m_scrollVBar.setScrollMax(dwHeight);
		m_scrollHBar.setRedraw(true);
		m_scrollVBar.setRedraw(true);
	}
	INT getScrollPosX()const{return m_scrollHBar.getScrollPos();}
	INT getScrollPosY()const{return m_scrollVBar.getScrollPos();}

	virtual DWORD onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY);
protected:

	virtual void onWindowSize(FLOAT fWidth,FLOAT fHeight);
	virtual void onScrollPos(INT iBar,DWORD dwPos){}

	
	AflSpriteScroll m_scrollVBar;
	AflSpriteScroll m_scrollHBar;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteStrings
// 文字列表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct AflStringParam
{
	std::string strLine;
	SIZE sizePoint;
	AflSprite* pSprite;
	bool bFlag;
};

class AflSpriteStrings : public AflMeshList
{
public:
	AflSpriteStrings();
	void setStartPoint(INT iX,INT iY);
	bool open(LPCSTR pFileName);
	void setLimitWidth(INT iWidth = 0);
	void getStringsSize(LPSIZE pSize);
	void setClipper();
	void onDraw();
	void setLimitSize(INT iWidth,INT iHeight);
	void setRedraw(bool bFlag=true){m_bRedraw = bFlag;}
	void addString(LPCSTR pString);
protected:
	AflFont m_font;
	std::list<AflStringParam> m_listString;
	bool m_bRedraw;
	SIZE m_sizeLimit;
	POINT m_pointStart;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflStringsWindow
// 文字列表示用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflStringsWindow : public AflSpriteWindow
{
public:
	AflStringsWindow();
	bool open(LPCSTR pFileName);
	INT printf(LPCSTR pString,...);

protected:
	virtual void onScrollPos(INT iBar,DWORD dwPos);
	virtual void onWindowMove(FLOAT fWidth,FLOAT fHeight);
	virtual void onWindowSize(FLOAT fWidth,FLOAT fHeight);

	AflSpriteStrings m_spriteStrings;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflWindowList
// アクティブウインドウ設定用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflWindowList
{
public:
	AflWindowList();
	void addWindow(AflSpriteWnd* pWindow);
	void activeWindow(AflSpriteWnd* pWindow);
	void sortWindow();
	virtual DWORD onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY);
	virtual DWORD onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY);

protected:
	FLOAT m_fBaseW;
	std::list<AflSpriteWnd*> m_listWindow;
};