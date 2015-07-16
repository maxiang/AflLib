#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DUNITCUSTOM

#include "aflD3DUnit.h"


namespace AFL{



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - チップ型2Dマップ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitMap : public Unit
{
	struct MAPVERTEX
	{
		NVector2 pos;
		NVector2 tex;
	};
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
	void setVertex(MAPVERTEX* vertex,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT tx1,FLOAT ty1,FLOAT tx2,FLOAT ty2);
	Texture* getTexture() const;
	bool setVertex();
	virtual bool onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z);
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


}

#define __INC_AFLDIRECT3DUNITCUSTOM
#endif