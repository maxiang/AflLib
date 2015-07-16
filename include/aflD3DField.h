#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DFIELD

#include "aflD3DUnit.h"

namespace AFL{


struct VERTEXFIELD
{
	NVector3 position;	// オブジェクト座標
	NVector3 normal;	// 法線
	NVector3 tangent;	
	NVector3 binormal;
	FLOAT u,v;			// テクスチャ座標
	FLOAT u2,v2;			// テクスチャ座標
};

class FieldData
{
public:
	FieldData();
	bool create(INT width,INT height,INT high,INT low);
	FLOAT getFieldHeight(INT x,INT y);
	INT getFieldIndex(INT x,INT y);
	INT getWidth()const{return m_fieldSize.cx;}
	INT getHeight()const{return m_fieldSize.cy;}
	DWORD getFieldFlag(INT x,INT y);
	void setFieldFlag(INT x,INT y,DWORD flag);
	void setOutIndex(INT index);
	void setOutHeight(FLOAT height);
	void clear(INT index);
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
	bool create(INT width,INT height,INT high=80,INT low=40);
	bool openTexture(LPCSTR fileName,LPCSTR fileName2=NULL);
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
	void setOutHeight(FLOAT height);
	void setOutIndex(INT index);
	void clear(INT index);
protected:
	bool onDeviceRestore();
	void _init();
	bool build();
	VERTEXFIELD* getVertexField(INT x,INT y);
	void setVertexField(INT x,INT y,FLOAT px,FLOAT py,FLOAT u,FLOAT v,FLOAT u2,FLOAT v2,INT imageWidthCount);
	bool onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z);

	bool m_build;
	bool m_init;
	POINT m_startPoint;
	SIZE m_tipsSize;
	SIZE m_drawSize;
	NVector m_unitSize;
	Mesh* m_mesh;
	FieldData m_fieldData;
	std::vector<VERTEXFIELD> m_vertexField;
};
class FieldWater : public FieldUnit
{
public:
	FieldWater();
	bool FieldWater::openTexture(LPCSTR fileName);

protected:
};
}
#define __INC_AFLDIRECT3DFIELD
#endif
