#include <windows.h>
#include <tchar.h>

#include "aflDirect3DUnitCustom.h"

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

namespace AFL{namespace DIRECT3D{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VectorObject
// DirectX - 図形用頂点データ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
VectorObject::VectorObject()
{
}
void VectorObject::drawBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height,DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_BOX;
	vd.x = x;
	vd.y = y;
	vd.z = 0.0f;
	vd.width = width;
	vd.height = height;
	vd.depth = 0.0f;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}
void VectorObject::drawLine(FLOAT x,FLOAT y,FLOAT x2,FLOAT y2,DWORD color,FLOAT bold)
{
	VectorData vd;
	vd.cmd = VECTOR_LINE;
	if(((x-x2) - (y-y2)) < 0)
	{
		vd.x = x;
		vd.y = y;
		vd.z = 0.0f;
		vd.x2 = x2;
		vd.y2 = y2;
		vd.z2 = 0.0f;
	}
	else
	{
		vd.x = x2;
		vd.y = y2;
		vd.z = 0.0f;
		vd.x2 = x;
		vd.y2 = y;
		vd.z2 = 0.0f;
	}
	vd.color1 = color;
	vd.bold = bold;
	m_vectorData.push_back(vd);
}

void VectorObject::drawTriangle(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT x3,FLOAT y3,DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x1;
	vd.y = y1;
	vd.z = 0.0f;
	vd.x2 = x2;
	vd.y2 = y2;
	vd.z2 = 0.0f;
	vd.x3 = x3;
	vd.y3 = y3;
	vd.z3 = 0.0f;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}
void VectorObject::drawLineBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height,DWORD color,FLOAT bold)
{
	drawLine(x,y,x+width-bold,y,color,bold);
	drawLine(x,y+bold,x,y+height-bold,color,bold);
	drawLine(x+width-bold,y+bold,x+width-bold,y+height-bold,color,bold);
	drawLine(x+bold,y+height-bold,x+width-bold,y+height-bold,color,bold);
}
INT VectorObject::getVertexCount() const
{
	INT count = 0;
	std::list<VectorData>::const_iterator it;
	for(it=m_vectorData.begin();it!=m_vectorData.end();++it)
	{
		switch(it->cmd)
		{
		case VECTOR_TRIANGLE:
			count += 3;
			break;
		case VECTOR_LINE:
			if(it->x!=it->x2 && it->y!=it->y2)
			{
				count += 12;
				break;
			}
		case VECTOR_QUADRANGLE:
		case VECTOR_BOX:
			count += 6;
			break;
		}
	}
	return count;
}
VERTEXVECTOR* VectorObject::getVertexData()
{
	INT index = 0;
	INT vcount = getVertexCount();
	VERTEXVECTOR* vv = NEW VERTEXVECTOR[vcount];


	std::list<VectorData>::const_iterator it;
	for(it=m_vectorData.begin();it!=m_vectorData.end();++it)
	{
		switch(it->cmd)
		{
		case VECTOR_TRIANGLE:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x2;
			vv[index].vectPosition.y = it->y2;
			vv[index].vectPosition.z = it->z2;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x3;
			vv[index].vectPosition.y = it->y3;
			vv[index].vectPosition.z = it->z3;
			vv[index].dwColor = it->color1;
			index++;
			break;
		case VECTOR_BOX:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;

			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			break;
		case VECTOR_QUADRANGLE:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x2;
			vv[index].vectPosition.y = it->y2;
			vv[index].vectPosition.z = it->z2;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x4;
			vv[index].vectPosition.y = it->y4;
			vv[index].vectPosition.z = it->z4;
			vv[index].dwColor = it->color1;
			index++;

			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x4;
			vv[index].vectPosition.y = it->y4;
			vv[index].vectPosition.z = it->z4;
			vv[index].dwColor = it->color1;
			index++;
			vv[index].vectPosition.x = it->x3;
			vv[index].vectPosition.y = it->y3;
			vv[index].vectPosition.z = it->z3;
			vv[index].dwColor = it->color1;
			index++;
			break;
		case VECTOR_LINE:
			{
				FLOAT x1 = it->x;
				FLOAT y1 = it->y;
				FLOAT z1 = it->z;
				FLOAT x2 = it->x2;
				FLOAT y2 = it->y2;
				FLOAT z2 = it->z2;
				FLOAT width = x2 - x1;
				FLOAT height = y2 - y1;
				FLOAT bold = it->bold;

				if(width==0 || height==0)
				{
					if(x1 > x2)
						x1 += bold;
					else
						x2 += bold;
					if(y1 > y2)
						y1 += bold;
					else
						y2 += bold;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z2;
					vv[index].dwColor = it->color1;
					index++;
					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].dwColor = it->color1;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].dwColor = it->color1;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].dwColor = it->color1;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].dwColor = it->color1;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z1;
					vv[index].dwColor = it->color1;
					index++;
				}
				else
				{
					if(x1 > x2)
					{
						x1 = it->x2;
						y1 = it->y2;
						x2 = it->x;
						y2 = it->y;
					}
					if(y1 > y2)
					{
						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2;
						vv[index].vectPosition.y = y2;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x1+bold;
						vv[index].vectPosition.y = y1+bold;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x1+bold;
						vv[index].vectPosition.y = y1+bold;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1+bold;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
					}
					else
					{
						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x1+bold;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2+bold;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;

						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x2;
						vv[index].vectPosition.y = y2+bold;
						vv[index].vectPosition.z = z2;
						vv[index].dwColor = it->color1;
						index++;
						vv[index].vectPosition.x = x1;
						vv[index].vectPosition.y = y1+bold;
						vv[index].vectPosition.z = z1;
						vv[index].dwColor = it->color1;
						index++;
					}
				}

			}
			break;
		}
	}
	return vv;
}
void VectorObject::add(VectorObject* vo)
{
	m_vectorData.insert(m_vectorData.end(),vo->m_vectorData.begin(),vo->m_vectorData.end());
}
void VectorObject3D::drawTriangle(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3,DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x1;
	vd.y = y1;
	vd.z = z1;
	vd.x2 = x2;
	vd.y2 = y2;
	vd.z2 = z2;
	vd.x3 = x3;
	vd.y3 = y3;
	vd.z3 = z3;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}

