#ifndef __ANDROID__
	#include <windows.h>
#else
	#include "AndroidApp.h"
	#include <android/log.h>
#endif


#include "afl3DField.h" 
#include "afl3DWorld.h" 

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

namespace AFL{


FieldData::FieldData()
{
	m_outHeight = 0.0f;
	m_outIndex = 3;
	m_fieldSize.cx = 0;
	m_fieldSize.cy = 0;
}
bool FieldData::create(INT width,INT height,INT high,INT low)
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
			if(high == 0)
				m_fieldHeight[j*width+i] = 0;
			else
				m_fieldHeight[j*width+i] = (FLOAT)((INT)rand()%high-low);
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
	if(x>=0 && y>=0 && y<m_fieldSize.cy && x<m_fieldSize.cx)
	{
		m_fieldHeight[y*m_fieldSize.cx+x] = height;
	}
}
void FieldData::setFieldIndex(INT x,INT y,INT index)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return;
	m_fieldIndex[y*m_fieldSize.cx+x] = index;
}

INT FieldData::getFieldIndex(INT x,INT y)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return m_outIndex;
	return m_fieldIndex[y*m_fieldSize.cx+x];
}
DWORD FieldData::getFieldFlag(INT x,INT y)
{
	if(x<1 || y<1 || y>=m_fieldSize.cy-1 || x>=m_fieldSize.cx-1)
		return 0;
	return m_fieldFlag[y*m_fieldSize.cx+x];
}
void FieldData::setFieldFlag(INT x,INT y,DWORD flag)
{
	if(x<0 || y<0 || y>=m_fieldSize.cy || x>=m_fieldSize.cx)
		return;
	m_fieldFlag[y*m_fieldSize.cx+x] = flag;
}
void FieldData::setOutIndex(INT index)
{
	m_outIndex = index;
}
void FieldData::setOutHeight(FLOAT height)
{
	m_outHeight = height;
}
void FieldData::clear(INT index)
{
	INT i;
	INT size = m_fieldIndex.size();
	for(i=0;i<size;i++)
	{
		m_fieldIndex[i] = index;
	}
}



void FieldUnit::_init()
{
	static bool init = false;
	if(init)
		return;
#if !defined(_OPENGL) & !defined(__ANDROID__)
	//頂点配置の設定
	const static D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD_", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		0
	};
	//設定内容の登録
	if(!D3DDevice::getInputElement("FIELD"))
	{
		D3DDevice::addInputElement("FIELD",layout);
	}

#else
	//頂点データの設定
	static const GLLayout layout[] = 
	{
		"POSITION",3,GL_FLOAT,
		"NORMAL",3,GL_FLOAT,
		"TANGENT",3,GL_FLOAT,
		"BINORMAL",3,GL_FLOAT,
		"TEXCOORD",2,GL_FLOAT,
		"TEXCOORD_",2,GL_FLOAT,

	};
	Vertex::addLayout("FIELD",layout,sizeof(layout)/sizeof(GLLayout));

#endif
}

