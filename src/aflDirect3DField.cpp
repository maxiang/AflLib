#include <windows.h>
#include "aflDirect3DField.h" 
#include "aflDirect3DWorld.h" 

#if defined(_MSC_VER) && defined(_DEBUG)
	//メモリリークテスト
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
#endif 

namespace AFL{namespace DIRECT3D{
#include "fx\field.h"


FieldData::FieldData()
{
	m_outHeight = -50.0f;
	m_outIndex = 3;
	m_fieldSize.cx = 0;
	m_fieldSize.cy = 0;
}
bool FieldData::create(INT width,INT height)
{
	m_fieldSize.cx = width;
	m_fieldSize.cy = height;
	m_fieldHeight.resize(width*height);
	m_fieldIndex.resize(width*height);
	m_fieldFlag.resize(width*height);

	INT i,j;
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			FLOAT height = getFieldHeight(i-1,j-1) + getFieldHeight(i,j-1) + getFieldHeight(i-1,j);
			height /= 3;
			m_fieldHeight[j*width+i] = height + (FLOAT)((INT)rand()%80) - 40.0f;
			//m_fieldHeight[j*width+i] = (FLOAT)((INT)rand()%200);
			//m_fieldHeight[j*width+i] = 0;
		}
	}
	return true;
}
FLOAT FieldData::getFieldHeight(INT x,INT y)
{
	if(x<1 || y<1 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return m_outHeight;
	return m_fieldHeight[y*m_fieldSize.cx+x];
}
void FieldData::setFieldHeight(INT x,INT y,FLOAT height)
{
	if(x<1 || y<1 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return;
	m_fieldHeight[y*m_fieldSize.cx+x] = height;
}
INT FieldData::getFieldIndex(INT x,INT y)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return m_outIndex;
	return m_fieldIndex[y*m_fieldSize.cx+x];
}
void FieldData::setFieldIndex(INT x,INT y,INT index)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return;
	m_fieldIndex[y*m_fieldSize.cx+x] = index;
}
DWORD FieldData::getFieldFlag(INT x,INT y)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return 0;
	return m_fieldFlag[y*m_fieldSize.cx+x];
}
void FieldData::setFieldFlag(INT x,INT y,DWORD flag)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return;
	m_fieldFlag[y*m_fieldSize.cx+x] = flag;
}




