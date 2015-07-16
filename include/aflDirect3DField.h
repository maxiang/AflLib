#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DFIELD

#include "aflDirect3DUnit.h"

namespace AFL{namespace DIRECT3D{


#define D3DFVF_VERTEXFIEALD (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
struct VERTEXFIELD
{
	NVector3 position;	// オブジェクト座標
	NVector3 normal;	// 法線
	FLOAT u,v;			// テクスチャ座標
};

class FieldData
{
public:
	FieldData();
	bool create(INT width,INT height);
	FLOAT getFieldHeight(INT x,INT y);
	void setFieldHeight(INT x,INT y,FLOAT height);
	INT getFieldIndex(INT x,INT y);
	void setFieldIndex(INT x,INT y,INT index);
	INT getWidth()const{return m_fieldSize.cx;}
	INT getHeight()const{return m_fieldSize.cy;}
	DWORD getFieldFlag(INT x,INT y);
	void setFieldFlag(INT x,INT y,DWORD flag);

protected:
	SIZE m_fieldSize;
	INT m_outIndex;
	FLOAT m_outHeight;
	std::vector<FLOAT> m_fieldHeight;
	std::vector<INT> m_fieldIndex;
	std::vector<INT> m_fieldFlag;
};

class FieldUnit : public Unit
{
public:
	FieldUnit();
	bool create(INT width,INT height);
	bool openTexture(LPCSTR fileName);
	FLOAT getPointHeight(FLOAT x,FLOAT y);
	FLOAT getPartsWidth()const{return m_unitSize.x;}
	FLOAT getPartsHeight()const{return m_unitSize.y;}
	INT getDataWidth(){return m_fieldData.getWidth();}
	INT getDataHeight(){return m_fieldData.getHeight();}

	FLOAT getFieldWidth();
	FLOAT getFieldHeight();
	DWORD getFieldPointFlag(FLOAT x,FLOAT y);
	void setFieldPointFlag(FLOAT x,FLOAT y,DWORD flag);
	void setFieldSide(PFLOAT dx,PFLOAT dy,FLOAT sx,FLOAT sy);
	INT getFieldIndex(INT x,INT y);
	void setFieldIndex(INT x,INT y,INT index);
	FLOAT getFieldHeight(INT x,INT y);
	void setFieldHeight(INT x,INT y,FLOAT height);
protected:
	void createVertex();
	bool onDeviceRestore();

	VERTEXFIELD* getVertexField(INT x,INT y);
	void setVertexField(INT x,INT y,FLOAT px,FLOAT py,FLOAT u,FLOAT v,INT imageWidthCount);
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);

	POINT m_startPoint;
	SIZE m_tipsSize;
	SIZE m_drawSize;
	D3DVECTOR m_unitSize;
	FLOAT m_sphereFactor;
	Mesh* m_mesh;
	FieldData m_fieldData;
	std::vector<VERTEXFIELD> m_vertexField;
	bool m_initVertex;
};

}}

#define __INC_AFLDIRECT3DFIELD
#endif
