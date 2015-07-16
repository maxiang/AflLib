#include <windows.h>

#include "aflMap.h" 
#include "aflXml.h" 

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

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflMapObjectList
// マップオブジェクトリスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool AflMapObjectList::save(LPCSTR pFileName)
{
	AflXml xml;
	xml.setName("AFL_DATA");
	AflXml* pXml = xml.addDown("OBJECT_SET");
	
	std::list<AflMapObject>::iterator it;
	for(it = m_listSprite.begin();it != m_listSprite.end();it++)
	{
		AflXml* pXmlData = pXml->addDown("OBJECT");
		pXmlData->addOption("NAME",(*it).getName());
		pXmlData->addOption("SRC",(*it).getImage());
		pXmlData->addOption("CX1",(*it).getCollideX1());
		pXmlData->addOption("CY1",(*it).getCollideY1());
		pXmlData->addOption("CZ1",(*it).getCollideZ1());
		pXmlData->addOption("CX2",(*it).getCollideX2());
		pXmlData->addOption("CY2",(*it).getCollideY2());
		pXmlData->addOption("CZ2",(*it).getCollideZ2());
		pXmlData->addOption("RX",(*it).getRotationX());
		pXmlData->addOption("RY",(*it).getRotationY());
		pXmlData->addOption("RZ",(*it).getRotationZ());
	}
	return xml.save(pFileName);
}
bool AflMapObjectList::open(LPCSTR pFileName)
{
	AflXml xml;
	if(!xml.open(pFileName))
		return false;
	if(strcmp(xml.getName(),"AFL_DATA") != 0)
		return false;
	AflXml* pXml;
	for(pXml = xml.getDown();pXml;pXml=xml.getNext())
	{
		if(strcmp(pXml->getName(),"OBJECT_SET") == 0)
		{
			AflXml* pXmlData;
			for(pXmlData = pXml->getDown();pXmlData;pXmlData=pXmlData->getNext())
			{
				if(strcmp(pXmlData->getName(),"OBJECT") == 0)
					addObject(pXmlData);
			}

		}
	}
	return true;
}
void AflMapObjectList::addObject(AflXml* pXml)
{
	AflMapObject mapObject;

	LPCSTR pImageName = pXml->getOption("NAME");
	LPCSTR pImageSrc = pXml->getOption("SRC");
	LPCSTR pImageCX1 = pXml->getOption("CX1");
	LPCSTR pImageCY1 = pXml->getOption("CY1");
	LPCSTR pImageCZ1 = pXml->getOption("CZ1");
	LPCSTR pImageCX2 = pXml->getOption("CX2");
	LPCSTR pImageCY2 = pXml->getOption("CY2");
	LPCSTR pImageCZ2 = pXml->getOption("CZ2");
	LPCSTR pImageRX = pXml->getOption("RX");
	LPCSTR pImageRY = pXml->getOption("RY");
	LPCSTR pImageRZ = pXml->getOption("RZ");

	if(pImageName) mapObject.setName(pImageName);
	if(pImageSrc) mapObject.setImage(pImageSrc);
	if(pImageCX1) mapObject.setCollideX1(atoi(pImageCX1));
	if(pImageCY1) mapObject.setCollideY1(atoi(pImageCY1));
	if(pImageCZ1) mapObject.setCollideZ1(atoi(pImageCZ1));
	if(pImageCX2) mapObject.setCollideX2(atoi(pImageCX2));
	if(pImageCY2) mapObject.setCollideY2(atoi(pImageCY2));
	if(pImageCZ2) mapObject.setCollideZ2(atoi(pImageCZ2));
	if(pImageRX) mapObject.setRotationX(atoi(pImageRX));
	if(pImageRY) mapObject.setRotationY(atoi(pImageRY));
	if(pImageRZ) mapObject.setRotationZ(atoi(pImageRZ));

	
	m_listSprite.push_back(mapObject);

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflSpriteMapObject
// マップオブジェクトスプライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
AflSpriteMapObject::~AflSpriteMapObject()
{
	std::list<AflMapObject>::iterator it = m_listObject.begin();
	for(;it != m_listObject.end();it++)
	{
		AflSprite* pSprite = (AflSprite*)(*it).getAddr();
		delete pSprite;
	}

	std::map<std::string,AflSprite*>::iterator itMap;
	for(itMap = m_mapImage.begin();itMap != m_mapImage.end();itMap++)
		delete (*itMap).second;

}
AflSprite* AflSpriteMapObject::openImage(LPCSTR pFileName)
{
	AflSprite* pImage = m_mapImage[pFileName];
	if(!pImage)
	{
		pImage = new AflSprite;
		pImage->setPointStat(POINT_BOTTOM);
		if(pImage->openSpriteImage(pFileName))
		{
			m_mapImage[pFileName] = pImage;
			return pImage;
		}
		delete pImage;
		return NULL;
	}
	return pImage;
}
bool AflSpriteMapObject::save(LPCSTR pFileName)
{
	AflXml xml;
	xml.setName("COLLIDE");


	std::list<AflMapObject>::iterator it = m_listObject.begin();
	for(;it != m_listObject.end();it++)
	{
		AflXml* pXml = xml.addDown(NULL,true);
		
		pXml->setName("OBJECT");
		pXml->addOption("NAME",(*it).getName());
		pXml->addOption("SRC",(*it).getImage());
		pXml->addOption("X",(*it).getPointX());
		pXml->addOption("Y",(*it).getPointY());
		pXml->addOption("Z",(*it).getPointZ());
		pXml->addOption("CX1",(*it).getCollideX1());
		pXml->addOption("CY1",(*it).getCollideY1());
		pXml->addOption("CZ1",(*it).getCollideZ1());
		pXml->addOption("CX2",(*it).getCollideX2());
		pXml->addOption("CY2",(*it).getCollideY2());
		pXml->addOption("CZ2",(*it).getCollideZ2());
		pXml->addOption("RX",(*it).getRotationX());
		pXml->addOption("RY",(*it).getRotationY());
		pXml->addOption("RZ",(*it).getRotationZ());
	}

	return xml.save(pFileName);

}
bool AflSpriteMapObject::open(LPCSTR pFileName)
{
	AflXml xml;
	if(!xml.open(pFileName))
		return false;
	AflXml* pXml;
	for(pXml = xml.getDown();pXml;pXml = pXml->getNext())
	{
		LPCSTR pName = pXml->getOption("NAME");
		LPCSTR pImageSrc = pXml->getOption("SRC");
		LPCSTR pImageX = pXml->getOption("X");
		LPCSTR pImageY = pXml->getOption("Y");
		LPCSTR pImageZ = pXml->getOption("Z");
		LPCSTR pImageCX1 = pXml->getOption("CX1");
		LPCSTR pImageCY1 = pXml->getOption("CY1");
		LPCSTR pImageCZ1 = pXml->getOption("CZ1");
		LPCSTR pImageCX2 = pXml->getOption("CX2");
		LPCSTR pImageCY2 = pXml->getOption("CY2");
		LPCSTR pImageCZ2 = pXml->getOption("CZ2");
		LPCSTR pImageRX = pXml->getOption("RX");
		LPCSTR pImageRY = pXml->getOption("RY");
		LPCSTR pImageRZ = pXml->getOption("RZ");

		if(!pImageSrc)
			return false;

		AflMapObject mapObject;
		if(pName) mapObject.setName(pName);
		if(pImageSrc) mapObject.setImage(pImageSrc);
		if(pImageX)	mapObject.setPointX(atoi(pImageX));
		if(pImageY)	mapObject.setPointY(atoi(pImageY));
		if(pImageZ)	mapObject.setPointZ(atoi(pImageZ));
		if(pImageCX1) mapObject.setCollideX1(atoi(pImageCX1));
		if(pImageCY1) mapObject.setCollideY1(atoi(pImageCY1));
		if(pImageCZ1) mapObject.setCollideZ1(atoi(pImageCZ1));
		if(pImageCX2) mapObject.setCollideX2(atoi(pImageCX2));
		if(pImageCY2) mapObject.setCollideY2(atoi(pImageCY2));
		if(pImageCZ2) mapObject.setCollideZ2(atoi(pImageCZ2));
		if(pImageRX) mapObject.setRotationX(atoi(pImageRX));
		if(pImageRY) mapObject.setRotationY(atoi(pImageRY));
		if(pImageRZ) mapObject.setRotationZ(atoi(pImageRZ));

		addObject(&mapObject);
		
	}
	return true;
}
bool AflSpriteMapObject::isCollide(AflSprite* pSprite)
{
	std::list<AflMapObject>::iterator it = m_listObject.begin();
	for(;it != m_listObject.end();it++)
	{
		AflSprite* pObject = (AflSprite*)(*it).getAddr();
		if(pObject->isCollide(pSprite))
			return true;
	}
	return false;
}
void AflSpriteMapObject::setDataToSprite(AflSprite* pSprite,AflMapObject* pObject)
{
	pSprite->setPoint(pObject->getPointX(),pObject->getPointY(),pObject->getPointZ());
	pSprite->setRotationX(pObject->getRotationX());
	pSprite->setRotationY(pObject->getRotationY());
	pSprite->setRotationZ(pObject->getRotationZ());
	pSprite->setCollideRect(&AflRect(
		pObject->getCollideX1(),pObject->getCollideY1(),pObject->getCollideZ1(),
		pObject->getCollideX2(),pObject->getCollideY2(),pObject->getCollideZ2()));
	
	pSprite->setClipper(getClipX(),getClipY(),getClipZ(),getClipWidth(),getClipHeight(),getClipDepth());
}
bool AflSpriteMapObject::addObject(AflMapObject* pObject)
{
	AflSprite* pImage = openImage(pObject->getImage());
	if(!pImage)
		return false;

	AflSprite* pSprite = new AflSprite;
	*pSprite = *pImage;
	pObject->setAddr(pSprite);

	//スプライトにデータを設定
	setDataToSprite((AflSprite*)pSprite,pObject);

	addList(pSprite);
	m_listObject.push_back(*pObject);

	return true;
}
bool AflSpriteMapObject::delObject(AflMapObject* pObject)
{
	//スプライトリストから削除
	AflSprite* pSprite = (AflSprite*)pObject->getAddr();
	delete pSprite;
	//オブジェクトリストから削除	
	m_listObject.remove(*pObject);
	return true;
}

//------------------------------------------------------------
// AflSpriteMap
// マップ表示用
//------------------------------------------------------------
AflSpriteMap::AflSpriteMap()
{
	int i;
	for(i=4;i>=0;i--)
	{
		m_AflSpriteImage[i].setMap(this,i);
		m_AflSpriteImage[i].setPoint(0,0,-i*1.0f,1.0f);
		AflSprite::addList(&m_AflSpriteImage[i]);
	}
	//マップオブジェクトの追加
	addList(&m_aflMapObject);
	
	AflSprite::addList(&m_AflSpriteList);
	m_fMapX = 0;
	m_fMapY = 0;
	m_bDrawCollide = false;
}
void AflSpriteMap::setClipper(FLOAT fX,FLOAT fY,FLOAT fZ,FLOAT fWidth,FLOAT fHeight,FLOAT fDepth)
{
	m_aflMapObject.setClipper(fX,fY,fZ,fWidth,fHeight,fDepth);
	AflSprite::setClipper(fX,fY,fZ,fWidth,fHeight,fDepth);
}

void AflSpriteMap::moveMap(FLOAT fX,FLOAT fY)
{
	m_fMapX += fX;
	m_fMapY += fY;
	m_AflSpriteList.setPoint(-m_fMapX,-m_fMapY,0);
}
void AflSpriteMap::setMap(FLOAT fX,FLOAT fY)
{
	m_fMapX = fX;
	m_fMapY = fY;
	m_AflSpriteList.setPoint(-m_fMapX,-m_fMapY,0);
}
bool AflSpriteMap::addObject(AflMapObject* pObject)
{
	return m_aflMapObject.addObject(pObject);
}


bool AflSpriteMap::openMap(LPCSTR pcstrFileName)
{
	if(!m_aflMapData.openMap(pcstrFileName))
		return false;
	
	AflFileName fileName(pcstrFileName);
	fileName.pushPath();
	fileName.changePath();
	m_aflMapObject.open(m_aflMapData.getObjectFileName());
	
	//スプライトの設定
    INT i;
    for(i=0;i<4;i++)
    {
        m_AflSpriteImage[i].setTipSize(m_aflMapData.getTipWidth(),m_aflMapData.getTipHeight());
	    m_AflSpriteImage[i].openSpriteImage(m_aflMapData.getImageName(i));
    }

	fileName.popPath();
	return true;
}

bool AflSpriteMap::saveMap(LPCSTR pcstrFileName)
{
	bool bFlag =   m_aflMapData.saveMap(pcstrFileName);
	if(*m_aflMapData.getObjectFileName())
		m_aflMapObject.save(m_aflMapData.getObjectFileName());
	return bFlag;
}
bool AflSpriteMap::openImage(INT iIndex)
{
	if(iIndex >= 0)
	{
	    return m_AflSpriteImage[iIndex].openSpriteImage(m_aflMapData.getImageName(iIndex));
	}
	//イメージ読み出し
    INT i;
    for(i=0;i<4;i++)
    {
	    m_AflSpriteImage[i].openSpriteImage(m_aflMapData.getImageName(i));
    }
    return true;
}
bool AflSpriteMap::setTipSize(INT iWidth,INT iHeight)
{
    INT i;
    m_aflMapData.setTipSize(iWidth,iHeight);
    for(i=0;i<4;i++)
    {
        m_AflSpriteImage[i].setTipSize(iWidth,iHeight);
    }
    return true;
}
bool AflSpriteMap::isMapCollide(AflSprite* pSprite)
{
	if(m_aflMapData.isMapCollide(pSprite) || m_aflMapObject.isCollide(pSprite))
		return true;
	return false;
}

//------------------------------------------------------------
// AflSpriteMapImage
// マップイメージ表示用
//------------------------------------------------------------
AflSpriteMapImage::AflSpriteMapImage()
{
	m_pAflSpriteMap = NULL;
	m_iLevel = 0;

}
void AflSpriteMapImage::setMap(LPAflSpriteMap pAflSpriteMap,INT iLevel)
{
	m_pAflSpriteMap = pAflSpriteMap;
	m_iLevel = iLevel;
}
void AflSpriteMapImage::onDraw()
{
	int i,j;

	if(!m_pAflSpriteMap || !getAflTexture())
		return;

	AflD3DWorld* paflDevice = AflRenderUnit::getAflDevice();
	
	AflDrawUnit* pSprite = m_pAflSpriteMap->getDrawSprite();
	INT iTipWidth = getTipWidth();
	INT iTipHeight = getTipHeight();
	if(!iTipWidth || !iTipHeight)
		return;
	
	//接触判定表示用スプライトの作成
	if((INT)pSprite->getImageWidth()-4 != getTipWidth() || (INT)pSprite->getImageHeight()-4 != getTipHeight())
	{
		pSprite->delPolygon();
		pSprite->createBox(2,2,getTipWidth()-4,getTipHeight()-4,0x88cc6666);
		pSprite->setZBuffer(false);
		pSprite->setClipper(m_pAflSpriteMap->getClipX(),m_pAflSpriteMap->getClipY(),m_pAflSpriteMap->getClipZ(),
			m_pAflSpriteMap->getClipWidth(),m_pAflSpriteMap->getClipHeight(),m_pAflSpriteMap->getClipDepth());
	}

	//クリップ領域の指定
	setClipper(m_pAflSpriteMap->getClipX(),m_pAflSpriteMap->getClipY(),m_pAflSpriteMap->getClipZ(),
		m_pAflSpriteMap->getClipWidth(),m_pAflSpriteMap->getClipHeight(),m_pAflSpriteMap->getClipDepth());


	INT iLevel = m_iLevel;
	FLOAT fMapX = m_pAflSpriteMap->getMapX();
	FLOAT fMapY = m_pAflSpriteMap->getMapY();

	//描画範囲の設定
	FLOAT fDrawWidth = m_pAflSpriteMap->getClipWidth();
	FLOAT fDrawHeight = m_pAflSpriteMap->getClipHeight();
	if(!fDrawWidth)
		fDrawWidth = paflDevice->getDeviceWidth();
	if(!fDrawHeight)
		fDrawHeight = paflDevice->getDeviceHeight();

	//ビットのスライド位置を設定
	FLOAT fStartBitX = (INT)fMapX % iTipWidth;
	FLOAT fStartBitY = (INT)fMapY % iTipHeight;
	if(fMapX < 0)
		fStartBitX = iTipWidth + fStartBitX;
	if(fMapY < 0)
		fStartBitY = iTipHeight + fStartBitY;

	//描画開始マップパーツの設定
	INT iMapX,iMapY;
	iMapX = (fMapX-fStartBitX) / iTipWidth;
	iMapY = (fMapY-fStartBitY) / iTipHeight;

	//描画パーツ数の設定
	INT iCountX,iCountY;
	iCountX = (fDrawWidth+iTipWidth-1) / iTipWidth;
	iCountY = (fDrawHeight+iTipHeight-1) / iTipHeight;
	if(fStartBitX)
		iCountX++;
	if(fStartBitY)
		iCountY++;

	for(j=0;j<iCountY;j++)
	{
		for(i=0;i<iCountX;i++)
		{
			INT iIndex = m_pAflSpriteMap->getMapIndex(iLevel,iMapX+i,iMapY+j); 
			//通常描画
			if(iIndex > 0 || (iLevel == 0 && iIndex==0))
			{
				//パーツの選択
				setImageIndex(iIndex);
				//相対位置の設定
				setRelativity(-fStartBitX+i*iTipWidth,-fStartBitY+j*iTipHeight,0);
				//描画
				drawImage(fX,fY,fZ);
			}
			//接触判定用
			if(m_pAflSpriteMap->isDrawCollide())
			{
				bool bCollide = m_pAflSpriteMap->isMapCollide(iMapX+i,iMapY+j);
				if(bCollide)
				{
					pSprite->setRelativity(-fStartBitX+i*iTipWidth,-fStartBitY+j*iTipHeight,0);
					pSprite->drawImage(fX,fY,fZ-2);
				}
			}
		}
	}
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflMapData
// 2Dマップデータ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

LPCSTR AflMapData::m_strMapHeader[] = 
{
	"AGSMAP-BIN","AGSMAP-TXT",
	"VERSION:","MAPNAME:","BITMAP:","SPRITE:",
	"WIDTH:","HEIGHT:","OPATTERN:","PARTSWIDTH:","PARTSHEIGHT:",
	"MAPDATA0:","MAPDATA1:","MAPDATA2:","MAPDATA3:","MAPDATA4:",
	"PAERSCOLLIDE0:","PAERSCOLLIDE1:","PAERSCOLLIDE2:",
    "PAERSCOLLIDE3:","PAERSCOLLIDE4:","MAPFLAG0:","OBJECT:",NULL
};
AflMapData::AflMapData()
{
}
bool AflMapData::setRelativePath()
{
	AflFileName fileName;

	INT i;
	for(i=0;i<4;i++)
	{
		fileName.setFileName(m_strBmpName[i].c_str());
		m_strBmpName[i] = fileName.relativePath(getMapFileName());
	}
	fileName.setFileName(m_strObjectFileName.c_str());
	m_strObjectFileName = fileName.relativePath(getMapFileName());
	
	return true;
}

bool AflMapData::isMapCollide(AflSprite* pSprite)
{
	AflRect rect;
	INT i;
	INT iTipWidth = getTipWidth();
	INT iTipHeight = getTipHeight();

	pSprite->getCollideRect2(&rect);
	if(rect.left < 0)
		rect.left -= iTipWidth;
	if(rect.top < 0)
		rect.top -= iTipHeight;
	int X1 = rect.left / iTipWidth;
	int X2 = rect.right / iTipWidth;
	int Y1 = rect.top / iTipHeight;
	int Y2 = rect.bottom / iTipHeight;
	for(;X1<=X2;X1++)
	{
		for(i=Y1;i<=Y2;i++)
		{
			if(isMapCollide(X1,i))
				return true;
		}

	}
	return false;
}

bool AflMapData::isMapCollide(INT iX,INT iY) 
{
	if(iX < 0 || iY < 0 || iX >= getMapWidth() || iY >= getMapHeight())
		return true;
	BYTE byFlag = getMapFlag(iX,iY);
	return (byFlag & 0x80) != 0;
}
void AflMapData::setMapCollide(INT iX,INT iY,bool bFlag)
{
	BYTE byFlag = getMapFlag(iX,iY);
	if(bFlag)
	{
		byFlag |= 0x80;
	}
	else
	{
		byFlag &= ~(BYTE)0x80;
	}
	setMapFlag(iX,iY,byFlag);
}
bool AflMapData::isTipCollide(INT iLevel,INT iIndex)const
{
	return (m_byCollide[iLevel][iIndex/8] & (BYTE)(1<<(iIndex%8))) != 0;
}
void AflMapData::setTipCollide(INT iLevel,INT iIndex,bool bFlag)
{
	if(bFlag)
		m_byCollide[iLevel][iIndex/8] |= (BYTE)(1<<(iIndex%8));
	else
		m_byCollide[iLevel][iIndex/8] &= ~(BYTE)(1<<(iIndex%8));
}
//--------------------------------
// 
//--------------------------------
bool AflMapData::isZero(int iLevel)
{
	int i;
	PSHORT pData = m_ptrData[iLevel].get();
	for(i=0;i<m_iMapWidth * m_iMapHeight;i++)
	{
		if(pData[i])
			return false;
	}
	return true;
}
//--------------------------------
// 
//--------------------------------
void AflMapData::writeHeader(FILE* pFile,LPCSTR pHeader,LPCVOID pData,int nSize)
{
	//-----------------------------
	DWORD dwLength;
	dwLength = strlen(pHeader)+1;				//ヘッダの長さ
	fwrite(&dwLength,sizeof(DWORD),1,pFile);
	fwrite(pHeader,dwLength,1,pFile);			//ヘッダ出力
	dwLength = (DWORD)nSize;					//データの長さ
	fwrite(&dwLength,sizeof(DWORD),1,pFile);
	fwrite(pData,dwLength,1,pFile);				//データ出力
	//-----------------------------

}
//--------------------------------
// 
//--------------------------------
void AflMapData::writeHeader(FILE* pFile,LPCSTR pHeader,LPCSTR pData)
{
	writeHeader(pFile,pHeader,pData,strlen(pData)+1);
}
//--------------------------------
// 
//--------------------------------
void AflMapData::writeHeader(FILE* pFile,LPCSTR pHeader,LPCWSTR pData)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"%lc",pData);
	writeHeader(pFile,pHeader,cBuff);
}
//--------------------------------
// 
//--------------------------------
void AflMapData::writeHeader(FILE* pFile,LPCSTR pHeader,int nData)
{
	writeHeader(pFile,pHeader,&nData,sizeof(int));
}
//--------------------------------
// 
//--------------------------------
bool AflMapData::saveMap(LPCSTR pFileName)
{
	int i,j,k;
	FILE* pFile;

	pFile = fopen(pFileName,TEXT("wb"));
	if(!pFile)
		return false;

	m_strFileName = pFileName;
	setRelativePath();

	fwrite(m_strMapHeader[MAP_BINHEADER],strlen(m_strMapHeader[MAP_BINHEADER])+1,1,pFile);
	
	//-----------------------------
	writeHeader(pFile,m_strMapHeader[MAP_VERSION],getVersion());
	writeHeader(pFile,m_strMapHeader[MAP_MAPNAME],getMapName());
	writeHeader(pFile,m_strMapHeader[MAP_BITMAP],getImageName(0));
	writeHeader(pFile,m_strMapHeader[MAP_SPRITE],getImageName(1));
	writeHeader(pFile,m_strMapHeader[MAP_WIDTH],getMapWidth());
	writeHeader(pFile,m_strMapHeader[MAP_HEIGHT],getMapHeight());
	writeHeader(pFile,m_strMapHeader[MAP_OPATTERN],getOutTipIndex());
	writeHeader(pFile,m_strMapHeader[MAP_PARTSWIDTH],getTipWidth());
	writeHeader(pFile,m_strMapHeader[MAP_PARTSHEIGHT],getTipHeight());
	writeHeader(pFile,m_strMapHeader[MAP_OBJECT],getObjectFileName());

	//-----------------------------
	//マップデータ
	for(k=0;k<4;k++)
	{
		if(!isZero(k))
		{
			PSHORT pData = m_ptrData[k].get();
			writeHeader(pFile,m_strMapHeader[MAP_MAPDATA0+k],
				pData,getMapWidth()*getMapHeight()*sizeof(short));
		}
	}

	//-----------------------------
	//パーツ接触フラグ
    for(j=0;j<5;j++)
    {
		int nCollide;
		for(nCollide=i=0;i<0xffff/8;i++)
			if(m_byCollide[j][i])
				nCollide = i+1;
		writeHeader(pFile,m_strMapHeader[MAP_PAERSCOLLIDE0+j],m_byCollide,nCollide);
	}
	//-----------------------------

	//-----------------------------
	//マップ個別フラグ
	writeHeader(pFile,m_strMapHeader[MAP_MAPFLAG0],getMapFlag(),getMapWidth()*getMapHeight());
	//-----------------------------
	fclose(pFile);
	return true;
}