FieldUnit::FieldUnit()
{

	m_frame = SP<Frame>(NEW Frame());
	m_frame->add(&Mesh());
	m_mesh = m_frame->getMesh();

	//マテリアルの設定
	Material* material = m_mesh->getMaterial();
	material->Emissive.r = 0.3f;
	material->Emissive.g = 0.3f;
	material->Emissive.b = 0.3f;
	material->Ambient.a = 1.0f;
	material->Ambient.r = 0.4f;
	material->Ambient.g = 0.4f;
	material->Ambient.b = 0.4f;	
	material->Diffuse.a = 1.0f;
	material->Diffuse.r = 1.0f;
	material->Diffuse.g = 1.0f;
	material->Diffuse.b = 1.0f;	

	m_startPoint.x = -100;
	m_startPoint.y = -100;
	m_tipsSize.cx = 32;
	m_tipsSize.cy = 32;
	m_drawSize.cx = 60;
	m_drawSize.cy = 60;
	m_unitSize.x = 256.0f;
	m_unitSize.y = 256.0f;
	m_sphereFactor = 600;

	m_initVertex = true;
	setTextureFilter(D3DTEXF_LINEAR);
}
bool FieldUnit::openTexture(LPCSTR fileName)
{
	Texture* texture = NEW Texture();
	texture->openImage(fileName,0 ,D3DPOOL_MANAGED);

	m_mesh->setTexture(SP<Texture>(texture));

	//if(!Device::getVShader("field"))
	//	Device::getVShader("field");
	m_mesh->setVertexShader("field");

	return true;
}
VERTEXFIELD* FieldUnit::getVertexField(INT x,INT y)
{
	INT width = m_fieldData.getWidth()+2;
	INT height = m_fieldData.getHeight()+2;
	x++;
	y++;
	if(x >= 0 && y >= 0 && x < width && y < height)
	{
		return &m_vertexField[(width*y+x)*6];
	}
	return &m_vertexField[(width*height)*6];

}
void FieldUnit::setVertexField(INT x,INT y,FLOAT px,FLOAT py,FLOAT u,FLOAT v,INT imageWidthCount)
{
	VERTEXFIELD* vertexField = getVertexField(x,y);
	if(!vertexField || !imageWidthCount)
		return;
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;
	//UV基本設定
	INT	tipsIndex = m_fieldData.getFieldIndex(x,y);
	INT tipsIndexX = tipsIndex % imageWidthCount;
	INT tipsIndexY = tipsIndex / imageWidthCount;
	FLOAT u1 = tipsIndexX * u+0.02f;
	FLOAT u2 = (tipsIndexX+1) * u - 0.02f;
	FLOAT v1 = tipsIndexY * v+0.02f;
	FLOAT v2 = (tipsIndexY+1) * v - 0.02f;

	FLOAT h1,h2,h3,h4;
	h1 = m_fieldData.getFieldHeight(x,y);
	h2 = m_fieldData.getFieldHeight(x+1,y);
	h3 = m_fieldData.getFieldHeight(x,y+1);
	h4 = m_fieldData.getFieldHeight(x+1,y+1);
	//NVector vect1 = NVectorSet(unitWidth,0,h2-h1,0.0f);
	//NVector vect2 = NVectorSet(0,unitHeight,h3-h1,0.0f);

	vertexField[0].position.x = px;
	vertexField[0].position.y = py;
	vertexField[0].position.z = h1;
	vertexField[0].u = u1;
	vertexField[0].v = v1;
	vertexField[1].position.x = px+unitWidth;
	vertexField[1].position.y = py;
	vertexField[1].position.z = h2;
	vertexField[1].u = u2;
	vertexField[1].v = v1;
	vertexField[2].position.x = px;
	vertexField[2].position.y = py+unitHeight;
	vertexField[2].position.z = h3;
	vertexField[2].u = u1;
	vertexField[2].v = v2;

	vertexField[3].position.x = px+unitWidth;
	vertexField[3].position.y = py+unitHeight;
	vertexField[3].position.z = h4;
	vertexField[3].u = u2;
	vertexField[3].v = v2;
	vertexField[4].position.x = px;
	vertexField[4].position.y = py+unitHeight;
	vertexField[4].position.z = h3;
	vertexField[4].u = u1;
	vertexField[4].v = v2;
	vertexField[5].position.x = px+unitWidth;
	vertexField[5].position.y = py;
	vertexField[5].position.z = h2;
	vertexField[5].u = u2;
	vertexField[5].v = v1;

	NVector vect0,vect1,vect2;
	NVector vect0A;
	vect0A = vertexField[2].position - vertexField[0].position;
	NVector vect0B;
	vect0B = vertexField[1].position - vertexField[0].position;
	NVector vect1A;
	vect1A = vertexField[2].position - vertexField[1].position;
	NVector vect1B;
	vect1B = vertexField[3].position - vertexField[1].position;
	vect0 = vect0B.cross(vect0A);
	vect0 = vect0.normal();
	vect1 = vect1B.cross(vect1A);
	vect1 = vect1.normal();
	vect2 = vect0+vect1;
	vect2 /= 2;
	vertexField[0].normal = vect0;
	vertexField[1].normal = vect2;
	vertexField[2].normal = vect2;
	vertexField[3].normal = vect1;
	vertexField[4].normal = vect2;
	vertexField[5].normal = vect2;
}
bool FieldUnit::create(INT width,INT height)
{
	m_fieldData.create(width,height);


	const static D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL , 0 },
		{ 0, 24,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	Mesh* mesh = m_mesh;


	INT indexCount = m_drawSize.cx * m_drawSize.cy;
	mesh->createVertexBuffer(indexCount*6,sizeof(VERTEXFIELD),D3DFVF_VERTEXFIEALD,D3DPOOL_DEFAULT);
	mesh->setDeclaration(decl);


	createVertex();
	return true;
}
void FieldUnit::createVertex()
{
	INT i,j;
	//頂点の作成
	INT width = m_fieldData.getWidth();
	INT height = m_fieldData.getHeight();
	m_vertexField.resize(((width+2)*(height+2)+1)*6);
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;
	INT tipsWidth = m_tipsSize.cx;
	INT tipsHeight = m_tipsSize.cy;
	INT imageWidth = m_mesh->getTexture()->getImageWidth();
	INT imageHeight = m_mesh->getTexture()->getImageHeight();
	INT imageWidthCount = (INT)(imageWidth / tipsWidth);
	FLOAT u = (FLOAT)tipsWidth / imageWidth;
	FLOAT v = (FLOAT)tipsHeight / imageHeight;


	for(j=-1;j<=height;j++)
	{
		FLOAT py = unitHeight*j;
		for(i=-1;i<=width;i++)
		{
			FLOAT px = unitWidth*i;
			setVertexField(i,j,px,py,u,v,imageWidthCount);
		}
	}
	setVertexField(-5,-5,0,0,u,v,imageWidthCount);
	m_initVertex = false;
	resetRenderFlag();

}


