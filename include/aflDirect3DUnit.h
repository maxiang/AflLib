#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DUNIT
#include <gdiplus.h>

#include "aflWinTool.h"
#include "aflImage.h"
#include "aflDirect3D.h"


namespace AFL{namespace DIRECT3D{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Unit
// ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Unit : public Object
{
public:
	Unit();
	virtual ~Unit();
#ifdef _AFL_D3DX
	bool openXFile(LPCSTR pcFileName,D3DXFILEFORMAT xfileFormat=_3DS_CONVERTER);
#endif
	bool openFile(LPCWSTR fileName);
	bool copyImage(Unit* unit);

	bool createShadow();

	INT getFrameCount();
	INT getMeshCount();
	INT getAllFrameCount();
	INT getAllUnitCount();
	INT getAllMeshCount();

	bool isViewClip() const;
	void setViewClip(bool flag);
	void setPosX(FLOAT x);
	void setPosY(FLOAT y);
	void setPosZ(FLOAT y);
	void setPosW(FLOAT y);
	void setPos(FLOAT x,FLOAT y);
	void setPos(FLOAT x,FLOAT y,FLOAT z);
	virtual void setPos(FLOAT x,FLOAT y,FLOAT z,FLOAT w);
	FLOAT getPosX() const;
	FLOAT getPosY() const;
	FLOAT getPosZ() const;
	FLOAT getPosW() const;
	FLOAT getAbsX();
	FLOAT getAbsY();
	FLOAT getAbsZ();
	FLOAT getAbsW();
	void setScaleX(FLOAT fScale);
	void setScaleY(FLOAT fScale);
	void setScaleZ(FLOAT fScale);
	void setScale(FLOAT fScale);
	FLOAT getScaleX();
	FLOAT getScaleY();
	FLOAT getScaleZ();

	FLOAT getCenterX() const;
	FLOAT getCenterY() const;
	FLOAT getCenterZ() const;
	void setCenter(FLOAT x,FLOAT y,FLOAT z=0.0f);
	void setRotationX(FLOAT fAngle);
	void setRotationY(FLOAT fAngle);
	void setRotationZ(FLOAT fAngle);
	FLOAT getRotationX();
	FLOAT getRotationY();
	FLOAT getRotationZ();
	void setRelativityX(FLOAT fPoint);
	void setRelativityY(FLOAT fPoint);
	void setRelativityZ(FLOAT fPoint);
	FLOAT getRelativityX();
	FLOAT getRelativityY();
	FLOAT getRelativityZ();
	void setClipX(FLOAT fClip);
	void setClipY(FLOAT fClip);
	void setClipZ(FLOAT fClip);
	FLOAT getClipX();
	FLOAT getClipY();
	FLOAT getClipZ();
	void setClipWidth(FLOAT fClip);
	void setClipHeight(FLOAT fClip);
	void setClipDepth(FLOAT fClip);
	FLOAT getClipWidth();
	FLOAT getClipHeight();
	FLOAT getClipDepth();

	bool isAnimation()const;
	bool isShadow()const;
	void setShadow(bool bShadow);
	bool isLight()const;
	void setLight(bool bLight);
	bool isView2D()const;
	void setView2D(bool bView);
	bool isVisible()const;
	void setVisible(bool bVisible);
	bool isZBuffer()const;
	void setZBuffer(bool bZBuffer);
	INT getBlendMode()const;
	void setBlendMode(INT iMode);
	void setLimit(bool bFlag);
	bool isLimit()const;
	void setZSort(bool bFlag);
	bool isZSort()const;
	void setChainW(bool flag);
	bool isChainW() const;
	void setChainClip(bool flag);
	bool isChainClip() const;
	void setAlpha(DWORD alpha);
	DWORD getAlpha() const;

	void del(Unit* unit);
	void add(Unit* unit);
	bool getAnimationMatrix(LPCSTR frameName,NMatrix* matrix);
	void addAnimationTime(DWORD count);
	void setAnimation(LPCSTR name,DWORD count=0,bool loop=true);
	LPCSTR getAnimationName();

	void createBoundingBox();
	void getBoundingBox(NVector* vect);
	void getBoundingBox(Frame* frame,NVector* vect);

	void setShader(bool flag){m_frame->setShader(flag);}

	virtual void onAction(World* world,LPVOID value);
	virtual void onStart(World* world,LPVOID value);
	virtual void onIdel(World* world,LPVOID value);
	virtual void onEnd(World* world,LPVOID value);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void onRenderEnd();

	void setMatrix(NMatrix* matrix,FLOAT x,FLOAT y,FLOAT z);
	std::list<Unit*>* getChilds();
	Frame* getFrame();
	Frame* getFrame(LPCSTR name);
	RECT* getViewClip(RECT* rect=NULL);
	void setViewClip(const RECT* rect);
	void setTextureFilter(D3DTEXTUREFILTERTYPE filter);
	D3DTEXTUREFILTERTYPE getTextureFilter() const;
	void resetRenderFlag(){m_renderFlag = false;}
	bool isRenderFlag()const{return m_renderFlag;}
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	bool createObject(FileObject* fileObject);
	Frame* readFrame(struct FrameData* frameData);
	void readMesh(Frame* frame,struct MeshData* meshData);
	void readMesh(Frame* frame,MESHOPTIMIZE* xfileMesh,bool shadow=false);
	bool optimizeMesh(MESHOPTIMIZE& meshOptimize,std::list<VERTEXBASE2>& listVertex);

	Unit* m_unitParent;
	std::list<Unit*> m_unitChilds;

	FLOAT m_w;
	RECT m_viewClip;
	D3DVECTOR m_point;
	D3DVECTOR m_scale;
	D3DVECTOR m_center;
	D3DVECTOR m_rotation;
	D3DVECTOR m_relativity;
	D3DVECTOR m_clipPoint;
	D3DVECTOR m_clipSize;
	bool m_shadow;
	bool m_zsort;
	bool m_zbuffer;
	bool m_view2D;
	bool m_visible;
	bool m_light;
	bool m_limit;
	bool m_clip;
	bool m_renderFlag;
	bool m_chainW;
	bool m_chainClip;
	INT m_blendMode;
	D3DTEXTUREFILTERTYPE m_textureFilter;
	DWORD m_alpha;

	std::list<Animation> m_anime;
	std::map<std::string,ANIMATIONSET> m_animationSet;
	SP<Frame> m_frame;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AflVertex
// 頂点管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define D3DFVF_VERTEXSPRITE (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
struct VERTEXSPRITE
{
	bool operator==(VERTEXSPRITE vt)
	{
		return memcmp(this,&vt,sizeof(VERTEXSPRITE)) == 0;
	}
	D3DVECTOR vectPosition;	// オブジェクト座標
	D3DVECTOR vectNormal;	// オブジェクト法線
	DWORD dwColor;			// 色情報			        
	FLOAT fTu,fTv;			// テクスチャ座標
};
enum
{
	POINT_LEFT = 1,
	POINT_RIGHT = 2,
	POINT_TOP = 4,
	POINT_BOTTOM = 8
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSprite
// 2D用スプライトユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class UnitSprite : public Unit
{
typedef struct tagGetVertex
{
	FLOAT fX1,fY1,fX2,fY2;
	FLOAT fTX,fTY,fTWidth,fTHeight;
}GETVERTEX,*PGETVERTEX,*LPGETVERTEX;

public:
	UnitSprite();
	bool createTarget(INT width,INT height,D3DFORMAT format=D3DFMT_A8R8G8B8);
	bool createImage(HWND hwnd);
	bool createImage(INT width,INT height,D3DFORMAT format=D3DFMT_A8R8G8B8,D3DPOOL pool=D3DPOOL_MANAGED);
	bool createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true,D3DFORMAT format=D3DFMT_A8R8G8B8);
	bool createText(LPCWSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true,D3DFORMAT format=D3DFMT_A8R8G8B8);
	bool createText(LPCSTR string,HFONT font,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true,D3DFORMAT format=D3DFMT_A8R8G8B8);
	bool createText(LPCWSTR string,HFONT font,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true,D3DFORMAT format=D3DFMT_A8R8G8B8);
	bool openImage(LPCSTR fileName);
	bool openImage(LPCWSTR fileName);

	Material* getMaterial() const;
	Texture* getTexture() const;
	IDirect3DSurface9* getSurface() const;
	INT getImageWidth()const;
	INT getImageHeight()const;
	HDC getDC() const;
	void releaseDC(HDC hdc) const;
	void setTarget() const;

	bool lock(D3DLOCKED_RECT* rect);
	bool unlock();
	D3DFORMAT getFormat() const;

	void drawText(INT x,INT y,LPCWSTR text,HFONT font,DWORD color,DWORD bcolor);
	bool clear(DWORD color);
	void setPartSize(INT width,INT height);
	INT getPartWidth()const;
	INT getPartHeight()const;
	INT getPartIndex()const;
	void setPartIndex(INT index);
protected:
	Mesh* getMesh() const;
	bool createFromTexture(Texture* texture);
	bool createFromTexture(SP<Texture>& texture);
	bool getPointVertex(PGETVERTEX vertexSprite,Texture* textureSrc);
	bool resetVertex();

	INT m_pointStat;
	FLOAT m_centerX,m_centerY;
	INT m_partWidth;
	INT m_partHeight;
	INT m_partIndex;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitText
// テキスト表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitText : public UnitSprite
{
public:
	UnitText();
	void setText(LPCSTR text);
	void setText(LPCWSTR text);
	void setTextColor(DWORD color);
	void setBackColor(DWORD color);
	void setFontSize(INT size);
	LPCSTR getText();
	WINDOWS::Font* getFont();
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);

	WINDOWS::Font m_font;
	DWORD m_color;
	DWORD m_bcolor;
	WString m_drawString;
	WString m_drawedString;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitDIB
// DIB表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitDIB : public UnitSprite
{
public:
	UnitDIB();
	void setSize(INT width,INT height);
	WINDOWS::DIB* getDIB();
	HDC getDC() const;
	void setRedraw();
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);

	bool m_redraw;
	WINDOWS::DIB m_dibImage;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GDIPlus
// GDIPlus初期化用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class GDIPlus
{
public:
	GDIPlus();
	~GDIPlus();
protected:
	ULONG_PTR m_token;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitGdip
// GDIPlus表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitGdip : public UnitSprite
{
	static GDIPlus m_gdiPlus;
public:
	UnitGdip();
	virtual ~UnitGdip();
	void setSize(INT width,INT height,D3DFORMAT format=D3DFMT_A8R8G8B8);
	Gdiplus::Graphics* getGraphics();
	void releaseGraphics();
	void clear(DWORD color=0xffffffff);
	void drawBoxLine(INT x,INT y,INT width,INT height,DWORD color);
	void drawLine(INT x,INT y,INT x2,INT y2,DWORD color);
	void drawBox(INT x,INT y,INT width,INT height,DWORD color);
	void drawString(LPCWSTR text,HFONT font,DWORD color);
	void drawString(INT x,INT y,LPCWSTR text,HFONT font,DWORD color);
protected:
	virtual bool onDeviceLost();
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	SP<BYTE> m_data;
	SP<Gdiplus::Bitmap> m_bitmap;
	SP<Gdiplus::Graphics> m_graphics;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWorld
// 再レンダリング用ワールドユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitWorld : public UnitSprite
{
public:
	UnitWorld();
	~UnitWorld();
	void setUnit(Unit* unit);
	World* getWorld() const;
protected:
	virtual void onAction(World* world,LPVOID value);
	virtual void onStart(World* world,LPVOID value);
	virtual void onIdel(World* world,LPVOID value);
	virtual void onEnd(World* world,LPVOID value);
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	World* m_world;
};
}}
#define __INC_AFLDIRECT3DUNIT
#endif