FieldUnit::FieldUnit()
{
	_init();

	m_frame = SP<Frame>(NEW Frame());
	Mesh mesh;
	m_frame->add(&mesh);
	m_mesh = m_frame->getMesh();

	m_startPoint.x = -100;
	m_startPoint.y = -100;
	m_tipsSize.cx = 32;
	m_tipsSize.cy = 32;
	m_drawSize.cx = 60;
	m_drawSize.cy = 60;
	m_unitSize.x = 256.0f;
	m_unitSize.y = 256.0f;
}
bool FieldUnit::openTexture(LPCSTR fileName,LPCSTR fileName2)
{
	Texture* texture = NEW Texture();
	texture->open(fileName);
	m_mesh->setTexture(texture);
	
	if(fileName2)
	{
		texture = NEW Texture();
		texture->open(fileName2);
		m_mesh->addTexture(texture);
	}

	m_mesh->createLayout("FIELD","FIELD");
	m_mesh->setPShader("FIELD");
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
void createTB(VERTEXFIELD* vertexField)
{
	NVector v0[3] =
	{
		vertexField[0].position.x,vertexField[0].u2,vertexField[0].v2,0.0f,
		vertexField[0].position.y,vertexField[0].u2,vertexField[0].v2,0.0f,
		vertexField[0].position.z,vertexField[0].u2,vertexField[0].v2,0.0f,
	};
	NVector v1[3] =
	{
		vertexField[1].position.x,vertexField[1].u2,vertexField[1].v2,0.0f,
		vertexField[1].position.y,vertexField[1].u2,vertexField[1].v2,0.0f,
		vertexField[1].position.z,vertexField[1].u2,vertexField[1].v2,0.0f,
	};
	NVector v2[3] =
	{
		vertexField[2].position.x,vertexField[2].u2,vertexField[2].v2,0.0f,
		vertexField[2].position.y,vertexField[2].u2,vertexField[2].v2,0.0f,
		vertexField[2].position.z,vertexField[2].u2,vertexField[2].v2,0.0f,
	};
	float U[ 3 ], V[ 3 ];
	for ( int i = 0; i < 3; ++i )
	{
		NVector V1 = v1[ i ] - v0[ i ];
		NVector V2 = v2[ i ] - v1[ i ];
		NVector ABC;
		ABC = V1.cross(V2);

		if ( ABC.x != 0.0f )
		{
			U[ i ] = - ABC.y / ABC.x;
			V[ i ] = - ABC.z / ABC.x;
		}
	}
	NVector u;
	u = *(NVector3*)U;
	NVector v;
	v = *(NVector3*)V;
	u = u.normal3();
	v = v.normal3();
	int i;
	for(i=0;i<3;i++)
	{
		vertexField[i].tangent = u;
		vertexField[i].binormal = v;
	}
}
void FieldUnit::setVertexField(INT x,INT y,FLOAT px,FLOAT py,FLOAT u,FLOAT v,FLOAT uu,FLOAT vv,INT imageWidthCount)
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
	FLOAT u1 = tipsIndexX * u + 0.01f;
	FLOAT u2 = (tipsIndexX+1) * uu - 0.01f;
	FLOAT v1 = tipsIndexY * v + 0.01f;
	FLOAT v2 = (tipsIndexY+1) * vv - 0.01f;

	FLOAT h11 = m_fieldData.getFieldHeight(x-1,y-1);
	FLOAT h12 = m_fieldData.getFieldHeight(x+0,y-1);
	FLOAT h13 = m_fieldData.getFieldHeight(x+1,y-1);
	FLOAT h21 = m_fieldData.getFieldHeight(x-1,y+0);
	FLOAT h22 = m_fieldData.getFieldHeight(x+0,y+0);
	FLOAT h23 = m_fieldData.getFieldHeight(x+1,y+0);
	FLOAT h31 = m_fieldData.getFieldHeight(x-1,y+1);
	FLOAT h32 = m_fieldData.getFieldHeight(x+0,y+1);
	FLOAT h33 = m_fieldData.getFieldHeight(x+1,y+1);

	NVector n1 = {0,0,0,0};
	NVector n2 = {128.0f,128.0f,0,0};
	NVector n3 = {0,256.0f,0,0};


	n1.z = h11;
	n2.z = vertexField[0].position.z;
	n3.z = h21;
	vertexField[0].normal = (n2-n1).cross(n3-n1).normal3();
	vertexField[0].position.x = px;
	vertexField[0].position.y = py;
	vertexField[0].position.z = h22;
	vertexField[0].u = u1;
	vertexField[0].v = v1;
	vertexField[0].u2 = 0.0f;
	vertexField[0].v2 = 0.0f;

	n1.z = h12;
	n2.z = vertexField[1].position.z;
	n3.z = h22;
	vertexField[1].normal = (n2-n1).cross(n3-n1).normal3();
	vertexField[1].position.x = px+unitWidth;
	vertexField[1].position.y = py;
	vertexField[1].position.z = h23;
	vertexField[1].u = u2;
	vertexField[1].v = v1;
	vertexField[1].u2 = 1.0f;
	vertexField[1].v2 = 0.0f;

	n1.z = h21;
	n2.z = vertexField[2].position.z;
	n3.z = h31;
	vertexField[2].normal = (n2-n1).cross(n3-n1).normal3();	
	vertexField[2].position.x = px;
	vertexField[2].position.y = py+unitHeight;
	vertexField[2].position.z = h32;
	vertexField[2].u = u1;
	vertexField[2].v = v2;
	vertexField[2].u2 = 0.0f;
	vertexField[2].v2 = 1.0f;

	n1.z = h22;
	n2.z = vertexField[3].position.z;
	n3.z = h32;
	vertexField[3].normal = (n2-n1).cross(n3-n1).normal3();	
	vertexField[3].position.x = px+unitWidth;
	vertexField[3].position.y = py+unitHeight;
	vertexField[3].position.z = h33;
	vertexField[3].u = u2;
	vertexField[3].v = v2;
	vertexField[3].u2 = 1.0f;
	vertexField[3].v2 = 1.0f;

	vertexField[4] = vertexField[2];
	vertexField[5] = vertexField[1];

	createTB(vertexField);
	createTB(vertexField+3);
}

