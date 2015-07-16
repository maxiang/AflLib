#include <windows.h>
#include <tchar.h>

#include "afl3DUnitCustom.h"

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

namespace AFL{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - チップ型2Dマップ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitMap::UnitMap()
{
	const static D3D11_INPUT_ELEMENT_DESC layoutTexture[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		0
	};

	if(!D3DDevice::getInputElement("TEXTURE"))
	{
		D3DDevice::addInputElement("TEXTURE",layoutTexture);
	}


	setView2D(true);
	setZBuffer(false);
	setLight(false);
	//分割サイズ
	m_partWidth = 32;
	m_partHeight = 32;
	m_viewWidth = 128;
	m_viewHeight = 128;
	m_drawX = 0;
	m_drawY = 0;
	//メッシュの作成
	Mesh mesh;
	mesh.createLayout("TEXTURE","TEXTURE");
	mesh.setPShader("TEXTURE");
	//フレームの作成
	m_frame = SP<Frame>(new Frame());
	m_frame->add(&mesh);
	m_spriteMesh = m_frame->getMesh();

	m_map = NULL;
}
UnitMap::~UnitMap()
{
	delete[] m_map;
}
void UnitMap::setDrawPoint(INT x,INT y)
{
	if(m_drawX != x || m_drawY != y)
	{
		m_drawX = x;
		m_drawY = y;
		resetRenderFlag();
	}
}
void UnitMap::setViewSize(INT x,INT y)
{
	m_viewWidth = x;
	m_viewHeight = y;
}
INT UnitMap::getDrawPointX() const
{
	return m_drawX;
}
INT UnitMap::getDrawPointY() const
{
	return m_drawY;
}

void UnitMap::setMapSize(INT width,INT height)
{
	INT size = width*height;
	m_map = new BYTE[size];
	m_mapWidth = width;
	m_mapHeight = height;
	ZeroMemory(m_map,size);
	resetRenderFlag();
}
void UnitMap::setMapIndex(INT x,INT y,INT index)
{
	if(x>=0 && x<m_mapWidth && y>=0 && y<m_mapHeight)
		m_map[y*m_mapWidth+x] = index;
	resetRenderFlag();
}
INT UnitMap::getMapIndex(INT x,INT y)
{
	if(x>=0 && x<m_mapWidth && y>=0 && y<m_mapHeight)
		return (INT)m_map[y*m_mapWidth+x];
	return 0;
}

bool UnitMap::openImage(LPCSTR fileName)
{
	return openImage(UCS2(fileName));
}
bool UnitMap::openImage(LPCWSTR fileName)
{
	//テクスチャの作成
	SP<Texture> texture = new Texture();
	if(!texture->open(fileName))
		return false;
	Material material;
 	//メッシュの作成
	m_spriteMesh->addTexture(texture);
	m_spriteMesh->setMaterial(material);


	resetRenderFlag();
	return true;
}
void UnitMap::setVertex(MAPVERTEX* vertex,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT tx1,FLOAT ty1,FLOAT tx2,FLOAT ty2)
{
	vertex[0].pos.x = x1;
	vertex[0].pos.y = y1;
	vertex[0].tex.x = tx1;
	vertex[0].tex.y = ty1;

	vertex[1].pos.x = x2;
	vertex[1].pos.y = y1;
	vertex[1].tex.x = tx2;
	vertex[1].tex.y = ty1;

	vertex[2].pos.x = x1;
	vertex[2].pos.y = y2;
	vertex[2].tex.x = tx1;
	vertex[2].tex.y = ty2;

	vertex[3].pos.x = x2;
	vertex[3].pos.y = y2;
	vertex[3].tex.x = tx2;
	vertex[3].tex.y = ty2;
}
Texture* UnitMap::getTexture() const
{
	if(m_spriteMesh && m_spriteMesh->getMaterial())
		return m_spriteMesh->getTexture();
	else
		return NULL;
}
bool UnitMap::setVertex()
{
	Texture* texture = getTexture();
	if(!texture)
		return false;
	INT width = texture->getImageWidth();
	INT height = texture->getImageHeight();
	INT textureWidth = texture->getTextureWidth();
	INT textureHeight = texture->getTextureHeight();
	FLOAT txWidth = (FLOAT)width/textureWidth/(width/m_partWidth);
	FLOAT txHeight = (FLOAT)height/textureHeight/(height/m_partHeight);


	INT countX = m_viewWidth/m_partWidth+1;
	INT countY = m_viewHeight/m_partHeight+1;
	INT vertexCount = countX*countY*4;
	MAPVERTEX* vertexSprite = new MAPVERTEX[vertexCount];
	INT i,j;
	INT index = 0;

	INT pdx = (INT)m_drawX % m_partWidth;
	INT pdy = (INT)m_drawY % m_partHeight;
	INT indexX = (INT)m_drawX / m_partWidth;
	INT indexY = (INT)m_drawY / m_partHeight;
	if(pdx < 0)
	{
		pdx = m_partWidth + pdx;
		--indexX;
	}
	if(pdy < 0)
	{
		pdy = m_partHeight + pdy;
		--indexY;
	}
	for(j=0;j<countY;j++)
	{
		for(i=0;i<countX;i++)
		{
			FLOAT x1 = (FLOAT)i*m_partWidth-pdx;
			FLOAT y1 = (FLOAT)j*m_partHeight-pdy;
			FLOAT x2 = x1 + m_partWidth;
			FLOAT y2 = y1 + m_partHeight;
			INT mapIndex = getMapIndex(i+indexX,j+indexY);
			FLOAT ix = (mapIndex % m_partWidth)*txWidth;
			FLOAT iy = (mapIndex / m_partHeight)*txHeight;
			setVertex(&vertexSprite[index],x1,y1,x2,y2,ix,iy,ix+txWidth-0.005f,iy+txHeight-0.005f);
			index += 4;
		}
	}
	m_spriteMesh->createVertex(vertexSprite,vertexCount*sizeof(MAPVERTEX));
	delete[] vertexSprite;


	INT indexCount = countX*countY;
	LPWORD indexData = new WORD[indexCount*6];
	for(i=0;i<indexCount;i++)
	{
		indexData[i*6+0] = i*4+0;
		indexData[i*6+1] = i*4+1;
		indexData[i*6+2] = i*4+2;
		indexData[i*6+3] = i*4+3;
		indexData[i*6+4] = i*4+2;
		indexData[i*6+5] = i*4+1;
	}

	m_spriteMesh->createIndex(indexData,indexCount*6*sizeof(WORD));
	delete[] indexData;
	return true;
}
bool UnitMap::onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag())
	{
		setVertex();
	}
	return true;
}
}