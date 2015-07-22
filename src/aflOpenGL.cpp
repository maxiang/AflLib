#ifndef __ANDROID__
	#include <windows.h>
	#include "aflWinTool.h"
#else
	#include "AndroidApp.h"
	#include <android/log.h>
#endif
#include <stdio.h>
#include "aflOpenGLUnit.h"
//
//----------------------------------------------------
//メモリリークテスト用
#if !defined(CHECK_MEMORY_LEAK)
	#if _MSC_VER && !defined(_WIN32_WCE) && _DEBUG
		#include <crtdbg.h>
		inline static void*  operator new(const size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size,_NORMAL_BLOCK,  strFileName, iLine);}
		inline static void operator delete(void* adr, LPCSTR strFileName, INT iLine)
			{_free_dbg(adr,_NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif
#endif

//----------------------------------------------------

void GLInputLayout::setLayout(const GLLayout* layout, INT count)
{
	m_layoutData.resize(count);
	int i;

	//レイアウトサイズを計算
	int stride = 0;
	for(i=0;i<count;i++)
	{
		int size = 0;
		if (layout[i].type == GL_FLOAT || layout[i].type == GL_INT)
			size = sizeof(GLfloat);
		stride += size * layout[i].count;
	}


	int offset = 0;
	for (i = 0; i<count; i++)
	{
		GLLayoutData& data = m_layoutData[i];
		data.name = layout[i].name;
		data.count = layout[i].count;
		data.type = layout[i].type;
		data.stride = stride;
		data.size = data.count * sizeof(GLfloat);
		data.offset = offset;
		offset += data.size;
	}
}
INT GLInputLayout::getCount() const
{
	return (INT)m_layoutData.size();
}
const GLLayoutData* GLInputLayout::getData() const
{
	if (!m_layoutData.size())
		return NULL;
	return &m_layoutData[0];
}
//----------------------------------------------------

#ifdef __ANDROID__
GLDevice::GLDevice()
{
}

GLDevice::~GLDevice()
{
}
#endif
String GLDevice::m_texturePath;
std::map<std::pair<GLenum,String>,SP<GLShader> > GLDevice::m_shaders;

SP<GLShader> GLDevice::getShader(GLenum type,LPCSTR name)
{
	return m_shaders[std::pair<GLenum,LPCSTR>(type,name)];
}
bool GLDevice::isShader(GLenum type,LPCSTR name)
{
	std::map<std::pair<GLenum,String>,SP<GLShader> >::const_iterator it = m_shaders.find(std::pair<GLenum,LPCSTR>(type,name));
	return it != m_shaders.end();
}
bool GLDevice::addShader(LPCSTR name,GLenum type, const char* fileName)
{
	SP<GLShader> shader = new GLShader;
	if(!shader->loadShaderFile(type,fileName))
		return false;
	m_shaders[std::pair<GLenum,LPCSTR>(type,name)] = shader;
	return true;
}
std::list<Object*> GLDevice::m_listObject;

void GLDevice::addObject(Object* object)
{
	m_listObject.push_back(object);
}
void GLDevice::delObject(Object* object)
{
	if(m_listObject.size())
		m_listObject.remove(object);
}
bool GLDevice::lost()
{
	//ロスト通知
	std::list<Object*>::iterator it;
	for(it=m_listObject.begin();it!=m_listObject.end();++it)
	{
		(*it)->setLost();
		(*it)->onDeviceLost();
	}
	return true;
}
bool GLDevice::lost2()
{
	//ロスト通知
	std::list<Object*>::iterator it;
	for (it = m_listObject.begin(); it != m_listObject.end(); ++it)
	{
		if((*it)->isLost())
			(*it)->onDeviceLost();
	}
	return true;
}
bool GLDevice::restore()
{
	std::list<Object*>::iterator it;
	for(it=m_listObject.begin();it!=m_listObject.end();++it)
	{
		if((*it)->isLost())
		if ((*it)->onDeviceRestore())
			(*it)->clearLost();
	}
	return true;
}

void GLDevice::getInputMap(std::map<std::pair<String,UINT>,UINT>& map,const GLInputLayout* desc)
{
	int i;
	UINT size = 0;
	INT count = desc->getCount();
	const GLLayoutData* data = desc->getData();
	for(i=0;i<count;i++)
	{
		map[std::pair<String,UINT>(data[i].name,0)] = size;
		size += data[i].count * sizeof(float);
	}
}
LPCSTR GLDevice::getTexturePath()
{
	return m_texturePath.c_str();
}
void GLDevice::setTexturePath(LPCSTR path)
{
	if(path)
		m_texturePath = path;
	else
		m_texturePath.clear();
}
#ifdef __ANDROID__
INT GLDevice::loadShaders(LPCSTR path)
{
	AAssetManager* am = AndroidApp::getAssetManager();
	AAssetDir* dir = AAssetManager_openDir(am, path);
	LPCSTR listName;
	while((listName = AAssetDir_getNextFileName(dir))!=NULL)
	{
		String fileName;
		fileName.printf("%s/%s",path,listName);
		INT nameLength = strlen(listName+4) - 4;
		LPSTR name = new CHAR[nameLength + 1];
		strncpy(name,listName+3,nameLength);
		name[nameLength] = 0;

		if(strncmp(listName,"VS_",2)==0)
		{
			addShader(name,GL_VERTEX_SHADER,fileName);
			__android_log_print( ANDROID_LOG_FATAL,"FILE","%s\n",fileName.c_str());
		}
		else if(strncmp(listName,"PS_",2)==0)
		{
			addShader(name,GL_FRAGMENT_SHADER,fileName);
			__android_log_print( ANDROID_LOG_FATAL,"FILE","%s\n",fileName.c_str());
		}
		delete[] name;

	}
	AAssetDir_close(dir);

	return 0;
}
#else
INT GLDevice::loadShaders(LPCWSTR path)
{
	WINDOWS::PathName pathName(path);
	pathName.pushPath();
	pathName.changePath();

	INT count = 0;
	WIN32_FIND_DATAW findFileData;
	HANDLE handle;
	if(handle = FindFirstFileW(L"*.glsl",&findFileData))
	{
		do
		{
			File file;
			if(file.open(findFileData.cFileName))
			{
				INT size = (INT)file.getLength();
				LPBYTE data = new BYTE[size];
				file.read(data,size);

				INT nameLength = wcslen(findFileData.cFileName+4) - 4;
				LPWSTR name = new WCHAR[nameLength + 1];
				wcsncpy(name,findFileData.cFileName+3,nameLength);
				name[nameLength] = 0;
				
				if(wcsncmp(findFileData.cFileName,L"VS_",2)==0)
				{
					addShader(UTF8(name),GL_VERTEX_SHADER,SJIS(findFileData.cFileName));
				}
				else if(wcsncmp(findFileData.cFileName,L"PS_",2)==0)
				{
					addShader(UTF8(name),GL_FRAGMENT_SHADER,SJIS(findFileData.cFileName));
				}
				delete[] name;
				delete[] data;
				count++;
			}
		}while(FindNextFileW(handle,&findFileData));
	}
	pathName.popPath();
	return count;
}
#endif


#ifndef __ANDROID__
GLDevice::GLDevice()
{
	m_hRC = NULL;
}
GLDevice::~GLDevice()
{
	m_listObject.clear();
	if(m_hRC)
	{
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(m_hRC);
	}
}

bool GLDevice::init(HWND hwnd)
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
	pfd.cStencilBits = 8;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	HDC hDC = GetDC(hwnd);
	GLuint iPixelFormat = ChoosePixelFormat( hDC, &pfd );

	PIXELFORMATDESCRIPTOR bestMatch_pfd;
	DescribePixelFormat( hDC, iPixelFormat, sizeof(pfd), &bestMatch_pfd );
	SetPixelFormat( hDC, iPixelFormat, &pfd);

	m_hRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC,m_hRC );

	glewInit();
	const GLubyte* v = glGetString(GL_VERSION);


	return true;
}



