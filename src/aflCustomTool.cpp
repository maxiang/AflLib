#include <windows.h>
#include "aflCustomTool.h"

/*
#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define new new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif
*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteFade
// 画面フェード用スプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflSpriteFade::AflSpriteFade()
{
	m_pVertex = NULL;
	m_iLevel = 0;
	createCounter(0,100,true);
	m_iMode = 0;
}
//-----------------------------------------------
bool AflSpriteFade::create(FLOAT fWidth,FLOAT fHeight)
{
	AFLVERTEX vertImage[] =
	{
		{ 0,         -0,          0, 0,0,-1, 0, 0,0},
		{ 0,         -0-fHeight,  0, 0,0,-1, 0, 0,0},
		{ 0+fWidth  ,-0,          0, 0,0,-1, 0, 0,0},
		{ 0+fWidth  ,-0-fHeight,  0, 0,0,-1, 0, 0,0},
		{ 0+fWidth/2,-0-fHeight/2,0, 0,0,-1, 0, 0,0},
	};
	WORD wIndex[]={ 0,1,4, 1,3,4, 2,0,4, 2,3,4,};

	AflVertexObject* pPolygon = addPolygon();
	AflMeshBuffer* pVertex = pPolygon->getAflMeshBuffer();
	m_pVertex = pVertex;
	pVertex->createVertexBuffer(vertImage,5);
	pVertex->createIndexBuffer(wIndex,12);
	return true;
}
//-----------------------------------------------
void AflSpriteFade::setFadeLevel(INT iLevel)
{
	if(!m_pVertex)
		return;
	INT i;
	m_iLevel = iLevel;


	INT iLevel1 = -iLevel;
	INT iLevel2 = iLevel1*2;
	if(iLevel1 < 0)			iLevel1 = 0;
	else if(iLevel1 > 255)	iLevel1 = 255;
	if(iLevel2 < 0)			iLevel2 = 0;
	else if(iLevel2 > 255)	iLevel2 = 255;
	

	AFLVERTEX* pvertImage;
	m_pVertex->getVertexBuffer()->Lock( 0, sizeof(AFLVERTEX)*5,(BYTE**)&pvertImage, 0 );
	for(i=0;i<4;i++)
	{
		pvertImage[i].dwColor = iLevel2<<24 | 0x000044;
	}
	pvertImage[i].dwColor = (iLevel1<<24) | 0x888888;
	m_pVertex->getVertexBuffer()->Unlock();

}
//-----------------------------------------------
void AflSpriteFade::fadeIn()
{
	m_iMode = FADE_IN;
	m_iLevel = -255;
	clearCounter(0);
	setVisible(true);
}
//-----------------------------------------------
void AflSpriteFade::fadeOut()
{
	m_iMode = FADE_OUT;
	m_iLevel = 0;
	clearCounter(0);
	setVisible(true);
}
//-----------------------------------------------
void AflSpriteFade::fadeNormal()
{
	m_iMode = 0;
	setFadeLevel(255);
	setVisible(false);
}
//-----------------------------------------------
void AflSpriteFade::onCounterCallback(DWORD dwID)
{
	INT iLevel = m_iLevel;
	switch(m_iMode)
	{
	case 0:
		return;
	case FADE_OUT:
		iLevel--;
		if(iLevel == -255)
			m_iMode = 0;
		break;
	case FADE_IN:
		iLevel++;
		if(iLevel == 0)
		{
			m_iMode = 0;
			setVisible(false);
		}
		break;
	}
	setFadeLevel(iLevel);
}
//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteSelect
// キャラクター選択表示用スプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflSpriteSelect::AflSpriteSelect()
{
	m_ptargetSprite = NULL;
	createCounter(0,100);
}
//-----------------------------------------------
void AflSpriteSelect::setTarget(AflMeshList* pTarget)
{
	m_ptargetSprite = pTarget;
}
//-----------------------------------------------
AflMeshList* AflSpriteSelect::getTarget()const
{
	return m_ptargetSprite;
}
//-----------------------------------------------
void AflSpriteSelect::onDraw()
{
	if(m_ptargetSprite)
	{
		AflD3DWorld* pDevice = AflRenderUnit::getAflDevice();

		D3DVECTOR vectData;
		pDevice->getWorldToScreen(vectData,
			m_ptargetSprite->getAbsPointX(),
			-m_ptargetSprite->getAbsPointY(),
			m_ptargetSprite->getAbsPointZ());

		setPoint(vectData.x,vectData.y,vectData.z);
	}
	AflMeshList::onDraw(fX,fY,fZ);
}
//-----------------------------------------------
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflChara
// キャラクター用カスタムクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflChara::AflChara()
{
	m_ppointRoute = NULL;
	m_iRouteCount = 0;
	m_iNowPoint = 0;
	createCounter(0,300);

	m_dwActionMode = ACTION_MANUAL;

}
//-----------------------------------------------
AflChara::~AflChara()
{
	if(m_ppointRoute)
		delete m_ppointRoute;
}
//-----------------------------------------------
void AflChara::setTimeSpeed(INT dwData)
{
	setSpeed(0,dwData);

}
//-----------------------------------------------
void AflChara::setActionMode(DWORD dwActionMode)
{
	m_dwActionMode = dwActionMode;
}
//-----------------------------------------------
DWORD AflChara::getActionMode()const
{
	return m_dwActionMode;
}
//-----------------------------------------------
void AflChara::resetRoute()
{
	if(m_ppointRoute)
	{
		delete m_ppointRoute;
		m_ppointRoute = NULL;
		m_iRouteCount = 0;
		m_iNowPoint = 0;
	}
}
//-----------------------------------------------
void AflChara::setRoute(AflMapRoute* pmapRoute,INT iX,INT iY)
{
	resetRoute();

	INT iStartX = getPointX() / 32;//pMap->getTipWidth();
	INT iStartY = getPointY() / 32;//pMap->getTipHeight();
	INT iTargetX = iX / 32;//pMap->getTipWidth();
	INT iTargetY = iY / 32;//pMap->getTipHeight();

	LPPOINT pPoint = pmapRoute->getRoute(iStartX,iStartY,iTargetX,iTargetY);
	if(pPoint)
	{
		if(m_ppointRoute)
			delete m_ppointRoute;
		m_iRouteCount = pmapRoute->getRouteCount();
		m_ppointRoute = pPoint;
	}
}
//-----------------------------------------------
void AflChara::moveRoute(AflSpriteMap* pMap)
{
	if(m_ppointRoute)
	{
		if(m_iNowPoint < m_iRouteCount)
		{
			INT iCenterX = 0;
			INT iCenterY = pMap->getTipHeight() / 2;
			INT iX = m_ppointRoute[m_iNowPoint].x * pMap->getTipWidth() + iCenterX + 16;
			INT iY = m_ppointRoute[m_iNowPoint].y * pMap->getTipHeight() + iCenterY;

			m_pointTarget.x = iX;
			m_pointTarget.y = iY;
			if(iX == getPointX() && iY == getPointY())
				m_iNowPoint++;
		}
		else
		{
			resetRoute();
			m_pointTarget.x = getPointX();
			m_pointTarget.y = getPointY();
		}
	}
}
//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteWnd
// 自前描画ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AflSpriteWnd::AflSpriteWnd()
{
	//WindowClient領域を設定
	*this += m_spriteTitle;
	*this += m_meshSizeLine;

	m_spriteTitle.setView(false);
	m_spriteTitle.setZBuffer(false);
	m_spriteTitle.setPointW(-1);
	m_meshSizeLine.setView(false);
	m_meshSizeLine.setZBuffer(false);
	m_meshSizeLine.setPointW(-1);
	m_iMode = AFL_NOMAL;

	m_colFrame1 = 0xffffffff;
	m_colFrame2 = 0xff666666;
	m_fSBWidth = 16.0f;
	m_fSBHeight = 16.0f;

	m_fFrameWidth = 3;
	m_fFrameHeight = 2;
	m_fTitleHeight = 18;

	m_dwWindowStyle = AFL_STYLE_TITLE |	AFL_STYLE_SIZE;

	m_bCreate = false;

}
//-----------------------------------------------
void AflSpriteWnd::setTitleText(LPCSTR pString)
{
	m_strTitle = pString;
	m_spriteTitle.createText(m_strTitle.c_str(),m_rectTitle.getHeight()-1,0xffffff,0x444444);
}

//-----------------------------------------------
bool AflSpriteWnd::createWindow(FLOAT fWidth,FLOAT fHeight)
{
	//最低限の幅を確保
	if(m_dwWindowStyle & AFL_STYLE_SIZE && fWidth < m_fSBWidth*2 + m_fFrameWidth*2)
		fWidth = m_fSBWidth*2 + m_fFrameWidth*2;
	if(m_dwWindowStyle & AFL_STYLE_TITLE && fHeight < m_fSBHeight*2 + m_fTitleHeight + m_fFrameHeight * 4)
		fHeight = m_fSBHeight*2 + m_fTitleHeight + m_fFrameHeight * 4;

	//各サイズの計算
	m_rectWindow.fLeft = 0;
	m_rectWindow.fTop = 0;
	m_rectWindow.fRight = fWidth;
	m_rectWindow.fBottom = fHeight;

	FLOAT fTitleHeight = m_fTitleHeight;
	if(m_dwWindowStyle & AFL_STYLE_TITLE)
	{
		m_rectTitle.fLeft = m_fFrameWidth;
		m_rectTitle.fTop = m_fFrameHeight;
		m_rectTitle.fRight = fWidth - m_fFrameWidth;
		m_rectTitle.fBottom = fTitleHeight - m_fFrameHeight;
	}
	else
	{
		ZeroMemory(&m_rectTitle,sizeof(m_rectTitle));
		fTitleHeight = 0;
	}

	m_rectClient.fLeft = m_fFrameWidth;
	m_rectClient.fTop = fTitleHeight + m_fFrameHeight;
	m_rectClient.fRight = fWidth - m_fFrameWidth;
	m_rectClient.fBottom = fHeight + m_fFrameHeight;

	if(m_dwWindowStyle & AFL_STYLE_SIZE)
	{
		m_rectSize.fLeft = fWidth - m_fSBWidth;
		m_rectSize.fTop = fHeight - m_fSBHeight;
		m_rectSize.fRight = fWidth - m_fFrameWidth;
		m_rectSize.fBottom = fHeight + m_fFrameHeight;
	}
	//スプライトの位置設定
	m_spriteTitle.setPoint(m_rectTitle.fLeft,m_rectTitle.fTop);



	AFLVERTEX vertTitle[20];

	static WORD wFrameIndex[]={0,1,2,1,2,3, 4,5,6,5,6,7, 
					8,9,10,9,10,11, 12,13,14,13,14,15,
					16,17,18,17,18,19};

	AflVertexObject* pPolygon;
	AflMeshBuffer* pTitleVertex;
	AflMeshBuffer* pClientVertex;
	AflMeshBuffer* pSizeVertex;
	
	if(!m_bCreate)
	{
		//タイトル用頂点の確保
		pPolygon = addPolygon();
		pTitleVertex = pPolygon->getAflMeshBuffer();
		pTitleVertex->createIndexBuffer(wFrameIndex,30);
		pTitleVertex->createVertexBuffer(20);
	
		//クライアント用頂点の確保
		pPolygon = addPolygon();
		pClientVertex = pPolygon->getAflMeshBuffer();
		pClientVertex->createIndexBuffer(wFrameIndex,30);
		pClientVertex->createVertexBuffer(20);

		//サイズ変更ボタン用頂点の確保
		pPolygon = m_meshSizeLine.addPolygon();
		pSizeVertex = pPolygon->getAflMeshBuffer();
		pSizeVertex->createIndexBuffer(wFrameIndex,6);
		pSizeVertex->createVertexBuffer(4);

		m_bCreate = true;
		m_pTitleVertex = pTitleVertex;
		m_pClientVertex = pClientVertex;
		m_pSizeVertex = pSizeVertex;
	}
	else
	{
		pTitleVertex = m_pTitleVertex;
		pClientVertex = m_pClientVertex;
		pSizeVertex = m_pSizeVertex;
	}


	//タイトル
	if(m_dwWindowStyle & AFL_STYLE_TITLE)
		crateFrameVertex(vertTitle,0,0,fWidth,fTitleHeight,0xee0000ee,0xee6666ee);
	else
		ZeroMemory(vertTitle,sizeof(vertTitle));
	pTitleVertex->copyVertexBuffer(vertTitle,20);

	//クライアント
	crateFrameVertex(vertTitle,0,fTitleHeight,fWidth,fHeight-fTitleHeight,0xbbeeeeee,0xbbeeeeee);
	pClientVertex->copyVertexBuffer(vertTitle,20);
	

	FLOAT fX1;
	FLOAT fY1;
	FLOAT fSBX = m_fSBWidth;
	FLOAT fSBY = m_fSBHeight;
	fX1 = fWidth - fSBX;
	fY1 = fHeight - fSBY;
	FLOAT fX2 = fWidth - m_fFrameWidth;
	FLOAT fY2 = fHeight- m_fFrameHeight;
	AFLVERTEX vertImage[] =
	{
		{ fX1,-fY1,0, 0,0,-1, 0xffcccccc, 0,0},
		{ fX1,-fY2,0, 0,0,-1, 0xffaaaaaa, 0,0},
		{ fX2,-fY1,0, 0,0,-1, 0xffaaaaaa, 0,0},
		{ fX2,-fY2,0, 0,0,-1, 0xff555555, 0,0},
	};

	if(!(m_dwWindowStyle & AFL_STYLE_TITLE))
		ZeroMemory(vertImage,sizeof(vertImage));
	pSizeVertex->copyVertexBuffer(vertImage,4);
	
	onWindowSize(fWidth,fHeight);
	
	return true;
}
//-----------------------------------------------
void AflSpriteWnd::crateFrameVertex(LPAFLVERTEX pVertex,FLOAT fX,FLOAT fY,FLOAT fWidth,FLOAT fHeight,D3DCOLOR colWindow1,D3DCOLOR colWindow2)
{
	FLOAT fX1 = fX;
	FLOAT fY1 = fY;
	FLOAT fX2 = fX + fWidth;
	FLOAT fY2 = fY + fHeight;
	FLOAT fFX = m_fFrameWidth;
	FLOAT fFY = m_fFrameHeight;

	DWORD dwColor1 = m_colFrame1;
	DWORD dwColor2 = m_colFrame2;

	AFLVERTEX vertFrame[] =
	{
		//上横 X1,Y1 - X2,Y1
		{ fX1,       -fY1,     0, 0,0,-1, dwColor1, 0,0},
		{ fX1,       -fY1-fFY, 0, 0,0,-1, dwColor1, 0,0},
		{ fX2,       -fY1,     0, 0,0,-1, dwColor1, 0,0},
		{ fX2,       -fY1-fFY, 0, 0,0,-1, dwColor1, 0,0},
		//左縦 X1,Y1 - X1,Y2
		{ fX1,       -fY1, 0, 0,0,-1, dwColor1, 0,0},
		{ fX1+fFX,   -fY1, 0, 0,0,-1, dwColor1, 0,0},
		{ fX1,       -fY2, 0, 0,0,-1, dwColor1, 0,0},
		{ fX1+fFX,   -fY2, 0, 0,0,-1, dwColor1, 0,0},
		//下横 X1,Y2 - X2,Y2
		{ fX1,       -fY2,     0, 0,0,-1, dwColor2, 0,0},
		{ fX1,       -fY2+fFY, 0, 0,0,-1, dwColor2, 0,0},
		{ fX2,       -fY2,     0, 0,0,-1, dwColor2, 0,0},
		{ fX2,       -fY2+fFY, 0, 0,0,-1, dwColor2, 0,0},
		//右縦 X2,Y1 - X2,Y2
		{ fX2,       -fY1, 0, 0,0,-1, dwColor2, 0,0},
		{ fX2-fFX,   -fY1, 0, 0,0,-1, dwColor2, 0,0},
		{ fX2,       -fY2, 0, 0,0,-1, dwColor2, 0,0},
		{ fX2-fFX,   -fY2, 0, 0,0,-1, dwColor2, 0,0},
		//xクライアント
		{ fX1+fFX, -fY1-fFY, 0, 0,0,-1, colWindow1, 0,0},
		{ fX1+fFX, -fY2+fFY, 0, 0,0,-1, colWindow1, 0,0},
		{ fX2-fFX, -fY1-fFY, 0, 0,0,-1, colWindow2, 0,0},
		{ fX2-fFX, -fY2+fFY, 0, 0,0,-1, colWindow2, 0,0},
	};
	CopyMemory(pVertex,vertFrame,sizeof(vertFrame));
}
//-----------------------------------------------
bool AflSpriteWnd::isHit(LPAFLFRECT pRect,FLOAT fX,FLOAT fY)
{
	FLOAT fWX1 = getAbsPointX() + pRect->fLeft;
	FLOAT fWY1 = getAbsPointY() + pRect->fTop;
	FLOAT fWX2 = getAbsPointX() + pRect->fRight;
	FLOAT fWY2 = getAbsPointY() + pRect->fBottom;
	if(fX >= fWX1 && fY >= fWY1 && fX <= fWX2 && fY <= fWY2)
		return true;
	return false;
}
//-----------------------------------------------
INT AflSpriteWnd::hitTest(FLOAT fX,FLOAT fY)
{
	if(!isHit(&m_rectWindow,fX,fY))
		return AFL_HIT_NO;
	if(isHit(&m_rectSize,fX,fY))
		return AFL_HIT_SIZE;
	if(isHit(&m_rectTitle,fX,fY))
		return AFL_HIT_TITLE;
	if(isHit(&m_rectClient,fX,fY))
		return AFL_HIT_CLIENT;
	return AFL_HIT_FRAME;
}
//-----------------------------------------------
void AflSpriteWnd::clickEnd()
{
	if(m_iMode == AFL_SIZING)
	{
		m_spriteTitle.createSpriteImage(m_rectTitle.getWidth(),m_rectTitle.getHeight());
	}
	m_iMode = AFL_NOMAL;
}
//-----------------------------------------------
void AflSpriteWnd::size(FLOAT fX,FLOAT fY)
{
	FLOAT fSizeX = -m_fClickPointX+fX + m_fClickPointBaseX;
	FLOAT fSizeY = -m_fClickPointY+fY + m_fClickPointBaseY;
	createWindow(fSizeX,fSizeY);
}
//-----------------------------------------------
void AflSpriteWnd::move(FLOAT fX,FLOAT fY)
{
	FLOAT fWindowX = -m_fClickPointX+fX + m_fClickPointBaseX;
	FLOAT fWindowY = -m_fClickPointY+fY + m_fClickPointBaseY;
	setPoint(fWindowX,fWindowY);
	onWindowMove(fWindowX,fWindowY);
}
//-----------------------------------------------
DWORD AflSpriteWnd::onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	INT iHit = hitTest(fX,fY);
	if(!iHit)
		return 0;

	if(iHit == AFL_HIT_TITLE)
	{
		m_iMode = AFL_MOVING;
		m_fClickPointBaseX = getPointX();
		m_fClickPointBaseY = getPointY();
		m_fClickPointX = fX;
		m_fClickPointY = fY;
	}
	else if(iHit == AFL_HIT_SIZE)
	{
		m_iMode = AFL_SIZING;
		m_fClickPointBaseX = m_rectWindow.getWidth();
		m_fClickPointBaseY = m_rectWindow.getHeight();
		m_fClickPointX = fX;
		m_fClickPointY = fY;
	}
	return 1;
}
//-----------------------------------------------
DWORD AflSpriteWnd::onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	if(m_iMode)
	{
		clickEnd();
		return 1;
	}
	return 0;
}
//-----------------------------------------------
DWORD AflSpriteWnd::onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	if(dwFlags == MK_LBUTTON)
	{
		if(m_iMode == AFL_MOVING)
			move(fX,fY);
		else if(m_iMode == AFL_SIZING)
			size(fX,fY);
		if(m_iMode)
		{
			FLOAT fX = getPointX() + m_rectTitle.fLeft;
			FLOAT fY = getPointY() + m_rectTitle.fTop;
			m_spriteTitle.setClipper(fX,fY,0,m_rectTitle.getWidth(),m_rectTitle.getHeight(),0);
		}
	}
	else if(m_iMode)
	{
		clickEnd();
		return 0;
	}
	return hitTest(fX,fY);
}

//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteScroll
// スクロールバー表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflSpriteScroll::AflSpriteScroll()
{
	setWindowStyle(0);
	setFrameSize (1,1);
	m_fButtonSize = 14;

	m_dwBarColor1 = 0xeecccccc;
	m_dwBarColor2 = 0xee999999;
	m_dwButtonColor1 = 0xeeaaaaaa;
	m_dwButtonColor2 = 0xee888888;

	m_iScrollPos = 0;
	m_dwScrollMax = 100;
	m_dwScrollPage = 100;
	m_dwScrollMin = 16;

	m_bCreate = false;
	m_dwBarStyle = SBS_VERT;
}
//-----------------------------------------------
bool AflSpriteScroll::createWindow(FLOAT fWidth,FLOAT fHeight,DWORD dwStyle)
{
	AflSpriteWnd::createWindow(fWidth,fHeight);
	if(dwStyle != (UINT)-1)
		m_dwBarStyle = dwStyle;

	static WORD wFrameIndex[]={0,1,2,1,2,3, 4,5,6,5,6,7, 
				8,9,10,9,10,11, 12,13,14,13,14,15,
				16,17,18,17,18,19};
	INT iBarSize;

	DWORD dwScrollPage;
	if(m_dwScrollPage < m_dwScrollMax)
		dwScrollPage = m_dwScrollPage;
	else
		dwScrollPage = m_dwScrollMax;
	iBarSize = m_fScrollClient * dwScrollPage / m_dwScrollMax;
	if(iBarSize < 8)
		iBarSize = 8;

	if(m_dwBarStyle & SBS_VERT)
	{
		m_rectScroll1.fLeft = m_rectClient.fLeft;
		m_rectScroll1.fTop = m_rectClient.fTop;
		m_rectScroll1.fRight = m_rectClient.fRight;
		m_rectScroll1.fBottom = m_rectClient.fTop + m_fButtonSize;

		m_rectScroll2.fLeft = m_rectClient.fLeft;
		m_rectScroll2.fTop = m_rectClient.fBottom - m_fButtonSize;
		m_rectScroll2.fRight = m_rectClient.fRight;
		m_rectScroll2.fBottom = m_rectClient.fBottom;

		m_fScrollClient = m_rectClient.getHeight() - m_fButtonSize*2;

		m_rectScrollBar.fLeft = m_rectClient.fLeft;
		m_rectScrollBar.fTop = m_rectScroll1.fBottom +
			m_iScrollPos*m_fScrollClient/m_dwScrollMax;
		m_rectScrollBar.fRight = m_rectClient.fRight;
		m_rectScrollBar.fBottom = m_rectScrollBar.fTop + iBarSize;

		INT iWork = m_rectClient.fBottom - m_rectScrollBar.fBottom;
		if(iWork < 0)
		{
			m_rectScrollBar.fTop += iWork;
			m_rectScrollBar.fBottom += iWork;
		}
	}
	else
	{
		m_rectScroll1.fLeft = m_rectClient.fLeft;
		m_rectScroll1.fTop = m_rectClient.fTop;
		m_rectScroll1.fRight = m_rectClient.fLeft + m_fButtonSize;
		m_rectScroll1.fBottom = m_rectClient.fBottom;

		m_rectScroll2.fLeft = m_rectClient.fRight - m_fButtonSize;
		m_rectScroll2.fTop = m_rectClient.fTop;
		m_rectScroll2.fRight = m_rectClient.fRight;
		m_rectScroll2.fBottom = m_rectClient.fBottom;

		m_fScrollClient = m_rectClient.getWidth() - m_fButtonSize*2;

		m_rectScrollBar.fLeft = m_rectScroll1.fRight +
			m_iScrollPos*m_fScrollClient/m_dwScrollMax;
		m_rectScrollBar.fTop = m_rectScroll1.fTop;
		m_rectScrollBar.fRight = m_rectScrollBar.fLeft + iBarSize;
		m_rectScrollBar.fBottom = m_rectScroll1.fBottom;
	
		INT iWork = m_rectClient.fRight - m_rectScrollBar.fRight;
		if(iWork < 0)
		{
			m_rectScrollBar.fLeft += iWork;
			m_rectScrollBar.fRight += iWork;
		}
	}

	AflVertexObject* pPolygon;
	AflMeshBuffer* pButtonVertex1;
	AflMeshBuffer* pButtonVertex2;
	AflMeshBuffer* pBarVertex;


	if(!m_bCreate)
	{
		pPolygon = addPolygon();
		pButtonVertex1 = pPolygon->getAflMeshBuffer();
		pButtonVertex1->createIndexBuffer(wFrameIndex,30);
		pButtonVertex1->createVertexBuffer(20);
	
		pPolygon = addPolygon();
		pButtonVertex2 = pPolygon->getAflMeshBuffer();
		pButtonVertex2->createIndexBuffer(wFrameIndex,30);
		pButtonVertex2->createVertexBuffer(20);

		pPolygon = addPolygon();
		pBarVertex = pPolygon->getAflMeshBuffer();
		pBarVertex->createIndexBuffer(wFrameIndex,30);
		pBarVertex->createVertexBuffer(20);

		m_pButtonVertex1 = pButtonVertex1;
		m_pButtonVertex2 = pButtonVertex2;
		m_pBarVertex1 = pBarVertex;

		m_bCreate = true;
	}
	else
	{
		pButtonVertex1 = m_pButtonVertex1;
		pButtonVertex2 = m_pButtonVertex2;
		pBarVertex = m_pBarVertex1;
	}
	
	AFLVERTEX vertImage[20];
	crateFrameVertex(vertImage,
		m_rectScroll1.fLeft,m_rectScroll1.fTop,
		m_rectScroll1.getWidth(),m_rectScroll1.getHeight(),
		m_dwButtonColor1,m_dwButtonColor2);
	pButtonVertex1->copyVertexBuffer(vertImage,20);

	crateFrameVertex(vertImage,
		m_rectScroll2.fLeft,m_rectScroll2.fTop,
		m_rectScroll2.getWidth(),m_rectScroll2.getHeight(),
		m_dwButtonColor1,m_dwButtonColor2);
	pButtonVertex2->copyVertexBuffer(vertImage,20);

	crateFrameVertex(vertImage,
		m_rectScrollBar.fLeft,m_rectScrollBar.fTop,
		m_rectScrollBar.getWidth(),m_rectScrollBar.getHeight(),
		m_dwBarColor1,m_dwBarColor2);
	pBarVertex->copyVertexBuffer(vertImage,20);

	m_bRedraw = true;	
	return true;
}

//-----------------------------------------------
DWORD AflSpriteScroll::onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	INT iHit = hitTest(fX,fY);
	if(iHit == AFL_HIT_SCROLLBAR)
	{
		m_fClickPointBaseX = getPointX();
		m_fClickPointBaseY = getPointY();
		m_fClickPointX = fX;
		m_fClickPointY = fY;
		m_iBasePos = m_iScrollPos;
		m_iMode = AFL_SCROLLBAR;
		return 1;
	}
	if(iHit == AFL_HIT_SCROLL1)
	{
		setScrollPos(m_iScrollPos-m_dwScrollMin);
		return 1;
	}
	if(iHit == AFL_HIT_SCROLL2)
	{
		setScrollPos(m_iScrollPos+m_dwScrollMin);
		return 1;
	}	
	return iHit;
}
//-----------------------------------------------
DWORD AflSpriteScroll::onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	m_iMode = 0;
	return 0;
}
//-----------------------------------------------
DWORD AflSpriteScroll::onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	if(dwFlags == MK_LBUTTON)
	{
		if(m_iMode == AFL_SCROLLBAR)
		{
			FLOAT fMoveSize;
			if(m_dwBarStyle & SBS_VERT)
				fMoveSize = -m_fClickPointY+fY;
			else
				fMoveSize = -m_fClickPointX+fX;
			INT iPos = m_iBasePos + fMoveSize * m_dwScrollMax / m_fScrollClient;
			setScrollPos(iPos);
			return 1;
		}
	}
	return 0;
}
//-----------------------------------------------
INT AflSpriteScroll::hitTest(FLOAT fX,FLOAT fY)
{
	if(isHit(&m_rectScrollBar,fX,fY))
		return AFL_HIT_SCROLLBAR;
	if(isHit(&m_rectScroll1,fX,fY))
		return AFL_HIT_SCROLL1;
	if(isHit(&m_rectScroll2,fX,fY))
		return AFL_HIT_SCROLL2;
	return AflSpriteWnd::hitTest(fX,fY);

}
//-----------------------------------------------
void AflSpriteScroll::onDraw()
{
	if(m_bRedraw)
	{
		createWindow(m_rectWindow.getWidth(),m_rectWindow.getHeight());
		m_bRedraw = false;	
	}
	AflSpriteWnd::onDraw(fX,fY,fZ);
}
//-----------------------------------------------
void AflSpriteScroll::setScrollPos()
{
	setScrollPos(m_iScrollPos);
}
//-----------------------------------------------
void AflSpriteScroll::setScrollPos(INT iPos)
{
	DWORD dwScrollPage;
	if(m_dwScrollPage > m_dwScrollMax)
		dwScrollPage = m_dwScrollMax;
	else
		dwScrollPage = m_dwScrollPage;
	
	if(iPos < 0)
		iPos = 0;
	else if(iPos > (INT)m_dwScrollMax - (INT)dwScrollPage)
		iPos = m_dwScrollMax - dwScrollPage;
	m_iScrollPos = iPos;
	m_bRedraw = true;
}
//-----------------------------------------------
bool AflSpriteScroll::isLastPos() const
{
	DWORD dwScrollPage;
	if(m_dwScrollPage > m_dwScrollMax)
		dwScrollPage = m_dwScrollMax;
	else
		dwScrollPage = m_dwScrollPage;
	return m_iScrollPos == (INT)m_dwScrollMax - (INT)dwScrollPage;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteScroll
// スクロールバー表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflSpriteWindow::AflSpriteWindow()
{
	*this += m_scrollHBar;
	*this += m_scrollVBar;
}
//-----------------------------------------------
void AflSpriteWindow::onWindowSize(FLOAT fWidth,FLOAT fHeight)
{
	FLOAT fBarWidth = m_rectClient.getWidth()-m_rectSize.getWidth()+1;
	FLOAT fBarHeight = m_rectClient.getHeight()-m_rectSize.getHeight();

	m_scrollHBar.setScrollPage(fBarWidth);
	m_scrollHBar.createWindow(fBarWidth,m_fSBHeight-1,SBS_HORZ);
	m_scrollHBar.setPoint(m_rectWindow.fLeft+1,m_rectClient.fBottom-m_fSBHeight-2);
	m_scrollHBar.setView(false);
	m_scrollHBar.setZBuffer(false);
	m_scrollHBar.setPointW(-1);
	m_scrollHBar.setScrollPos();

	m_scrollVBar.setScrollPage(fBarHeight);
	m_scrollVBar.createWindow(m_fSBWidth,fBarHeight);
	m_scrollVBar.setPoint(m_rectWindow.fRight-m_fSBWidth-2,m_rectClient.fTop);
	m_scrollVBar.setView(false);
	m_scrollVBar.setZBuffer(false);
	m_scrollVBar.setPointW(-1);
	m_scrollVBar.setScrollPos();

	AflSpriteWnd::onWindowSize(fWidth,fHeight);
}
//-----------------------------------------------
DWORD AflSpriteWindow::onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	INT iPos;
	
	iPos = m_scrollVBar.getScrollPos();
	if(m_scrollVBar.onLButtonDown(dwFlags,fX,fY))
	{
		if(iPos != m_scrollVBar.getScrollPos())
			onScrollPos(SBS_VERT,m_scrollVBar.getScrollPos());
		return 1;
	}

	iPos = m_scrollHBar.getScrollPos();
	if(m_scrollHBar.onLButtonDown(dwFlags,fX,fY))
	{
		if(iPos != m_scrollHBar.getScrollPos())
			onScrollPos(SBS_HORZ,m_scrollHBar.getScrollPos());
		return 1;
	}

	return AflSpriteWnd::onLButtonDown(dwFlags,fX,fY);
}
//-----------------------------------------------
DWORD AflSpriteWindow::onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	if(m_scrollVBar.onLButtonUp(dwFlags,fX,fY))
		return 1;
	if(m_scrollHBar.onLButtonUp(dwFlags,fX,fY))
		return 1;
	return AflSpriteWnd::onLButtonUp(dwFlags,fX,fY);
}
//-----------------------------------------------
void AflSpriteWindow::setScrollPos(INT iBar,DWORD dwPos)
{
	if(iBar == SBS_HORZ)
	{
		m_scrollHBar.setScrollPos(dwPos);
		onScrollPos(iBar,m_scrollHBar.getScrollPos());
	}
	else
	{
		m_scrollVBar.setScrollPos(dwPos);
		onScrollPos(iBar,m_scrollVBar.getScrollPos());
	}
}
//-----------------------------------------------
DWORD AflSpriteWindow::onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	INT iPos;
	iPos = m_scrollVBar.getScrollPos();
	if(m_scrollVBar.onMouseMove(dwFlags,fX,fY))
	{
		if(iPos != m_scrollVBar.getScrollPos())
			onScrollPos(SBS_VERT,m_scrollVBar.getScrollPos());
		return 1;
	}
	iPos = m_scrollHBar.getScrollPos();
	if(m_scrollHBar.onMouseMove(dwFlags,fX,fY))
	{
		if(iPos != m_scrollHBar.getScrollPos())
			onScrollPos(SBS_HORZ,m_scrollHBar.getScrollPos());
		return 1;
	}
	return AflSpriteWnd::onMouseMove(dwFlags,fX,fY);
}
//-----------------------------------------------
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflSpriteStrings
// 文字列表示用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflSpriteStrings::AflSpriteStrings()
{
	m_pointStart.x = 0;
	m_pointStart.y = 0;
	m_sizeLimit.cx = 200;
	m_sizeLimit.cy = 200;

	m_font.createFont();

	AflStringParam paramString;
	paramString.pSprite = NULL;
	paramString.bFlag = false;
	m_listString.push_back(paramString);
}
//-----------------------------------------------
void AflSpriteStrings::setStartPoint(INT iX,INT iY)
{
	m_pointStart.x = iX;
	m_pointStart.y = iY;
	setRedraw();
}
//-----------------------------------------------
bool AflSpriteStrings::open(LPCSTR pFileName)
{
	CHAR cBuff[1024];
	AflFile file;
	if(!file.open(pFileName,AflFile::file_ascii|AflFile::file_in))
		return false;


	while(!file.isEof())
	{
		AflStringParam paramString;
		paramString.pSprite = 0;
		file.gets(cBuff,1024);
		addString(cBuff);
	}
	return true;
}
void AflSpriteStrings::addString(LPCSTR pString)
{
	AflStringParam paramString;
	INT i;
	for(i=0;pString[i];i++)
	{
		if(pString[i] == '\n')
		{
			m_listString.back().strLine.append(pString,i);
			if(m_listString.back().pSprite)
			{
				delete m_listString.back().pSprite;
				m_listString.back().pSprite = NULL;
			}
			paramString.pSprite = NULL;
			paramString.bFlag = false;
			m_listString.push_back(paramString);
			pString += i;
			i=0;
		}
	}
	if(i)
	{
		m_listString.back().strLine.append(pString,i-1);
		if(m_listString.back().pSprite)
		{
			delete m_listString.back().pSprite;
			m_listString.back().pSprite = NULL;
		}
	}
}

//-----------------------------------------------
void AflSpriteStrings::setLimitWidth(INT iWidth)
{
	SIZE size;
	std::list<AflStringParam>::iterator it;
	for(it=m_listString.begin();it!=m_listString.end();it++)
	{
		m_font.getFontSize(&size,(*it).strLine.c_str(),(*it).strLine.length(),iWidth);
		(*it).sizePoint = size;
	}

}
//-----------------------------------------------
void AflSpriteStrings::getStringsSize(LPSIZE pSize)
{
	INT iWidth = 0;
	INT iHeight = 0;
	std::list<AflStringParam>::iterator it;
	for(it=m_listString.begin();it!=m_listString.end();it++)
	{
		if(iWidth < (*it).sizePoint.cx)
			iWidth = (*it).sizePoint.cx;
		iHeight += (*it).sizePoint.cy;
	}
	pSize->cx = iWidth;
	pSize->cy = iHeight;
}
//-----------------------------------------------
void AflSpriteStrings::setClipper()
{
	FLOAT fPointX = getAbsPointX();
	FLOAT fPointY = getAbsPointY();
	std::list<AflStringParam>::iterator it;
	for(it=m_listString.begin();it!=m_listString.end();it++)
	{
		AflSprite* pSprite = (*it).pSprite;
		if(pSprite)
		{
			pSprite->setClipper(fPointX,fPointY,0,
				m_sizeLimit.cx,m_sizeLimit.cy,0);
		}
	}
}
//-----------------------------------------------
void AflSpriteStrings::onDraw()
{
	if(m_bRedraw)
	{
		std::list<AflStringParam>::iterator it;
		for(it=m_listString.begin();it!=m_listString.end();it++)
		{
			(*it).bFlag = false;
		}
		INT iPointY;
		for(iPointY=0,it=m_listString.begin();it!=m_listString.end();it++)
		{
			iPointY += (*it).sizePoint.cy;
			if(iPointY >= m_pointStart.y)
				break;
		}
		iPointY = iPointY - m_pointStart.y - (*it).sizePoint.cy;
		FLOAT fPointX = getAbsPointX();
		FLOAT fPointY = getAbsPointY();
		for(;it!=m_listString.end();it++)
		{
			(*it).bFlag = true;
			AflSprite* pSprite = (*it).pSprite;
			if(!pSprite)
			{
				pSprite = new AflSprite;
				pSprite->createText((*it).strLine.c_str(),m_font,RGB(255,255,255),0);
				(*it).pSprite = pSprite;
				*this += *pSprite;
				pSprite->setPointW(-1);
				pSprite->setView(false);
				pSprite->setZBuffer(false);
			}
			pSprite->setPoint(-m_pointStart.x,iPointY);
			pSprite->setClipper(fPointX,fPointY,0,
				m_sizeLimit.cx,m_sizeLimit.cy,0);
			if(iPointY > m_sizeLimit.cy)
				break;
			iPointY += (*it).sizePoint.cy;
		}

		//不要なスプライトの削除
		for(it=m_listString.begin();it!=m_listString.end();it++)
		{
			if(!(*it).bFlag)
			{
				AflSprite* pSprite = (*it).pSprite;
				if(pSprite)
				{
					delete pSprite;
					(*it).pSprite = NULL;
				}
			}
		}
		m_bRedraw = false;
	}
}
//-----------------------------------------------
void AflSpriteStrings::setLimitSize(INT iWidth,INT iHeight)
{
	m_sizeLimit.cx = iWidth;
	m_sizeLimit.cy = iHeight;
}
//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflStringsWindow
// 文字列表示用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
void AflStringsWindow::onScrollPos(INT iBar,DWORD dwPos)
{
	m_spriteStrings.setStartPoint(getScrollPosX(),getScrollPosY());
}
//-----------------------------------------------
void AflStringsWindow::onWindowMove(FLOAT fWidth,FLOAT fHeight)
{
	AflFRect rect;
	getClientRect(&rect);
	m_spriteStrings.setPoint(rect.fLeft,rect.fTop);
	m_spriteStrings.setClipper();
	AflSpriteWindow::onWindowMove(fWidth,fHeight);
}
//-----------------------------------------------
void AflStringsWindow::onWindowSize(FLOAT fWidth,FLOAT fHeight)
{
	AflFRect rect;
	getClientRect(&rect);
	m_spriteStrings.setLimitSize(rect.getWidth(),rect.getHeight());
	m_spriteStrings.setPoint(rect.fLeft,rect.fTop);
	m_spriteStrings.setRedraw();
	AflSpriteWindow::onWindowSize(fWidth,fHeight);
}
//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflStringsWindow
// 文字列表示用ウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AflStringsWindow::AflStringsWindow()
{
	*this += m_spriteStrings;
}
//-----------------------------------------------
bool AflStringsWindow::open(LPCSTR pFileName)
{
	if(!m_spriteStrings.open(pFileName))
		return false;
	m_spriteStrings.setLimitWidth();

	SIZE size;
	m_spriteStrings.getStringsSize(&size);
	setScrollBars(size.cx,size.cy);

	AflFRect rect;
	getClientRect(&rect);
	m_spriteStrings.setLimitSize(rect.getWidth(),rect.getHeight());
	m_spriteStrings.setPoint(rect.fLeft,rect.fTop);
	m_spriteStrings.setRedraw();
	return true;
}
//-----------------------------------------------
INT AflStringsWindow::printf(LPCSTR pString,...)
{
	bool bFlag = false;
	if(m_scrollVBar.isLastPos())
		bFlag = true;

	
	CHAR cBuff[2048];

	va_list vaList;
	va_start( vaList, pString );
	
	INT iRet = vsprintf(cBuff,pString,vaList);
	va_end(vaList);

	m_spriteStrings.addString(cBuff);

	m_spriteStrings.setLimitWidth();

	SIZE size;
	m_spriteStrings.getStringsSize(&size);
	setScrollBars(size.cx,size.cy);		
	
	AflFRect rect;
	getClientRect(&rect);
	m_spriteStrings.setLimitSize(rect.getWidth(),rect.getHeight());
	m_spriteStrings.setPoint(rect.fLeft,rect.fTop);
	m_spriteStrings.setRedraw();

	if(bFlag)
	{
		setScrollPos(SBS_VERT,100000);
	}
	return iRet;
}
//-----------------------------------------------

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflWindowList
// アクティブウインドウ設定用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflWindowList::AflWindowList()
{
	m_fBaseW = -500;
}
//-----------------------------------------------
void AflWindowList::addWindow(AflSpriteWnd* pWindow)
{
	m_listWindow.remove(pWindow);
	m_listWindow.push_back(pWindow);
	sortWindow();
}
//-----------------------------------------------
void AflWindowList::activeWindow(AflSpriteWnd* pWindow)
{
	m_listWindow.remove(pWindow);
	m_listWindow.push_front(pWindow);
	sortWindow();
}
//-----------------------------------------------
void AflWindowList::sortWindow()
{
	FLOAT fW = m_fBaseW;
	std::list<AflSpriteWnd*>::iterator it;
	for(it=m_listWindow.begin();it!=m_listWindow.end();it++)
	{
		(*it)->setPointW(fW);
		fW+=10;
	}
	
}
//-----------------------------------------------
DWORD AflWindowList::onLButtonDown(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	std::list<AflSpriteWnd*>::iterator it;
	for(it=m_listWindow.begin();it!=m_listWindow.end();it++)
	{
		DWORD dwRet = (*it)->onLButtonDown(dwFlags,fX,fY);
		if(dwRet)
		{
			if(it!=m_listWindow.begin())
				activeWindow(*it);
			return dwRet;
		}
	}
	return 0;
}
//-----------------------------------------------
DWORD AflWindowList::onLButtonUp(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	std::list<AflSpriteWnd*>::iterator it;
	for(it=m_listWindow.begin();it!=m_listWindow.end();it++)
	{
		DWORD dwRet = (*it)->onLButtonUp(dwFlags,fX,fY);
		if(dwRet)
			return dwRet;
	}
	return 0;
}
//-----------------------------------------------
DWORD AflWindowList::onMouseMove(DWORD dwFlags,FLOAT fX,FLOAT fY)
{
	std::list<AflSpriteWnd*>::iterator it;
	for(it=m_listWindow.begin();it!=m_listWindow.end();it++)
	{
		DWORD dwRet = (*it)->onMouseMove(dwFlags,fX,fY);
		if(dwRet)
			return dwRet;
	}
	return 0;
}
//-----------------------------------------------