void VectorObject3D::drawQuadrangle(FLOAT x,FLOAT y,FLOAT z,FLOAT x2,FLOAT y2,FLOAT z2,DWORD color,FLOAT bold)
{
	//VectorData vd;
	//vd.cmd = VECTOR_BOX;

	NVector v;
	v.x = x - x2;
	v.y = y - y2;
	v.z = z - z2;
	v = v.normal3();

	FLOAT b;
	b = bold*0.5f;
	v *= b;

	VectorData vd;


	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x;
	vd.y = y;
	vd.z = z;
	vd.x2 = x-v.y;
	vd.y2 = y+v.x;
	vd.z2 = z;
	vd.x3 = x-v.z;
	vd.y3 = y;
	vd.z3 = z+v.x;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x;
	vd.y = y;
	vd.z = z;
	vd.x2 = x-v.y;
	vd.y2 = y+v.x;
	vd.z2 = z;
	vd.x3 = x;
	vd.y3 = y-v.z;
	vd.z3 = z+v.y;
	vd.color1 = color;
	m_vectorData.push_back(vd);
/*
	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x-v.y;
	vd.y = y+v.x;
	vd.z = z;
	vd.x2 = x+v.y;
	vd.y2 = y-v.x;
	vd.z2 = z;
	vd.x3 = x2-v.y;
	vd.y3 = y2+v.x;
	vd.z3 = z2;
	vd.x4 = x2+v.y;
	vd.y4 = y2-v.x;
	vd.z4 = z2;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x-v.z;
	vd.y = y;
	vd.z = z+v.x;
	vd.x2 = x+v.z;
	vd.y2 = y;
	vd.z2 = z-v.x;
	vd.x3 = x2-v.z;
	vd.y3 = y2;
	vd.z3 = z2+v.x;
	vd.x4 = x2+v.z;
	vd.y4 = y2;
	vd.z4 = z2-v.x;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x;
	vd.y = y-v.z;
	vd.z = z+v.y;
	vd.x2 = x;
	vd.y2 = y+v.z;
	vd.z2 = z-v.y;
	vd.x3 = x2;
	vd.y3 = y2-v.z;
	vd.z3 = z2+v.y;
	vd.x4 = x2;
	vd.y4 = y2+v.z;
	vd.z4 = z2-v.y;
	vd.color1 = color;
	m_vectorData.push_back(vd);
	*/
}