bool AflMapData::openMap(LPCSTR pcstrFileName)
{
	std::fstream file(pcstrFileName,std::ios::in | std::ios::binary);
	if(!file.is_open())
		return false;

	//ヘッダチェック
	CHAR cBuff[1024];
	file.read(cBuff,strlen(m_strMapHeader[MAP_BINHEADER])+1);
	if(strcmp(cBuff,m_strMapHeader[MAP_BINHEADER]) != 0)
		return false;

	m_strFileName = pcstrFileName;
	
	INT iWidth = 0;
	INT iHeight = 0;
	m_iOutTipIndex = 0;


	DWORD dwSize;
	LPVOID pVoid;
	while(readHeader(file,cBuff,&pVoid,&dwSize))
	{
		int nIndex = getHeaderIndex(cBuff);
		switch(nIndex)
		{
		case MAP_VERSION:
			break;
		case MAP_MAPNAME:
			m_strMapName = (LPCSTR)pVoid;
			break;
		case MAP_BITMAP:
			m_strBmpName[0] = (LPCSTR)pVoid;
			break;
		case MAP_SPRITE:
			m_strBmpName[1] = (LPCSTR)pVoid;
			break;
		case MAP_WIDTH:
			iWidth = *(LPINT)pVoid;
			break;
		case MAP_HEIGHT:
			iHeight = *(LPINT)pVoid;
			break;
		case MAP_OPATTERN:
			m_iOutTipIndex = *(LPINT)pVoid;
			break;
		case MAP_PARTSWIDTH:
			m_iTipWidth = *(LPINT)pVoid;
			break;
		case MAP_PARTSHEIGHT:
			m_iTipHeight = *(LPINT)pVoid;
			break;
		case MAP_MAPDATA0:
			createMap(iWidth,iHeight);
		case MAP_MAPDATA1:
		case MAP_MAPDATA2:
		case MAP_MAPDATA3:
		case MAP_MAPDATA4:
			CopyMemory((PCHAR)m_ptrData[nIndex-MAP_MAPDATA0].get(),pVoid,dwSize);
			break;
		case MAP_PAERSCOLLIDE0:
		case MAP_PAERSCOLLIDE1:
		case MAP_PAERSCOLLIDE2:
		case MAP_PAERSCOLLIDE3:
		case MAP_PAERSCOLLIDE4:
			CopyMemory(m_byCollide[nIndex-MAP_PAERSCOLLIDE0],pVoid,dwSize);
			break;
		case MAP_MAPFLAG0:
			CopyMemory(getMapFlag(),pVoid,dwSize);
			break;
		case MAP_OBJECT:
			m_strObjectFileName = (LPCSTR)pVoid;
			break;
		}
		delete (LPBYTE)pVoid;
        cBuff[0] = 0;
	}
	
	return true;
}
bool AflMapData::createTipFlag(INT iWidth,INT iHeight)
{
	//チップフラグ
    std::auto_ptr<BYTE> byMapFlag(new BYTE[iWidth*iHeight]);
	m_ptrMapFlag = byMapFlag;
	ZeroMemory(m_ptrMapFlag.get(),sizeof(BYTE)*iWidth*iHeight);
    return true;
}
bool AflMapData::createMap(INT iWidth,INT iHeight)
{
	int i;
	for(i=0;i<5;i++)
	{
    	std::auto_ptr<SHORT> sData(NEW SHORT[iWidth*iHeight]);
		m_ptrData[i] = sData;
		ZeroMemory(m_ptrData[i].get(),sizeof(SHORT)*iWidth*iHeight);
	}
	createTipFlag(iWidth,iHeight);

	m_iMapWidth = iWidth;
	m_iMapHeight = iHeight;
	return true;
}
bool AflMapData::setMapSize(INT iWidth,INT iHeight)
{
	int i,j,k;
	for(i=0;i<5;i++)
	{
		PSHORT pData = NEW SHORT[iWidth*iHeight];
		ZeroMemory(pData,sizeof(SHORT)*iWidth*iHeight);
		for(j=0;j < iHeight ;j++)
		{
			for(k=0;k < iWidth ;k++)
				pData[iWidth*j+k] = getMapIndex(i,k,j);
		}

    	std::auto_ptr<SHORT> sData(pData);
		m_ptrData[i] = sData;
	}

	m_iMapWidth = iWidth;
	m_iMapHeight = iHeight;
	return true;
}
bool AflMapData::copyMap(INT iX1,INT iY1,INT iWidth,INT iHeight,AflMapData* pMapData,INT iX2,INT iY2)
{
	int i,j,k;
	if(iWidth == 0)
		iWidth = pMapData->getMapWidth();
	if(iHeight == 0)
		iHeight = pMapData->getMapHeight();

	for(i=0;i<5;i++)
	{
		for(j=0;j < iHeight ;j++)
		{
			for(k=0;k < iWidth ;k++)
            {
            	INT iIndex = pMapData->getMapIndex(i,k+iX2,j+iY2);
                if(iIndex >= 0)
					setMapIndex(i,iX1+k,j+iY1,iIndex);
				if(i==0)
				{
					setMapFlag(iX1+k,j+iY1, pMapData->getMapFlag(k+iX2,j+iY2));
				}
			}
        }
	}
	return true;
}
bool AflMapData::clearMap(INT iIndex)
{
	int i,j,k;
	INT iWidth = m_iMapWidth;
	INT iHeight = m_iMapHeight;
	for(i=0;i<5;i++)
	{
		PSHORT pData = m_ptrData[i].get();
		for(j=0;j < iHeight ;j++)
		{
			for(k=0;k < iWidth ;k++)
				pData[iWidth*j+k] = iIndex;
		}
	}
	return true;
}
BYTE AflMapData::getMapFlag(INT iX,INT iY)
{
	if(iX < 0 || iY < 0 || iX >= m_iMapWidth || iY >= m_iMapHeight)
	{
		return 0;
	}
	LPBYTE pbyFlag = getMapFlag();
	return pbyFlag[iX + getMapWidth()*iY];
}
void AflMapData::setMapFlag(INT iX,INT iY,BYTE byFlag)
{
	if(iX < 0 || iY < 0 || iX >= m_iMapWidth || iY >= m_iMapHeight)
		return;
	LPBYTE pbyFlag = getMapFlag();
	 pbyFlag[iX + getMapWidth()*iY] = byFlag;
}

