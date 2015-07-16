#include <windows.h>
#include <ddraw.h>
#include "aflDirectDraw.h"

#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif

const GUID __CLSID_DirectDraw = {0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35};
const GUID __IID_IDirectDraw = {0x6C14DB80,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60};

namespace AFL{namespace DirectDraw{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device
// DirectX - DirectDraw基本ラッパー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//--------------------------------
// 
//--------------------------------
Device::Device()
{
	m_pDirectDraw = NULL;
	::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	::CoCreateInstance(__CLSID_DirectDraw,NULL, CLSCTX_INPROC_SERVER, __IID_IDirectDraw, (LPVOID*)&m_pDirectDraw);
	if(m_pDirectDraw)
		m_pDirectDraw->Initialize(NULL);
}

//--------------------------------
// 
//--------------------------------
Device::~Device()
{
	if(m_pDirectDraw)
		m_pDirectDraw->Release();
	//::CoUninitialize();
}

//--------------------------------
// 
//--------------------------------
bool Device::setFullMode(HWND hWnd,int nWidth,int nHeight,int nBits)
{
	if(!m_pDirectDraw)
		return false;

	if(nBits == 15)
		nBits = 16;
	
	if(m_pDirectDraw->SetCooperativeLevel(hWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN)!=DD_OK)
		return false;
	//現在のビット深度に合わせる
	if(!nBits)
	{
		DDSURFACEDESC descSurface;
		descSurface.dwSize = sizeof(DDSURFACEDESC);
		m_pDirectDraw->GetDisplayMode(&descSurface);
			nBits = descSurface.ddpfPixelFormat.dwRGBBitCount;
	}
	if (m_pDirectDraw->SetDisplayMode(nWidth,nHeight,nBits) != DD_OK)
		return false;
	return true;
}

//--------------------------------
// 
//--------------------------------
bool Device::setWindowMode()
{
	if(!m_pDirectDraw)
		return false;
	if(m_pDirectDraw->SetCooperativeLevel(NULL,DDSCL_NORMAL) == DD_OK)
		if(m_pDirectDraw->RestoreDisplayMode() == DD_OK)
			return true;
	return false;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// DirectX - DirectDrawスクリーンモード設定
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//--------------------------------
// 
//--------------------------------
Screen::Screen()
{
	m_pDDraw = NULL;
	m_bFullScreen = false;
}

//--------------------------------
// 
//--------------------------------
Screen::~Screen()
{
	if(m_pDDraw)
		delete m_pDDraw;
}
//--------------------------------
// 
//--------------------------------
bool Screen::setScreenMode(HWND hWnd,bool bMode,INT iWidth,INT iHeight)
{
	if(bMode)
		return changeDirectMode(hWnd,iWidth,iHeight);
	else
		return changeWindowMode();
}
//--------------------------------
// 
//--------------------------------
Device* Screen::getDevice()
{
	if(!m_pDDraw)
		m_pDDraw = NEW Device;
	return m_pDDraw;
}
//--------------------------------
// 
//--------------------------------
bool Screen::changeDirectMode(HWND hWnd,INT iWidth,INT iHeight)
{
	if(m_bFullScreen)
		return false;
	if(!hWnd)
		hWnd = m_hWnd;
	else
		m_hWnd = hWnd;
	::GetWindowRect(hWnd,&m_rectWindow);
	m_dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	m_dwStyleEx = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	m_hWndParent = ::GetParent(hWnd);
	if(m_hWndParent)
		::SetParent(m_hWnd,0);
	SetWindowLong(hWnd, GWL_STYLE, WS_POPUP| WS_CLIPCHILDREN );
	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	m_bFullScreen = true;
	setFullScreenMode(iWidth,iHeight);
	if(!m_bFullScreen)
	{
		m_bFullScreen = true;
		changeWindowMode();
	}
	return m_bFullScreen;
}

//--------------------------------
// 
//--------------------------------
bool Screen::setFullScreenMode(INT iWidth,INT iHeight)
{
	if(m_bFullScreen)
	{
		m_bFullScreen = getDevice()->setFullMode(m_hWnd,iWidth,iHeight,0);
	}
	return m_bFullScreen;
}

//--------------------------------
// 
//--------------------------------
bool Screen::changeWindowMode()
{
	if(!IsWindow(m_hWnd))
		return false;
	if(m_bFullScreen)
	{
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, m_dwStyleEx);
		::SetWindowLong(m_hWnd, GWL_STYLE, m_dwStyle);
		if(m_hWndParent)
			::SetParent(m_hWnd,m_hWndParent);
		if(m_hMenu)
		{
			SetMenu(m_hWnd,m_hMenu);
			m_hMenu = 0;
		}
		getDevice()->setWindowMode();
		::SetWindowPos(m_hWnd,NULL,m_rectWindow.left,m_rectWindow.top,
			m_rectWindow.right-m_rectWindow.left,m_rectWindow.bottom-m_rectWindow.top,SWP_FRAMECHANGED);
	}
	m_bFullScreen = false;
	return true;
}

}}
