#include <windows.h>

#include <map>
#include <vector>
#include <INITGUID.H>
//#define INITGUID
#include "aflD3D11.h"
#include "aflD3DUnit.h"
//#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "DXGUID.lib")
#pragma comment(lib, "D3D11.lib")
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

using namespace Gdiplus;

namespace AFL
{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Direct3DDevice11
// D3DDevice管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Direct3DDevice11::Direct3DDevice11()
{
	//デバイスポインタの初期化
	m_device = NULL;
	m_context = NULL;
	m_blendAlpha = NULL;
	m_blendAdd = NULL;
	m_blendNone = NULL;

	HRESULT hr;
	//デフォルトレベルでデバイスの作成
	hr = D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,0,NULL,0,D3D11_SDK_VERSION,&m_device,&m_level,&m_context);
	//失敗したらWARPを作成
	if(hr != S_OK)
		hr = D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_WARP,NULL,0,NULL,0,D3D11_SDK_VERSION,&m_device,&m_level,&m_context);
	
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc,sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc,&m_blendAlpha);

	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc,&m_blendAdd);

	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = false;
	m_device->CreateBlendState(&blendDesc,&m_blendNone);

	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendDesc,&m_blendSet);


	//GDI+の初期化
	m_token=0;
	Gdiplus::GdiplusStartupInput si;
	Gdiplus::GdiplusStartup(&m_token, &si, NULL);

}
ID3D11BlendState* Direct3DDevice11::getBlendAlpha() const
{
	return m_blendAlpha;
}
ID3D11BlendState* Direct3DDevice11::getBlendSet() const
{
	return m_blendSet;
}
ID3D11BlendState* Direct3DDevice11::getBlendAdd() const
{
	return m_blendAdd;
}
ID3D11BlendState* Direct3DDevice11::getBlendNone() const
{
	return m_blendNone;
}

Direct3DDevice11::~Direct3DDevice11()
{
	Gdiplus::GdiplusShutdown(m_token);
	if(m_device)
		m_device->Release();
	if(m_context)
		m_context->Release();
}
Direct3DDevice11::operator ID3D11Device*() const
{
	return m_device;
}
ID3D11Device* Direct3DDevice11::operator->() const
{
	return m_device;
}

ID3D11DeviceContext* Direct3DDevice11::getContext() const
{
	return m_context;
}
D3D_FEATURE_LEVEL Direct3DDevice11::getFeatureLevel() const
{
	return m_level;
}

bool Direct3DDevice11::isDevice() const
{
	return m_device != NULL;
}
ID3D11Device* D3DDevice::getDevice()
{
	return m_device;
}
bool D3DDevice::isDevice()
{
	return m_device.isDevice();
}
ID3D11DeviceContext* D3DDevice::getContext()
{
	return m_device.getContext();
}