INT AflMapData::getMapIndex(INT iLevel,INT iX,INT iY)const
{
	if(iX < 0 || iY < 0 || iX >= m_iMapWidth || iY >= m_iMapHeight)
	{
		if(iLevel > 0)
			return 0;
		else
			return m_iOutTipIndex;
	}
	PSHORT pData = m_ptrData[iLevel].get();
	return pData[iY*m_iMapWidth+iX];
}
INT AflMapData::setMapIndex(INT iLevel,INT iX,INT iY,INT iIndex)
{
	PSHORT pData = m_ptrData[iLevel].get();
    if(iX < m_iMapWidth && iY < m_iMapHeight)
		return pData[iY*m_iMapWidth+iX] = iIndex;
    return false;
}
int AflMapData::getHeaderIndex(LPCSTR pcstrHeader)
{	
	int i;
	for(i=0;m_strMapHeader[i];i++)
		if(!strcmp(m_strMapHeader[i],pcstrHeader))
			break;
	if(m_strMapHeader[i])
		return i;
	return -1;
}

BOOL AflMapData::readHeader(std::fstream& file,LPSTR pHeader,LPVOID* ppData,LPDWORD pdwSize)
{
	DWORD dwSize=0;
	if(file.eof())
		return false;
	file.read((PCHAR)&dwSize,sizeof(DWORD));
	file.read((PCHAR)pHeader,dwSize);
	file.read((PCHAR)&dwSize,sizeof(DWORD));
	*ppData = NEW BYTE[dwSize];
	file.read((PCHAR)*ppData,dwSize);
	if(pdwSize)
		*pdwSize = dwSize;
	return true;
}

