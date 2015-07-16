#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DUNITCUSTOM

#include "aflDirect3DUnit.h"


namespace AFL{namespace DIRECT3D{

enum VECTOR_COMMAND
{
	VECTOR_LINE,
	VECTOR_BOX,
	VECTOR_TRIANGLE,
	VECTOR_QUADRANGLE,
};
struct VectorData
{
	VECTOR_COMMAND cmd;
	FLOAT x,y,z;
	FLOAT x2,y2,z2;
	FLOAT x3,y3,z3;
	FLOAT x4,y4,z4;
	FLOAT width;
	FLOAT height;
	FLOAT depth;
	DWORD color1;
	DWORD color2;
	FLOAT bold;
};

#define D3DFVF_VERTEXVECTOR (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define D3DFVF_VERTEXVECTOR3D (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)
struct VERTEXVECTOR
{
	bool operator<(const VERTEXVECTOR& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXVECTOR)) < 0;
	}
	bool operator==(const VERTEXVECTOR& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXVECTOR)) == 0;
	}
	NVector3 vectPosition;	// オブジェクト座標
	DWORD dwColor;			// 色情報			        
};


struct VERTEXVECTOR3D
{
	bool operator=(const VERTEXVECTOR& vt) 
	{
		vectPosition = vt.vectPosition;
		dwColor = vt.dwColor;
		return true;
	}
	bool operator<(const VERTEXVECTOR3D& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXVECTOR3D)) < 0;
	}
	bool operator==(const VERTEXVECTOR3D& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXVECTOR3D)) == 0;
	}
	NVector vectPosition;	// オブジェクト座標
	NVector vectNormal;
	DWORD dwColor;			// 色情報			        
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VectorObject
// DirectX - 図形用頂点データ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class VectorObject
{
public:
	VectorObject();
	void drawBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height,DWORD color);
	void drawLine(FLOAT x,FLOAT y,FLOAT x2,FLOAT y2,DWORD color,FLOAT bold=1.0f);
	void drawLineBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height,DWORD color,FLOAT bold=1.0f);
	void drawTriangle(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT x3,FLOAT y3,DWORD color);
	INT getVertexCount() const;
	VERTEXVECTOR* getVertexData();
	void add(VectorObject* vo);
protected:
	std::list<VectorData> m_vectorData;
};
class VectorObject3D : public VectorObject
{
public:
	void drawQuadrangle(FLOAT x,FLOAT y,FLOAT z,FLOAT x2,FLOAT y2,FLOAT z2,DWORD color,FLOAT bold=1.0f);
	void drawLine(FLOAT x,FLOAT y,FLOAT z,FLOAT x2,FLOAT y2,FLOAT z2,DWORD color,FLOAT bold=1.0f);
	void drawTriangle(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3,DWORD color);
	void drawLineBox(FLOAT x,FLOAT y,FLOAT z,FLOAT width,FLOAT height,FLOAT depth,DWORD color,FLOAT bold=1.0f);
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVector
// DirectX - 図形用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitVector : public Unit
{
public:
	UnitVector();
	virtual ~UnitVector(){}
	void create(VectorObject* vo);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
protected:
	VectorObject m_vectorObject;
	bool m_flag3d;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitFPS
// DirectX - FPS表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitFPS : public UnitText
{
public:
	UnitFPS();
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	INT m_count;
	INT m_time;
	INT m_fps;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - チップ型2Dマップ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitMap : public Unit
{
public:
	UnitMap();
	~UnitMap();
	void setPartSize(INT width,INT height);
	void setDrawPoint(INT x,INT y);
	void setViewSize(INT x,INT y);
	INT getDrawPointX() const;
	INT getDrawPointY() const;
	void setMapSize(INT width,INT height);
	void setMapIndex(INT x,INT y,INT index);
	INT getMapIndex(INT x,INT y);
	bool openImage(LPCSTR fileName);
	bool openImage(LPCWSTR fileName);
	void setVertex(VERTEXSPRITE* vertex,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT tx1,FLOAT ty1,FLOAT tx2,FLOAT ty2);
	Texture* getTexture() const;
	bool setVertex();
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
protected:
	Mesh* m_spriteMesh;
	INT m_partWidth;
	INT m_partHeight;
	LPBYTE m_map;
	INT m_viewWidth;
	INT m_viewHeight;
	INT m_mapWidth;
	INT m_mapHeight;

	INT m_drawX;
	INT m_drawY;

};


}}

#define __INC_AFLDIRECT3DUNITCUSTOM
#endif