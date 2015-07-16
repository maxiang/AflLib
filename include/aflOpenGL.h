#pragma once

#ifdef __ANDROID__
	#define GL_GLEXT_PROTOTYPES
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#define glMapBuffer glMapBufferOES
	#define glUnmapBuffer glUnmapBufferOES
	#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
	#define GL_READ_ONLY GL_READ_ONLY_OES
#else
	#define GLEW_STATIC
	#include <gdiplus.h>
	#include "GL/glew.h"
#endif
#include <list>
#include "afl3DBase.h"

using namespace AFL;



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Object
// 3Dオブジェクト基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Object
{
	friend class GLDevice;
public:
	Object();
	virtual ~Object();
protected:
	void setLost(){m_lost = true;}
	void clearLost(){m_lost = false;}
	bool isLost(){return m_lost;}
	bool isRestore()const{return m_restore;}
	bool clearRestore(){m_restore = false;return true;}

	virtual bool onDeviceInit(){return true;}
	virtual bool onDeviceLost() = 0;
	virtual bool onDeviceRestore() = 0;
	bool m_lost;
	bool m_restore;
};
struct GLLayout
{
	String name;
	GLint count;
	GLenum type;
};
struct GLLayoutData
{
	String name;
	GLint count;
	GLenum type;
	GLint size;
	GLint stride;
	INT offset;
};

class GLInputLayout
{
public:
	void setLayout(const GLLayout* layout, INT count);
	INT getCount() const;
	const GLLayoutData* getData() const;
protected:
	std::vector<GLLayoutData> m_layoutData;
};

class GLShader;

class GLDevice
{
public:
	GLDevice();
	~GLDevice();
	#ifndef __ANDROID__
		bool init(HWND hwnd);
	#endif
	
	static SP<GLShader> getShader(GLenum type,LPCSTR name);
	static bool isShader(GLenum type,LPCSTR name);
	static bool addShader(LPCSTR name,GLenum type, const char* fileName);
	static void addObject(Object* object);
	static void delObject(Object* object);
	static bool lost();
	static bool lost2();
	static bool restore();
	static void getInputMap(std::map<std::pair<String,UINT>,UINT>& map,const GLInputLayout* desc);
	static void setTexturePath(LPCSTR path);
	static LPCSTR getTexturePath();

	static INT loadShaders(LPCWSTR path);
	static INT loadShaders(LPCSTR path);
protected:
	#ifndef __ANDROID__
		HGLRC  m_hRC;
	#endif
	static std::map<std::pair<GLenum,String>,SP<GLShader> > m_shaders;
	static std::list<Object*> m_listObject;
	static AFL::String m_texturePath;
};


#ifndef __ANDROID__
class GDIP
{
public:
	GDIP();
	~GDIP();
protected:
	ULONG_PTR m_gdiplusToken;
};
class GLImage
{
public:
	GLImage();
	~GLImage();
	bool openImage(LPCSTR fileName,bool filter=true);
	bool createImage(INT width,INT height,LPVOID buff);
	void release();
	Gdiplus::BitmapData* getBuffer();
	void releaseBuffer();
	bool resize(INT width,INT height);
	Gdiplus::Graphics* GLImage::getGraphics();

	void drawText(LPCWSTR text, HFONT font, DWORD color);

	bool getTextSize(LPCWSTR text, HFONT font,LPSIZE size);
protected:
	SP<Gdiplus::Graphics> m_graphics;
	Gdiplus::Bitmap* m_bitmap;
	Gdiplus::BitmapData m_data;
	static GDIP m_gdip;
};
#endif


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GLShader
// シェーダリソース管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct ShaderConstant
{
	INT index;
	INT size;
	INT type;
};