void FieldUnit::getPointVector(NVector2& vect,FLOAT x,FLOAT y)
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

	FLOAT baseHeight,xHeight,yHeight;
	if(fx <= (1.0-fy))
	{
		baseHeight = m_fieldData.getFieldHeight(iX,iY);
		xHeight = m_fieldData.getFieldHeight(iX+1,iY) - baseHeight;
		yHeight = m_fieldData.getFieldHeight(iX,iY+1) - baseHeight;
	}
	else
	{
		baseHeight =  m_fieldData.getFieldHeight(iX+1,iY+1);
		xHeight = baseHeight - m_fieldData.getFieldHeight(iX,iY+1);
		yHeight = baseHeight - m_fieldData.getFieldHeight(iX+1,iY);
	}
	vect.x = xHeight / unitWidth;
	vect.y = yHeight / unitHeight;
}

bool FieldUnit::create(INT width,INT height,INT high,INT low)
{
	INT indexCount = m_drawSize.cx * m_drawSize.cy;
	m_fieldData.create(width,height,high,low);
	m_vertexField.resize(((width+2)*(height+2)+1)*6);
	return build();
}
bool FieldUnit::build()
{
	Mesh* mesh = m_mesh;
	INT width = m_fieldData.getWidth();
	INT height = m_fieldData.getHeight();
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;
	INT tipsWidth = m_tipsSize.cx;
	INT tipsHeight = m_tipsSize.cy;
	Material* material = m_mesh->getMaterial();
	INT imageWidth = mesh->getTexture()->getImageWidth();
	INT imageHeight = mesh->getTexture()->getImageHeight();
	INT imageWidthCount = (INT)(imageWidth / tipsWidth);
	FLOAT u = (FLOAT)tipsWidth / imageWidth;
	FLOAT v = (FLOAT)tipsHeight / imageHeight;
	FLOAT u2 = (FLOAT)(tipsWidth-1) / imageWidth;
	FLOAT v2 = (FLOAT)(tipsHeight-1) / imageHeight;

	INT i,j;
	for(j=-1;j<=height;j++)
	{
		FLOAT py = unitHeight*j;
		for(i=-1;i<=width;i++)
		{
			FLOAT px = unitWidth*i;
			setVertexField(i,j,px,py,u,v,u2,v2,imageWidthCount);
		}
	}
	setVertexField(-5,-5,0,0,u,v,u2,v2,imageWidthCount);

	m_init = true;
	m_build = false;
	return true;
}



