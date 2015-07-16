#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLMAP

#pragma warning( disable : 4786 )	//STLの警告外し

#include "aflWinTool.h"
#include "aflGraphics.h"

#include <stdio.h>
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflMapObject
// マップオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class AflMapObject
{
public:
	AflMapObject()
	{
		m_iX = 0;
		m_iY = 0;
		m_iZ = 0;
		m_iCX1 = m_iCY1 = m_iCZ1 = 0;
		m_iCX2 = m_iCY2 = m_iCZ2 = 0;
		m_fRotationX = m_fRotationY = m_fRotationZ = 0;
		m_pvoidAddr = NULL;
	}
	bool operator==(const AflMapObject& mapObject)const{return m_pvoidAddr == mapObject.m_pvoidAddr;}

	void setPointX(INT iX){m_iX = iX;}
	void setPointY(INT iY){m_iY = iY;}
	void setPointZ(INT iZ){m_iZ = iZ;}
	void setCollideX1(INT iX){m_iCX1 = iX;}
	void setCollideY1(INT iY){m_iCY1 = iY;}
	void setCollideZ1(INT iZ){m_iCZ1 = iZ;}
	void setCollideX2(INT iX){m_iCX2 = iX;}
	void setCollideY2(INT iY){m_iCY2 = iY;}
	void setCollideZ2(INT iZ){m_iCZ2 = iZ;}
	void setRotationX(FLOAT fRX){m_fRotationX = fRX;}
	void setRotationY(FLOAT fRY){m_fRotationY = fRY;}
	void setRotationZ(FLOAT fRZ){m_fRotationZ = fRZ;}

	void setName(LPCSTR pString){m_strName = pString;}
	void setImage(LPCSTR pImage){m_strImage = pImage;}

	void setAddr(LPVOID pVoid){m_pvoidAddr = pVoid;}
	LPVOID getAddr()const{return m_pvoidAddr;}

	LPCSTR getName()const{return m_strName.c_str();}
	LPCSTR getImage()const{return m_strImage.c_str();}

	INT getPointX()const{return m_iX;}
	INT getPointY()const{return m_iY;}
	INT getPointZ()const{return m_iZ;}
	INT getCollideX1()const{return m_iCX1;}
	INT getCollideY1()const{return m_iCY1;}
	INT getCollideZ1()const{return m_iCZ1;}
	INT getCollideX2()const{return m_iCX2;}
	INT getCollideY2()const{return m_iCY2;}
	INT getCollideZ2()const{return m_iCZ2;}
	FLOAT getRotationX()const{return m_fRotationX;}
	FLOAT getRotationY()const{return m_fRotationY;}
	FLOAT getRotationZ()const{return m_fRotationZ;}

	void getCollideRect2(AflRect* pRect)const
	{
		pRect->setRect(m_iCX1,m_iCY1,m_iCZ1,m_iCX2,m_iCY2,m_iCZ2);
		pRect->offsetRect(m_iX,m_iY,m_iZ);
	}	
protected:
	std::string m_strName;
	std::string m_strImage;
	INT m_iX,m_iY,m_iZ;
	INT m_iCX1,m_iCY1,m_iCZ1;
	INT m_iCX2,m_iCY2,m_iCZ2;
	INT m_fRotationX;
	INT m_fRotationY;
	INT m_fRotationZ;
	LPVOID m_pvoidAddr;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflMapObjectList
// マップオブジェクトリスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class AflMapObjectList
{

public:
	bool save(LPCSTR pFileName);
	bool open(LPCSTR pFileName);
	void addObject(class AflXml* pXml);
	std::list<AflMapObject>* getObjectList(){return &m_listSprite;}
protected:
	std::list<AflMapObject> m_listSprite;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflSpriteMapObject
// マップオブジェクトスプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class AflSpriteMapObject : public AflSprite
{
public:
	~AflSpriteMapObject();
	AflSprite* openImage(LPCSTR pFileName);
	void setDataToSprite(AflSprite* pSprite,AflMapObject* pObject);
	bool save(LPCSTR pFileName);
	bool open(LPCSTR pFileName);
	bool isCollide(AflSprite* pSprite);
	bool delObject(AflMapObject* pObject);
	bool addObject(AflMapObject* pObject);
	std::list<AflMapObject>* getObjectList(){return &m_listObject;}

protected:
	std::list<AflMapObject> m_listObject;
	std::map<std::string,AflSprite*> m_mapImage;
	FLOAT m_fClipWidth;
	FLOAT m_fClipHeight;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflMapData
// 2Dマップデータ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <fstream>
class AflMapData
{
	enum
	{
		MAP_BINHEADER,MAP_TXTHEADER,
		MAP_VERSION,MAP_MAPNAME,MAP_BITMAP,MAP_SPRITE,
		MAP_WIDTH,MAP_HEIGHT,MAP_OPATTERN,MAP_PARTSWIDTH,MAP_PARTSHEIGHT,
		MAP_MAPDATA0,MAP_MAPDATA1,MAP_MAPDATA2,MAP_MAPDATA3,MAP_MAPDATA4,
		MAP_PAERSCOLLIDE0,MAP_PAERSCOLLIDE1,MAP_PAERSCOLLIDE2,
        MAP_PAERSCOLLIDE3,MAP_PAERSCOLLIDE4,
        MAP_MAPFLAG0,
		MAP_OBJECT
	};

public:
	AflMapData();
	bool saveMap(LPCSTR pFileName);
	bool openMap(LPCSTR pcstrFileName);
	bool createTipFlag(INT iWidth,INT iHeight);
	bool createMap(INT iWidth,INT iHeight);
	INT setMapIndex(INT iLevel,INT iX,INT iY,INT iIndex);
	INT getMapIndex(INT iLevel,INT iX,INT iY)const;
	bool clearMap(INT iIndex);
	bool copyMap(INT iX1,INT iY1,INT iWidth,INT iHeight,AflMapData* pMapData,INT iX2,INT iY2);
	BYTE getMapFlag(INT iX,INT iY);

	LPCSTR getImageName(INT iIndex)const{return m_strBmpName[iIndex].c_str();}
	void setImageName(INT iIndex,LPCSTR pString){m_strBmpName[iIndex] = pString;}
	INT getTipWidth()const{return m_iTipWidth;}
	INT getTipHeight()const{return m_iTipHeight;}
	INT getMapWidth()const{return m_iMapWidth;}
	INT getMapHeight()const{return m_iMapHeight;}
	bool setMapSize(INT iWidth,INT iHeight);
	void setTipSize(INT iWidth,INT iHeight){m_iTipWidth = iWidth;m_iTipHeight = iHeight;}

	LPCSTR getMapFileName()const{return m_strFileName.c_str();}
	LPCSTR getMapName()const{return m_strMapName.c_str();}
	LPCSTR getObjectFileName()const{return m_strObjectFileName.c_str();}

	void setMapFileName(LPCSTR pString){m_strFileName = pString;}
	void setMapName(LPCSTR pString){m_strMapName = pString;}
	void setOutTipIndex(INT iIndex){m_iOutTipIndex = iIndex;}
	void setObjectFileName(LPCSTR pString){m_strObjectFileName = pString;}
	INT getOutTipIndex()const{return m_iOutTipIndex;}
	INT getVersion()const{return 100;}

	LPBYTE getMapFlag(){return m_ptrMapFlag.get();}
	void setMapFlag(INT iX,INT iY,BYTE byFlag);
	void setMapCollide(INT iX,INT iY,bool bFlag);
	bool isMapCollide(INT iX,INT iY);
	bool isMapCollide(AflSprite* pSprite);
	void setTipCollide(INT iLevel,INT iIndex,bool bFlag);
	bool isTipCollide(INT iLevel,INT iIndex)const;

    AflMapObjectList* getBaseObject(){return &m_listBaseObject;}

	bool setRelativePath();
protected:
	bool isZero(int iLevel);
	void writeHeader(FILE* pFile,LPCSTR pHeader,LPCVOID pData,int nSize);
	void writeHeader(FILE* pFile,LPCSTR pHeader,LPCSTR pData);
	void writeHeader(FILE* pFile,LPCSTR pHeader,LPCWSTR pData);
	void writeHeader(FILE* pFile,LPCSTR pHeader,int nData);

	
	int getHeaderIndex(LPCSTR pcstrHeader);
	BOOL readHeader(std::fstream& file,LPSTR pHeader,LPVOID* ppData,LPDWORD pdwSize);

	std::string m_strFileName;
	std::string m_strMapName;
	std::string m_strBmpName[4];
	std::string m_strObjectFileName;
	INT m_iOutTipIndex;
	int m_nCollide;

	INT m_iTipWidth;
	INT m_iTipHeight;

	BOOL m_bDarFile;
	BYTE m_byCollide[5][0xffff/8];

    AflMapObjectList m_listBaseObject;

	std::auto_ptr<BYTE> m_ptrMapFlag;
	std::auto_ptr<SHORT> m_ptrData[5];
	INT m_iMapWidth;
	INT m_iMapHeight;
	static LPCSTR AflMapData::m_strFileHeder;
	static LPCSTR AflMapData::m_strMapHeader[]; 
};

//------------------------------------------------------------
// AflSpriteMap
// マップ表示用
//------------------------------------------------------------
typedef class AflSpriteMap AflSpriteMap,*PAflSpriteMap,*LPAflSpriteMap;
typedef class AflSpriteMapImage AflSpriteMapIMAGE,*PAflSpriteMapIMAGE,*LPAflSpriteMapIMAGE;
class AflSpriteMapImage : public AflSprite
{
public:
	AflSpriteMapImage();
	void setMap(LPAflSpriteMap pAflSpriteMap,INT iLevel);
	virtual void onDraw(D3DDevice* device,FLOAT fX,FLOAT fY,FLOAT fZ);
protected:
	INT m_iLevel;
	LPAflSpriteMap m_pAflSpriteMap;
};

class AflSpriteMap : public AflSprite
{
public:
	AflSpriteMap();
	void setMap(FLOAT fX,FLOAT fY);
	void moveMap(FLOAT fX,FLOAT fY);
	bool openMap(LPCSTR pcstrFileName);
	bool saveMap(LPCSTR pcstrFileName);
    bool openImage(INT iIndex=-1);
	bool createMap(INT iWidth,INT iHeight){return m_aflMapData.createMap(iWidth,iHeight);}

	void addList(AflMeshList* pSprite){m_AflSpriteList.addList(pSprite);}
	void delList(AflMeshList* pSprite){m_AflSpriteList.delList(pSprite);}


	bool isMapCollide(INT iX,INT iY){return m_aflMapData.isMapCollide(iX,iY);}
	void setTipCollide(INT iLevel,INT iIndex,bool bFlag){m_aflMapData.setTipCollide(iLevel,iIndex,bFlag);}
	bool isTipCollide(INT iLevel,INT iIndex)const{return m_aflMapData.isTipCollide(iLevel,iIndex);}
	BYTE getMapFlag(INT iX,INT iY){return m_aflMapData.getMapFlag(iX,iY);};
	void setMapCollide(INT iX,INT iY,bool bFlag){m_aflMapData.setMapCollide(iX,iY,bFlag);}
	LPBYTE getMapFlag(){return m_aflMapData.getMapFlag();}

    bool setTipSize(INT iWidth,INT iHeight);
	INT getTipWidth()const{return m_aflMapData.getTipWidth();}
	INT getTipHeight()const{return m_aflMapData.getTipHeight();}
	FLOAT getMapX()const{return m_fMapX;}
	FLOAT getMapY()const{return m_fMapY;}
	INT getMapIndex(INT iLevel,INT iX,INT iY)const{return m_aflMapData.getMapIndex(iLevel,iX,iY);}
	INT getMapWidth()const{return m_aflMapData.getMapWidth();}
	INT getMapHeight()const{return m_aflMapData.getMapHeight();}
	AflMapData* getMapData(){return &m_aflMapData;}
    LPAflSpriteMapIMAGE getSprite(INT iIndex){return m_AflSpriteImage + iIndex;}

	LPCSTR getMapFileName()const{return m_aflMapData.getMapFileName();}
	LPCSTR getMapName()const{return m_aflMapData.getMapName();}
	void setMapFileName(LPCSTR pString){m_aflMapData.setMapFileName(pString);}
	void setMapName(LPCSTR pString){m_aflMapData.setMapName(pString);}
	INT getOutTipIndex()const{return m_aflMapData.getOutTipIndex();}
	void setOutTipIndex(INT iIndex){m_aflMapData.setOutTipIndex(iIndex);}
	INT getVersion()const{return m_aflMapData.getVersion();}

	AflDrawUnit* getDrawSprite(){return &m_AflSpriteDraw;}
	
	bool isDrawCollide()const{return m_bDrawCollide;}
	void setDrawCollide(bool bFlag){m_bDrawCollide = bFlag;}
	bool isMapCollide(AflSprite* pSprite);

	bool addObject(AflMapObject* pObject);
	bool delObject(AflMapObject* pObject){return m_aflMapObject.delObject(pObject);}
	AflSprite* openObjectImage(LPCSTR pFileName)
	{
		return m_aflMapObject.openImage(pFileName);
	}
	AflSpriteMapObject* getMapObject(){return &m_aflMapObject;}
	std::list<AflMapObject>* getObjectList(){return m_aflMapObject.getObjectList();}
	void setClipper(FLOAT fX,FLOAT fY,FLOAT fZ,FLOAT fWidth,FLOAT fHeight,FLOAT fDepth);

protected:
	AflMapData m_aflMapData;
	AflSpriteMapIMAGE m_AflSpriteImage[5];
	AflSpriteMapIMAGE m_AflSpriteList;
	AflSpriteMapObject m_aflMapObject;

	AflDrawUnit m_AflSpriteDraw;
	bool m_bDrawCollide;

	FLOAT m_fMapX;
	FLOAT m_fMapY;
    LPDWORD m_pdwFlag;
};

//------------------------------------------------------------
// AflMapRoute
// マップ経路探索用クラス
//------------------------------------------------------------
class AflMapRoute
{
public:
	AflMapRoute()
	{
		m_iCountMax = 100;
		m_piCollide = NULL;
		m_piCollideMaster = NULL;
	}
	~AflMapRoute()
	{
		if(m_piCollide)
			delete m_piCollide;
		if(m_piCollideMaster)
			delete m_piCollideMaster;
	}
	
	bool createRouteBuffer(AflSpriteMap* pMap);

	LPPOINT getRoute(INT iX1,INT iY1,INT iX2,INT iY2);
	INT getRouteCount()const{return m_iRouteCount;}
	LPPOINT getRouteData(INT iX,INT iY);
	void setCollideData(INT iX1,INT iY1,INT iFlag=0);
	INT getCollideCount(INT iX,INT iY)const;
	void setCollideCount(INT iX,INT iY,INT iCount)const;

	void setMaxRoute(INT iMax){m_iCountMax = iMax;}
	INT getMaxRoute()const{return m_iCountMax;}
protected:
	INT m_iCountMax;
	INT m_iRouteCount;
	PINT m_piCollide;
	PINT m_piCollideMaster;
	INT m_iTargetCount;
	INT m_iTargetX;
	INT m_iTargetY;
	INT m_iWidth;
	INT m_iHeight;
};


#define __INC_AFLMAP
#endif