class GLShader : public Object
{
	friend class GLShaderProgram;
public:
	GLShader();
	~GLShader();
	void release();
	void setShader(GLint shader);
	GLint getShader() const;
	bool loadShader(GLenum type, const char* shaderSrc);
	bool loadShaderFile(GLenum type, const char* fileName);

protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	GLuint m_shader;
	String m_backupSource;
	INT m_backupType;
	String m_fileName;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GLShaderProgram
// シェーダ制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class GLShaderProgram : public Object
{
public:
	GLShaderProgram();
	~GLShaderProgram();
	void release();
	GLuint getProgram();

	void attachShader(GLShader* shader);
	void detachShader(GLShader* shader);

	void bindAttribLocation(GLuint index, const GLchar* name);
	void linkProgram();
	void useProgram();
	void uniformMatrix(const GLchar* name,NMatrix* matrix,INT count=1);
	void uniformFloat(const GLchar* name,float* data,INT count=1);
	void uniformInt(const GLchar* name,int* data,INT count=1);
	void setLayout(GLInputLayout* layout);
	GLInputLayout* getLayout();
	void update(const GLchar* name,LPCVOID data);

protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	GLuint m_program;
	std::list<GLShader*> m_shader;
	GLInputLayout m_inputLayout;
	SP<std::map<String,ShaderConstant> > m_cbIndex;

};





//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Buffer
// バッファデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Buffer : public Object
{
public:
	Buffer();
	~Buffer();
	UINT getSize() const;
	bool create(LPCVOID data,UINT size,UINT usage=GL_STATIC_DRAW);
	bool create(UINT size);
	void release();
	operator GLuint();
	LPVOID lock() const;
	void unlock() const;
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	GLuint m_buffer;
	LPBYTE m_backupData;
	GLint m_backupSize;
	GLint m_backupUsage;
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
	static void addLayout(LPCSTR name,const GLLayout* layout,INT count);
	static GLInputLayout* getLayout(LPCSTR name);
	static INT getLayoutCount(LPCSTR name);
protected:
	static std::map<String,GLInputLayout> m_layout;
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
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Texture : public Object
{
public:
	Texture();
	~Texture();
	void release();
	bool open(LPCSTR fileName,bool filter=true);
	bool create(UINT width,UINT height);
	bool createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true);
	bool createText(LPCWSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true);
	operator GLuint();
	bool clear(DWORD color);
	INT getImageHeight() const;
	INT getImageWidth() const;
	INT getTextureWidth() const;
	INT getTextureHeight() const;
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();

	GLuint m_texture;
	INT m_imageWidth;
	INT m_imageHeight;
	INT m_textureWidth;
	INT m_textureHeight;
	LPVOID m_backupData;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderBuffer
// オフラインレンダリング用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef GL_DEPTH24_STENCIL8
	#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

class RenderBuffer : public Object
{
public:
	RenderBuffer();
	~RenderBuffer();
	void release();
	bool create(INT width,INT height);
	operator GLuint() const;
	INT getWidth() const;
	INT getHeight() const;
	Texture& getTexture();
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	Texture m_texture;
	GLuint m_renderBuffer;
	INT m_width;
	INT m_height;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TextureDepth
// 深度テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TextureDepth : public Object
{
public:
	TextureDepth();
	~TextureDepth();
	void release();
	operator GLuint();
	bool getDepth(INT width,INT height);
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	GLuint m_texture;
};

typedef struct tagGetVertex
{
	FLOAT fX1,fY1,fX2,fY2;
	FLOAT fTX,fTY,fTWidth,fTHeight;
}GETVERTEX,*PGETVERTEX,*LPGETVERTEX;


struct VERTEXPT
{
	bool operator < ( const VERTEXPT& v) const
	{
		if(memcmp(data,v.data,size) < 0)
			return true;
		return false;
	}
	VERTEXPT(LPCVOID data,UINT size)
	{
		VERTEXPT::data = data;
		VERTEXPT::size = size;
	}
	LPCVOID data;
	UINT size;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// Meshデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Mesh
{
public:
	Mesh(const Mesh &obj);
	Mesh();
	~Mesh();
	bool createLayout(LPCSTR VName="PLANE",LPCSTR layoutName="PLANE");
	bool setVShader(LPCSTR VName = "BASE");
	bool setPShader(LPCSTR PName);
	bool createIndexAuto(INT count);

	GLint getVertexShader() const;
	GLint getFragmentShader() const;
	Vertex* getVertex() const;
	Index* getIndex() const;
	Texture* getTexture() const;
	Texture* getTexture(INT index) const;
	UINT getStrideSize() const;
	void bindLayout();
	bool createMesh(LPCVOID data,UINT size);
	bool createVertex(LPCVOID data,UINT size);
	bool createIndex(LPCVOID data,UINT size);
	bool createVertex(UINT size);
	bool createIndex(UINT size);
	bool openTexture(LPCSTR fileName,bool filter);
	void addTexture(Texture* texture);
	void addTexture(SP<Texture>& texture);
	void setTexture(Texture* texture);
	void setTexture(SP<Texture>& texture);

	bool createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth=0,bool mline=true);
	

	void setMaterial(Material& material);
	Material* getMaterial();
	
	std::vector<BONEMATRIX>* getBoneMatrix();
	void setBoneMatrix(std::vector<BONEMATRIX>& matrices);
	
	NVector* getVertexRange();
	void setVertexRange(NVector* min,NVector* max);
	
	const GLLayoutData* getLayout() const;
	INT getLayoutCount();
	void useProgram();
	GLShaderProgram* getProgram();
	INT getTextureCount() const;
	void setBlendMode(INT mode);
	INT getBlendMode() const;
	bool isShadow()const;
	void setShadow(bool flag);
	void updateVS(LPCSTR name, LPCVOID data);
	void updatePS(LPCSTR name, LPCVOID data);


protected:
	bool _createFromTexture(SP<Texture>& texture);
	bool _getPointVertex(PGETVERTEX vertexSprite, Texture* textureSrc);

	SP<GLShaderProgram> m_program;
	SP<GLShader> m_vertexShader;
	SP<GLShader> m_fragmentShader;
	std::list<SP<Texture> > m_texture;
	SP<Vertex> m_vertex;
	SP<Index> m_index;
	UINT m_strideSize;
	Material m_material;
	std::vector<BONEMATRIX> m_boneMatrices;
	NVector m_range[2];
	INT m_blendMode;
	bool m_shadow;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// オフラインバッファ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Screen : public Object
{
public:
	Screen();
	~Screen();
	void release();
	bool clear(FLOAT r=0.0f,FLOAT g=0.0f,FLOAT b=0.0f,FLOAT a=0.0f);
	bool present();
	bool createScreen(UINT width,UINT height,LPVOID hWnd);
	bool setSize(UINT width,UINT height);
	//bool draw(Mesh* mesh);
	void setCamera(Camera* camera);
	INT getWidth() const;
	INT getHeight() const;
	void setTarget();
protected:
	virtual bool onDeviceLost();
	virtual bool onDeviceRestore();
	Camera* m_camera;
	GLuint m_frameBuffer;
	RenderBuffer m_renderBuffer;
	Mesh m_mesh;
	INT m_width;
	INT m_height;
};
class Unit;
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