bool FieldUnit::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	INT i,j,k;

	//頂点データの初期化が必要か
	if(m_initVertex)
		createVertex();

	FLOAT drawPointX = 0.0f;
	FLOAT drawPointY = 0.0f;
	INT drawWidth = m_drawSize.cx;
	INT drawHeight = m_drawSize.cy;
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;

	Camera3D* camera = (Camera3D*)world->getCamera();
	if(camera)
	{
		NVector vect = {0.0f,1.0f,0.0f,1.0f};
		vect = vect.transformNormal(camera->getView());
		vect = vect.normal3();

		float pointX = camera->getX();
		float pointY = camera->getY();
		drawPointX = pointX + vect.x * unitWidth * drawWidth*0.4f;
		drawPointY = pointY + vect.y * unitHeight * drawHeight*0.4f;
	}

	INT startX = (INT)(drawPointX/unitWidth) - drawWidth/2;
	INT startY = (INT)(drawPointY/unitHeight) - drawHeight/2;

	if(isRenderFlag() && m_startPoint.x == startX && m_startPoint.y == startY)
	{
		return true;
	}
	m_startPoint.x = startX;
	m_startPoint.y = startY;

	INT width = m_fieldData.getWidth();
	INT height = m_fieldData.getHeight();

	VERTEXFIELD vertexWork[6];
	VERTEXFIELD* vertexField = (VERTEXFIELD*)m_mesh->getVertexBuffer()->lock();
	for(j=0;j<drawHeight;j++)
	{
		INT py = startY+j;
		for(i=0;i<drawWidth;i++)
		{
			INT px = startX+i;
			VERTEXFIELD* vertexSrc = getVertexField(px,py);
			CopyMemory(vertexWork,vertexSrc,sizeof(VERTEXFIELD)*6);
			if(px < -1 || py < -1 || px >= width+1 || py >= height+1)
			{
				for(k=0;k<6;k++)
				{
					vertexWork[k].position.x += px * unitWidth;
					vertexWork[k].position.y += py * unitHeight;
				}
			}
			CopyMemory(vertexField,vertexWork,sizeof(VERTEXFIELD)*6);
			vertexField += 6;
		}
	}
	m_mesh->getVertexBuffer()->unlock();
	return true;
}