#pragma comment (lib, "gdiplus.lib")

GDIP::GDIP()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}
GDIP::~GDIP()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}
GDIP GLImage::m_gdip;

GLImage::GLImage()
{
	m_bitmap = NULL;
}
GLImage::~GLImage()
{
	release();

}
Gdiplus::Graphics* GLImage::getGraphics()
{
	return m_graphics.get();
}
bool GLImage::createImage(INT width,INT height,LPVOID buff)
{
	release();
	m_bitmap = new Gdiplus::Bitmap(width,height,width*4,PixelFormat32bppARGB,(BYTE*)buff);
	if(!m_bitmap)
		return false;
	m_graphics = new Gdiplus::Graphics(m_bitmap);
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	m_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);

	return true;
}

bool GLImage::openImage(LPCSTR fileName,bool filter)
{
	//読み込みパスのカスタム設定を適用
	String name;
	LPCSTR path = GLDevice::getTexturePath();
	if(*path && fileName[0] != '\\' && (fileName[0] && fileName[1] !=':'))
	{
		if(path[strlen(path)-1] != '\\')
		{
			name.appendf("%s\\%s",path,fileName);
		}
		else
		{
			name.appendf("%s%s",path,fileName);
		}
	}
	else
	{
		name = fileName;
	}

	m_bitmap = Gdiplus::Bitmap::FromFile(UCS2(fileName), PixelFormat32bppARGB);
	if (!m_bitmap || m_bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		m_bitmap = Gdiplus::Bitmap::FromFile(UCS2(name), PixelFormat32bppARGB);
		if (!m_bitmap)
		{
			String s;
			s.printf("File not found: %s\n", fileName);
			OutputDebugString(s.c_str());
			return false;
		}
	}
	if(m_bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete m_bitmap;
		m_bitmap = NULL;
		return false;
	}
	return true;
}
void GLImage::release()
{
	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = NULL;
	}
}
Gdiplus::BitmapData* GLImage::getBuffer()
{
	m_bitmap->LockBits(NULL,Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &m_data);
	return &m_data;
}
void GLImage::releaseBuffer()
{
	m_bitmap->UnlockBits(&m_data);
}
bool GLImage::resize(INT width,INT height)
{
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap( width, height,PixelFormat32bppARGB );
	if(!bitmap)
		return false;
	Gdiplus::Graphics* g = Gdiplus::Graphics::FromImage(bitmap);

	g->DrawImage (m_bitmap,0, 0, width, height);
	delete g;
	releaseBuffer();

	m_bitmap = bitmap;

	return true;
}
bool GLImage::getTextSize(LPCWSTR text, HFONT font, LPSIZE size)
{
	HDC hDC = CreateCompatibleDC(NULL);
	Gdiplus::Graphics g(hDC);


	Gdiplus::RectF rect, rect2;
	g.MeasureString(text, wcslen(text), &Gdiplus::Font(hDC, font), rect, &rect2);
	size->cx = (INT)rect2.Width;
	size->cy = (INT)rect2.Height;
	DeleteDC(hDC);
	return true;
}


