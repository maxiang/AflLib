#pragma once
#include "aflWinTool.h"
#include "aflD3D11.h"
//#include "aflD3DXFile.h"
namespace AFL{

class Unit
{
public:
	Unit();
	virtual ~Unit();
	NVector& getPos();
	NVector& getScale();
	NVector& getRot();
	bool openXFile(LPCWSTR fileName);
	bool openFile(LPCSTR fileName);
	bool openFile(LPCWSTR fileName);
	bool copyImage(Unit* unit);

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
	void setScale(FLOAT fScale);
	void setScaleX(FLOAT fScale);
	void setScaleY(FLOAT fScale);
	void setScaleZ(FLOAT fScale);
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
	bool getAnimationMatrix(LPCSTR frameName,NMatrix& matrix);
	void addAnimationTime(DWORD count);
	void setAnimation(LPCSTR name,DWORD count=0,bool loop=true);
	LPCSTR getAnimationName();

	void createBoundingBox();
	void getBoundingBox(NVector* vect);
	void getBoundingBox(Frame* frame,NVector* vect);

	void setShader(bool flag);

	virtual void onAction(LPVOID value);
	virtual void onStart(LPVOID value);
	virtual void onIdel(LPVOID value);
	virtual void onEnd(LPVOID value);
	virtual bool onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual bool onRenderMesh(Mesh* mesh,LPVOID value);
	virtual void onRenderEnd();

	std::list<Unit*>* getChilds();
	Frame* getFrame();
	Frame* getFrame(LPCSTR name);
	void setFrame(Frame* frame);

	RECT* getViewClip(RECT* rect=NULL);
	void setViewClip(const RECT* rect);
	NMatrix getMatrix();
	NMatrix getMatrix(FLOAT x,FLOAT y,FLOAT z);
	void resetRenderFlag(){m_renderFlag = true;}
	bool isRenderFlag()const{return m_renderFlag;}

	void setVSConstant(int index,ConstantBuffer& cb)
	{
		m_vsConstant[index] = cb;
	}
	void setPSConstant(int index,ConstantBuffer& cb)
	{
		m_psConstant[index] = cb;
	}
	std::map<int,ConstantBuffer>& getVSConstant()
	{
		return m_vsConstant;
	}
	std::map<int,ConstantBuffer>& getPSConstant()
	{
		return m_psConstant;
	}
	void addMesh(Mesh* mesh)
	{
		if (!m_frame.get())
			m_frame = SP<Frame>(new Frame());
		m_frame->add(mesh);
	}
	Mesh* getMesh() const
	{
		if (!m_frame.get())
			return NULL;
		return m_frame->getMesh();
	}
protected:
	LPCSTR m_elementName;
	LPCSTR m_vshaderName;
	LPCSTR m_pshaderName;
	SP<Frame> m_frame;
	std::list<Animation> m_anime;
	std::map<std::string,ANIMATIONSET> m_animationSet;
	Unit* m_unitParent;
	std::list<Unit*> m_unitChilds;

	bool createObject(FileObject* fileObject);
	Frame* readFrame(struct FrameData* frameData);
	void readMesh(Frame* frame,struct MeshData* meshData);
	void readMesh(Frame* frame,MESHOPTIMIZE* xfileMesh,bool shadow=false);
	bool optimizeMesh(MESHOPTIMIZE& meshOptimize,std::vector<INT>& index,std::vector<VERTEXBASE2>& data);

	static bool m_init;

	RECT m_viewClip;
	NVector m_pos;
	NVector m_scale;
	NVector m_center;
	NVector m_rotation;
	NVector m_relativity;
	NVector m_clipPoint;
	NVector m_clipSize;
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
	DWORD m_alpha;
	std::map<int,ConstantBuffer> m_vsConstant;
	std::map<int,ConstantBuffer> m_psConstant;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSprite
// 2D�p�X�v���C�g���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
enum POINT_VECTOR
{
	POINT_LEFT = 1,
	POINT_RIGHT = 2,
	POINT_TOP = 4,
	POINT_BOTTOM = 8
};

class UnitSprite : public Unit
{
	typedef struct tagGetVertex
	{
		FLOAT fX1,fY1,fX2,fY2;
		FLOAT fTX,fTY,fTWidth,fTHeight;
	}GETVERTEX,*PGETVERTEX,*LPGETVERTEX;

public:
	UnitSprite();
	bool createTarget(INT width,INT height);
	bool createImage(HWND hwnd);
	bool createImage(INT width,INT height,D3D11_USAGE usage=D3D11_USAGE_DEFAULT);
	bool createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true);
	bool createText(LPCWSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true);
	bool openImage(LPCSTR fileName,D3D11_USAGE usage=D3D11_USAGE_DEFAULT);
	bool openImage(LPCWSTR fileName,D3D11_USAGE usage=D3D11_USAGE_DEFAULT);