//------------------------------------------------------------
// AflMapRoute
// マップ経路探索用クラス
//------------------------------------------------------------
bool AflMapRoute::createRouteBuffer(AflSpriteMap* pMap)
{
	INT i,j;
	INT iWidth = pMap->getMapWidth();
	INT iHeight = pMap->getMapHeight();
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if(m_piCollide)
		delete m_piCollide;
	if(m_piCollideMaster)
		delete m_piCollideMaster;

	//経路探索作業領域の確保
	m_piCollideMaster = NEW INT[iWidth * iHeight];
	m_piCollide = NEW INT[iWidth * iHeight];
	//接触条件の設定
	for(j=0;j<iHeight;j++)
	{
		for(i=0;i<iWidth;i++)
		{
			if(pMap->isMapCollide(i,j))
				m_piCollideMaster[iWidth*j + i] = -1;
			else
				m_piCollideMaster[iWidth*j + i] = 10000;
		}
	}

	//オブジェクト情報を接触判定に追加
	std::list<AflMapObject>* pObject = pMap->getObjectList();
	std::list<AflMapObject>::iterator it;
	for(it=pObject->begin();it!=pObject->end();it++)
	{
		INT iTipWidth = pMap->getTipWidth();
		INT iTipHeight = pMap->getTipHeight();

		AflRect rect;
		(*it).getCollideRect2(&rect);
		INT iStartX = rect.left / iTipWidth;
		INT iStartY = rect.top / iTipHeight;
		INT iEndX = rect.right / iTipWidth;
		INT iEndY = rect.bottom / iTipHeight;

		INT i,j;
		for(j=iStartY;j<=iEndY;j++)
		{
			for(i=iStartX;i<=iEndX;i++)
			{
				m_piCollideMaster[iWidth*j + i] = -1;
			}
		}
	}
	return true;
}