void GLImage::drawText(LPCWSTR text, HFONT font, DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if (!g)
		return;

	HDC hDC = CreateCompatibleDC(NULL);
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	g->DrawString(text, -1, &Gdiplus::Font(hDC, font),
		Gdiplus::PointF(0, 0), Gdiplus::StringFormat::GenericDefault(), &Gdiplus::SolidBrush(Gdiplus::Color(color)));
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	g->Flush();
	DeleteDC(hDC);
}
#endif
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Object
// 3Dオブジェクト基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Object::Object()
{
	m_lost = false;;
	m_restore = false;;
	GLDevice::addObject(this);
}
Object::~Object()
{
	GLDevice::delObject(this);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GLShader
// シェーダリソース管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

GLShader::GLShader()
{
	m_shader = 0;
}
GLShader::~GLShader()
{
	release();
}
void GLShader::release()
{
	if(m_shader)
	{
		glDeleteShader(m_shader);
		m_shader = 0;
	}
}
void GLShader::setShader(GLint shader)
{
	release();
	m_shader = shader;
}
GLint GLShader::getShader() const
{
	return m_shader;
}

//シェーダーをロードする関数
bool GLShader::loadShader(GLenum type, const char* shaderSrc)
{
	m_backupSource = shaderSrc;
	m_backupType = type;
	m_lost = true;
	return true;
}
LPBYTE readFile(LPCSTR fileName)
{
#ifndef __ANDROID__
	File file;
	if (!file.open(fileName))
	{
		return NULL;
	}
	INT length = (INT)file.getLength();
	LPBYTE fileData = new BYTE[length + 1];
	file.read(fileData, length);
	fileData[length] = 0;
#else
	LPBYTE fileData = (LPBYTE)AndroidApp::readFile(fileName);
#endif
	return fileData;
}
class LineData
{
public:
	LineData(char* data)
	{
		m_point = 0;
		m_data = data;
	
		static const unsigned char bom[] = {0xef,0xbb,0xbf};
		int i;
		for (i = 0; i < 3 && m_data[i]; i++)
		{
			if (bom[i] != (unsigned char)m_data[i])
				break;
		}
		if (i == 3)
		{
			m_point = 3;
		}

	}
	bool get(String& s)
	{
		int i;
		char c;
		for (i = 0; c = m_data[m_point + i]; i++)
		{
			//改行まで読み込む
			if (c == '\r' || c == '\n')
			{
				s.assign(m_data + m_point, i);
				m_point += i+1;
				char c2 = m_data[m_point];
				if (c == '\r' && c2 == '\n')
					m_point++;
				else if (c == '\n' && c2 == '\r')
					m_point++;
				return true;
			}
		}
		if (i == 0)
		{	
			s.clear();
			return false;
		}
		//ファイル終端処理
		s.assign(m_data + m_point, i);
		m_point += i;
		return true;
	}
protected:
	INT m_point;
	char* m_data;
};

LPBYTE readFile2(LPCSTR fileName)
{
	LPBYTE fileData = readFile(fileName);

	if (!fileData)
		return NULL;

	String path;
	LPCSTR p;
	p = strrchr(fileName, '/');
	if(!p)
		p = strrchr(fileName, '\\');
	if (p)
	{
		path.assign(fileName, 0, p - fileName);
	}

	String work;


	const char* key = "#include";

	String s;
	LineData lineData((char*)fileData);
	while (lineData.get(s))
	{
		const char* pt = strstr(s.c_str(), "//");
		if (pt)
		{
			s.assign(s.c_str(),pt - s.c_str());
		}
		//include句のチェック
		if (strncmp(s.c_str(), key, strlen(key)) == 0)
		{
			LPCSTR data = s.c_str() + strlen(key);
			char fileName2[1024];
			if (sscanf(data, " \"%[^\"]\"", fileName2) == 1)
			{
				LPBYTE fileData2 = readFile(fileName2);
				if (!fileData2 && path.length())
				{
					String newPath;
					newPath.printf("%s/%s", path.c_str(), fileName2);
					fileData2 = readFile(newPath);
				}
				if (fileData2)
				{
					s = (char*)fileData2;
					delete[] fileData2;
				}
				else
					s.clear();
			}
		}
		work.appendf("%s\n",s.c_str());
	}
	//OutputDebugStringA(work);
	delete[] fileData;
	fileData = (LPBYTE)new char[work.length()+1];
	strcpy((char*)fileData, work);

	return fileData;

}
bool GLShader::loadShaderFile(GLenum type, const char* fileName)
{
	LPBYTE fileData = readFile2(fileName);
	if (!fileData)
		return NULL;
	m_fileName = fileName;

	bool ret = loadShader(type,(const char*)fileData);
	delete[] fileData;
	if(!ret)
	{
		#ifndef __ANDROID__
			OutputDebugStringA(fileName);
		#endif
		return false;
	}
	return true;
}
bool GLShader::onDeviceLost()
{
	release();
	return true;
}
bool GLShader::onDeviceRestore()
{
	release();
	if(!m_backupSource.size())
		return false;

	//シェーダーオブジェクト
	GLuint shader = 0;
	//シェーダーオブジェクトを作成する
	shader = glCreateShader(m_backupType);
	if(shader == 0)
		return false;

	//シェーダーソースをロードする
	LPCSTR src = m_backupSource.c_str();
	glShaderSource(shader, 1, &src, (const int*)0);
	//シェーダーをコンパイルする
	glCompileShader(shader);


	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE)
	{
		int length;
		//glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length );
		length = 512;
		// エラーログを取得
		char* log = new char[ length ];
		glGetShaderInfoLog( shader, length, NULL, log );
		#ifdef __ANDROID__
		__android_log_print( ANDROID_LOG_FATAL,"SHADER","[%s]\n%d %s\n",m_fileName.c_str(),length,log);
		#else
			OutputDebugStringA(log);
		#endif
		delete[] log;
		glDeleteShader(shader);
		return false;
	}
	m_shader = shader;
	clearLost();
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GLShaderPrrogam
// シェーダ制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
GLShaderProgram::GLShaderProgram()
{
	m_program = 0;
	m_cbIndex = new std::map<String,ShaderConstant>();
}
GLShaderProgram::~GLShaderProgram()
{
	release();
}
void GLShaderProgram::release()
{
	if(m_program)
	{
		glDeleteProgram(m_program);
		m_program = 0;
	}
}
GLuint GLShaderProgram::getProgram()
{
	if(isLost())
	{
		release();
		clearLost();
	}
	if(!m_program)
	{
		m_program = glCreateProgram();
		std::list<GLShader*>::iterator it;
		for(it=m_shader.begin();it!=m_shader.end();++it)
		{
			glAttachShader(getProgram(),(*it)->getShader());
		}

		//GLint m_strideSize = m_inputLayout.getData()[0].stride;

		//m_program->attachShader(m_vertexShader.get());

		const GLLayoutData* layout = m_inputLayout.getData();
		int count = m_inputLayout.getCount();
		int i;
		for(i=0;i<count;i++)
		{
			bindAttribLocation(i,layout[i].name);
		}
		linkProgram();
	}
	return m_program;
}

void GLShaderProgram::attachShader(GLShader* shader)
{
	m_shader.push_back(shader);
	setLost();
}
void GLShaderProgram::detachShader(GLShader* shader)
{
	m_shader.remove(shader);
	setLost();
}
void GLShaderProgram::bindAttribLocation(GLuint index, const GLchar* name)
{
	glBindAttribLocation(getProgram(),index,name);
}
void GLShaderProgram::linkProgram()
{
	glLinkProgram(getProgram());
}
void GLShaderProgram::useProgram()
{
	glUseProgram(getProgram());
}
void GLShaderProgram::uniformMatrix(const GLchar* name,NMatrix* matrix,INT count)
{
	GLint nMtxLoc = glGetUniformLocation( getProgram(), name );
	if(nMtxLoc > -1)
		glUniformMatrix4fv(nMtxLoc, count, GL_FALSE, (FLOAT*)matrix);
}
void GLShaderProgram::uniformFloat(const GLchar* name,float* data,INT count)
{
	GLint nMtxLoc = glGetUniformLocation( getProgram(), name );
	if(nMtxLoc > -1)
		glUniform1fv(nMtxLoc, count, (FLOAT*)data);
}
void GLShaderProgram::uniformInt(const GLchar* name,int* data,INT count)
{
	GLint nMtxLoc = glGetUniformLocation( getProgram(), name );
	if(nMtxLoc > -1)
		glUniform1iv(nMtxLoc, count, (INT*)data);
}
void GLShaderProgram::setLayout(GLInputLayout* layout)
{
	m_inputLayout = *layout;
}
GLInputLayout* GLShaderProgram::getLayout()
{
	return &m_inputLayout;
}
void GLShaderProgram::update(const GLchar* name,LPCVOID data)
{
	std::map<String,ShaderConstant>::iterator it = m_cbIndex->find(name);
	if(it == m_cbIndex->end())
		return;
	ShaderConstant& sc = it->second;
	switch(sc.type)
	{
	case GL_INT:
		glUniform1iv(sc.index,sc.size,(GLint*)data);
		break;
	case GL_FLOAT:
		glUniform1fv(sc.index,sc.size,(GLfloat*)data);
		break;
	case GL_FLOAT_VEC4:
		glUniform4fv(sc.index,sc.size,(GLfloat*)data);
		break;
	case GL_FLOAT_MAT4:
		glUniformMatrix4fv(sc.index,sc.size,false,(GLfloat*)data);
		break;
	}
}

bool GLShaderProgram::onDeviceLost()
{
	release();
	return true;
}
bool GLShaderProgram::onDeviceRestore()
{
	m_cbIndex->clear();

	GLint count;
	glGetProgramiv(getProgram(),GL_ACTIVE_UNIFORMS,&count);

	int i;
	for(i=0;i<count;i++)
	{
		GLsizei length,size;
		GLenum type;
		GLchar buff[200];
		glGetActiveUniform(getProgram(),i,200,&length,&size,&type,buff);
		//配列の名称を修正
		GLchar* bl = strchr(buff,'[');
		if (bl)
			*bl = '\0';
		ShaderConstant sc;
		sc.index = i;
		sc.size = size;
		sc.type = type;
		(*m_cbIndex.get())[buff] = sc;
	}

	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderBuffer
// オフラインレンダリング用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

RenderBuffer::RenderBuffer()
{
	m_renderBuffer = 0;
}
RenderBuffer::~RenderBuffer()
{
	release();
}
void RenderBuffer::release()
{
	if(m_renderBuffer)
	{
		glDeleteRenderbuffers(1,&m_renderBuffer);
		m_renderBuffer = 0;
	}
}
bool RenderBuffer::create(INT width,INT height)
{
	m_texture.create(width,height);


	m_width = m_texture.getTextureWidth();
	m_height = m_texture.getTextureHeight();

	setLost();
	return true;
}
RenderBuffer::operator GLuint() const
{
	return m_renderBuffer;
}
INT RenderBuffer::getWidth() const
{
	return m_width;
}
INT RenderBuffer::getHeight() const
{
	return m_height;
}
Texture& RenderBuffer::getTexture()
{
	return m_texture;
}
bool RenderBuffer::onDeviceLost()
{
	release();
	return true;
}
bool RenderBuffer::onDeviceRestore()
{
	if (!m_texture)
	{
		return false;
	}
	glGenRenderbuffers(1, &m_renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_texture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);

	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// オフラインバッファ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Screen::Screen()
{
	m_width = 0;
	m_height = 0;
	m_frameBuffer = 0;

	//塗り潰し用の頂点設定
	const static GLLayout layout[] =
	{
		"POSITION", 3, GL_FLOAT,
		"TEXCOORD",2,GL_FLOAT
	};

	if(!Vertex::getLayout("FLAT"))
	{
		Vertex::addLayout("FLAT",layout,2);
	}


	m_mesh.createLayout("FLAT","FLAT");
	m_mesh.setPShader("TEXTURE");

	const static FLOAT vertex[] = 
	{
		-1.0f,-1.0f, 0.0f, 0,0,
		-1.0f, 1.0f ,0.0f, 0,1,
		1.0f, 1.0f,0.0f,   1,1,
		-1.0f,-1.0f, 0.0f, 0,0,
		1.0f, 1.0f,0.0f,   1,1,
		1.0f,-1.0f ,0.0f,  1,0
	};

	m_mesh.createMesh(vertex,sizeof(vertex));
}
Screen::~Screen()
{

}
bool Screen::onDeviceLost()
{
	release();
	return true;
}
bool Screen::onDeviceRestore()
{
	if (m_renderBuffer == 0)
		return false;

	GLint oldBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldBuffer);

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderBuffer.getTexture(), 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, oldBuffer);

	return true;
}
void Screen::release()
{
	if(m_frameBuffer)
	{
		m_renderBuffer.release();
		glDeleteFramebuffers(1,&m_frameBuffer);
		m_frameBuffer = 0;
	}
}
bool Screen::clear(FLOAT r,FLOAT g,FLOAT b,FLOAT a)
{
	setTarget();
	glDepthMask(GL_TRUE);
	//glStencilMask(GL_TRUE);
	glClearStencil(0);
	glClearColor( r,g,b,a );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
	return true;
}
void Screen::setTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLint width,height;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH , &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT , &height);

	glViewport(0,0,width,height);
}
bool Screen::present()
{
#if defined(_OPENGL) || defined(__ANDROID__)
	glViewport(0, 0, getWidth(), getHeight());
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLint width,height;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH , &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT , &height);
	glViewport(0,0,width,height);
	//glViewport(0,0,640,480);

	m_mesh.useProgram();
	m_mesh.bindLayout();

	glCullFace(GL_FRONT);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	
	glBindTexture(GL_TEXTURE_2D, m_renderBuffer.getTexture());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *m_mesh.getIndex());
	glDrawElements(GL_TRIANGLES,m_mesh.getIndex()->getSize()/2, GL_UNSIGNED_SHORT, 0);
	
	return true;
}
bool Screen::createScreen(UINT width,UINT height,LPVOID hWnd)
{
	m_width = width;
	m_height = height;
	m_renderBuffer.create(width,height);
	setLost();
	return true;
}
bool Screen::setSize(UINT width,UINT height)
{
	return true;
}
void Screen::setCamera(Camera* camera)
{
	m_camera = camera;
}
INT Screen::getWidth() const
{
	return m_renderBuffer.getWidth();
}
INT Screen::getHeight() const
{
	return m_renderBuffer.getHeight();
}
	



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Buffer
// バッファデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Buffer::Buffer()
{
	m_backupData = NULL;
	m_buffer = 0;
}
Buffer::~Buffer()
{
	release();
	if(m_backupData)
		delete[] m_backupData;
}