bool FieldUnit::onRender(LPVOID data,FLOAT& x,FLOAT& y,FLOAT& z)
{
	INT i,j,k;

	World* world = (World*)data;

	FLOAT drawPointX = 0.0f;
	FLOAT drawPointY = 0.0f;
	INT drawWidth = m_drawSize.cx;
	INT drawHeight = m_drawSize.cy;
	FLOAT unitWidth = m_unitSize.x;
	FLOAT unitHeight = m_unitSize.y;

	Camera3D* camera = (Camera3D*)world->getCamera();
	if(camera)
	{
		NVector vect = {0.0f,1.0f,0.0f,0.0f};
		vect = vect.transformNormal(camera->getView());
		vect = vect.normal();

		float pointX = camera->getX();
		float pointY = camera->getY();
		drawPointX = pointX + vect.x * unitWidth * drawWidth*0.4f;
		drawPointY = pointY + vect.y * unitHeight * drawHeight*0.4f;
	}

	INT startX = (INT)(drawPointX/unitWidth) - drawWidth/2;
	INT startY = (INT)(drawPointY/unitHeight) - drawHeight/2;

	if(m_build)
		build();

	if(!m_init && m_startPoint.x == startX && m_startPoint.y == startY)
	{
		return true;
	}
	m_init = false;
	m_startPoint.x = startX;
	m_startPoint.y = startY;

	INT width = m_fieldData.getWidth();
	INT height = m_fieldData.getHeight();

	VERTEXFIELD vertexWork[6];
//	VERTEXFIELD* vertexField = (VERTEXFIELD*)m_mesh->lockVertex();

	char* dest = new char[m_drawSize.cx * m_drawSize.cy * 6 * sizeof(VERTEXFIELD)];
	VERTEXFIELD* vertexField = (VERTEXFIELD*)dest;

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
//	m_mesh->unlockVertex();
	m_mesh->createVertex(dest,m_drawSize.cx * m_drawSize.cy * 6 * sizeof(VERTEXFIELD));
	delete[] dest;
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
FLOAT FieldUnit::getPartsWidth() const
{
	return m_unitSize.x;
}
FLOAT FieldUnit::getPartsHeight() const
{
	return m_unitSize.y;
}
INT FieldUnit::getDataWidth()
{
	return m_fieldData.getWidth();
}
INT FieldUnit::getDataHeight()
{
	return m_fieldData.getHeight();
}
void FieldUnit::setFieldHeight(INT x,INT y,FLOAT height)
{
	m_build = true;
	m_fieldData.setFieldHeight(x,y,height);
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
void FieldUnit::setOutHeight(FLOAT height)
{
	m_fieldData.setOutHeight(height);
	m_init = true;
}
void FieldUnit::setOutIndex(INT index)
{
	m_fieldData.setOutIndex(index);
	m_build = true;
}
void FieldUnit::clear(INT index)
{
	m_fieldData.clear(index);
	m_build = true;
}
INT FieldUnit::getFieldIndex(INT x,INT y)
{
	
	return m_fieldData.getFieldIndex(x,y);
}
void FieldUnit::setFieldIndex(INT x,INT y,INT index)
{
	m_fieldData.setFieldIndex(x,y,index);
	m_init = true;
}
bool FieldUnit::onDeviceRestore()
{
	m_init = true;
	return true;
}


FieldWater::FieldWater()
{
}
bool FieldWater::onRender(LPVOID data,FLOAT& x,FLOAT& y,FLOAT& z)
{
#if !defined(_OPENGL) & !defined(__ANDROID__)
	float noize = ((FLOAT)GetTickCount())*0.0001f;
	NVector v;
	v = noize;
	VertexShader* vs = D3DDevice::getVShader("FIELD_W");
	vs->update("ConstantBuffer3",&v);
#endif
	return FieldUnit::onRender(data, x, y,z);
}
bool FieldWater::openTexture(LPCSTR fileName)
{
	Texture* texture = NEW Texture();
	texture->open(fileName);
	m_mesh->setTexture(texture);

	texture = NEW Texture();
	texture->open("p4.jpg");
	m_mesh->addTexture(texture);

	m_mesh->createLayout("FIELD_W","FIELD");
	m_mesh->setPShader("FIELD_W");

	return true;
}

}