LPPOINT AflMapRoute::getRoute(INT iX1,INT iY1,INT iX2,INT iY2)
{
	m_iTargetCount = 10000;
	m_iTargetX = iX2;
	m_iTargetY = iY2;

	CopyMemory(m_piCollide,m_piCollideMaster,sizeof(INT)*m_iWidth*m_iHeight);
	//起点の初期化
	setCollideCount(iX1,iY1,0);
	//経路探索配列の作成
	setCollideData(iX1,iY1);
	//経路の設定
	LPPOINT pPoint =  getRouteData(iX2,iY2);

/*	
	if(pPoint)
	{
		FILE* pFile = fopen("test.txt","wt");
		for(j=0;j<iHeight;j++)
		{
			for(i=0;i<iWidth;i++)
			{
				fprintf(pFile,"%02X ",(BYTE)getCollideCount(i,j));
			}
			fprintf(pFile,"\n");
		}

		for(i=0;i<m_iRouteCount;i++)
		{
			fprintf(pFile,"(%02d,%02d)\n",pPoint[i].x,pPoint[i].y);
		}
		fclose(pFile);
	}
*/

	
	return pPoint;
}


LPPOINT AflMapRoute::getRouteData(INT iX,INT iY)
{
	//経路までの最短距離の取得
	INT iCount = getCollideCount(iX,iY);
	m_iRouteCount = iCount;
	//移動可能か
	if(iCount < 0 || iCount >= m_iCountMax)
		return NULL;
	
	//経路情報領域の確保
	LPPOINT pPoint = NEW POINT[iCount];
	INT i;
	POINT point={iX,iY};
	for(i=iCount-1;i>=0;i--)
	{
		//経路の保存
		*(pPoint+i) = point;
		if(rand()%2)
		{
			if(getCollideCount(point.x,point.y-1) == i)			//上探索
				--point.y;
			else if(getCollideCount(point.x,point.y+1) == i)	//下探索
				++point.y;
			else if(getCollideCount(point.x-1,point.y) == i)	//左探索
				--point.x;
			else if(getCollideCount(point.x+1,point.y) == i)	//右探索
				++point.x;
		}
		else
		{
			if(getCollideCount(point.x+1,point.y) == i)			//右探索
				++point.x;
			else if(getCollideCount(point.x-1,point.y) == i)	//左探索
				--point.x;
			else if(getCollideCount(point.x,point.y+1) == i)	//下探索
				++point.y;
			else if(getCollideCount(point.x,point.y-1) == i)	//上探索
				--point.y;
		}
	}
	return pPoint;
}
void AflMapRoute::setCollideData(INT iX1,INT iY1,INT iFlag)
{
	//現在位置の歩数取得
	INT iNowCount = getCollideCount(iX1,iY1);
	//歩数限界値もしくは判明済みターゲットカウントより大きかったら
	if(iNowCount == -1 || iNowCount > m_iTargetCount || iNowCount >= m_iCountMax)
		return;
	//現在位置がターゲット座標か
	if(m_iTargetX == iX1 && m_iTargetY == iY1)
	{
		//最小ターゲットカウントを記録
		if(iNowCount <= m_iTargetCount)
			m_iTargetCount = iNowCount;
	}
	iNowCount++;
	//上の歩数を設定
	if(iFlag!=1 && iY1+1 < m_iHeight)	//範囲取得
	{
		INT iCount = getCollideCount(iX1,iY1+1);
		if(iCount>=0 && iCount > iNowCount)
		{
			//最短歩数の設定
			setCollideCount(iX1,iY1+1,iNowCount);
			//最短経路の捜索継続
			setCollideData(iX1,iY1+1,2);
		}
	}
	//下の歩数を設定
	if(iFlag!=2 && iY1-1 >= 0)			//範囲取得
	{
		INT iCount = getCollideCount(iX1,iY1-1);
		if(iCount>=0 && iCount > iNowCount)
		{
			//最短歩数の設定
			setCollideCount(iX1,iY1-1,iNowCount); 
			//最短経路の捜索継続
			setCollideData(iX1,iY1-1,1);
		}
	}
	//右の歩数を設定
	if(iFlag!=3 && iX1+1 < m_iWidth)	//範囲取得
	{
		INT iCount = getCollideCount(iX1+1,iY1);
		if(iCount>=0 && iCount > iNowCount)
		{
			//最短歩数の設定
			setCollideCount(iX1+1,iY1,iNowCount);
			//最短経路の捜索継続
			setCollideData(iX1+1,iY1,4);
		}
	}
	//左の歩数を設定
	if(iFlag!=4 && iX1-1 >= 0)			//範囲取得
	{
		INT iCount = getCollideCount(iX1-1,iY1);
		if(iCount>=0 && iCount > iNowCount)
		{
			//最短歩数の設定
			setCollideCount(iX1-1,iY1,iNowCount); 
			//最短経路の捜索継続
			setCollideData(iX1-1,iY1,3);
		}
	}

}
//経路歩数の取得
INT AflMapRoute::getCollideCount(INT iX,INT iY)const
{
	INT iWidth = m_iWidth;
	INT iHeight = m_iHeight;
	if(iX < 0 || iX >= iWidth || iY < 0 || iY > iHeight)
		return -1;
	return m_piCollide[iY*iWidth+iX];
}
//経路歩数の設定
void AflMapRoute::setCollideCount(INT iX,INT iY,INT iCount)const
{
	INT iWidth = m_iWidth;
	INT iHeight = m_iHeight;
	if(iX < 0 || iX >= iWidth || iY < 0 || iY > iHeight)
		return;
	m_piCollide[iY*iWidth+iX] = iCount;
}