FLOAT FieldUnit::getPointHeight(FLOAT x,FLOAT y)
{
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;
	INT iX = (INT)(x / unitWidth);
	INT iY = (INT)(y / unitHeight);
	FLOAT fx = (x - iX * unitWidth) / unitWidth;
	FLOAT fy = (y - iY * unitHeight) / unitHeight;
	if(x < 0)
		--iX;
	if(y < 0)
		--iY;

	FLOAT height;
	if(fx <= (1.0-fy))
	{
		FLOAT baseHeight,xHeight,yHeight;
		baseHeight = m_fieldData.getFieldHeight(iX,iY);
		xHeight = m_fieldData.getFieldHeight(iX+1,iY) - baseHeight;
		yHeight = m_fieldData.getFieldHeight(iX,iY+1) - baseHeight;
		height = baseHeight + xHeight * fx + yHeight * fy;
	}
	else
	{
		FLOAT baseHeight,xHeight,yHeight;
		baseHeight =  m_fieldData.getFieldHeight(iX+1,iY+1);
		xHeight = m_fieldData.getFieldHeight(iX,iY+1) - baseHeight;
		yHeight = m_fieldData.getFieldHeight(iX+1,iY) - baseHeight;
		height = baseHeight + xHeight * (1.0f - fx) + yHeight * (1.0f - fy);
	}
	return height;
}
FLOAT FieldUnit::getFieldWidth()
{
	return (FLOAT)m_fieldData.getWidth() * m_unitSize.x;
}
FLOAT FieldUnit::getFieldHeight()
{
	return (FLOAT)m_fieldData.getHeight() * m_unitSize.y;
}
DWORD FieldUnit::getFieldPointFlag(FLOAT x,FLOAT y)
{
	INT px = (INT)(x / m_unitSize.x);
	INT py = (INT)(y / m_unitSize.y);
	return m_fieldData.getFieldFlag(px,py);
}
void FieldUnit::setFieldPointFlag(FLOAT x,FLOAT y,DWORD flag)
{
	INT px = (INT)(x / m_unitSize.x);
	INT py = (INT)(y / m_unitSize.y);
	m_fieldData.setFieldFlag(px,py,flag);
}
void FieldUnit::setFieldSide(PFLOAT dx,PFLOAT dy,FLOAT sx,FLOAT sy)
{
	FLOAT x1,x2,y1,y2;
	FLOAT fx1,fx2,fy1,fy2;
	fx1 = (FLOAT)m_unitSize.x * (INT)(*dx / m_unitSize.x);
	fy1 = (FLOAT)m_unitSize.y * (INT)(*dy / m_unitSize.y);
	fx2 = fx1 + m_unitSize.x;
	fy2 = fy1 + m_unitSize.y;
	if(*dx > sx)
	{
		x1 = sx;
		x2 = *dx;
	}
	else
	{
		x1 = *dx;
		x2= sx;
	}
	if(*dy > sy)
	{
		y1 = sy;
		y2 = *dy;
	}
	else
	{
		y1 = *dy;
		y2= sy;
	}

	if(x2 > fx1 && x1 < fx2)
	{
		if(fy1 > y1 && fy1 < y2)
		{
			*dy = fy1-1;
		}
		else if(fy2 > y1 && fy2 < y2)
		{
			*dy = fy2+1;
		}
	}
	if(y2 > fy1 && y1 < fy2)
	{
		if(fx1 > x1 && fx1 < x2)
		{
			*dx = fx1-1;
		}
		else if(fx2 > x1 && fx2 < x2)
		{
			*dx = fx2+1;
		}
	}
}
INT FieldUnit::getFieldIndex(INT x,INT y)
{
	return m_fieldData.getFieldIndex(x,y);
}
void FieldUnit::setFieldIndex(INT x,INT y,INT index)
{
	m_fieldData.setFieldIndex(x,y,index);
	m_initVertex = true;
		
}
FLOAT FieldUnit::getFieldHeight(INT x,INT y)
{
	return m_fieldData.getFieldHeight(x,y);
}
void FieldUnit::setFieldHeight(INT x,INT y,FLOAT height)
{
	m_fieldData.setFieldHeight(x,y,height);
	m_initVertex = true;
}
bool FieldUnit::onDeviceRestore()
{
	m_initVertex = true;

	return true;
}

}}