LPVOID Buffer::lock() const
{
	if(!m_buffer)
		return NULL;
	return glMapBuffer(m_buffer,GL_WRITE_ONLY);
}
void Buffer::unlock() const
{
	glUnmapBuffer(m_buffer);
}

UINT Buffer::getSize() const
{
	if(!m_buffer)
		return 0;
	return m_backupSize;
}

bool Buffer::create(LPCVOID data,UINT size,UINT usage)
{
	if(m_backupData)
	{
		delete[] m_backupData;
	}
	m_backupData = new BYTE[size];
	memcpy(m_backupData,data,size);
	m_backupSize = size;
	m_backupUsage = usage;

	setLost();
	return true;
}
bool Buffer::create(UINT size)
{
	//リソースの解放
	release();

	m_backupData = new BYTE[size];
	m_backupSize = size;
	return true;
}
void Buffer::release()
{
	if(m_buffer)
	{
		glDeleteBuffers(1,&m_buffer);
		m_buffer = 0;
	}
}
Buffer::operator GLuint()
{
	if (this == NULL)
		return 0;
	if(!m_buffer && m_backupData)
	{
		//バッファの確保
		glGenBuffers(1, &m_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		glBufferData(GL_ARRAY_BUFFER, m_backupSize, m_backupData, GL_STATIC_DRAW);
	}
	return m_buffer;
}
bool Buffer::onDeviceLost()
{
	release();
	return true;
}
bool Buffer::onDeviceRestore()
{
	if (*this)
		return true;
	return false;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// 頂点データ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Vertex::Vertex()
{
}
Vertex::~Vertex()
{
}
void Vertex::addLayout(LPCSTR name,const GLLayout* layout,INT count)
{
	m_layout[name].setLayout(layout,count);
}
GLInputLayout* Vertex::getLayout(LPCSTR name)
{
	std::map<String,GLInputLayout>::iterator it = m_layout.find(name);
	if(it == m_layout.end())
		return NULL;
	return &it->second;
}
INT Vertex::getLayoutCount(LPCSTR name)
{
	return m_layout[name].getCount();
}
std::map<String,GLInputLayout> Vertex::m_layout;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Index
// インデックスデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Index::Index()
{
}
Index::~Index()
{
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Texture::Texture()
{
	m_texture = 0;
	m_backupData = NULL;
}
Texture::~Texture()
{
	release();
	if(m_backupData)
		delete[] (char*)m_backupData;
}
void Texture::release()
{
	if(m_texture)
	{
		glDeleteTextures(1,&m_texture);
		m_texture = 0;
	}
}
INT getB2(INT value)
{
	int i;
	int c = 0;
	INT v = value;
	for(i=0;v;i++)
	{
		if(v&1)
			c++;
		v >>= 1;
	}
	return 1<<(i+(c>1?0:-1));
}


#ifdef __ANDROID__
bool Texture::open(LPCSTR fileName, bool filter)
{
	//読み込みパスのカスタム設定を適用
	String name;
	LPCSTR path = GLDevice::getTexturePath();
	if (*path && fileName[0] != '\\' && fileName[0] != '/' && (fileName[0] && fileName[1] != ':'))
	{
		if (path[strlen(path) - 1] != '\\' && path[strlen(path) - 1] != '/')
		{
			name.appendf("%s/%s", path, fileName);
		}
		else
		{
			name.appendf("%s%s", path, fileName);
		}
	}
	else
	{
		name = fileName;
	}

	if (m_backupData)
	{
		delete[](char*)m_backupData;
		m_backupData = NULL;
	}
	__android_log_print(ANDROID_LOG_FATAL, "Image", "%s", name.c_str());

	SIZE size;
	if (!AndroidApp::getImageSize(name, &size))
		return false;

	GLuint textureWidth = getB2(size.cx);
	GLuint textureHeight = getB2(size.cy);
	__android_log_print(ANDROID_LOG_FATAL, "Image", "%d,%d\n", textureWidth, textureHeight);

	int length = textureWidth*textureHeight * 4;
	char* data = new char[length];
	if (!AndroidApp::openImage(name, data, textureWidth, textureHeight, filter))
	{
		return false;
	}

	m_backupData = data;
	m_imageWidth = size.cx;
	m_imageHeight = size.cy;
	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;
	//リソースロスト通知
	setLost();

	return true;
}
#else
bool Texture::open(LPCSTR fileName, bool filter)
{
	GLImage image;
	if (!image.openImage(fileName))
	{
		return false;
	}

	Gdiplus::BitmapData* data = image.getBuffer();
	m_imageWidth = data->Width;
	m_imageHeight = data->Height;

	GLuint textureWidth = getB2(data->Width);
	GLuint textureHeight = getB2(data->Height);

	if (data->Width != textureWidth || data->Height != textureHeight)
	{
		image.releaseBuffer();
		image.resize(textureWidth, textureHeight);
		data = image.getBuffer();
	}

	LPBYTE p = (LPBYTE)data->Scan0;
	UINT i, j;
	for (j = 0; j<textureHeight; j++)
	{
		for (i = 0; i<textureWidth; i++)
		{
			BYTE b = p[i * 4 + 0];
			p[i * 4 + 0] = p[i * 4 + 2];
			p[i * 4 + 2] = b;
		}
		p += data->Stride;
	}

	//イメージデータの保存
	if (m_backupData)
		delete[](char*)m_backupData;
	INT length = textureHeight*textureWidth * 4;
	m_backupData = new BYTE[length];
	memcpy(m_backupData, data->Scan0, length);
	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;

	setLost();
	return true;
}
#endif
bool Texture::create(UINT width, UINT height)
{
	release();
	m_textureWidth = width;
	m_textureHeight = height;
	m_imageWidth = width;
	m_imageHeight = height;

	if (m_backupData)
		delete[](char*)m_backupData;
	INT length = m_textureWidth*m_textureHeight * 4;
	m_backupData = new BYTE[length];
	return true;
}
bool Texture::createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth,bool mline)
{
	return createText(UCS2(string),size,color,bcolor,limitWidth,mline);
}
bool Texture::createText(LPCWSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth,bool mline)
{
	//フォントサイズの取得
	INT imageSize[2];
#ifdef __ANDROID__
	AndroidApp::getFontSize(imageSize,string,size,limitWidth,mline);
#else
	GLImage image;
	WINDOWS::Font font;
	font.setSize(size);
	image.getTextSize(string,font,(LPSIZE)&imageSize);
#endif
	if(!imageSize[0] || !imageSize[1])
		return false;

	//テクスチャサイズを2のべき乗に
	GLuint textureWidth = getB2(imageSize[0]);
	GLuint textureHeight = getB2(imageSize[1]);

	//バッファサイズ
	int length = textureWidth*textureHeight*4;
	char* data = new char[length];
	ZeroMemory(data,length);

	//フォントイメージの取得
#ifdef __ANDROID__
	AndroidApp::getFontImage(data,textureWidth,textureHeight,string,size,color,bcolor,limitWidth,mline);
#else
//	GLImage image;
	image.createImage(textureWidth,textureHeight,data);
	image.drawText(string,font,color);
#endif

	//サイズの保存
	m_imageWidth = imageSize[0];
	m_imageHeight = imageSize[1];
	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;

	//イメージデータの保存
	if(m_backupData)
		delete[] (char*)m_backupData;
	m_backupData = data;
	setLost();
	return true;
}

bool Texture::clear(DWORD color)
{
	return true;
}
INT Texture::getImageHeight() const
{
	return m_imageHeight;
}
INT Texture::getImageWidth() const
{
	return m_imageWidth;
}
INT Texture::getTextureWidth() const
{
	return m_textureWidth;
}
INT Texture::getTextureHeight() const
{
	return m_textureHeight;
}
Texture::operator GLuint()
{
	if (!m_texture && m_backupData)
	{
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_backupData);
	}
	return m_texture;
}
bool Texture::onDeviceLost()
{
	release();
	return true;
}
bool Texture::onDeviceRestore()
{
	if (*this)
		return true;
	return false;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TextureDepth
// 深度テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
TextureDepth::TextureDepth()
{
	m_texture = 0;
}
TextureDepth::~TextureDepth()
{
	release();
}
void TextureDepth::release()
{
	if(m_texture)
	{
		glDeleteTextures(1,&m_texture);
		m_texture = 0;
	}
}
bool TextureDepth::onDeviceLost()
{
	release();
	return true;
}
bool TextureDepth::onDeviceRestore()
{
	release();
	clearLost();
	return true;
}
TextureDepth::operator GLuint()
{
	if(!m_texture)
	{
		glGenTextures( 1, &m_texture );
		glBindTexture( GL_TEXTURE_2D, m_texture);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	return m_texture;
}

bool TextureDepth::getDepth(INT width,INT height)
{
	glBindTexture( GL_TEXTURE_2D, *this );
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,0, 0, width, height,0 );
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// Meshデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Mesh::Mesh()
{
	m_blendMode = 0;
	m_strideSize = 0;
	m_shadow = false;
	m_program = new GLShaderProgram();
}
Mesh::~Mesh()
{
}
Mesh::Mesh(const Mesh &obj)
{
	m_program = obj.m_program;
	m_vertexShader = obj.m_vertexShader;
	m_fragmentShader = obj.m_fragmentShader;
	m_texture = obj.m_texture;
	m_vertex = obj.m_vertex;
	m_index = obj.m_index;
	m_boneMatrices = obj.m_boneMatrices;
	m_material = obj.m_material;
	CopyMemory(m_range, obj.m_range, sizeof(m_range));
	m_strideSize = obj.m_strideSize;
	m_blendMode = obj.m_blendMode;
	m_shadow = obj.m_shadow;


}
bool Mesh::createLayout(LPCSTR VName,LPCSTR layoutName)
{
	SP<GLShader> shader = GLDevice::getShader(GL_VERTEX_SHADER,VName);
	if(!shader.get())
		return false;

	if(m_vertexShader.get())
		m_program->detachShader(m_vertexShader.get());

	m_vertexShader = shader;
	m_program->attachShader(m_vertexShader.get());

	GLInputLayout* inputLayout = Vertex::getLayout(layoutName);
	if(!inputLayout)
		return false;

	m_program->setLayout(inputLayout);
	m_strideSize = inputLayout->getData()[0].stride;


	return true;
}
bool Mesh::createMesh(LPCVOID data,UINT size)
{
	if (!m_strideSize)
		return false;

	INT i;
	INT count = size / m_strideSize;
	LPCBYTE adr = (LPCBYTE)data;

	std::vector<USHORT> convertIndex;
	convertIndex.reserve(count);
	std::map<VERTEXPT,USHORT>::iterator itMap;
	std::map<VERTEXPT,USHORT> dataMap;

	//頂点データから最適なインデックスを作成
	USHORT index = 0;
	for(i=0;i<count;i++)
	{
		VERTEXPT vpt(adr+m_strideSize*i,m_strideSize);
		itMap = dataMap.find(vpt);
		if(itMap != dataMap.end())
			index = (*itMap).second;
		else
		{
			index = (USHORT)dataMap.size();
			dataMap[vpt] = index;
		}
		convertIndex.push_back(index);
	}

	UINT vertexCount = dataMap.size();
	LPBYTE vertexNew = NEW BYTE[vertexCount*m_strideSize];
	for(itMap=dataMap.begin();itMap!=dataMap.end();++itMap)
	{
		INT index = itMap->second;
		const VERTEXPT* vpt = &itMap->first;
		memcpy(vertexNew+m_strideSize*index,vpt->data,vpt->size);
	}
	m_index = NEW Index;
	m_index->create(&convertIndex[0],convertIndex.size()*sizeof(USHORT));
	m_vertex = NEW Vertex;
	m_vertex->create(vertexNew,vertexCount*m_strideSize);
	//bindLayout();
	delete[] vertexNew;



	return true;
}
void Mesh::bindLayout()
{
	if(getVertex())
	{
		//レイアウトの取得
		const GLLayoutData* layout = getLayout();
		int count = getLayoutCount();
		int i;
		glBindBuffer(GL_ARRAY_BUFFER, *getVertex());
		for(i=0;i<count;i++)
		{
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, layout[i].count, layout[i].type, GL_FALSE, layout[i].stride, (LPBYTE)layout[i].offset);
		}
		for(;i<16;i++)
			glDisableVertexAttribArray(i);
	}
}

GLint Mesh::getVertexShader() const
{
	return m_vertexShader->getShader();
}
GLint Mesh::getFragmentShader() const
{
	return m_fragmentShader->getShader();
}
Vertex* Mesh::getVertex() const
{
	return m_vertex.get();
}
Index* Mesh::getIndex() const
{
	return m_index.get();
}
Texture* Mesh::getTexture() const
{
	if(m_texture.size())
		return m_texture.front().get();
	return NULL;
}
Texture* Mesh::getTexture(INT index) const
{
	if(index >= (INT)m_texture.size())
		return NULL;
	std::list<SP<Texture> >::const_iterator it = m_texture.begin();
	std::advance( it, index );
	return (*it).get();
}
bool Mesh::createVertex(LPCVOID data,UINT size)
{
	m_vertex = new Vertex;
	bool ret = m_vertex->create(data,size);
	return ret;
}
bool Mesh::createIndex(LPCVOID data,UINT size)
{
	m_index = new Index;
	return m_index->create(data,size);
}

bool Mesh::openTexture(LPCSTR fileName,bool filter)
{
	Texture* texture = new Texture();
	if(!texture->open(fileName,filter))
	{
		delete texture;
		return false;
	}
	setTexture(texture);
	_createFromTexture(m_texture.front());
	return true;
}
void Mesh::addTexture(Texture* texture)
{
	m_texture.push_back(texture);
}
void Mesh::addTexture(SP<Texture>& texture)
{
	m_texture.push_back(texture);
}
void Mesh::setTexture(Texture* texture)
{
	m_texture.clear();
	m_texture.push_back(texture);
}
void Mesh::setTexture(SP<Texture>& texture)
{
	m_texture.clear();
	m_texture.push_back(texture);
}

void Mesh::setMaterial(Material& material)
{
	m_material = material;
}
Material* Mesh::getMaterial()
{
	return &m_material;
}
void Mesh::setBoneMatrix(std::vector<BONEMATRIX>& matrices)
{
	m_boneMatrices = matrices;
}
std::vector<BONEMATRIX>*  Mesh::getBoneMatrix()
{
	return &m_boneMatrices;
}
UINT Mesh::getStrideSize() const
{
	return m_strideSize;
}
NVector* Mesh::getVertexRange()
{
	return m_range;
}
void Mesh::setVertexRange(NVector* min,NVector* max)
{
	m_range[0] = *min;
	m_range[1] = *max;
}

const GLLayoutData* Mesh::getLayout() const
{
	return m_program->getLayout()->getData();
}
INT Mesh::getLayoutCount()
{
	return m_program->getLayout()->getCount();  
}
void Mesh::useProgram()
{
	m_program->useProgram();
}
GLShaderProgram* Mesh::getProgram()
{
	return m_program.get();
}
INT Mesh::getTextureCount() const
{
	return m_texture.size();
}
void Mesh::setBlendMode(INT mode)
{
	m_blendMode = mode;
}
INT Mesh::getBlendMode() const
{
	return m_blendMode;
}
bool Mesh::isShadow()const
{
	return m_shadow; 
}
void Mesh::setShadow(bool flag)
{
	m_shadow = flag; 
}
bool Mesh::setPShader(LPCSTR PName)
{
	SP<GLShader> shader = GLDevice::getShader(GL_FRAGMENT_SHADER, PName);
	if (!shader.get())
		return false;

	if (m_fragmentShader.get())
		m_program->detachShader(m_fragmentShader.get());

	m_fragmentShader = shader;
	m_program->attachShader(m_fragmentShader.get());
	return true;
}
bool Mesh::setVShader(LPCSTR VName)
{
	SP<GLShader> shader = GLDevice::getShader(GL_VERTEX_SHADER, VName);
	if (!shader.get())
		return false;

	if (m_vertexShader.get())
		m_program->detachShader(m_vertexShader.get());

	m_vertexShader = shader;
	m_program->attachShader(m_vertexShader.get());
	return true;
}
void Mesh::updateVS(LPCSTR name, LPCVOID data)
{
	m_program->update(name, data);
}
void Mesh::updatePS(LPCSTR name, LPCVOID data)
{
	m_program->update(name, data);
}
bool Mesh::_createFromTexture(SP<Texture>& texture)
{
	GETVERTEX vertex;
	if(!_getPointVertex(&vertex,texture.get()))
		return false;

	FLOAT vertexSprite[] =
	{
		 vertex.fX1,vertex.fY1, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX,             vertex.fTY,
		 vertex.fX2,vertex.fY1, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX+vertex.fTWidth,vertex.fTY,
		 vertex.fX1,vertex.fY2, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX,             vertex.fTY+vertex.fTHeight,
		 vertex.fX2,vertex.fY2, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX+vertex.fTWidth,vertex.fTY+vertex.fTHeight
	};

	GLushort index[]={0,1,2,3,2,1};

	//メッシュの作成
	createLayout("BILLBOARD","PLANE");
	setPShader("TCOLOR");
	createVertex(vertexSprite,sizeof(vertexSprite));
	createIndex(index,sizeof(index));

	return true;
}
bool Mesh::createIndexAuto(INT count)
{
	static WORD wIndex[] = { 0, 1, 2, 3, 2, 1 };
	LPWORD data = new WORD[count * 6];
	INT i;
	for (i = 0; i < count; i++)
	{
		data[i * 6 + 0] = i * 4 + 0;
		data[i * 6 + 1] = i * 4 + 1;
		data[i * 6 + 2] = i * 4 + 2;
		data[i * 6 + 3] = i * 4 + 3;
		data[i * 6 + 4] = i * 4 + 2;
		data[i * 6 + 5] = i * 4 + 1;
	}
	bool flag = createIndex(data, sizeof(WORD)*count * 6);
	delete[] data;
	return flag;
}
bool Mesh::_getPointVertex(PGETVERTEX vertexSprite,Texture* textureSrc)
{
	if(!textureSrc)
		return false;

	//サイズの保存
	INT width = textureSrc->getImageWidth();
	INT height = textureSrc->getImageHeight();
	INT textureWidth = textureSrc->getTextureWidth();
	INT textureHeight = textureSrc->getTextureHeight();

	INT x=0;
	INT y=0;


	vertexSprite->fX1 = 0.0f;
	vertexSprite->fX2 = (FLOAT)width;
	vertexSprite->fY1 = 0.0f;
	vertexSprite->fY2 = (FLOAT)height;

	vertexSprite->fTX = x / (FLOAT)textureWidth;
	vertexSprite->fTY = y / (FLOAT)textureHeight;
	vertexSprite->fTWidth = (FLOAT)width/textureWidth;
	vertexSprite->fTHeight = (FLOAT)height/textureHeight;

	return true;
}
/*
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// Frameデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Frame::Frame()
{
	m_matrix.setIdentity();
}
NMatrix Frame::getMatrix() const
{
	return m_matrix;
}

void Frame::setName(LPCSTR name)
{
	m_frameName = name;
}
void Frame::add(Frame* frame)
{
	m_frameChilds.push_back(*frame);
}
void Frame::add(Mesh* mesh)
{
	m_meshes.push_back(*mesh);
}
void Frame::setMatrix(NMatrix* matrix)
{
	m_matrix = *matrix;
}
INT Frame::getFrameCount()
{
	return (INT)m_frameChilds.size();
}
INT Frame::getMeshCount()
{
	return (INT)m_meshes.size();
}
INT Frame::getAllFrameCount()
{
	INT iCount = 0;
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it)->getAllFrameCount();
	}
	iCount += getFrameCount();
	return iCount;
}
INT Frame::getAllMeshCount()
{
	INT iCount = 0;
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it)->getAllMeshCount();
	}
	iCount += getMeshCount();
	return iCount;
}

void Frame::createBoundingBox()
{

	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		(*itFrame)->createBoundingBox();
	}

	if(m_meshes.size())
	{
		NVector vect[2];
		std::list<Mesh>::iterator itMesh;
		itMesh = m_meshes.begin();
		vect[0] = itMesh->getVertexRange()[0];
		vect[1] = itMesh->getVertexRange()[1];

		for(;itMesh!=m_meshes.end();++itMesh)
		{
			NVector v[2] = {itMesh->getVertexRange()[0],itMesh->getVertexRange()[1]};
			vect[0] = vect[0].minimum(v[0]);
			vect[1] = vect[1].maximum(v[1]);
		}

		m_bounding[0] = vect[0];
		m_bounding[1] = vect[1];
	}
	else
	{
		ZeroMemory(m_bounding,sizeof(m_bounding));
	}

	NVector vectBox[8] =
	{
		m_bounding[0].x,m_bounding[0].y,m_bounding[0].z,
		m_bounding[0].x,m_bounding[0].y,m_bounding[1].z,
		m_bounding[0].x,m_bounding[1].y,m_bounding[0].z,
		m_bounding[1].x,m_bounding[0].y,m_bounding[0].z,
		m_bounding[1].x,m_bounding[1].y,m_bounding[0].z,
		m_bounding[0].x,m_bounding[1].y,m_bounding[1].z,
		m_bounding[1].x,m_bounding[0].y,m_bounding[1].z,
		m_bounding[1].x,m_bounding[1].y,m_bounding[1].z,
	};
	memcpy(m_boundingBox,vectBox,sizeof(NVector)*8);
}

void Frame::getBoundingBox(Unit* unit,NMatrix* pTopMatrix,NMatrix* pMatrix,NVector* vect)
{
	NMatrix matrix,matAnimation;

	if(unit->getAnimationMatrix(getFrameName(),matAnimation))
		matrix = matAnimation * *pMatrix;
	else
		matrix = (NMatrix)m_matrix * *pMatrix;

	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		(*itFrame)->getBoundingBox(unit,pTopMatrix,&matrix,vect);
	}

	if(m_meshes.size() == 0)
		return;

	//m_matrixCache = matrix * *pTopMatrix;
	NMatrix matrixCache = matrix * *pTopMatrix;

	INT i;
	for(i=0;i<8;i++)
	{
		NVector vectSrc;
		vectSrc = m_boundingBox[i].transformCoord(matrixCache);
		vect[0] = vect[0].minimum(vectSrc);
		vect[1] = vect[1].maximum(vectSrc);
	}

}


std::list<SP<Frame> >& Frame::getFrameChilds()
{
	return m_frameChilds;
}
std::list<Mesh>& Frame::getMeshes()
{
	return m_meshes;
}
LPCSTR Frame::getFrameName() const
{
	return m_frameName.c_str();
}
*/