void VectorObject3D::drawLine(FLOAT x,FLOAT y,FLOAT z,FLOAT x2,FLOAT y2,FLOAT z2,DWORD color,FLOAT bold)
{
	VectorData vd;
	vd.cmd = VECTOR_LINE;
	if(((x-x2) - (y-y2)) < 0)
	{
		vd.x = x;
		vd.y = y;
		vd.z = z;
		vd.x2 = x2;
		vd.y2 = y2;
		vd.z2 = z2;
	}
	else
	{
		vd.x = x2;
		vd.y = y2;
		vd.z = z2;
		vd.x2 = x;
		vd.y2 = y;
		vd.z2 = z;
	}
	vd.color1 = color;
	vd.bold = bold;
	m_vectorData.push_back(vd);
}

void VectorObject3D::drawLineBox(FLOAT x,FLOAT y,FLOAT z,FLOAT width,FLOAT height,FLOAT depth,DWORD color,FLOAT bold)
{
	drawLine(x,y,z,x+width-bold,y,z,color,bold);
	drawLine(x,y+bold,z, x,y+height-bold,z, color,bold);
	drawLine(x+width-bold,y+bold,z,x+width-bold,y+height-bold,z,color,bold);
	drawLine(x+bold,y+height-bold,z,x+width-bold,y+height-bold,z,color,bold);

	drawLine(x,y,z+depth,x+width-bold,y,z+depth,color,bold);
	drawLine(x,y+bold,z+depth, x,y+height-bold,z+depth, color,bold);
	drawLine(x+width-bold,y+bold,z+depth,x+width-bold,y+height-bold,z+depth,color,bold);
	drawLine(x+bold,y+height-bold,z+depth,x+width-bold,y+height-bold,z+depth,color,bold);

	drawLine(x,y,z,x,y,z+depth,color,bold);
	drawLine(x,y+height-bold,z, x,y+height-bold,z+depth, color,bold);
	drawLine(x+width-bold,y,z,x+width-bold,y+height-bold,z+depth,color,bold);
	//drawLine(x+bold,y+height-bold,z+depth,x+width-bold,y+height-bold,z+depth,color,bold);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVector
// DirectX - 図形用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitVector::UnitVector()
{
	setView2D(true);
	//setZBuffer(false);
	setZSort(false);
	//setLight(false);
	m_flag3d = true;
}
void UnitVector::create(VectorObject* vo)
{
	resetRenderFlag();
	m_vectorObject = *vo;
}
bool UnitVector::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag())
		return Unit::onRender(world,x,y,z);
	INT vcount = m_vectorObject.getVertexCount();
	VERTEXVECTOR* vv = m_vectorObject.getVertexData();

	WORD* index = NEW WORD[vcount];


	INT i;
	if(m_flag3d)
	{
		VERTEXVECTOR3D* v = NEW VERTEXVECTOR3D[vcount];

		std::map<VERTEXVECTOR3D,INT>::iterator it;
		std::map<VERTEXVECTOR3D,INT> opMap;
		for(i=0;i<vcount;i++)
		{
			//法線生成
			v[i] = vv[i];
			if(i%3 == 2)
			{
				NVector vec1, vec2;
				vec1 = v[i-1].vectPosition - v[i-2].vectPosition;
				vec2 = v[i-0].vectPosition - v[i-1].vectPosition;
				vec1 = vec1.cross(vec2).normal();

				v[i-2].vectNormal = vec1;
				v[i-1].vectNormal = vec1;
				v[i-0].vectNormal = vec1;
			}
		}
		for(i=0;i<vcount;i++)
		{
			it = opMap.find(v[i]);
			if(it == opMap.end())
			{
				index[i] = (INT)opMap.size();
				opMap[v[i]] = index[i];
			}
			else
			{
				index[i] = it->second;
			}
		}

		INT vcount2 = (INT)opMap.size();
		VERTEXVECTOR3D* vv2 = NEW VERTEXVECTOR3D[vcount2];
		for(it=opMap.begin();it!=opMap.end();++it)
		{
			vv2[it->second] = it->first;
		}

		//メッシュの作成
		Mesh mesh;
		mesh.createVertexBuffer(vv2,vcount2,sizeof(VERTEXVECTOR3D),D3DFVF_VERTEXVECTOR3D);
		mesh.createIndexBuffer(index,vcount);

		COLOR4 c(0.5f,0.5f,0.5f,1.0f);
		COLOR4 c2(0.2f,0.2f,0.2f,1.0f);
		mesh.getMaterial()->Diffuse = c;
		mesh.getMaterial()->Ambient = c;
		mesh.getMaterial()->Emissive = c2;

		//フレームの作成
		m_frame = SP<Frame>(NEW Frame());
		m_frame->add(&mesh);
		delete[] vv2;
	}
	else
	{
		//頂点データから最適化インデックスの作成

		std::map<VERTEXVECTOR,INT>::iterator it;
		std::map<VERTEXVECTOR,INT> opMap;
		for(i=0;i<vcount;i++)
		{
			it = opMap.find(vv[i]);
			if(it == opMap.end())
			{
				index[i] = (INT)opMap.size();
				opMap[vv[i]] = index[i];
			}
			else
			{
				index[i] = it->second;
			}
		}
		INT vcount2 = (INT)opMap.size();
		VERTEXVECTOR* vv2 = NEW VERTEXVECTOR[vcount2];
		for(it=opMap.begin();it!=opMap.end();++it)
		{
			vv2[it->second] = it->first;
		}

		//メッシュの作成
		Mesh mesh;
		mesh.createVertexBuffer(vv2,vcount2,sizeof(VERTEXVECTOR),D3DFVF_VERTEXVECTOR);
		mesh.createIndexBuffer(index,vcount);
		//フレームの作成
		m_frame = SP<Frame>(NEW Frame());
		m_frame->add(&mesh);	
		delete[] vv2;
	}



	delete[] vv;

	delete[] index;
	return Unit::onRender(world,x,y,z);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitFPS
// DirectX - FPS表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitFPS::UnitFPS()
{
	setTextureFilter(D3DTEXF_LINEAR);
	setZBuffer(false);
	setPosW(2000);
	m_count = 0;
	m_time = 0;
	m_fps = 0;
	setTextColor(0xffffffff);
	setBackColor(0x80000000);
	m_font.setBold(1000);
}

bool UnitFPS::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	INT timeCount;
	m_count++;
	timeCount = GetTickCount() - m_time;
	setScaleX(1.0f-abs(timeCount-500)/800.0f);
	setScaleY(1.0f-abs(timeCount-500)/800.0f);
	setCenter(getImageWidth()/2.0f,getImageHeight()/2.0f,0);
	if(timeCount >= 1000)
	{
		INT fps;
		CHAR buff[100];
		fps = m_count*1000 / timeCount;
		if(m_fps != fps)
		{
			m_fps = fps;
			sprintf(buff,"%3d FPS",fps);
			setText(buff);
		}
		m_count = 0;
		m_time = GetTickCount();
	}
	return UnitText::onRender(world,x,y,z);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - チップ型2Dマップ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitMap::UnitMap()
{
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
void UnitMap::setPartSize(INT width,INT height)
{
	m_partWidth = width;
	m_partHeight = height;
	resetRenderFlag();
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
	//テクスチャの作成
	SP<Texture> texture = new Texture();
	if(!texture->openImage(fileName))
		return false;
	m_spriteMesh->setTexture(texture);
	resetRenderFlag();
	return true;
}
bool UnitMap::openImage(LPCWSTR fileName)
{
	//テクスチャの作成
	SP<Texture> texture = new Texture();
	if(!texture->openImage(fileName))
		return false;
	m_spriteMesh->setTexture(texture);
	//メッシュの作成
	resetRenderFlag();
	return true;
}
void UnitMap::setVertex(VERTEXSPRITE* vertex,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT tx1,FLOAT ty1,FLOAT tx2,FLOAT ty2)
{
	vertex[0].vectPosition.x = x1;
	vertex[0].vectPosition.y = y1;
	vertex[0].vectPosition.z = 0.0f;
	vertex[0].vectNormal.x = 0.0f;
	vertex[0].vectNormal.y = 0.0f;
	vertex[0].vectNormal.z = -1.0f;
	vertex[0].dwColor = 0xffffffff;
	vertex[0].fTu = tx1;
	vertex[0].fTv = ty1;

	vertex[1].vectPosition.x = x2;
	vertex[1].vectPosition.y = y1;
	vertex[1].vectPosition.z = 0.0f;
	vertex[1].vectNormal.x = 0.0f;
	vertex[1].vectNormal.y = 0.0f;
	vertex[1].vectNormal.z = -1.0f;
	vertex[1].dwColor = 0xffffffff;
	vertex[1].fTu = tx2;
	vertex[1].fTv = ty1;

	vertex[2].vectPosition.x = x1;
	vertex[2].vectPosition.y = y2;
	vertex[2].vectPosition.z = 0.0f;
	vertex[2].vectNormal.x = 0.0f;
	vertex[2].vectNormal.y = 0.0f;
	vertex[2].vectNormal.z = -1.0f;
	vertex[2].dwColor = 0xffffffff;
	vertex[2].fTu = tx1;
	vertex[2].fTv = ty2;

	vertex[3].vectPosition.x = x2;
	vertex[3].vectPosition.y = y2;
	vertex[3].vectPosition.z = 0.0f;
	vertex[3].vectNormal.x = 0.0f;
	vertex[3].vectNormal.y = 0.0f;
	vertex[3].vectNormal.z = -1.0f;
	vertex[3].dwColor = 0xffffffff;
	vertex[3].fTu = tx2;
	vertex[3].fTv = ty2;
}
Texture* UnitMap::getTexture() const
{
	if(m_spriteMesh)
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
	FLOAT txWidth = ((FLOAT)m_partWidth / width) * (textureWidth / width);
	FLOAT txHeight = ((FLOAT)m_partHeight / height) * (textureHeight / height);


	INT countX = m_viewWidth/m_partWidth+1;
	INT countY = m_viewHeight/m_partHeight+1;
	INT vertexCount = countX*countY*4;
	VERTEXSPRITE* vertexSprite = new VERTEXSPRITE[vertexCount];
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
	m_spriteMesh->createVertexBuffer(vertexSprite,vertexCount,sizeof(VERTEXSPRITE),D3DFVF_VERTEXSPRITE);
	delete[] vertexSprite;

	INT indexCount = countX*countY;
	m_spriteMesh->createIndexBuffer(indexCount*6);
	Index* idx = m_spriteMesh->getIndexBuffer();
	LPWORD indexData = (LPWORD)idx->lock();
	for(i=0;i<indexCount;i++)
	{
		indexData[i*6+0] = i*4+0;
		indexData[i*6+1] = i*4+1;
		indexData[i*6+2] = i*4+2;
		indexData[i*6+3] = i*4+3;
		indexData[i*6+4] = i*4+2;
		indexData[i*6+5] = i*4+1;
	}
	idx->unlock();
	return true;
}
bool UnitMap::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		setVertex();
	}
	return true;
}
}}
