#pragma once

#include <windows.h>
#include <list>
#include <D3D11.h>
#include <D3Dcompiler.inl>
#include <gdiplus.h>
#include "aflStd.h"
#include "afl3DBase.h"

namespace AFL{
class Mesh;
class Frame;
class Unit;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ConstantBuffer
// シェーダ定数設定用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class ConstantBuffer
{
public:
	ConstantBuffer();
	bool create(UINT size);
	bool update(LPVOID data);
	operator ID3D11Buffer*() const;
	operator ID3D11Buffer**();
protected:
	CP<ID3D11Buffer> m_buffer;
	ID3D11Buffer* m_buffer2;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// DepthStencil
// 深度バッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class DepthStencil
{
public:
	DepthStencil();
	bool create(UINT width,UINT height);
	operator ID3D11DepthStencilView*() const;
	ID3D11Texture2D* getTexture()const;
protected:
	D3D11_TEXTURE2D_DESC m_desc;
	CP<ID3D11Texture2D> m_texture;
	CP<ID3D11DepthStencilView> m_depthStencilView;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Texture
{
public:
	Texture();
	~Texture();
	void release();
	bool create(UINT width,UINT height,D3D11_USAGE usage=D3D11_USAGE_DYNAMIC);
	bool create(HANDLE handle);
	D3D11_MAPPED_SUBRESOURCE* lock();
	void unlock();
	bool setTarget(ID3D11DeviceContext* context);
	ID3D11Texture2D* getTexture()const;
	bool isTexture() const;
	operator ID3D11Texture2D*() const;
	ID3D11ShaderResourceView* getResourceView() const;
	bool open(LPCWSTR fileName,D3D11_USAGE usage=D3D11_USAGE_DEFAULT);
	bool open(LPCSTR fileName,D3D11_USAGE usage=D3D11_USAGE_DEFAULT);
	UINT getImageHeight() const;
	UINT getImageWidth() const;
	UINT getTextureHeight() const;
	UINT getTextureWidth() const;
	bool clear(DWORD color);
	bool clear(D3D11_MAPPED_SUBRESOURCE* r,DWORD color);

	bool drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,
		COLORREF colText,COLORREF colBack,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCWSTR pText,INT iSize,
		COLORREF colText=0xffffff,COLORREF colBack=-1,INT iLimitWidth=-1,bool mline=true);
	bool drawOutlineText(int iX,int iY,LPCWSTR pText,HFONT hFont,
		COLORREF colText,COLORREF colBack,INT iLimitWidth=-1,bool mline=true);
	HDC getDC() const;
	void releaseDC() const;
protected:
	bool drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline(D3D11_MAPPED_SUBRESOURCE& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline2(D3D11_MAPPED_SUBRESOURCE& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool drawGlyphOutline2(INT iX,INT iY,COLORREF colText,LONG tmAscent,
		LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);



	ID3D11Texture2D* m_texture;
	D3D11_MAPPED_SUBRESOURCE m_resource;
	ID3D11RenderTargetView* m_targetView;
	ID3D11ShaderResourceView* m_resourceView;
	D3D11_TEXTURE2D_DESC m_desc;
};
class TextureTarget : public Texture
{
public:
	bool create(UINT width,UINT height,D3D11_USAGE usage=D3D11_USAGE_DYNAMIC);
	void setDepth(ID3D11DepthStencilView* sview);
	bool clear(FLOAT r=0.0f,FLOAT g=0.0f,FLOAT b=0.0f,FLOAT a=0.0f);
	ID3D11DeviceContext* getContext() const;
	CP<ID3D11DeviceContext> m_deviceContext;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// スワップチェイン用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Screen
{
public:
	Screen();
	~Screen();
	void release();
	ID3D11DeviceContext* getContext() const;
	bool clear(FLOAT r=0.0f,FLOAT g=0.0f,FLOAT b=0.0f,FLOAT a=0.0f);
	bool present();
	bool createScreen(HWND hwnd,UINT width,UINT height,DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	bool setSize(UINT width,UINT height);
	bool draw(Mesh* mesh);
	void setCamera(Camera* camera);
	UINT getWidth() const;
	UINT getHeight() const;
	ID3D11DepthStencilView* getDepth() const;
	ID3D11Texture2D* getDepthTexture() const;
	ID3D11Texture2D* getTexture() const;
	void setTarget();
protected:
	DXGI_SWAP_CHAIN_DESC m_desc;
	CP<IDXGISwapChain> m_screen;
	CP<ID3D11DeviceContext> m_deviceContext;
	CP<ID3D11RenderTargetView> m_renderTargetView;
	CP<IDXGIOutput> m_output;
	Camera* m_camera;
	ID3D11Texture2D* m_texture;
	DepthStencil m_depth;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Direct3DDevice11
// D3DDevice管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Direct3DDevice11
{
public:
	Direct3DDevice11();
	~Direct3DDevice11();
	operator ID3D11Device*() const;
	ID3D11Device* operator->() const;
	bool isDevice() const;
	ID3D11DeviceContext* getContext() const;
	D3D_FEATURE_LEVEL getFeatureLevel() const;
	ID3D11BlendState* getBlendAlpha() const;
	ID3D11BlendState* getBlendAdd() const;
	ID3D11BlendState* getBlendSet() const;
	ID3D11BlendState* getBlendNone() const;
protected:
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;
	ID3D11BlendState* m_blendAlpha;
	ID3D11BlendState* m_blendAdd;
	ID3D11BlendState* m_blendSet;
	ID3D11BlendState* m_blendNone;
	D3D_FEATURE_LEVEL m_level;
	ULONG_PTR m_token;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Buffer
// バッファデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Buffer
{
public:
	Buffer();
	~Buffer();
	D3D11_MAPPED_SUBRESOURCE* lock();
	void unlock();
	UINT getSize() const;
protected:
	ID3D11Buffer* m_buffer;
	D3D11_MAPPED_SUBRESOURCE m_resource;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// 頂点データ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Vertex : public Buffer
{
public:
	Vertex();
	~Vertex();
	//頂点バッファの作成
	bool create(UINT size);
	bool create(LPCVOID data,UINT size);
	operator ID3D11Buffer*() const;

protected:
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Index
// インデックスデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Index : public Buffer
{
public:
	Index();
	~Index();
	//頂点バッファの作成
	bool create(UINT size);
	bool create(LPCVOID data,UINT size);
	UINT getCount() const;
	operator ID3D11Buffer*() const;
protected:
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Shader
// シェーダ管理基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct ShaderConstant
{
	INT index;
	ConstantBuffer constantBuffer;
};

class Shader
{
public:
	Shader();
	LPVOID getData();
	INT getSize();
	bool set(LPCVOID src,size_t size);
	bool update(LPCSTR name,LPCVOID data);
	int getCBIndex(LPCSTR name);
	bool setConstant(ID3D11DeviceContext* context);
	virtual bool setConstantBuffer(ID3D11DeviceContext* context,INT index,ConstantBuffer* constant) = 0;
protected:
	SP<std::vector<BYTE> > m_data;
	SP<std::map<String,ShaderConstant> > m_cbIndex;
	static pD3DCompile _D3DCompile;
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
	void release();
	bool create(LPCSTR cmd,LPCSTR name="VS",LPCWSTR path=NULL);
	bool create(LPCVOID data,size_t size);
	bool open(LPCWSTR fileName,LPCSTR name="VS");
	bool open(LPCSTR fileName,LPCSTR name="VS");
	operator ID3D11VertexShader*() const;
protected:
	virtual bool setConstantBuffer(ID3D11DeviceContext* context,INT index,ConstantBuffer* constant);
	CP<ID3D11VertexShader> m_vertexShader;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PixelShader
// ピクセルシェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PixelShader : public Shader
{
public:
	PixelShader();
	~PixelShader();
	bool open(LPCWSTR fileName,LPCSTR name="PS");
	bool open(LPCSTR fileName,LPCSTR name="PS");
	void release();
	bool create(LPCSTR cmd,LPCSTR name="PS");
	bool create(LPCVOID data,size_t size);
	operator ID3D11PixelShader*() const;
protected:
	virtual bool setConstantBuffer(ID3D11DeviceContext* context,INT index,ConstantBuffer* constant);
	CP<ID3D11PixelShader> m_pixelShader;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputLayout
// 頂点レイアウト用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class InputLayout
{
public:
	InputLayout();
	~InputLayout();
	void release();
	operator ID3D11InputLayout*() const;
	bool create(const D3D11_INPUT_ELEMENT_DESC* desc,UINT elements,LPCVOID adr,size_t size);
protected:
	CP<ID3D11InputLayout > m_vertexLayout;
};



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// Meshデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Mesh
{
public:
	Mesh();
	~Mesh();
	bool createLayout(D3D11_INPUT_ELEMENT_DESC* desc);
	bool createLayout(LPCSTR VName="BASE",LPCSTR EName="PLANE");
	bool setVShader(LPCSTR VName = "BASE");
	bool setPShader(LPCSTR PName);
	VertexShader* getVertexShader();
	PixelShader* getPixelShader();
	Vertex* getVertex() const;
	Index* getIndex() const;
	Texture* getTexture() const;
	Texture* getTexture(INT index) const;
	InputLayout* getInputLayout();
	UINT getStrideSize() const;
	bool createMesh(LPCVOID data,UINT size);
	bool createVertex(LPCVOID data,UINT size);
	bool createIndex(LPCVOID data,UINT size);
	bool createVertex(UINT size);
	bool createIndex(UINT size);
	bool createIndexAuto(INT count);
	LPVOID lockVertex() const;
	void unlockVertex() const;
	LPVOID lockIndex() const;
	void unlockIndex() const;

	void setMaterial(Material& material);
	Material* getMaterial();
	std::vector<BONEMATRIX>* getBoneMatrix();
	void setBoneMatrix(std::vector<BONEMATRIX>& matrices);

	bool openTexture(LPCWSTR fileName);

	void addTexture(Texture* texture);
	void addTexture(SP<Texture>& texture);
	void setTexture(Texture* texture);
	void setTexture(SP<Texture>& texture);

	NVector* getVertexRange();
	void setVertexRange(NVector* min,NVector* max);
	INT getTextureCount() const;

	void updateVS(LPCSTR name, LPCVOID data);
	void updatePS(LPCSTR name, LPCVOID data);

	void setRasterizer(D3D11_RASTERIZER_DESC* desc);
	ID3D11RasterizerState* getRasterizer();
	void setDepthStencil(D3D11_DEPTH_STENCIL_DESC* desc);
	ID3D11DepthStencilState* getDepthStencil();
	void setBlend(D3D11_BLEND_DESC* desc);
	ID3D11BlendState* getBlend();

protected:
	CP<ID3D11RasterizerState> m_rasterizer;
	CP<ID3D11DepthStencilState> m_depthStencil;
	CP<ID3D11BlendState> m_blend;
	VertexShader m_vertexShader;
	PixelShader m_pixelShader;
	std::list<SP<Texture> > m_texture;
	SP<Vertex> m_vertex;
	SP<Index> m_index;
	SP<InputLayout> m_inputLayout;
	UINT m_strideSize;
	Material m_material;
	std::vector<BONEMATRIX> m_boneMatrices;
	NVector m_range[2];
};
struct D3D11_INPUT_ELEMENT_DESC_CUSTOM : public D3D11_INPUT_ELEMENT_DESC
{
	D3D11_INPUT_ELEMENT_DESC_CUSTOM();
	~D3D11_INPUT_ELEMENT_DESC_CUSTOM();
	D3D11_INPUT_ELEMENT_DESC_CUSTOM(const D3D11_INPUT_ELEMENT_DESC& src);
	D3D11_INPUT_ELEMENT_DESC_CUSTOM& operator =(const D3D11_INPUT_ELEMENT_DESC& src);
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// D3DDevice
// デバイス操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class D3DDevice
{
public:
	static ID3D11Device* getDevice();
	static bool isDevice();
	static ID3D11DeviceContext* getContext();
	static bool createTexture(HWND hwnd);
	static LPCSTR getVSName();
	static LPCSTR getPSName();
	static void addInputElement(LPCSTR name,const D3D11_INPUT_ELEMENT_DESC* desc);
	static D3D11_INPUT_ELEMENT_DESC* getInputElement(LPCSTR name);
	static size_t getInputCount(LPCSTR name);
	static VertexShader* getVShader(LPCSTR name);
	static PixelShader* getPShader(LPCSTR name);
	static void addVShader(LPCSTR name,VertexShader& shader);
	static void addPShader(LPCSTR name,PixelShader& shader);
	static INT getDXGIFormatSize(DXGI_FORMAT format);
	static void getInputMap(std::map<std::pair<String,UINT>,UINT>& map,const D3D11_INPUT_ELEMENT_DESC* desc,UINT size);
	static ID3D11BlendState* getBlendSet() {return m_device.getBlendSet();}
	static ID3D11BlendState* getBlendAlpha() {return m_device.getBlendAlpha();}
	static ID3D11BlendState* getBlendAdd() {return m_device.getBlendAdd();}
	static ID3D11BlendState* getBlendNone() {return m_device.getBlendNone();}
	static bool getAdapterLUID(UINT Adapter,LUID *pLUID);
	static INT loadShaders(LPCWSTR path);
	static LPCWSTR getTexturePath();
	static void setTexturePath(LPCWSTR path);
protected:
	static Direct3DDevice11 m_device;
	static std::map<String,std::vector<D3D11_INPUT_ELEMENT_DESC_CUSTOM> > m_mapElement;
	static std::map<String,VertexShader> m_mapVShader;
	static std::map<String,PixelShader> m_mapPShader;
	static AFL::WString m_texturePath;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// Frameデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Frame
{
public:
	Frame();
	virtual ~Frame(){}
	NMatrix getMatrix() const;
	void setName(LPCSTR name);
	void add(Frame* frame);
	void add(Mesh* mesh);
	void addShadow(Mesh* mesh);
	void setParent();
	void setMatrix(NMatrix* matrix);
	INT getFrameCount();
	INT getMeshCount();
	INT getAllFrameCount();
	INT getAllMeshCount();
	Mesh* getMesh();
	bool createShadow();
	void createBoundingBox();
	void setBoundingBox(NVector* vect);
	void getBoundingBox(Unit* unit,NMatrix* pTopNMatrix,NMatrix* pNMatrix,NVector* vect);
	void getBoundingBox(NVector* vect,Unit* unit);
	void getLocalMatrix(NMatrix& matrix,Unit* unit);
	NMatrix getMatrix(Unit* unit);
	void setShader(bool flag);
	std::list<SP<Frame> >& getFrameChilds();
	std::list<Mesh>& getMeshes();
	std::list<Mesh>& getShadows();
	LPCSTR getFrameName() const;
	Frame* getFrame(LPCSTR name);
	Frame* getParentFrame() const;
	Frame& operator=(const Frame& frame);
protected:
	NMatrix m_matrix;
	Frame* m_frameParent;
	std::list<Mesh> m_meshes;
	std::list<Mesh> m_meshesShadow;
	std::list<SP<Frame> > m_frameChilds;
	std::string m_frameName;
	NVector m_bounding[2];
	NVector m_boundingBox[8];
};
}