	Texture* getTexture() const;
	INT getImageWidth()const;
	INT getImageHeight()const;
	HDC getDC() const;
	void releaseDC(HDC hdc) const;
	void setTarget() const;

	//bool lock(D3DLOCKED_RECT* rect);
	//bool unlock();
	//D3DFORMAT getFormat() const;

	void drawText(INT x,INT y,LPCWSTR text,HFONT font,DWORD color,DWORD bcolor);
	bool clear(DWORD color);
	void setPartSize(INT width,INT height);
	INT getPartWidth()const;
	INT getPartHeight()const;
	INT getPartIndex()const;
	void setPartIndex(INT index);

	void setVShaderName(LPCSTR name)
	{
		m_vsName = name;

		Frame* frame = m_frame.get();
		if (frame)
		{
			Mesh* mesh = frame->getMesh();
			if (mesh)
				mesh->setVShader(m_vsName);
		}
	}
	void setPShaderName(LPCSTR name)
	{
		m_psName = name;
		Frame* frame = m_frame.get();
		if (frame)
		{
			Mesh* mesh = frame->getMesh();
			if (mesh)
				mesh->setPShader(m_psName);
		}
	}
	void setPointStat(INT value)
	{
		m_pointStat = value;
	}
protected:
	virtual bool onRenderMesh(Mesh* mesh,LPVOID value);

	bool _createFromTexture(Texture* texture);
	bool _createFromTexture(SP<Texture>& texture);
	bool _getPointVertex(PGETVERTEX vertexSprite,Texture* textureSrc);
	bool _resetVertex();

	INT m_pointStat;
	FLOAT m_centerX,m_centerY;
	INT m_partWidth;
	INT m_partHeight;
	INT m_partIndex;
	LPCSTR m_vsName;
	LPCSTR m_psName;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitText
// �e�L�X�g�\���p���j�b�g
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
	LPCWSTR getText()
	{
		return m_drawString;
	}
protected:
	virtual bool onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z);

	AFL::WINDOWS::Font m_font;
	DWORD m_color;
	DWORD m_bcolor;
	WString m_drawString;
	WString m_drawedString;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitFPS
// DirectX - FPS�\���p���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitFPS : public UnitText
{
public:
	UnitFPS();
protected:
	virtual bool onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z);
	INT m_count;
	INT m_time;
	INT m_fps;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVector
// DirectX - �}�`�p���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitVector : public Unit
{
public:
	UnitVector();
	virtual ~UnitVector(){}
	void create(VectorObject* vo);
	virtual bool onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z);
protected:
	LPCSTR m_elementName;
	LPCSTR m_vsName;
	LPCSTR m_psName;
	VectorObject m_vectorObject;
	bool m_flag3d;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - �`�b�v�^2D�}�b�v�p
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
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GDIPlus
// GDIPlus�������p
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
// GDIPlus�\���p���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitGdip : public UnitSprite
{
	static GDIPlus m_gdiPlus;
public:
	UnitGdip();
	virtual ~UnitGdip();
	void setSize(INT width,INT height);
	Gdiplus::Graphics* getGraphics();
	void releaseGraphics();
	void clear(DWORD color=0xffffffff);
	void drawBoxLine(INT x,INT y,INT width,INT height,DWORD color);
	void drawLine(INT x,INT y,INT x2,INT y2,DWORD color);
	void drawBox(INT x,INT y,INT width,INT height,DWORD color);
	void drawString(LPCWSTR text,HFONT font,DWORD color);
	void drawString(INT x,INT y,LPCWSTR text,HFONT font,DWORD color);
	
protected:
	virtual bool onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z);
	SP<BYTE> m_data;
	Gdiplus::Bitmap* m_bitmap;
	Gdiplus::Graphics* m_graphics;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitEffect
// �G�t�F�N�g�p
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct EffectData
{
	NVector3 pos;
	FLOAT size;
	FLOAT rot;
	NVector color;
	INT count;
	INT countNow;
};
class UnitEffect : public Unit
{
public:
	UnitEffect();
	bool openImage(LPCSTR fileName);
	bool openImage(LPCWSTR fileName);
	void add(FLOAT x, FLOAT y, FLOAT z, FLOAT size, DWORD color = 0xffffffff, INT count = -1,FLOAT rot=0.0f);
	void clear();
protected:
	virtual bool onRenderMesh(Mesh* mesh, LPVOID value);
	virtual void onAction(LPVOID value);

	std::list<EffectData> m_effectData;

};

}