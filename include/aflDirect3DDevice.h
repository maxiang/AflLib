#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3D
#include <d3d9.h>
//#include <d3dx9.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include "afl3DBase.h"
#include "aflDirectDraw.h"

#if !defined(D3DENUM_NO_DRIVERVERSION)
	interface DECLSPEC_UUID("02177241-69FC-400C-8FF1-93A44DF6861D") IDirect3D9Ex;
	interface DECLSPEC_UUID("B18B10CE-2649-405a-870F-95F777D4313A") IDirect3DDevice9Ex;
	typedef interface IDirect3D9Ex                   IDirect3D9Ex;
	typedef interface IDirect3DDevice9Ex             IDirect3DDevice9Ex;
#endif

namespace AFL{namespace DIRECT3D{

struct Point2D
{
	FLOAT x,y;
};

enum D3DXFILEFORMAT
{
	_3DS_EXPORTER,
	_3DS_CONVERTER
};
class Object;
class World;
class Target;

#define isDeviceActive() (::AFL::DIRECT3D::Device::getStat() == ::AFL::DIRECT3D::D3D_DEVICE_ACTIVE)

enum D3D_DEVICESTAT
{
	D3D_DEVICE_NOTINIT,			//デバイス初期化前
	D3D_DEVICE_ERROR_DIRECTX,	//DirectXがインストールされていない
	D3D_DEVICE_ERROR_DEVIVE,	//デバイスが作成できない
	D3D_DEVICE_ACTIVE			//正常動作中
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InterfaceDevice
// DirectX - Direct3D用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InterfaceDevice
{
friend class Device;
public:
	IDirect3D9* getInterface() const;
	IDirect3D9Ex* getInterfaceEx() const;
	IDirect3DDevice9* getDeviceInterface() const;
	IDirect3DDevice9Ex* getDeviceInterfaceEx() const;
	IDirect3DStateBlock9* getStatusBlock();
	bool isDirectX() const;
	D3D_DEVICESTAT getStat() const;
	D3DCAPS9 const* getCaps() const;
	D3DPRESENT_PARAMETERS const* getParams()const;
	UINT getWidth()const;
	UINT getHeight()const;
	UINT getDeviceWidth()const;
	UINT getDeviceHeight()const;
	bool resetDevice();
	bool setDefaultStat();
	bool setFullScreen(bool bFlag){return true;}
	bool isFullScreen();
	HWND getWindowHandle(){return m_hWnd;}

protected:
	bool init(HWND hWnd=NULL,UINT deviceWidth=0,UINT deviceHeight=0,bool screenFull=false,bool videoSync=false);
	InterfaceDevice();
	virtual ~InterfaceDevice();

	HINSTANCE m_hLibrary;	//D3D.DLLのハンドル
	IDirect3D9* m_pd3d;		//IDirect3Dインタフェイス
	IDirect3D9Ex* m_pd3dEx;	//IDirect3Dインタフェイス

	HWND m_hWnd;
	D3D_DEVICESTAT m_deviceStat;
	D3DCAPS9 m_d3dCaps;

	IDirect3DDevice9Ex* m_pd3dDeviceEx;
	IDirect3DDevice9* m_pd3dDevice;
	IDirect3DStateBlock9* m_pd3dStatus;
	D3DPRESENT_PARAMETERS m_d3dpp; 
	bool m_screenFull;
	bool m_reset;

	std::list<Object*> m_listObject;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Shader
// シェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Shader
{
public:
	Shader();
	~Shader();
	virtual void release();
	bool create(LPCSTR cmd,LPCSTR name="VS",LPCSTR target="vs_2_0");
	bool create(LPCVOID data,size_t size,LPCSTR target="vs_2_0");
	bool open(LPCWSTR fileName,LPCSTR name="VS",LPCSTR target="vs_2_0");
	bool open(LPCSTR fileName,LPCSTR name="VS",LPCSTR target="vs_2_0");
	bool save(LPCWSTR fileName) const;
	size_t getSize() const;
	LPCVOID getData() const;
	bool isParam(LPCSTR name) const;
protected:
	SP<std::vector<BYTE> > m_data;
	std::map<String,INT> m_constName;
	std::map<INT,std::pair<INT,std::vector<BYTE> > > m_constData;
	String m_target;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VertexShader
// 頂点シェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class VertexShader : public Shader
{
public:
	VertexShader();
	~VertexShader();
	virtual void release();
	operator IDirect3DVertexShader9*();
	void setDefaultParam() const;
	void setParam(LPCSTR name,const FLOAT* data,INT count=1) const;
	void setParam(LPCSTR name,const NMatrix* data,INT count=1) const;
	void setParam(LPCSTR name,const COLOR4* data,INT count=1) const;
	void setParam(LPCSTR name,const NVector* data,INT count=1) const;
protected:
	CP<IDirect3DVertexShader9> m_vertexShader;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device
// DirectX - Direct3Dデバイス操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class Device
{
public:
	static bool init(HWND hWnd=NULL,UINT deviceWidth=0,UINT deviceHeight=0,bool screenFull=false,bool videoSync=false);
	static D3D_DEVICESTAT getStat();
	static IDirect3DDevice9* getInterface();
	static IDirect3DDevice9Ex* getInterfaceEx();
	static D3DCAPS9 const* getCaps();

	static bool resetDevice();
	static D3DPRESENT_PARAMETERS const* getParams(){return m_interfaceDevice.getParams();}
	static UINT getWidth(){return m_interfaceDevice.getWidth();}
	static UINT getHeight(){return m_interfaceDevice.getHeight();}
	static UINT getTargetWidth();
	static UINT getTargetHeight();

	static bool setClipper(FLOAT fX,FLOAT fY,FLOAT fZ,FLOAT fWidth,FLOAT fHeight,FLOAT fDepth);
	static bool setClipper(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3);
	static void addObject(Object* object);
	static void delObject(Object* object);
	static bool isLost();
	static bool setFullScreen(bool flag);
	static bool isFullScreen();
	static bool setRenderTarget(IDirect3DSurface9* backBuffer);
	static IDirect3DSurface9* getRenderTarget();
	static bool setDeviceSize(INT width,INT height);
	static void getAdapterDisplayMode(D3DDISPLAYMODE* pd3dDisplayMode);
	static UINT getDepthWidth();
	static UINT getDepthHeight();
	static void saveStatus();
	static void loadStatus();
	static bool present(HWND hWnd);
	static bool getAdapterLUID(UINT Adapter,LUID *pLUID);
	static void setTexturePath(LPCWSTR path)
	{
		if(path)
			m_texturePath = path;
		else
			m_texturePath.clear();
	}
	static LPCWSTR getTexturePath()
	{
		return m_texturePath.c_str();
	}
	static VertexShader* getVShader(LPCSTR name);
	//static PixelShader* getPShader(LPCSTR name);
	static void addVShader(LPCSTR name,VertexShader& shader);
	//static void addPShader(LPCSTR name,PixelShader& shader);

protected:
	static InterfaceDevice m_interfaceDevice;
	static AFL::DirectDraw::Screen m_screen;
	static AFL::WString m_texturePath;
	static std::map<String,VertexShader> m_mapVShader;
//	static std::map<String,PixelShader> m_mapPShader;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Object
// 3Dオブジェクト基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Object
{
	friend Device;
public:
	Object();
	virtual ~Object();
protected:
	void setLost(){m_lost = true;}
	void clearLost(){m_lost = false;}
	bool isLost(){return m_lost;}
	bool isRestore()const{return m_restore;}
	bool clearRestore(){m_restore = false;}

	virtual bool onDeviceInit(){return true;}
	virtual bool onDeviceLost() = 0;
	virtual bool onDeviceRestore() = 0;
	bool m_lost;
	bool m_restore;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Light
// ライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Light : public D3DLIGHT9
{
public:
	Light();

	D3DLIGHT9* getLight(){return (D3DLIGHT9*)this;}
	void setLight(D3DLIGHT9* light){(D3DLIGHT9)*this = *light;}
	bool isEnable(){return m_enable;}
	void setEnable(bool enable){m_enable = enable;}
protected:
	bool m_enable;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fog
// フォグ管理用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Fog
{
public:
	Fog();

	bool isEnable(){return m_enable;}
	void setEnable(bool enable){m_enable=enable;}
	FLOAT getFogStart(){return m_fogStart;}
	void setFogStart(FLOAT fStart){m_fogStart = fStart;}
	FLOAT getFogEnd(){return m_fogEnd;}
	void setFogEnd(FLOAT fEnd){m_fogEnd = fEnd;}
	DWORD getFogColor(){return m_fogColor;}
	void setFogColor(DWORD dwFogColor){m_fogColor=dwFogColor;}
protected:
	bool m_enable;
	FLOAT m_fogStart;
	FLOAT m_fogEnd;
	DWORD m_fogColor;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Texture : public Object
{
public:
	Texture();
	~Texture();
	bool openImage(LPCSTR fileName,DWORD filter=0,D3DPOOL pool=D3DPOOL_MANAGED);
	bool openImage(LPCWSTR fileName,DWORD filter=0,D3DPOOL pool=D3DPOOL_MANAGED);
	bool createTarget(INT width,INT height,INT format);
	bool createImage(INT width,INT height,INT format,HANDLE handle);

	bool createImage(INT width,INT height,D3DPOOL pool=D3DPOOL_MANAGED);
	bool createText(LPCSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true,INT format=D3DFMT_A8R8G8B8);
	bool createText(LPCWSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true,INT format=D3DFMT_A8R8G8B8);
	bool createText(LPCSTR pText,HFONT font,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true,INT format=D3DFMT_A8R8G8B8);
	bool createText(LPCWSTR pText,HFONT font,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true,INT format=D3DFMT_A8R8G8B8);

	bool release();
	IDirect3DTexture9* getInterface()const
	{
		return m_texture;
	}
	INT getImageWidth()const{return m_imageWidth;}
	INT getImageHeight()const{return m_imageHeight;}
	INT getTextureWidth()const{return m_descTexture.Width;}
	INT getTextureHeight()const{return m_descTexture.Height;}

	bool drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,
		COLORREF colText,COLORREF colBack,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCWSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCWSTR pText,HFONT hFont,
		COLORREF colText,COLORREF colBack,INT iLimitWidth=-1,bool mline=true);

	bool clear(COLORREF colText);
	bool lock(D3DLOCKED_RECT* lockRect,DWORD flag=0)const;
	bool unlock()const;
	HDC getDC() const;
	void releaseDC(HDC hdc) const;

	bool setTarget() const;
	D3DFORMAT getFormat() const;
	IDirect3DSurface9* getSurface() const
	{
		IDirect3DSurface9* surface = NULL;
		if(m_texture)
			m_texture->GetSurfaceLevel(0,&surface);
		return surface;
	}
	void setFilter(bool flag)
	{
		m_filter = flag;
	}
private:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	
	bool drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline(D3DLOCKED_RECT& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline2(D3DLOCKED_RECT& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline2(INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);

	IDirect3DTexture9* m_texture;
	D3DSURFACE_DESC m_descTexture;
	WString m_strFileName;

	INT m_imageWidth;		//イメージ幅
	INT m_imageHeight;		//イメージ高さ
	D3DCOLOR m_colorMask;
	bool m_filter;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Material
// マテリアル
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct Material
{
	COLOR4 Diffuse;
	COLOR4 Ambient;
	COLOR4 Specular;
	COLOR4 Emissive;
	FLOAT Power;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderFrame
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderUnit
{
	class Unit* unit;
	NMatrix matrix;
	FLOAT x,y,z,w;
	RECT clipRect;
	DWORD alpha;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderFrame
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderFrame
{
	RenderUnit* renderUnit;
	NMatrix matrix;
	class Frame* frame;
	FLOAT z;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderMesh
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderMesh
{
	RenderFrame* renderFrame;
	class Mesh* mesh;
	Material material;
	INT boneData[256];
	INT boneCount;
	bool shadow;
};

/*
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CameraMAX
// 3DSMAXビュー制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class CameraMAX : public Camera
{
public:
	CameraMAX();
	void setPersp(bool flag);
	void setDp(float nearZ,float farZ);
	void setWidth(FLOAT value);
protected:
	bool setAngle(UINT uDeviceWidth,UINT uDeviceHeight);
	FLOAT m_far;
	FLOAT m_near;
	FLOAT m_fov;
	UINT m_deviceWidth,m_deviceHeight;
	FLOAT m_width;
	bool m_persp;
};


*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// スワップチェイン
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Screen : public Object
{
public:
	Screen();
	~Screen();
	bool create(INT width,INT height,D3DFORMAT format=D3DFMT_UNKNOWN);
	bool releaseScreen();
	bool setTarget();
	bool present(HWND hWnd,bool scale=false);
	HDC getDC();
	void releaseDC(HDC hdc);
	INT getWidth() const;
	INT getHeight() const;
	IDirect3DSurface9* getSurface() const;
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	IDirect3DSwapChain9* m_swapChain;
	D3DPRESENT_PARAMETERS m_d3dpp; 
	INT m_width;
	INT m_height;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Target
// ターゲット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Target : public Object
{
public:
	Target();
	~Target();
	bool releaseTarget();
	bool create(INT width,INT height,D3DFORMAT format=D3DFMT_X8R8G8B8);
	IDirect3DSurface9* getSurface()const;
	bool setTarget();
	HDC getDC();
	void releaseDC(HDC hdc);
	INT getWidth()const;
	INT getHeight()const;
	bool lock(D3DLOCKED_RECT* lockRect)const;
	bool unlock()const;
	void clear(DWORD color) const;
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	IDirect3DSurface9* m_target;
	INT m_width;
	INT m_height;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// 頂点バッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Vertex : public Object
{
public:
	Vertex();
	~Vertex();
	bool createBuffer(INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool createBuffer(LPVOID data,INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool releaseBuffer();
	DWORD getPool()const{return m_vertexDesc.Pool;}

	INT getVertexSize()const{return m_vertexDesc.Size;}
	INT getStrideCount()const{return m_strideCount;}
	INT getStrideSize()const{return m_strideSize;}
	IDirect3DVertexBuffer9* getInterface()const{return m_vertexBuffer;}
	LPVOID lock(INT size=0);
	void unlock();
	bool setData(LPVOID data,INT size);
	DWORD getFVF()const{return m_vertexDesc.FVF;}
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	IDirect3DVertexBuffer9* m_vertexBuffer;
	D3DVERTEXBUFFER_DESC m_vertexDesc;
	INT m_strideSize;
	INT m_strideCount;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Index
// インデックスバッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Index : public Object
{
public:
	Index();
	~Index();
	bool createBuffer(INT count,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool createBuffer(LPVOID data,INT count,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool releaseBuffer();
	DWORD getPool()const{return m_indexDesc.Pool;}
	INT getIndexSize(){return m_indexSize;}
	LPVOID lock();
	LPVOID lock(INT size);
	void unlock();
	bool setData(LPVOID data,INT size);
	IDirect3DIndexBuffer9* getInterface()const{return m_indexBuffer;}
	INT getSize()const{return m_indexSize;}
	INT getCount()const{return m_indexCount;}
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	IDirect3DIndexBuffer9* m_indexBuffer;
	D3DINDEXBUFFER_DESC m_indexDesc;
	INT m_indexSize;
	INT m_indexCount;

};

class Declaration
{
public:
	Declaration()
	{
		m_declaration = NULL;
	}
	Declaration(const Declaration& d)
	{
		m_declaration = d.m_declaration;
		if(m_declaration)
			m_declaration->AddRef();
	}
	~Declaration()
	{
		if(m_declaration)
			m_declaration->Release();
	}
	bool create(const D3DVERTEXELEMENT9* decl)
	{
		if(m_declaration)
		{
			m_declaration->Release();
			m_declaration = NULL;
		}
		return Device::getInterface()->CreateVertexDeclaration(decl,&m_declaration) == D3D_OK;
	}
	void set(IDirect3DVertexDeclaration9* value){m_declaration = value;}
	IDirect3DVertexDeclaration9* get()const{return m_declaration;}
protected:
	IDirect3DVertexDeclaration9* m_declaration;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// メッシュ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Mesh
{
public:
	Mesh();
	~Mesh();

	bool createIndexBuffer(INT count,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool createIndexBuffer(LPVOID data,INT count,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool createVertexBuffer(INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	bool createVertexBuffer(LPVOID data,INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool=D3DPOOL_MANAGED);
	void setMaterial(Material& material);
	Material* getMaterial();
	void setBoneMatrix(std::vector<BONEMATRIX>& matrices);

	std::vector<BONEMATRIX>* getBoneMatrix(){return &m_boneMatrices;}
	Index* getIndexBuffer()const{return m_indexBuffer.get();}
	Vertex* getVertexBuffer()const{return m_vertexBuffer.get();}
	D3DPRIMITIVETYPE getPrimitiveType()const{return m_typePrimitive;}

	void setDeclaration(const D3DVERTEXELEMENT9* decl);
	IDirect3DVertexDeclaration9* getDeclaration()const{return m_declaration.get();}
	void setVertexShader(LPCSTR shader="Default")
	{
		m_vertexShader = *Device::getVShader(shader);
	}
	VertexShader* getVertexShader()
	{
		return &m_vertexShader;
	}
	bool isShader()
	{
		return m_vertexShader!=NULL;
	}
	bool createShadow(Mesh& shadow) const;

	void getVertexRange(NVector* vect) const;

	bool openTexture(LPCWSTR fileName);
	void addTexture(Texture* texture);
	void addTexture(SP<Texture>& texture);
	void setTexture(Texture* texture);
	void setTexture(SP<Texture>& texture);
	INT getTextureCount() const
	{
		return m_texture.size();
	}
	Texture* getTexture() const;
	bool isShadow()const{return m_shadow;}
	void setShadow(bool flag){m_shadow=flag;}
protected:
	VertexShader m_vertexShader;
	Declaration m_declaration;
	D3DPRIMITIVETYPE m_typePrimitive;
	Material m_material;
	SP<Index> m_indexBuffer;
	SP<Vertex> m_vertexBuffer;
	std::vector<BONEMATRIX> m_boneMatrices;
	std::list<SP<Texture> > m_texture;
	bool m_shadow;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// Frameデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Frame
{
public:
	Frame();
	NMatrix getMatrix() const;
	void setName(LPCSTR name);
	void add(Frame* frame);
	void add(Mesh* mesh);
	void addShadow(Mesh* mesh);
	void setMatrix(NMatrix* matrix)
	{
		m_matrix = *matrix;
	}
	INT getFrameCount();
	INT getMeshCount();
	INT getAllFrameCount();
	INT getAllMeshCount();
	Mesh* getMesh(){return m_meshes.size()?&m_meshes.front():NULL;}
	void createBoundingBox();
	void getBoundingBox(Unit* unit,NMatrix* pTopNMatrix,NMatrix* pNMatrix,NVector* vect);
	void getBoundingBox(NVector* vect);
	void setShader(bool flag);
	std::list<Frame>& getFrameChilds();
	std::list<Mesh>& getMeshes();
	LPCSTR getFrameName() const
	{
		return m_frameName.c_str();
	}
protected:
	NMatrix m_matrix;
	std::list<Mesh> m_meshes;
	std::list<Frame> m_frameChilds;
	std::string m_frameName;
	NVector m_bounding[2];
	NVector m_boundingBox[8];

};





}}

#define __INC_AFLDIRECT3D
#endif