D3D11_INPUT_ELEMENT_DESC_CUSTOM::D3D11_INPUT_ELEMENT_DESC_CUSTOM()
{
	SemanticName = NULL;
}
D3D11_INPUT_ELEMENT_DESC_CUSTOM::~D3D11_INPUT_ELEMENT_DESC_CUSTOM()
{
	if(SemanticName)
		delete[] SemanticName;
}
D3D11_INPUT_ELEMENT_DESC_CUSTOM::D3D11_INPUT_ELEMENT_DESC_CUSTOM(const D3D11_INPUT_ELEMENT_DESC& src)
{
	LPSTR name = NEW CHAR[strlen(src.SemanticName)+1];
	strcpy(name,src.SemanticName);
	SemanticName = name;
	SemanticIndex = src.SemanticIndex;
	Format = src.Format;
	InputSlot = src.InputSlot;
	AlignedByteOffset = src.AlignedByteOffset;
	InputSlotClass = src.InputSlotClass;
	InstanceDataStepRate = src.InstanceDataStepRate;
}
D3D11_INPUT_ELEMENT_DESC_CUSTOM& D3D11_INPUT_ELEMENT_DESC_CUSTOM::operator =(const D3D11_INPUT_ELEMENT_DESC& src)
{
	if(SemanticName)
		delete[] SemanticName;

	LPSTR name = NEW CHAR[strlen(src.SemanticName)+1];
	strcpy(name,src.SemanticName);
	SemanticName = name;
	SemanticIndex = src.SemanticIndex;
	Format = src.Format;
	InputSlot = src.InputSlot;
	AlignedByteOffset = src.AlignedByteOffset;
	InputSlotClass = src.InputSlotClass;
	InstanceDataStepRate = src.InstanceDataStepRate;
	return *this;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// D3DDevice
// デバイス操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
LPCSTR D3DDevice::getVSName()
{
	switch(m_device.getFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_11_0:
		return "vs_5_0";
	case D3D_FEATURE_LEVEL_10_1:
		return "vs_4_1";
	case D3D_FEATURE_LEVEL_10_0:
		return "vs_4_0";
	case D3D_FEATURE_LEVEL_9_3:
		return "vs_4_0_level_9_3";
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
		return "vs_4_0_level_9_1";
	}
	return NULL;
}
LPCSTR D3DDevice::getPSName()
{
	switch(m_device.getFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_11_0:
		return "ps_5_0";
	case D3D_FEATURE_LEVEL_10_1:
		return "ps_4_1";
	case D3D_FEATURE_LEVEL_10_0:
		return "ps_4_0";
	case D3D_FEATURE_LEVEL_9_3:
		return "ps_4_0_level_9_3";
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
		return "ps_4_0_level_9_1";
	}
	return NULL;
}

bool D3DDevice::createTexture(HWND hwnd)
{

	LUID luidN={0,0};

	IDXGIDevice* giDevice = NULL;
	m_device->QueryInterface(__uuidof(IDXGIDevice),(LPVOID*)&giDevice);

	IDXGIAdapter* adapter;
	giDevice->GetAdapter(&adapter);
	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);


	HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
	INT (WINAPI* DwmpDxGetWindowSharedSurface)(HWND,LUID,DWORD,DWORD,DXGI_FORMAT*,HANDLE*,LPVOID*)
		=(INT (WINAPI*)(HWND,LUID,DWORD,DWORD,DXGI_FORMAT*,HANDLE*,LPVOID*))GetProcAddress(hDwmApi,(LPCSTR)100);

	INT (WINAPI* DwmpDxUpdateWindowSharedSurface)(HWND, int, int, int, HMONITOR, void*) = 
		(INT (WINAPI*)(HWND, int, int, int, HMONITOR, void*))GetProcAddress(hDwmApi, (LPCSTR)101);

	DXGI_FORMAT format;//=D3DFMT_UNKNOWN;
	HANDLE handle=NULL;
	DwmpDxGetWindowSharedSurface(hwnd,desc.AdapterLuid,0,0,&format,&handle,(LPVOID*)&luidN);

	if(!handle)
		return false;
	IDXGISurface1 * resource = NULL;

	m_device->OpenSharedResource(handle,__uuidof(IDXGISurface1),(LPVOID*)&resource);
	DXGI_SURFACE_DESC desc2;
	resource->GetDesc(&desc2);
	DwmpDxUpdateWindowSharedSurface(hwnd,0,0,0,0,0);

	HDC hdc = GetDC(NULL);
	HDC destDC;
	resource->GetDC(false,&destDC);
	BitBlt(destDC,0,0,100,100,hdc,0,0,SRCCOPY|CAPTUREBLT);
	resource->ReleaseDC(NULL);
	ReleaseDC(NULL,hdc);
	resource->Release();


	FreeLibrary(hDwmApi);

	return true;
}
void D3DDevice::getInputMap(std::map<std::pair<String,UINT>,UINT>& map,const D3D11_INPUT_ELEMENT_DESC* desc,UINT count)
{
	UINT i;
	UINT size = 0;
	for(i=0;i<count;i++)
	{
		map[std::pair<String,UINT>(desc[i].SemanticName,desc[i].SemanticIndex)] = size;
		size += D3DDevice::getDXGIFormatSize(desc[i].Format);
	}
}
void D3DDevice::addInputElement(LPCSTR name,const D3D11_INPUT_ELEMENT_DESC* desc)
{
	INT i;
	for(i=0;desc[i].SemanticName;i++);

	m_mapElement[name].clear();
	std::map<String,std::vector<D3D11_INPUT_ELEMENT_DESC_CUSTOM> >::iterator it = m_mapElement.find(name);
	it->second.resize(i);
	for(i=0;desc[i].SemanticName;i++)
	{
		it->second[i] = desc[i];
	}
}
D3D11_INPUT_ELEMENT_DESC* D3DDevice::getInputElement(LPCSTR name)
{
	std::map<String,std::vector<D3D11_INPUT_ELEMENT_DESC_CUSTOM> >::iterator it = m_mapElement.find(name);
	if(it == m_mapElement.end())
		return NULL;
	return &it->second[0];
}
size_t D3DDevice::getInputCount(LPCSTR name)
{
	std::map<String,std::vector<D3D11_INPUT_ELEMENT_DESC_CUSTOM> >::iterator it = m_mapElement.find(name);
	if(it == m_mapElement.end())
		return 0;
	return it->second.size();
}
VertexShader* D3DDevice::getVShader(LPCSTR name)
{
	std::map<String,VertexShader>::iterator it = m_mapVShader.find(name);
	if(it == m_mapVShader.end())
		return NULL;
	return &it->second;
}
PixelShader* D3DDevice::getPShader(LPCSTR name)
{
	std::map<String,PixelShader>::iterator it = m_mapPShader.find(name);
	if(it == m_mapPShader.end())
		return NULL;
	return &it->second;
}
void D3DDevice::addVShader(LPCSTR name,VertexShader& shader)
{
	m_mapVShader[name] = shader;
}
void D3DDevice::addPShader(LPCSTR name,PixelShader& shader)
{
	m_mapPShader[name] = shader;
}
INT D3DDevice::getDXGIFormatSize(DXGI_FORMAT format)
{
	if(format <= DXGI_FORMAT_UNKNOWN)
		return 0;
	if(format <= DXGI_FORMAT_R32G32B32A32_SINT)
		return 16;
	if(format <= DXGI_FORMAT_R32G32B32_SINT)
		return 12;
	if(format <= DXGI_FORMAT_X32_TYPELESS_G8X24_UINT)
		return 8;
	if(format <= DXGI_FORMAT_R8G8_SINT)
		return 4;
	if(format <= DXGI_FORMAT_R16_SINT)
		return 2;
	if(format <= DXGI_FORMAT_R1_UNORM)
		return 1;
	if(format <= DXGI_FORMAT_G8R8_G8B8_UNORM)
		return 4;
	if(format <= DXGI_FORMAT_BC5_SNORM)
		return 1;
	if(format <= DXGI_FORMAT_B5G5R5A1_UNORM)
		return 2;
	if(format <= DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)
		return 4;
	if(format <= DXGI_FORMAT_BC6H_TYPELESS)
		return 1;
	if(format <= DXGI_FORMAT_BC6H_SF16)
		return 3;
	if(format <= DXGI_FORMAT_BC7_UNORM_SRGB)
		return 1;
	return 0;
}
bool D3DDevice::getAdapterLUID(UINT Adapter,LUID *pLUID)
{
	IDXGIDevice* dxgiDevice;
	D3DDevice::getDevice()->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	if(!dxgiDevice)
		return false;
	//アダプターの作成
	IDXGIAdapter * adapter;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&adapter);

	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);
	*pLUID = desc.AdapterLuid;
	return true;
}
INT D3DDevice::loadShaders(LPCWSTR path)
{
	WINDOWS::PathName pathName(path);
	pathName.pushPath();
	pathName.changePath();

	INT count = 0;
	WIN32_FIND_DATAW findFileData;
	HANDLE handle;
	if(handle = FindFirstFileW(L"*.cso",&findFileData))
	{
		do
		{
			File file;
			if(file.open(findFileData.cFileName))
			{
				INT size = (INT)file.getLength();
				LPBYTE data = new BYTE[size];
				file.read(data,size);

				INT nameLength = wcslen(findFileData.cFileName+3) - 4;
				LPWSTR name = new WCHAR[nameLength + 1];
				wcsncpy(name,findFileData.cFileName+3,nameLength);
				name[nameLength] = 0;
				
				if(wcsncmp(findFileData.cFileName,L"VS_",2)==0)
				{
					VertexShader vertexShader;
					if(vertexShader.create(data,size))
					{

						D3DDevice::addVShader(UTF8(name),vertexShader);
					}
				}
				else if(wcsncmp(findFileData.cFileName,L"PS_",2)==0)
				{
					PixelShader pixelShader;
					if(pixelShader.create(data,size))
					{

						D3DDevice::addPShader(UTF8(name),pixelShader);
					}
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
LPCWSTR D3DDevice::getTexturePath()
{
	return m_texturePath.c_str();
}
void D3DDevice::setTexturePath(LPCWSTR path)
{
	if(path)
		m_texturePath = path;
	else
		m_texturePath.clear();
}
Direct3DDevice11 D3DDevice::m_device;
std::map<String,std::vector<D3D11_INPUT_ELEMENT_DESC_CUSTOM> > D3DDevice::m_mapElement;
std::map<String,VertexShader> D3DDevice::m_mapVShader;
std::map<String,PixelShader> D3DDevice::m_mapPShader;
WString D3DDevice::m_texturePath;


class ShaderInclude : public ID3D10Include
{
public:
	ShaderInclude()
	{
		m_data = NULL;
		m_path = false;
	}
	void setPath(LPCWSTR path)
	{
		m_path = true;
		m_pathName.setFileName(path);
	}
protected:
	STDMETHOD(Open)(D3D10_INCLUDE_TYPE IncludeType,LPCSTR pFileName,LPCVOID pParentData,LPCVOID *ppData,UINT *pBytes)
	{
		if(m_path)
		{
			m_pathName.pushPath();
			m_pathName.changePath();
		}
		File file;
		bool flag = file.open(pFileName);
		if(m_path)
			m_pathName.popPath();

		if(!flag)
			return S_FALSE;
		INT size = (INT)file.getLength();
		m_data = new BYTE[size];
		file.read(m_data,size);
		*ppData = m_data;
		*pBytes = size;
		return S_OK;
	}
	STDMETHOD(Close)(LPCVOID pData)
	{
		if(m_data)
		{
			delete[] m_data;
			m_data = NULL;
		}
		return S_OK;
	}
	LPBYTE m_data;
	WINDOWS::PathName m_pathName;
	bool m_path;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ShaderReader
// シェーダ情報取得用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MemoryData
{
public:
	void set(LPCVOID data, INT size)
	{
		m_data = data;
		m_dataSize = size;
		m_dataPt = 0;
	}
	bool read(LPVOID dest, INT size)
	{
		if (m_dataPt + size > m_dataSize)
			return false;
		CopyMemory(dest, (char*)m_data + m_dataPt, size);
		m_dataPt += size;
		return true;
	}
	bool seek(INT size)
	{
		if (m_dataPt + size > m_dataSize)
			return false;
		m_dataPt += size;
		return true;
	}
protected:
	LPCVOID m_data;
	INT m_dataSize;
	INT m_dataPt;
};
struct ConstantInfo
{
	AFL::String name;
	INT type;
	INT bindPoint;
	INT size;
};
class ShaderReader
{
public:
	bool open(LPCSTR fileName);
	bool set(LPCVOID data, INT size);

	bool read(MemoryData& md, LPVOID dest, INT size);
	bool getChank(MemoryData& md, AFL::String& chank);

	INT getConstantCount();
	ConstantInfo& getConstant(INT index);
protected:
	GUID m_guid;
	std::vector<ConstantInfo> m_constantInfo;
};

bool ShaderReader::set(LPCVOID data, INT size)
{
	MemoryData md;
	md.set(data, size);


	String chank;
	getChank(md, chank);
	if (chank != "DXBC")
		return false;
	md.read(&m_guid, sizeof(GUID));

	int count;
	md.read(&count, sizeof(int));


	int fileSize;
	md.read(&fileSize, sizeof(int));
	md.read(&count, sizeof(int));
	md.seek(count * 4);
	while (getChank(md, chank))
	{
		int chankSize;
		md.read(&chankSize, sizeof(int));

		char* buff = new char[chankSize];
		md.read(buff, chankSize);
		if (chank == "RDEF")
		{
			struct RDEF
			{
				INT CONSTANT_COUNT;
				INT CONSTANT_OFFSET;
				INT RESOURCE_COUNT;
				INT RESOURCE_OFFSET;
				INT TARGET;
				INT FLAG;
				INT CREATER_OFFSET;

			};
			struct RESOURCE
			{
				INT STRING_OFFSET;
				INT TYPE;
				INT RTYPE;
				INT DIMENSION;
				INT NUM;
				INT POINT;
				INT COUNT;
				INT FLAG;

			};
			struct CBUFFER
			{
				INT STRING_OFFSET;
				INT VARIABLE_COUNT;
				INT VARIABLE_OFFSET;
				INT CSIZE;
				INT CFLAG;
				INT CTYPE;
			};

			RDEF* rdef = (RDEF*)buff;
			//if (rdef->RESOURCE_COUNT != rdef->CONSTANT_COUNT)
			//	return false;

			RESOURCE* res = (RESOURCE*)(buff + rdef->RESOURCE_OFFSET);
			CBUFFER* cbuffer = (CBUFFER*)(buff + rdef->CONSTANT_OFFSET);

			m_constantInfo.reserve(rdef->RESOURCE_COUNT);
			INT i;
			for (i = 0; i < rdef->RESOURCE_COUNT; i++)
			{
				ConstantInfo ci;
				ci.type = res[i].TYPE;
				ci.name = (char*)(buff + res[i].STRING_OFFSET);
				ci.bindPoint = res[i].POINT;

				INT j;
				for (j = 0; j < rdef->CONSTANT_COUNT; j++)
				{
					if (ci.name == (char*)(buff + cbuffer[j].STRING_OFFSET))
						ci.size = cbuffer[j].CSIZE;
				}

				
				m_constantInfo.push_back(ci);
			}
		}
		delete[] buff;
	}
	return true;
}


bool ShaderReader::open(LPCSTR fileName)
{
	File file;
	if (!file.open(fileName))
		return false;

	INT length = (INT)file.getLength();
	char* data = new char[length];
	file.read(data, length);
	set(data, length);
	delete[] data;

	return true;
}
bool ShaderReader::read(MemoryData& md, LPVOID dest, INT size)
{
	if (!md.read(dest, size))
		return false;
	return true;
}
bool ShaderReader::getChank(MemoryData& md, String& chank)
{
	char buff[5];
	if (!md.read(buff, 4))
		return false;
	buff[4] = 0;
	chank = buff;
	return true;
}

INT ShaderReader::getConstantCount()
{
	return m_constantInfo.size();
}
ConstantInfo& ShaderReader::getConstant(INT index)
{
	return m_constantInfo[index];
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Shader
// シェーダ管理基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
pD3DCompile Shader::_D3DCompile;

Shader::Shader()
{
	if (_D3DCompile == NULL)
	{
		INT i;
		HMODULE handle;
		WString dllName;
		for (i = 47; i > 30; i--)
		{
			dllName.printf(L"d3dcompiler_%d.dll", i);
			handle = LoadLibraryW(dllName);
			if (handle)
			{
				_D3DCompile = (pD3DCompile)GetProcAddress(handle, "D3DCompile");
				break;
			}
		}
	}
}
LPVOID Shader::getData()
{
	return &(*m_data.get())[0];
}
INT Shader::getSize()
{
	return m_data->size();
}
bool Shader::set(LPCVOID src,size_t size)
{
	ShaderReader sr;
	sr.set(src, size);
	m_cbIndex = new std::map<String, ShaderConstant>();

	INT count = sr.getConstantCount();

	int i;
	for (i = 0;i< count; i++)
	{
		ConstantInfo& ci = sr.getConstant(i);
		if(ci.type == D3D10_SIT_CBUFFER)
		{
			ShaderConstant sc;
			//スロット番号
			sc.index = ci.bindPoint;
			//定数サイズ
			sc.constantBuffer.create(ci.size);
			//設定
			(*m_cbIndex.get())[ci.name] = sc;
		}
	}
	return true;
}

bool Shader::update(LPCSTR name,LPCVOID data)
{
	if(m_cbIndex.get() == NULL)
		return false;
	std::map<String,ShaderConstant>::iterator it;
	it = m_cbIndex.get()->find(name);
	if(it == m_cbIndex.get()->end())
		return false;
	it->second.constantBuffer.update((LPVOID)data);
	return true;
}

int Shader::getCBIndex(LPCSTR name)
{
	if(m_cbIndex.get() == NULL)
		return false;
	std::map<String,ShaderConstant>::iterator it;
	it = m_cbIndex.get()->find(name);
	if(it ==m_cbIndex.get()->end())
		return -1;
	return (*m_cbIndex.get())[name].index;
}
bool Shader::setConstant(ID3D11DeviceContext* context)
{
	if(m_cbIndex.get() == NULL)
		return false;

	std::map<String,ShaderConstant>::iterator it;
	std::map<String,ShaderConstant>& map = *m_cbIndex.get();
	for(it=map.begin();it!=map.end();++it)
	{
		setConstantBuffer(context,it->second.index,&it->second.constantBuffer);
	}
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VertexShader
// 頂点シェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
VertexShader::VertexShader()
{
	//m_vertexShader = NULL;
}
VertexShader::~VertexShader()
{
	release();
}
bool  VertexShader::setConstantBuffer(ID3D11DeviceContext* context,INT index,ConstantBuffer* constant)
{
	context->VSSetConstantBuffers(index,1,*constant);
	return true;
}


void VertexShader::release()
{
}
bool VertexShader::create(LPCSTR cmd,LPCSTR name,LPCWSTR path)
{
	release();
	ShaderInclude shaderInclude;
	shaderInclude.setPath(path);
	ID3DBlob* blob;
	ID3DBlob* error = NULL;
	#if _DEBUG
		_D3DCompile( cmd, strlen(cmd), NULL, NULL, &shaderInclude,name, D3DDevice::getVSName(),D3D10_SHADER_DEBUG,0,&blob,&error);
	#else
		_D3DCompile( cmd, strlen(cmd), NULL, NULL, &shaderInclude,name, D3DDevice::getVSName(),0,0,&blob,&error);
	#endif
	if(!blob)
	{
	#if _DEBUG
		OutputDebugStringA((LPCSTR)error->GetBufferPointer());
	#endif
		error->Release();
		return false;
	}

	bool flag = create(blob->GetBufferPointer(),blob->GetBufferSize());
	blob->Release();
	return flag;
}

bool VertexShader::create(LPCVOID data,size_t size)
{
	release();
	m_data = new std::vector<BYTE>;
	ID3D11VertexShader* vertexShader = NULL;
	HRESULT hr = D3DDevice::getDevice()->CreateVertexShader(data,size,NULL,&vertexShader);
	m_vertexShader = vertexShader;
	set(data,size);
	m_data->resize(size);
	memcpy(&(*m_data.get())[0],data,size);
	if(!m_vertexShader)
		return false;

	return true;
}

bool VertexShader::open(LPCWSTR fileName,LPCSTR name)
{
	File file;
	if(!file.open(fileName))
		return false;
	INT size = (INT)file.getLength();
	LPBYTE data = NEW BYTE[size+1];
	file.read(data,size);
	data[size] = 0;
	bool flag = create((LPCSTR)data,name,fileName);
	delete[] data;
	return flag;
}
bool VertexShader::open(LPCSTR fileName,LPCSTR name)
{
	return open(UCS2(fileName),name);
}

VertexShader::operator ID3D11VertexShader*() const
{
	return m_vertexShader;
}

	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PixelShader
// ピクセルシェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PixelShader::PixelShader()
{
}
PixelShader::~PixelShader()
{
	release();
}
void PixelShader::release()
{
}
bool PixelShader::create(LPCSTR cmd,LPCSTR name)
{
	release();
	ID3DBlob* blob = NULL;

	ID3DBlob* error = NULL;
	_D3DCompile( cmd, strlen(cmd), NULL, NULL, NULL,name, D3DDevice::getPSName(),0,0,&blob,&error);
	if(!blob)
	{
	#if _DEBUG
		OutputDebugStringA((LPCSTR)error->GetBufferPointer());
	#endif
		error->Release();
		return false;
	}
	bool flag =  create(blob->GetBufferPointer(),blob->GetBufferSize());
	blob->Release();
	return flag;
}
bool PixelShader::create(LPCVOID data,size_t size)
{
	release();
	m_data = new std::vector<BYTE>;
	ID3D11PixelShader* pixelShader = NULL;
	HRESULT hr = D3DDevice::getDevice()->CreatePixelShader(data,size,NULL,&pixelShader);
	set(data,size);

	m_pixelShader = pixelShader;
	m_data->resize(size);
	memcpy(&(*m_data.get())[0],data,size);
	if(pixelShader)
		return true;
	return false;
}
bool PixelShader::open(LPCWSTR fileName,LPCSTR name)
{
	File file;
	if(!file.open(fileName))
		return false;
	size_t size = (size_t)file.getLength();
	LPBYTE data = NEW BYTE[size+1];
	file.read(data,size);
	data[size] = 0;
	bool flag = create((LPCSTR)data,name);
	delete[] data;
	return flag;
}
bool PixelShader::open(LPCSTR fileName,LPCSTR name)
{
	return open(UCS2(fileName),name);
}
PixelShader::operator ID3D11PixelShader*() const
{
	return m_pixelShader;
}
bool  PixelShader::setConstantBuffer(ID3D11DeviceContext* context,INT index,ConstantBuffer* constant)
{
	context->PSSetConstantBuffers(index,1,*constant);
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Buffer
// バッファデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Buffer::Buffer()
{
	m_buffer = NULL;
}
Buffer::~Buffer()
{
	if(m_buffer)
		m_buffer->Release();
}
D3D11_MAPPED_SUBRESOURCE* Buffer::lock()
{
	if(!m_buffer)
		return NULL;
	D3DDevice::getContext()->Map(m_buffer,0,D3D11_MAP_WRITE_DISCARD,0,&m_resource);
	return &m_resource;
}
void Buffer::unlock()
{
	D3DDevice::getContext()->Unmap(m_buffer,0);
}
UINT Buffer::getSize() const
{
	if(!m_buffer)
		return 0;
	D3D11_BUFFER_DESC desc;
	m_buffer->GetDesc(&desc);
	return desc.ByteWidth;
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
bool Vertex::create(UINT size)
{
	if(m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}
	ID3D11Buffer* buffer = NULL;
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth      = size;
	bufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags      = 0;
	D3DDevice::getDevice()->CreateBuffer(&bufferDesc,NULL,&m_buffer);
		
	if(m_buffer)
		return true;
	return false;
}
bool Vertex::create(LPCVOID data,UINT size)
{
	if(m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}
	ID3D11Buffer* buffer = NULL;
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof(D3D11_BUFFER_DESC) );
	bufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth      = size;
	bufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags      = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory( &initData, sizeof(initData) );
	initData.pSysMem = data;

	D3DDevice::getDevice()->CreateBuffer(&bufferDesc,&initData,&m_buffer);
		
	if(m_buffer)
		return true;
	return false;
}
Vertex::operator ID3D11Buffer*() const
{
	return m_buffer;
}

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
bool Index::create(UINT size)
{
	if(m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}
	ID3D11Buffer* buffer = NULL;
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth      = size;
	bufferDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags      = 0;
	D3DDevice::getDevice()->CreateBuffer(&bufferDesc,NULL,&m_buffer);
		
	if(m_buffer)
		return true;
	return false;
}
bool Index::create(LPCVOID data,UINT size)
{
	if(m_buffer)
	{
		m_buffer->Release();
		m_buffer = NULL;
	}
	ID3D11Buffer* buffer = NULL;
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth      = size;
	bufferDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags      = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory( &initData, sizeof(initData) );
	initData.pSysMem = data;

	D3DDevice::getDevice()->CreateBuffer(&bufferDesc,&initData,&m_buffer);
	if(m_buffer)
		return true;
	return false;
}
Index::operator ID3D11Buffer*() const
{
	return m_buffer;
}
UINT Index::getCount() const
{
	return getSize() / sizeof(WORD);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Texture::Texture()
{
	m_texture = NULL;
	m_resourceView = NULL;
	m_targetView = NULL;
	ZeroMemory(&m_desc,sizeof(D3D11_TEXTURE2D_DESC));
}
Texture::~Texture()
{
	release();
}
void Texture::release()
{
	if(m_targetView)
	{
		m_targetView->Release();
	}
	if(m_resourceView)
	{
		m_resourceView->Release();
		m_resourceView = NULL;
	}
	if(m_texture)
	{
		m_texture->Release();
		m_texture = NULL;
	}
}
bool Texture::create(HANDLE handle)
{
	release();

	ID3D11Device* device = D3DDevice::getDevice();
	ID3D11Texture2D* texture = NULL;
	device->OpenSharedResource(handle,__uuidof(ID3D11Texture2D),(LPVOID*)&texture);
	if(!texture)
		return false;
	texture->GetDesc(&m_desc);
	m_texture = texture;
	D3DDevice::getDevice()->CreateShaderResourceView(m_texture,NULL,&m_resourceView);
	return true;
}
bool Texture::create(UINT width,UINT height,D3D11_USAGE usage)
{
	release();

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.Width              = width;
	desc.Height             = height;
	desc.MipLevels          = 1;
	desc.ArraySize          = 1;
	desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage              = usage;
	desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags     = 0;
	desc.MiscFlags          = 0;

	if(usage == D3D11_USAGE_DEFAULT)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
		//desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	}
	else if(usage == D3D11_USAGE_DYNAMIC || usage == D3D11_USAGE_STAGING)
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	D3DDevice::getDevice()->CreateTexture2D( &desc, NULL,&m_texture);
	if(!m_texture)
		return false;
	D3DDevice::getDevice()->CreateShaderResourceView(m_texture,NULL,&m_resourceView);
	m_desc = desc;

	if(usage == D3D11_USAGE_DEFAULT)
	{
		ID3D11Device* device = D3DDevice::getDevice();
		device->CreateRenderTargetView(m_texture,NULL,&m_targetView);
	}
	return true;
}
bool Texture::clear(D3D11_MAPPED_SUBRESOURCE* r,DWORD color)
{
	const DWORD width = getImageWidth();
	const DWORD height = getImageHeight();
	LPBYTE data = (LPBYTE)r->pData;
	DWORD i,j;
	for(j=0;j<height;j++)
	{
		for(i=0;i<width;i++)
		{
			((LPDWORD)data)[i] = color;
		}
		data += r->RowPitch;
	}
	return true;
}

bool Texture::clear(DWORD color)
{
	if(m_targetView)
	{
		LPBYTE c = (LPBYTE)&color;
		FLOAT data[] = {(FLOAT)c[2]/255,(FLOAT)c[1]/255,(FLOAT)c[0]/255,(FLOAT)c[3]/255};
		D3DDevice::getContext()->ClearRenderTargetView(m_targetView,data);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE* r = lock();
		clear(r,color);
		unlock();
	}
	return true;
}
bool Texture::drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline)
{
	return drawOutlineText(iX,iY,UCS2(pText),iSize,colText,colBack,iLimitWidth,mline);
}
bool Texture::drawOutlineText(int iX,int iY,LPCWSTR pText,INT iSize,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline)
{
	//フォントハンドルの取得
	AFL::WINDOWS::Font font;
	font.setSize(iSize);
	font.createFont();
	return drawOutlineText(iX,iY,pText,font,colText,colBack,iLimitWidth,mline); 
}
bool Texture::drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline)
{
	return drawOutlineText(iX,iY,UCS2(pText),hFont,colText,colBack,iLimitWidth,mline);
}
bool Texture::drawOutlineText(int iX,int iY,LPCWSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline)
{
	if(!pText)
		return false;

	D3D11_MAPPED_SUBRESOURCE* lockRect = lock();
	if(!lockRect)
		return false;
	clear(lockRect,0);

	GLYPHMETRICS metInfo;
	INT i;
	TEXTMETRIC tm; 
	HDC hDC = CreateCompatibleDC(NULL);
	HFONT hFontBack = (HFONT)SelectObject(hDC,hFont);
	GetTextMetrics(hDC,&tm);

	//回転無し
	MAT2 mx2 = {1,1,0,0,0,0,1,1};

	SIZE sizeMax = {0,0};
	INT iWidth = 0;
	INT iHeight = 0;
	SIZE sizeFont;
	GetTextExtentPoint32W(hDC,L" ",1,&sizeFont);
	for(i=0;pText[i];i++)
	{
		//タブ
		if(pText[i] == '\t')
		{
			GetTextExtentPoint32W(hDC,L" ",1,&sizeFont);
			iWidth += sizeFont.cx*4;
			iWidth -= iWidth % (sizeFont.cx*4);
			continue;
		}
		//改行
		if(pText[i] == '\n')
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
			continue;
		}
		//サイズ取得
		GetTextExtentPoint32W(hDC,pText+i,1,&sizeFont);
		//改行
		if(iLimitWidth > 0 && sizeFont.cx + iWidth > iLimitWidth)
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
		}
		WORD wCode = pText[i];

		INT bufferSize;
		if(colBack == -1)
		{
			bufferSize = GetGlyphOutlineW(hDC,wCode,GGO_GRAY8_BITMAP,&metInfo,NULL,NULL,&mx2);
			if(bufferSize >0)
			{
				LPBYTE pbyBitmap = NEW BYTE[bufferSize]; 
				GetGlyphOutlineW(hDC,wCode,GGO_GRAY8_BITMAP,&metInfo,bufferSize,pbyBitmap,&mx2);
				drawGlyphOutline(*lockRect,iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
				delete[] pbyBitmap;
			}
		}
		else
		{
			bufferSize = GetGlyphOutlineW(hDC,wCode,GGO_BITMAP,&metInfo,NULL,NULL,&mx2);
			if(bufferSize >0)
			{
				LPBYTE pbyBitmap = NEW BYTE[bufferSize]; 
				GetGlyphOutlineW(hDC,wCode,GGO_BITMAP,&metInfo,bufferSize,pbyBitmap,&mx2);
				if(colBack == -2)
					drawGlyphOutline2(*lockRect,iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
				else
				{
					drawGlyphOutline2(*lockRect,iWidth+iX,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(*lockRect,iWidth+iX+1,iHeight+iY,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(*lockRect,iWidth+iX+2,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(*lockRect,iWidth+iX+1,iHeight+iY+2,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(*lockRect,iWidth+iX+1,iHeight+iY+1,colText,tm.tmAscent,&metInfo,pbyBitmap);
				}
				delete[] pbyBitmap;
			}
		}

		iWidth += sizeFont.cx;
		if(sizeMax.cx < iWidth)
			sizeMax.cx = iWidth;
	}
	sizeMax.cy = iHeight+sizeFont.cy;
	SelectObject(hDC,hFontBack);
	DeleteDC(hDC);

	unlock();
	return true;
}
bool Texture::drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	D3D11_MAPPED_SUBRESOURCE* lockRect = lock();
	if(!lockRect)
		return false;

	bool flag = drawGlyphOutline(*lockRect,iX,iY,colText,tmAscent,pmetInfo,pbyBitmap);

	unlock();
	return flag;
}
bool Texture::drawGlyphOutline(D3D11_MAPPED_SUBRESOURCE& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	UINT i,j;
	DWORD dwColor;
	iX += pmetInfo->gmptGlyphOrigin.x;
	iY += tmAscent - pmetInfo->gmptGlyphOrigin.y;
	UINT iCountX = pmetInfo->gmBlackBoxX;
	UINT iCountY = pmetInfo->gmBlackBoxY;
	UINT iSrcPitch = (iCountX + 3) / 4 * 4;

	if(iCountX > getImageWidth() - iX)
		iCountX = getImageWidth() - iX;
	if(iCountY > getImageHeight() - iY)
		iCountY = getImageHeight() - iY;

	INT iDestPitch = lockRect.RowPitch;
	LPBYTE pbyDest = (LPBYTE)lockRect.pData;	//書き込み用ポインタ

	BYTE byAlpha = ((LPBYTE)&colText)[3];
	BYTE byRed = ((LPBYTE)&colText)[2];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[0];

	pbyDest += iDestPitch * iY + iX * 4;
	dwColor = (byRed << 16) + (byGreen << 8) + byBlue;
	for(j=0;j<iCountY;j++)
	{
		for(i=0;i<iCountX;i++)
		{
			byAlpha = pbyBitmap[i];
			if(byAlpha)
			{
				byAlpha = ((byAlpha-1) << 2) + 3;
				*(LPDWORD)(pbyDest+i*4) = dwColor;
				pbyDest[i*4+3] = byAlpha;
			}
		}
		pbyBitmap += iSrcPitch;
		pbyDest += iDestPitch;
	}
	return true;
}
bool Texture::drawGlyphOutline2(D3D11_MAPPED_SUBRESOURCE& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	UINT i,j;
	DWORD dwColor;
	iX += pmetInfo->gmptGlyphOrigin.x;
	iY += tmAscent - pmetInfo->gmptGlyphOrigin.y;
	UINT iCountX = pmetInfo->gmBlackBoxX;
	UINT iCountY = pmetInfo->gmBlackBoxY;
	INT iSrcPitch = (iCountX+31)/32*4;

	if(iCountX > getImageWidth() - iX)
		iCountX = getImageWidth() - iX;
	if(iCountY > getImageHeight() - iY)
		iCountY = getImageHeight() - iY;

	INT iDestPitch = lockRect.RowPitch;
	LPBYTE pbyDest = (LPBYTE)lockRect.pData;	//書き込み用ポインタ

	BYTE byAlpha = ((LPBYTE)&colText)[3];
	BYTE byRed = ((LPBYTE)&colText)[2];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[0];

	pbyDest += iDestPitch * iY + iX * 4;
	dwColor = colText|0xff000000;
	for(j=0;j<iCountY;j++)
	{
		for(i=0;i<iCountX;i++)
		{
			byAlpha = pbyBitmap[i/8] & (1<<(7-i%8));
			if(byAlpha)
			{
				*(LPDWORD)(pbyDest+i*4) = dwColor;
			}
		}
		pbyBitmap += iSrcPitch;
		pbyDest += iDestPitch;
	}
	return true;
}
bool Texture::drawGlyphOutline2(INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	D3D11_MAPPED_SUBRESOURCE* lockRect = lock();
	if(!lockRect)
		return false;

	bool flag = drawGlyphOutline2(*lockRect,iX,iY,colText,tmAscent,pmetInfo,pbyBitmap);

	unlock();
	return flag;
}

D3D11_MAPPED_SUBRESOURCE* Texture::lock()
{
	if(!m_texture)
		return NULL;
	HRESULT hr;
	hr = D3DDevice::getContext()->Map(m_texture,0,D3D11_MAP_WRITE_DISCARD,0,&m_resource);
	if(hr == S_OK)
		return &m_resource;
	return NULL;
}
void Texture::unlock()
{
	D3DDevice::getContext()->Unmap(m_texture,0);
}
bool Texture::open(LPCSTR fileName,D3D11_USAGE usage)
{
	return open(UCS2(fileName),usage);
}
bool Texture::open(LPCWSTR fileName,D3D11_USAGE usage)
{
	//読み込みパスのカスタム設定を適用
	WString name;
	LPCWSTR path = D3DDevice::getTexturePath();
	if(*path && fileName[0] != L'\\' && (fileName[0] && fileName[1] !=':'))
	{
		if(path[wcslen(path)-1] != L'\\')
		{
			name.appendf(L"%s\\%s",path,fileName);
		}
		else
		{
			name.appendf(L"%s%s",path,fileName);
		}
	}
	else
	{
		name = fileName;
	}
	UINT width;
	UINT height;
	Bitmap* bitmap = Bitmap::FromFile(fileName);

	if(!bitmap)
		return false;
	width = bitmap->GetWidth();
	height = bitmap->GetHeight();
	if(!width || !height)
	{
		delete bitmap;
		bitmap = Bitmap::FromFile(name);
		width = bitmap->GetWidth();
		height = bitmap->GetHeight();
		if(!width || !height)
		{
			delete bitmap;
			return false;
		}
	}
	PixelFormat format = bitmap->GetPixelFormat();
	Gdiplus::BitmapData bitmapData;

	if(!create(width,height,usage))
	{
		delete bitmap;
		return false;
	}
	bitmap->LockBits(NULL, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
	D3DDevice::getContext()->UpdateSubresource(m_texture,0,NULL,bitmapData.Scan0,bitmapData.Width*4,0);
	bitmap->UnlockBits(&bitmapData);

	delete bitmap;
	return true;
}
bool Texture::setTarget(ID3D11DeviceContext* context)
{
	if(context)
	{
		ID3D11DepthStencilView* stencilView=NULL;
		context->OMGetRenderTargets(0,NULL,&stencilView);
		context->OMSetRenderTargets( 1, &m_targetView, stencilView);
	}
	return true;	
}
bool TextureTarget::create(UINT width,UINT height,D3D11_USAGE usage)
{
	if(!Texture::create(width,height,usage))
		return false;
	ID3D11Device* device = D3DDevice::getDevice();
	if(!m_deviceContext)
	{
		ID3D11DeviceContext* deviceContext;
		device->CreateDeferredContext (0,&deviceContext);
		m_deviceContext = deviceContext;
	}

	m_deviceContext->OMSetRenderTargets( 1, &m_targetView, NULL );

	return true;	
}
bool TextureTarget::clear(FLOAT r,FLOAT g,FLOAT b,FLOAT a)
{
	if(!m_targetView)
		return false;
	FLOAT color[] = {r,g,b,a};
	m_deviceContext->ClearRenderTargetView( m_targetView, color );
	return true;
}
void TextureTarget::setDepth(ID3D11DepthStencilView* sview)
{
	m_deviceContext->OMSetRenderTargets( 1, &m_targetView, sview );

}
ID3D11DeviceContext* TextureTarget::getContext() const
{
	return m_deviceContext;
}
ID3D11Texture2D* Texture::getTexture()const
{
	return m_texture;
}
bool Texture::isTexture() const
{
	return m_texture != NULL;
}
Texture::operator ID3D11Texture2D*() const
{
	return m_texture;
}
ID3D11ShaderResourceView*  Texture::getResourceView() const
{
	return m_resourceView;
}


UINT Texture::getImageHeight() const
{
	return m_desc.Height;
}
UINT Texture::getImageWidth() const
{
	return m_desc.Width;
}
UINT Texture::getTextureHeight() const
{
	return m_desc.Height;
}
UINT Texture::getTextureWidth() const
{
	return m_desc.Width;
}
HDC  Texture::getDC() const
{
	HDC hdc = NULL;
	IDXGISurface1* surface = NULL;
	m_texture->QueryInterface(IID_IDXGISurface1,(LPVOID*)&surface);
	if(!surface)
		return NULL;
	surface->GetDC(true,&hdc);
	surface->Release();
	return hdc;
}
void  Texture::releaseDC() const
{
	HDC hdc = NULL;
	IDXGISurface1* surface = NULL;
	m_texture->QueryInterface(IID_IDXGISurface1,(LPVOID*)&surface);
	if(!surface)
		return;
	surface->ReleaseDC(NULL);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// DepthStencil
// 深度バッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
DepthStencil::DepthStencil()
{
	ZeroMemory(&m_desc,sizeof(D3D11_TEXTURE2D_DESC));
}
bool DepthStencil::create(UINT width,UINT height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(D3D11_TEXTURE2D_DESC) );
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D* texture = NULL;
	D3DDevice::getDevice()->CreateTexture2D(&desc, NULL, &texture );
	if(!texture)
		return false;
	m_desc = desc;
	m_texture = texture;


	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
	descDSV.Format = desc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView* depthStencilView = NULL;
	D3DDevice::getDevice()->CreateDepthStencilView(texture, &descDSV, &depthStencilView );
	if(!depthStencilView)
		return false;
	m_depthStencilView = depthStencilView;
	return true;
}
DepthStencil::operator ID3D11DepthStencilView*() const
{
	return m_depthStencilView;
}
ID3D11Texture2D* DepthStencil::getTexture()const
{
	return m_texture;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// スワップチェイン用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Screen::Screen()
{
	m_output = NULL;
	m_renderTargetView = NULL;
	m_deviceContext = NULL;
	m_screen = NULL;
	m_camera = NULL;
	ZeroMemory(&m_desc,sizeof(m_desc));
}
Screen::~Screen()
{
	release();
}
void Screen::release()
{
}
ID3D11DeviceContext* Screen::getContext() const
{
	return m_deviceContext;
}
bool Screen::clear(FLOAT r,FLOAT g,FLOAT b,FLOAT a)
{
	if(!m_renderTargetView)
		return false;
	FLOAT color[] = {r,g,b,a};
	m_deviceContext->ClearRenderTargetView( m_renderTargetView, color );
	m_deviceContext->ClearDepthStencilView( m_depth, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );
	return true;
}

bool Screen::present()
{
	if(!m_screen)
		return false;
	m_screen->Present( 0, 0 );
	return true;
}
void Screen::setTarget()
{
	if(!D3DDevice::isDevice())
		return;
	ID3D11RenderTargetView* target = m_renderTargetView;
	m_deviceContext->OMSetRenderTargets( 1, &target, m_depth );
}
//スワップチェインの作成
bool Screen::createScreen(HWND hwnd,UINT width,UINT height,DXGI_FORMAT format)
{
	if(!D3DDevice::isDevice())
		return NULL;
	release();

	ID3D11Device* device = D3DDevice::getDevice();
	if(!m_deviceContext)
	{
		ID3D11DeviceContext* deviceContext;
		device->GetImmediateContext(&deviceContext);
		m_deviceContext = deviceContext;
	}
	m_deviceContext->OMSetRenderTargets( 0, NULL, NULL );


	//DXGIオブジェクトの生成
	IDXGIDevice* dxgiDevice;
	D3DDevice::getDevice()->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	if(!dxgiDevice)
		return NULL;
	//アダプターの作成
	IDXGIAdapter * adapter;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&adapter);
	//ファクトリーの作成
	IDXGIFactory * factory;
	adapter->GetParent(__uuidof(IDXGIFactory), (void **)&factory);

	IDXGISwapChain* swapChain = NULL;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;//format;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	IDXGISwapChain* screen = NULL;
	factory->CreateSwapChain(D3DDevice::getDevice(),&sd,&screen);
	factory->Release();
	adapter->Release();
	dxgiDevice->Release();

	m_screen = screen;

	if(!m_screen)
	{
		DLINE("失敗 Screen::createScreen");
		return false;
	}

	//深度バッファの作成
	m_depth.create(width,height);
	m_desc = sd;

	ID3D11Texture2D  *backBuffer;
	m_screen->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer );
	m_texture = backBuffer;

	ID3D11RenderTargetView* renderTargetView = NULL;
	device->CreateRenderTargetView(backBuffer,NULL,&renderTargetView);
	backBuffer->Release();

	m_renderTargetView = renderTargetView;
	m_deviceContext->OMSetRenderTargets( 1, &renderTargetView, m_depth );

	IDXGIOutput* output;
	m_screen->GetContainingOutput(&output);
	m_output = output;


	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports( 1, &vp );

	//if(m_camera)
		//m_camera->setAngle(width,height);
	return true;
}
/*
bool Screen::draw(Unit* unit)
{
	if(!m_screen)
		return false;
	NMatrix unitMatrix;
	unitMatrix = NMatrixRotationX(unit->getRot().x*XM_PI/180.0f);
	unitMatrix *= NMatrixRotationY(unit->getRot().y*XM_PI/180.0f);
	unitMatrix *= NMatrixRotationZ(unit->getRot().z*XM_PI/180.0f);

	Frame* frame = unit->getFrame().get();

	NMatrix matrix[3];
	matrix[0] = NMatrixsetTranspose(frame->getMatrix()*unitMatrix);
	matrix[1] = NMatrixsetTranspose(m_camera.getBaseView());
	matrix[2] = NMatrixsetTranspose(m_camera.getProjection());
	m_constantWorld.update(matrix);
	m_constantViewProj.update(&matrix[1]);

	std::list<Mesh>::iterator it;
	foreach(it,frame->getMeshes())
	{
		Mesh* mesh = &*it;
		UINT offset = 0;
		UINT stride = mesh->getStrideSize();
		ID3D11Buffer* vertexBuffer = *mesh->getVertex();

		m_deviceContext->IASetInputLayout( *mesh->getInputLayout() );
		m_deviceContext->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );
		m_deviceContext->IASetIndexBuffer(*mesh->getIndex(), DXGI_FORMAT_R16_UINT, 0);
		m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		ID3D11Buffer* constant[] = {m_constantWorld,m_constantViewProj};
		m_deviceContext->VSSetShader(*mesh->getVertexShader(), NULL, 0 );
		m_deviceContext->VSSetConstantBuffers( 0, 2, constant );
		m_deviceContext->PSSetShader(*mesh->getPixelShader(), NULL, 0 );

		ID3D11ShaderResourceView* resourceView = mesh->getTexture()->getResourceView();
		m_deviceContext->PSSetShaderResources( 0, 1, &resourceView);

		m_deviceContext->DrawIndexed( mesh->getIndex()->getSize()/2, 0,0 );
	}
	return true;
}
bool Screen::draw(Frame* frame)
{
	if(!m_screen)
		return false;

	NMatrix matrix[3];
	matrix[0] = NMatrixsetTranspose(frame->getMatrix());
	matrix[1] = NMatrixsetTranspose(m_camera.getBaseView());
	matrix[2] = NMatrixsetTranspose(m_camera.getProjection());
	m_matrices.update(matrix);

	std::list<Mesh>::iterator it;
	foreach(it,frame->getMeshes())
	{
		Mesh* mesh = &*it;
		UINT offset = 0;
		UINT stride = mesh->getStrideSize();
		ID3D11Buffer* vertexBuffer = *mesh->getVertex();

		m_deviceContext->IASetInputLayout( *mesh->getInputLayout() );
		m_deviceContext->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );
		m_deviceContext->IASetIndexBuffer(*mesh->getIndex(), DXGI_FORMAT_R16_UINT, 0);
		m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		ID3D11Buffer* constant = m_matrices;
		m_deviceContext->VSSetShader(*mesh->getVertexShader(), NULL, 0 );
		m_deviceContext->VSSetConstantBuffers( 0, 1, &constant );
		m_deviceContext->PSSetShader(*mesh->getPixelShader(), NULL, 0 );

		ID3D11ShaderResourceView* resourceView = mesh->getTexture()->getResourceView();
		m_deviceContext->PSSetShaderResources( 0, 1, &resourceView);

		m_deviceContext->DrawIndexed( mesh->getIndex()->getSize()/2, 0,0 );
	}
	return true;
}
*/
bool Screen::draw(Mesh* mesh)
{
	if(!m_screen)
		return false;
	
	NMatrix matrix[3];

	matrix[0].setIdentity();
	if(m_camera)
	{
		matrix[1] = m_camera->getBaseView();
		matrix[1].setTranspose();
		matrix[2] = m_camera->getProjection();
		matrix[2].setTranspose();
	}
	else
	{
		matrix[1].setIdentity();
		matrix[2].setIdentity();
	}

//	m_constantWorld = matrix;
//	m_constantViewProj = matrix[1];




	UINT offset = 0;
	UINT stride = mesh->getStrideSize();
	ID3D11Buffer* vertexBuffer = *mesh->getVertex();
	ID3D11Buffer* constant[1];// = {m_constantWorld,m_constantViewProj};

	m_deviceContext->IASetInputLayout( *mesh->getInputLayout() );
	m_deviceContext->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );
	m_deviceContext->IASetIndexBuffer(*mesh->getIndex(), DXGI_FORMAT_R16_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	if(!mesh->getVertexShader() || !mesh->getPixelShader())
		return false;
	m_deviceContext->VSSetShader(*mesh->getVertexShader(), NULL, 0 );
	m_deviceContext->VSSetConstantBuffers( 0, 2, constant );
	m_deviceContext->PSSetShader(*mesh->getPixelShader(), NULL, 0 );

	if(mesh->getTexture())
	{
		ID3D11ShaderResourceView* resourceView = mesh->getTexture()->getResourceView();
		m_deviceContext->PSSetShaderResources( 0, 1, &resourceView);
	}
	else
		m_deviceContext->PSSetShaderResources( 0, 0, NULL);
	m_deviceContext->DrawIndexed( mesh->getIndex()->getSize()/2, 0,0 );
	return true;
}
void Screen::setCamera(Camera* camera)
{
	m_camera = camera;
}
UINT Screen::getWidth() const
{
	return m_desc.BufferDesc.Width;
}
UINT Screen::getHeight() const
{
	return m_desc.BufferDesc.Height;
}
ID3D11DepthStencilView* Screen::getDepth() const
{
	return m_depth;
}
ID3D11Texture2D* Screen::getDepthTexture() const
{
	return m_depth.getTexture();
}
ID3D11Texture2D* Screen::getTexture() const
{
	return m_texture;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputLayout
// 頂点レイアウト用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InputLayout::InputLayout()
{
}
InputLayout::~InputLayout()
{
	release();
}
void InputLayout::release()
{
}
InputLayout::operator ID3D11InputLayout*() const
{
	return m_vertexLayout;
}
bool InputLayout::create(const D3D11_INPUT_ELEMENT_DESC* desc,UINT elements,LPCVOID adr,size_t size)
{
	release();
	ID3D11InputLayout* layout;
	D3DDevice::getDevice()->CreateInputLayout( desc, elements, adr,size,&layout);
	m_vertexLayout = layout;
	if(layout)
		return true;
	return false;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// Meshデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Mesh::Mesh()
{
	m_strideSize = 0;
}
Mesh::~Mesh()
{
}

bool Mesh::openTexture(LPCWSTR fileName)
{
	Texture* texture = new Texture();
	if(!texture->open(fileName))
	{
		delete texture;
		return false;
	}
	setTexture(texture);
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

bool Mesh::setPShader(LPCSTR PName)
{
	PixelShader* s = D3DDevice::getPShader(PName);
	if(!s)
		return false;
	m_pixelShader = *D3DDevice::getPShader(PName);
	return true;
}
bool Mesh::setVShader(LPCSTR VName)
{
	VertexShader* vs = D3DDevice::getVShader(VName);
	if (!vs)
		return false;
	m_vertexShader = *vs;
	return true;
}

bool Mesh::createLayout(LPCSTR VName,LPCSTR EName)
{
	VertexShader* vs = D3DDevice::getVShader(VName);
	if(!vs)
		return false;

	const D3D11_INPUT_ELEMENT_DESC* desc = D3DDevice::getInputElement(EName);
	if(!desc)
		return false;

	INT count = D3DDevice::getInputCount(EName);

	INT i;
	INT size = 0;
	for(i=0;i<count;i++)
	{
		size += D3DDevice::getDXGIFormatSize(desc[i].Format);
	}
	m_vertexShader = *vs;
	m_inputLayout = NEW InputLayout;
	m_inputLayout->create( desc, i, m_vertexShader.getData(),m_vertexShader.getSize());
	m_strideSize = size;

	return true;
}

bool Mesh::createLayout(D3D11_INPUT_ELEMENT_DESC* desc)
{
	INT i;
	INT size = 0;
	for(i=0;desc[i].SemanticName;i++)
	{
		size += D3DDevice::getDXGIFormatSize(desc[i].Format);
	}
	m_inputLayout = NEW InputLayout;
	m_inputLayout->create( desc, i, m_vertexShader.getData(),m_vertexShader.getSize());
	m_strideSize = size;
	return true;
}
void Mesh::setMaterial(Material& material)
{
	m_material = material;
}
Material* Mesh::getMaterial()
{
	return &m_material;
}
std::vector<BONEMATRIX>* Mesh::getBoneMatrix()
{
	return &m_boneMatrices;
}

void Mesh::setBoneMatrix(std::vector<BONEMATRIX>& matrices)
{
	m_boneMatrices = matrices;
}

VertexShader* Mesh::getVertexShader()
{
	return &m_vertexShader;
}
PixelShader* Mesh::getPixelShader()
{
	return &m_pixelShader;
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
InputLayout* Mesh::getInputLayout()
{
	if(!m_inputLayout.get())
		m_inputLayout = NEW InputLayout();
	return m_inputLayout.get();
}
UINT Mesh::getStrideSize() const
{
	return m_strideSize;
}
struct VERTEXPT
{
	bool operator < ( CONST VERTEXPT& v) const
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

bool Mesh::createIndex(LPCVOID data,UINT size)
{
	m_index = NEW Index;
	return m_index->create(data,size);
}
bool Mesh::createIndex(UINT size)
{
	m_index = NEW Index;
	return m_index->create(size);
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
bool Mesh::createVertex(LPCVOID data,UINT size)
{
	m_vertex = NEW Vertex;
	return m_vertex->create(data,size);
}
bool Mesh::createVertex(UINT size)
{
	m_vertex = NEW Vertex;
	return m_vertex->create(size);
}

bool Mesh::createMesh(LPCVOID data,UINT size)
{
	if(m_strideSize == 0)
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
			index = dataMap.size();
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
	delete[] vertexNew;
	return true;
}
LPVOID Mesh::lockVertex() const
{
	if(m_vertex.get())
	{
		D3D11_MAPPED_SUBRESOURCE* res = m_vertex->lock();
		if(res)
			return res->pData;
	}
	return NULL;
}
void Mesh::unlockVertex() const
{
	if(m_vertex.get())
		m_vertex->unlock();
}
LPVOID Mesh::lockIndex() const
{
	if(m_index.get())
	{
		D3D11_MAPPED_SUBRESOURCE* res = m_index->lock();
		if(res)
			return res->pData;
	}
	return NULL;
}
void Mesh::unlockIndex() const
{
	if(m_index.get())
		m_index->unlock();
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
INT Mesh::getTextureCount() const
{
	return m_texture.size();
}
void Mesh::updateVS(LPCSTR name, LPCVOID data)
{
	m_vertexShader.update(name, data);
}
void Mesh::updatePS(LPCSTR name, LPCVOID data)
{
	m_pixelShader.update(name, data);
}
void Mesh::setRasterizer(D3D11_RASTERIZER_DESC* desc)
{
	ID3D11RasterizerState* rasterizer = NULL;
	D3DDevice::getDevice()->CreateRasterizerState(desc, &rasterizer);
	if (rasterizer)
		m_rasterizer = rasterizer;
}
ID3D11RasterizerState* Mesh::getRasterizer()
{
	return m_rasterizer.get();
}
void Mesh::setDepthStencil(D3D11_DEPTH_STENCIL_DESC* desc)
{
	ID3D11DepthStencilState* depthStencil = NULL;
	D3DDevice::getDevice()->CreateDepthStencilState(desc, &depthStencil);
	if (depthStencil)
		m_depthStencil = depthStencil;
}
ID3D11DepthStencilState* Mesh::getDepthStencil()
{
	return m_depthStencil.get();
}
void Mesh::setBlend(D3D11_BLEND_DESC* desc)
{
	ID3D11BlendState* blend = NULL;
	D3DDevice::getDevice()->CreateBlendState(desc, &blend);
	if (blend)
		m_blend = blend;
}
ID3D11BlendState* Mesh::getBlend()
{
	return m_blend.get();
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ConstantBuffer
// シェーダ定数設定用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ConstantBuffer::ConstantBuffer()
{
	m_buffer = NULL;
}

bool ConstantBuffer::create(UINT size)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc,sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = size;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;

	ID3D11Buffer* buffer;
	D3DDevice::getDevice()->CreateBuffer( &desc, NULL, &buffer );
	if(buffer)
	{
		m_buffer = buffer;
		return true;
	}
	return false;
}
bool ConstantBuffer::update(LPVOID data)
{
	if(!m_buffer)
		return false;
	D3DDevice::getContext()->UpdateSubresource( m_buffer, 0, NULL, data, 0, 0 );
	return true;
}
ConstantBuffer::operator ID3D11Buffer*() const
{
	return m_buffer;
}
ConstantBuffer::operator ID3D11Buffer**()
{
	m_buffer2 = m_buffer;
	return &m_buffer2;
}

}

