#pragma once

#include "aflDirect3DUnit.h"
#include "aflDirectShow.h"
#include <D3D11.h>


namespace AFL{namespace DIRECT3D{
	/*
class InterfaceDirect3D11
{
public:
	InterfaceDirect3D11()
	{
		m_surface = NULL;
		m_device = NULL;
		m_surface = NULL;

		m_hLibrary = LoadLibraryW(L"d3d11.dll");
		if(!m_hLibrary)
			return;

		m_D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(m_hLibrary,"D3D11CreateDevice");
		if(m_D3D11CreateDevice)
	   	 	m_D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,D3D10_CREATE_DEVICE_SINGLETHREADED|D3D10_CREATE_DEVICE_BGRA_SUPPORT|D3D10_CREATE_DEVICE_STRICT_VALIDATION,NULL,0,D3D11_SDK_VERSION,&m_device,&m_level,NULL);

		m_device->QueryInterface(__uuidof(IDXGIDevice), (void **)&m_dxgiDevice);
		if(!m_dxgiDevice)
			return;
	}
	void captureScreen(INT px,INT py,INT width,INT height)
	{

		IDXGIAdapter* adapter;
		m_dxgiDevice->GetAdapter(&adapter);

		IDXGIOutput* output;
		UINT i;
		for(i=0;adapter->EnumOutputs(i,&output) != DXGI_ERROR_NOT_FOUND;i++)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			output->Release();


			if(m_surface)
			{
				D3D11_TEXTURE2D_DESC surfaceDesc;
				m_surface->GetDesc(&surfaceDesc);
				if(surfaceDesc.Width != width || surfaceDesc.Height != height)
				{
					m_surface->Release();
					m_surface = NULL;
					m_surface1->Release();
				}
			}
			if(!m_surface)
			{
				D3D11_TEXTURE2D_DESC  surfaceDesc;
				memset( &surfaceDesc, 0, sizeof( surfaceDesc ) );
				surfaceDesc.BindFlags =  0;
				surfaceDesc.Usage = D3D11_USAGE_STAGING;
				surfaceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				surfaceDesc.Width = desc.DesktopCoordinates.right;
				surfaceDesc.Height = desc.DesktopCoordinates.bottom;
				surfaceDesc.SampleDesc.Count = 1;
				//surfaceDesc.SampleDesc.Quality = 0;
				surfaceDesc.CPUAccessFlags =0;
				surfaceDesc.MipLevels = 1;
				//surfaceDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
				surfaceDesc.SampleDesc.Count = 1;
				surfaceDesc.ArraySize = 1;
				m_device->CreateTexture2D(&surfaceDesc,NULL,&m_surface);
				m_surface->QueryInterface(__uuidof(IDXGISurface1),(LPVOID*)&m_surface1);
			}

			HDC hdc = GetDC(NULL);
			HDC destDC;
			m_surface1->GetDC(true,&destDC);
			BOOL b = BitBlt(destDC,0,0,width,height,hdc,px,py,SRCCOPY);
			m_surface1->ReleaseDC(NULL);
			ReleaseDC(NULL,hdc);
			//m_surface1->Release();
			break;
		}
		adapter->Release();
	}

	typedef BOOL WINAPI DWMGETDXSHAREDSURFACE(HWND,LUID,LPVOID,LPVOID,LPVOID); 
	bool createTexture(HWND hwnd)
	{

		LUID luidN={0,0};

		IDXGIDevice* giDevice = NULL;
		m_device->QueryInterface(__uuidof(IDXGIDevice),(LPVOID*)&giDevice);

		IDXGIAdapter* adapter;
		giDevice->GetAdapter(&adapter);
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);


		HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
		INT (WINAPI* DwmpDxGetWindowSharedSurface)(HWND,LUID,DWORD,DWORD,D3DFORMAT*,HANDLE*,LPVOID*)
			=(INT (WINAPI*)(HWND,LUID,DWORD,DWORD,D3DFORMAT*,HANDLE*,LPVOID*))GetProcAddress(hDwmApi,(LPCSTR)100);

		INT (WINAPI* DwmpDxUpdateWindowSharedSurface)(HWND, int, int, int, HMONITOR, void*) = 
			(INT (WINAPI*)(HWND, int, int, int, HMONITOR, void*))GetProcAddress(hDwmApi, (LPCSTR)101);

		D3DFORMAT format=D3DFMT_UNKNOWN;
		HANDLE handle=NULL;
		DwmpDxGetWindowSharedSurface(hwnd,desc.AdapterLuid,0,0,&format,&handle,(LPVOID*)&luidN);

		if(!handle)
			return false;
//*
		RECT rect;
		::GetWindowRect(hwnd,&rect);
		INT width = rect.right - rect.left;
		INT height = rect.bottom - rect.top;

		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = (DXGI_FORMAT)format;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.SampleDesc.Count = 1;

		m_device->CreateTexture2D(&desc,);
//
		IDXGISurface1 * resource = NULL;

		m_device->OpenSharedResource(handle,__uuidof(IDXGISurface1),(LPVOID*)&resource);
		DXGI_SURFACE_DESC desc2;
		resource->GetDesc(&desc2);
		DwmpDxUpdateWindowSharedSurface(hwnd,0,0,0,0,0);
		DXGI_MAPPED_RECT r;
		resource->Map(&r,DXGI_MAP_READ|DXGI_MAP_DISCARD);
		resource->Unmap();

		FreeLibrary(hDwmApi);

		return true;
	}
protected:
	HINSTANCE m_hLibrary;
	PFN_D3D11_CREATE_DEVICE m_D3D11CreateDevice;
	ID3D11Device* m_device;
	D3D_FEATURE_LEVEL m_level;
	IDXGIDevice * m_dxgiDevice;
	ID3D11Texture2D* m_surface;
	IDXGISurface1* m_surface1;
};
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVideo
// DirectX - 動画再生用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitVideo : public UnitGdip,public MediaSampler
{
public:
	UnitVideo();
	virtual ~UnitVideo();
	void releaseBuffer();
	void addEvent(ClassProc& classProc)
	{
		m_classProc = classProc;
	}
protected:
	virtual void onStatChange()
	{
		m_classProc.call(this);
	}
	virtual void onImageInit();
	bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void onImageDraw(LPVOID data,DWORD size);
	LPVOID m_dataPtr;
	INT m_dataSize;
	Critical m_critical;
	ClassProc m_classProc;
	bool m_initImage;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TargetRecoder
// DirectX - ターゲット取り込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class TargetRecoder
{
public:
	TargetRecoder();
	~TargetRecoder();
	bool start();
	bool stop();
	INT getVideoHeight() const;
	INT getVideoWidth() const;
	void setVideoWidth(INT value);
	void setVideoHeight(INT value);
	void setSize(INT width,INT height);
	void draw(Screen* screen);
	void draw(Target* screen);
	void draw(IDirect3DSurface9* src);
	DWORD getAudioBitrate() const;
	DWORD getVideoBitrate() const;
	FLOAT getVideoFPS() const;
	void setAudioBitrate(DWORD value);
	void setVideoBitrate(DWORD value);
	void setVideoFPS(FLOAT value);
	bool isNetwork() const;
	void setNetwork(bool flag=true);
	INT getNetworkPort() const;
	void setNetworkPort(INT value);
	LPCWSTR getFileName() const;
	void setFileName(LPCWSTR fileName);
	bool isFile() const;
	void setFile(bool flag=true);
	void setVideoQuality(INT value);
	INT getVideoQuality() const;
	bool isRecode() const;
protected:
	DWORD captureSound(LPVOID* data);
	DSOUND::SoundRecoder m_soundRecoder;

	Critical m_critical;
	Target m_target;
	WMWriter* m_writer;
	INSSBuffer* m_buffer;

	bool m_recode;
	INT m_pitch;
	QWORD m_timer;
	bool m_network;
	INT m_networkPort;
	bool m_file;
	WString m_fileName;

	DWORD m_audioBitrate;
	DWORD m_videoBitrate;
	FLOAT m_videoFPS;
	INT m_videoQuality;
	INT m_videoWidth;
	INT m_videoHeight;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitScreenCapture
// DirectX - スクリーンキャプチャ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitScreenCapture : public UnitSprite
{
public:
	UnitScreenCapture();
	void setScreenRect(RECT* rect);
protected:
	virtual bool onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z);
	RECT m_screen;
	UnitSprite m_image[2];
	INT m_index;
	ThreadTimer m_thread;
};
}}

