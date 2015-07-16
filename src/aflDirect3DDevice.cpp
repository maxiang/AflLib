#include <windows.h>
//#include <d3dx9.h>
#include "aflDirect3DUnit.h"

//#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

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

namespace AFL{namespace DIRECT3D{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InterfaceDevice
// DirectX - Direct3D用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
//InterfaceDevice::InterfaceDevice()
// ---  動作  ---
// d3d8.dllを動的に呼び出す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
InterfaceDevice::InterfaceDevice()
{
	//ポインタ初期化
	m_pd3d = NULL;
	m_pd3dEx = NULL;
	m_pd3dStatus = NULL;

	//Direct3DCreate8をDLLから参照
	//COM使えず、またLoadLibraryの時代に逆戻り
	m_hLibrary = LoadLibraryW(L"d3d9.dll");
	if(!m_hLibrary)
		return;
	/*
	//EXの設定
	
#if defined(D3DENUM_NO_DRIVERVERSION)
	HRESULT (WINAPI* direct3DCreate9Ex)(UINT,IDirect3D9Ex**) =
		 (HRESULT (WINAPI*)(UINT, IDirect3D9Ex**))GetProcAddress(m_hLibrary,"Direct3DCreate9Ex");
	if(direct3DCreate9Ex)
	{
		direct3DCreate9Ex(D3D_SDK_VERSION,&m_pd3dEx);
		if(m_pd3dEx)
			m_pd3dEx->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void**>(&m_pd3d));
	}

#endif
	*/
	if(!m_pd3d)
	{
		//DLLから関数のアドレスを取得
		IDirect3D9* (WINAPI *pd3dCreate9)(UINT);
		pd3dCreate9 = (IDirect3D9* (WINAPI *)(UINT))GetProcAddress(m_hLibrary,"Direct3DCreate9");
		if(pd3dCreate9)	//取得成功なら
			m_pd3d = pd3dCreate9(D3D_SDK_VERSION);
	}	

	m_d3dpp.BackBufferWidth = 640;
	m_d3dpp.BackBufferHeight = 480;
	m_deviceStat = D3D_DEVICE_NOTINIT;
	m_hWnd = NULL;

}

//-----------------------------------------------
//InterfaceDevice::~InterfaceDevice()
// ---  動作  ---
// d3d8.dllを解放
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
InterfaceDevice::~InterfaceDevice()
{
	if(m_pd3dStatus)
		m_pd3dStatus->Release();
	//D3D解放
	if(m_pd3d)
		m_pd3d->Release();

#if defined(D3DENUM_NO_DRIVERVERSION)
	//D3D解放
	if(m_pd3dEx)
		m_pd3dEx->Release();
#endif

#ifdef _MSC_VER
	//DLLの解放
	if(m_hLibrary)
		FreeLibrary(m_hLibrary);
#endif
}
//-----------------------------------------------
//IDirect3D9* InterfaceDevice::getD3D() const
// ---  動作  ---
// IDirect3D9の取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// IDirect3D9のポインタ
//-----------------------------------------------
IDirect3D9* InterfaceDevice::getInterface() const
{
	return m_pd3d;
}
IDirect3D9Ex* InterfaceDevice::getInterfaceEx() const
{
	return m_pd3dEx;
}
//-----------------------------------------------
//bool InterfaceDevice::isDirectX() const
// ---  動作  ---
// DirectXGraphicsの存在チェック
// ---  引数  ---
// 無し
// --- 戻り値 ---
// true:有効 false:無効
//-----------------------------------------------
bool InterfaceDevice::isDirectX() const
{
	return m_pd3d!=NULL;
}

//-----------------------------------------------
//bool InterfaceDevice::init() const
// ---  動作  ---
// DirectGraphicsの存在チェック
// ---  引数  ---
//  HWND hWnd         ウインドウハンドル
//  UINT deviceWidth  デバイスサイズ
//  UINT deviceHeight
//  bool screenFull   スクリーンモード初期状態
//  bool videoSync    垂直同期
// --- 戻り値 ---
// true:正常 false:異常
//-----------------------------------------------
bool InterfaceDevice::init(HWND hWnd,UINT deviceWidth,UINT deviceHeight,bool screenFull,bool videoSync)
{
	//DirectX8以降が存在しているかチェック
	if(!isDirectX())
	{
		m_deviceStat = D3D_DEVICE_ERROR_DIRECTX;
		return false;
	}
	m_reset = false;
	if(hWnd)
		m_hWnd = hWnd;
	else
		m_hWnd = GetActiveWindow();

	//デフォルトのアダプタの状態を取得
	D3DDISPLAYMODE d3dDisplayMode;
	getInterface()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3dDisplayMode );

	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp));				//構造体の初期化
	//サイズの設定
	if(deviceWidth)
		m_d3dpp.BackBufferWidth = deviceWidth;
	else
		m_d3dpp.BackBufferWidth = 640;

	if(deviceHeight)
		m_d3dpp.BackBufferHeight = deviceHeight;
	else
		m_d3dpp.BackBufferHeight = 480;
	
	m_screenFull = screenFull;

	m_d3dpp.Windowed = !m_screenFull;					//ウインドウ／フルスクリーン切り替えフラグ
	//m_d3dpp.BackBufferWidth = m_deviceWidth;			//デバイスサイズ幅
	//m_d3dpp.BackBufferHeight= m_deviceHeight;			//デバイスサイズ高さ
	m_d3dpp.BackBufferCount = 1;						//バックバッファの数
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;				//コピーモード
	m_d3dpp.BackBufferFormat = d3dDisplayMode.Format;		//カラーモードを現在の状態に合わせる
	m_d3dpp.EnableAutoDepthStencil = true;				//Zバッファ使用
	m_d3dpp.AutoDepthStencilFormat  = D3DFMT_D24S8;		//バッファ深度の設定
	m_d3dpp.hDeviceWindow = hWnd;						//ウインドウの指定

	if(videoSync)	//リフレッシュレートと同期に設定
		m_d3dpp.PresentationInterval = 0;
	else
		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

#if defined(D3DENUM_NO_DRIVERVERSION)
	//デバイスが既に存在していたら解放
	if(m_pd3dDeviceEx)
	{
		m_pd3dDeviceEx->Release();
		m_pd3dDeviceEx = NULL;
	}
#endif
	if(m_pd3dDevice)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	D3DCAPS9 caps;
	m_pd3d->GetDeviceCaps(0,D3DDEVTYPE_HAL,&caps);

#if defined(D3DENUM_NO_DRIVERVERSION)

	if(m_pd3dEx)
	{
		if(caps.VertexShaderVersion>=D3DVS_VERSION(1,1))
		{
			m_pd3dEx->CreateDeviceEx( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
					  D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE,
						  &m_d3dpp, NULL,&m_pd3dDeviceEx );
		}
		else
		{
			m_pd3dEx->CreateDeviceEx( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
					  D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						  &m_d3dpp, NULL,&m_pd3dDeviceEx );
		}
		if(m_pd3dDeviceEx)
			m_pd3dDeviceEx->QueryInterface(__uuidof(IDirect3DDevice9), reinterpret_cast<void**>(&m_pd3dDevice));
	}
	
#endif
	//デバイスの作成
	if(!m_pd3dDevice)
	{
		if(caps.VertexShaderVersion>=D3DVS_VERSION(1,1))
		{
			//ハードウエア頂点処理
			m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
									  D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE,
									  &m_d3dpp, &m_pd3dDevice );
		}
		else
		{
			//ソフトウエア頂点処理
			m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
								D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING,
								&m_d3dpp, &m_pd3dDevice );
		}
	}
	if(!m_pd3dDevice)
	{
		//ステンシルバッファを諦める
		m_d3dpp.AutoDepthStencilFormat  = D3DFMT_D16;		
		m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
							D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING,
							&m_d3dpp, &m_pd3dDevice );
	}
	if(!m_pd3dDevice)
	{
		//リファレンスラスタライザ
		m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF , hWnd,
						  D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING,
						  &m_d3dpp, &m_pd3dDevice );
	}
	if(m_pd3dDevice)
	{
		m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
		m_deviceStat = D3D_DEVICE_ACTIVE;


		setDefaultStat();
		return true;
	}
	m_deviceStat = D3D_DEVICE_ERROR_DEVIVE;
	return false;
}

IDirect3DDevice9* InterfaceDevice::getDeviceInterface() const
{
	return m_pd3dDevice;
}
IDirect3DDevice9Ex* InterfaceDevice::getDeviceInterfaceEx() const
{
	return m_pd3dDeviceEx;
}
bool InterfaceDevice::resetDevice()
{
	//デバイスのチェック
	if(!m_pd3dDevice)
		return false;


	//スクリーンモードの設定
//	if(m_screenFull)
//		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
//	else
//		m_d3dpp.PresentationInterval = 0;

	//デフォルトのアダプタの状態を取得
	D3DDISPLAYMODE d3dDisplayMode;
	getInterface()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3dDisplayMode );
	m_d3dpp.BackBufferFormat = d3dDisplayMode.Format;	//カラーモードを現在の状態に合わせる

	//デバイスのリセット
	bool bRet;
	bRet = m_pd3dDevice->Reset(&m_d3dpp) == D3D_OK;
	//フルスクリーンが維持できない場合、ウインドウモードへ
	if(!bRet && !m_d3dpp.Windowed)
	{
		m_d3dpp.PresentationInterval = 0;
		m_screenFull = false;
		m_d3dpp.Windowed = true;
		bRet = m_pd3dDevice->Reset(&m_d3dpp) == D3D_OK;
	}
	//ウインドウスタイルを戻す
	if(bRet && !m_screenFull)
		SetWindowPos(m_d3dpp.hDeviceWindow, NULL, 0, 0, 0, 0,SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);

	if(!bRet)
	{
		m_deviceStat = D3D_DEVICE_ERROR_DEVIVE;
		return false;
	}
	m_deviceStat = D3D_DEVICE_ACTIVE;
	setDefaultStat();
	return true;
}
IDirect3DStateBlock9* InterfaceDevice::getStatusBlock()
{
	if(m_pd3dStatus == NULL && m_pd3dDevice)
	{
		m_pd3dDevice->CreateStateBlock(D3DSBT_ALL,&m_pd3dStatus);
	}
	return m_pd3dStatus;
}

D3D_DEVICESTAT InterfaceDevice::getStat() const
{
	return m_deviceStat;
}
D3DPRESENT_PARAMETERS const* InterfaceDevice::getParams() const
{
	return &m_d3dpp;
}
UINT InterfaceDevice::getWidth() const
{
	return m_d3dpp.BackBufferWidth;
}
UINT InterfaceDevice::getHeight() const
{
	return m_d3dpp.BackBufferHeight;
}
UINT InterfaceDevice::getDeviceWidth() const
{
	return m_d3dpp.BackBufferWidth;
}
UINT InterfaceDevice::getDeviceHeight() const
{
	return m_d3dpp.BackBufferHeight;
}


D3DCAPS9 const* InterfaceDevice::getCaps() const
{
	return &m_d3dCaps;
}

bool InterfaceDevice::setDefaultStat()
{
	if(!m_pd3dDevice)
		return false;

	//Zバッファ有効
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE  ,D3DZB_TRUE);
	//αテスト設定
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,   true );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 8);
	//αブレンド設定
	m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   true );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA   );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA  );

	//m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);  


	m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, true );

	//フィルター設定
	m_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
	m_pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	
	m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, true );

	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device
// DirectX - Direct3Dデバイス操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
std::map<String,VertexShader> Device::m_mapVShader;
//std::map<String,PixelShader> Device::m_mapPShader;

UINT Device::getDepthWidth()
{
	if(!getInterface())
		return 0;
	D3DSURFACE_DESC desc;
	IDirect3DSurface9* m_depth;
	getInterface()->GetDepthStencilSurface(&m_depth);
	if(!m_depth)
		return 0;
	m_depth->GetDesc(&desc);
	m_depth->Release();
	return desc.Width;
}
UINT Device::getDepthHeight()
{
	if(!getInterface())
		return 0;
	D3DSURFACE_DESC desc;
	IDirect3DSurface9* m_depth;
	getInterface()->GetDepthStencilSurface(&m_depth);
	if(!m_depth)
		return 0;
	m_depth->GetDesc(&desc);
	m_depth->Release();
	return desc.Height;
}
//-----------------------------------------------
// bool Device::init(HWND hWnd,UINT deviceWidth,UINT deviceHeight,bool screenFull,bool videoSync)
// -----  動作  -----
// 3Dデバイスを初期化する
// -----  引数  -----
// hWnd         ウインドウハンドル
// deviceWidth  デバイスサイズ幅
// deviceHeight デバイスサイズ高さ
// screenFull   true:フルスクリーン false:ウインドウ
// videoSync    true:垂直同期待ちfalse:待たない
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Device::init(HWND hWnd,UINT deviceWidth,UINT deviceHeight,bool screenFull,bool videoSync)
{
	if(m_interfaceDevice.init(hWnd,deviceWidth,deviceHeight,screenFull,videoSync))
	{
		std::list<Object*>::iterator it;
		for(it=m_interfaceDevice.m_listObject.begin();it!=m_interfaceDevice.m_listObject.end();++it)
		{
			(*it)->onDeviceInit();
		}
		return true;
	}
	return false;
}
//-----------------------------------------------
// D3D_DEVICESTAT Device::getStat()
// -----  動作  -----
// デバイスの状態を取得する
// -----  引数  -----
// 無し
// ----- 戻り値 -----
//	D3D_DEVICE_NOTINIT		デバイス初期化前
//	D3D_DEVICE_ERROR_DIRECTX	DirectXがインストールされていない
//	D3D_DEVICE_ERROR_DEVIVE	デバイスが作成できない
//	D3D_DEVICE_ACTIVE			正常動作中
//-----------------------------------------------
D3D_DEVICESTAT Device::getStat()
{
	return m_interfaceDevice.getStat();
}
IDirect3DDevice9* Device::getInterface()
{
	return m_interfaceDevice.getDeviceInterface();
}
IDirect3DDevice9Ex* Device::getInterfaceEx()
{
	return m_interfaceDevice.getDeviceInterfaceEx();
}
D3DCAPS9 const* Device::getCaps()
{
	return m_interfaceDevice.getCaps();
}
UINT Device::getTargetWidth()
{
	if(getInterface())
	{
		IDirect3DSurface9* backBuffer = NULL;
		getInterface()->GetRenderTarget(0,&backBuffer);
		if(backBuffer)
		{
			D3DSURFACE_DESC descBack;
			backBuffer->GetDesc(&descBack);
			backBuffer->Release();
			return descBack.Width;
		}
	}
	return 0;
}
UINT Device::getTargetHeight()
{
	if(getInterface())
	{
		IDirect3DSurface9* backBuffer = NULL;
		getInterface()->GetRenderTarget(0,&backBuffer);
		if(backBuffer)
		{
			D3DSURFACE_DESC descBack;
			backBuffer->GetDesc(&descBack);
			backBuffer->Release();
			return descBack.Height;
		}
	}
	return 0;
}
IDirect3DSurface9* Device::getRenderTarget()
{
	IDirect3DSurface9* backBuffer = NULL;
	getInterface()->GetRenderTarget(0,&backBuffer);
	return backBuffer;
}

bool Device::setRenderTarget(IDirect3DSurface9* backBuffer)
{
	if(getInterface()->SetRenderTarget(0,backBuffer) != D3D_OK)
		return false;

	D3DSURFACE_DESC descBack,descDepth;
	backBuffer->GetDesc(&descBack);

	IDirect3DSurface9* m_depth;
	getInterface()->GetDepthStencilSurface(&m_depth);
	m_depth->GetDesc(&descDepth);
	m_depth->Release();

	UINT width,height;
	if(descBack.Width > descDepth.Width)
		width = descBack.Width;
	else
		width = descDepth.Width;
	if(descBack.Height > descDepth.Height)
		height = descBack.Height;
	else
		height = descDepth.Height;	
	if(descDepth.Width != width || descDepth.Height != height)
	{
		IDirect3DSurface9* m_depthNew;
		getInterface()->CreateDepthStencilSurface(
			width,height,descDepth.Format,D3DMULTISAMPLE_NONE,0,TRUE,&m_depthNew,NULL);
		getInterface()->SetDepthStencilSurface(m_depthNew);
		m_depthNew->Release();
	}
	return true;
}
bool Device::setDeviceSize(INT width,INT height)
{
	m_interfaceDevice.m_d3dpp.BackBufferHeight = height;
	m_interfaceDevice.m_d3dpp.BackBufferWidth = width;
	m_interfaceDevice.m_reset = true;
	//resetDevice();
	return true;
}

bool Device::setFullScreen(bool flag)
{
	return m_screen.setScreenMode(m_interfaceDevice.getWindowHandle(),flag,getWidth(),getHeight());
}
bool Device::isFullScreen()
{
	return m_screen.isScreenMode();
}
void Device::getAdapterDisplayMode(D3DDISPLAYMODE* pd3dDisplayMode)
{
	m_interfaceDevice.getInterface()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, pd3dDisplayMode );
}



InterfaceDevice Device::m_interfaceDevice;
AFL::DirectDraw::Screen Device::m_screen;

WString Device::m_texturePath;
//-----------------------------------------------
// bool Device::setClipper(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3)
// -----  動作  -----
// デバイスにクリッピングの指示を与える
// -----  引数  -----
// 
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool Device::setClipper(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	DWORD dwClip = D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2 | D3DCLIPPLANE3;
	FLOAT x01,y01,z01;
	FLOAT x02,y02,z02;
	NVector vect1;
	FLOAT length,length2;
	//X
	vect1.x = x1-x2;
	vect1.y = y1-y2;
	vect1.z = z1-z2;
	vect1 = vect1.normal3();
	x01 = x1*vect1.x;
	y01 = y1*vect1.y;
	z01 = z1*vect1.z;
	x02 = x2*vect1.x;
	y02 = y2*vect1.y;
	z02 = z2*vect1.z;
	length = sqrt(x01*x01+y01*y01+z01*z01)*(x01+y01+z01<0?-1:1);
	length2 = sqrt(x02*x02+y02*y02+z02*z02)*(x02+y02+z02<0?1:-1);
	FLOAT clip0[] = {-vect1.x,-vect1.y,-vect1.z,length};
	FLOAT clip1[] = {vect1.x,vect1.y,vect1.z,length2};
	pd3dDevice->SetClipPlane(0,clip0); 
	pd3dDevice->SetClipPlane(1,clip1); 
	//Y
	vect1.x = x1-x3;
	vect1.y = y1-y3;
	vect1.z = z1-z3;
	vect1 = vect1.normal3();
	x01 = x1*vect1.x;
	y01 = y1*vect1.y;
	z01 = z1*vect1.z;
	x02 = x3*vect1.x;
	y02 = y3*vect1.y;
	z02 = z3*vect1.z;

	length = sqrt(x01*x01+y01*y01+z01*z01)*(x01+y01+z01<0?-1:1);
	length2 = sqrt(x02*x02+y02*y02+z02*z02)*(x02+y02+z02<0?1:-1);
	FLOAT clip2[] = {-vect1.x,-vect1.y,-vect1.z,length};
	FLOAT clip3[] = {vect1.x,vect1.y,vect1.z,length2};
	pd3dDevice->SetClipPlane(2,clip2); 
	pd3dDevice->SetClipPlane(3,clip3); 
	
	pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,dwClip);
	return true;

}

bool Device::setClipper(FLOAT fX,FLOAT fY,FLOAT fZ,	FLOAT fWidth,FLOAT fHeight,FLOAT fDepth)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	DWORD dwClip = 0;
	if(fWidth)
	{
		FLOAT fClip1[] = {1,0,0,-fX};
		FLOAT fClip2[] = {-1,0,0,(fX+fWidth)};
		pd3dDevice->SetClipPlane(0,fClip1); 
		pd3dDevice->SetClipPlane(1,fClip2); 
		dwClip |= D3DCLIPPLANE0 | D3DCLIPPLANE1;
	}
	if(fHeight)
	{
		FLOAT fClip1[] = {0,-1,0,-fY};
		FLOAT fClip2[] = {0,1,0,(fY+fHeight)};
		pd3dDevice->SetClipPlane(2,fClip1); 
		pd3dDevice->SetClipPlane(3,fClip2); 
		dwClip |= D3DCLIPPLANE2 | D3DCLIPPLANE3;
	}
	if(fDepth)
	{
		FLOAT fClip1[] = {0,0,1,-fZ};
		FLOAT fClip2[] = {0,0,-1,(fZ+fDepth)};
		pd3dDevice->SetClipPlane(4,fClip1); 
		pd3dDevice->SetClipPlane(5,fClip2); 
		dwClip |= D3DCLIPPLANE4 | D3DCLIPPLANE5;
	}
	pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,dwClip);
	return true;
}
void Device::addObject(Object* object)
{
	m_interfaceDevice.m_listObject.push_back(object);
}
void Device::delObject(Object* object)
{
	m_interfaceDevice.m_listObject.remove(object);
}
bool Device::isLost()
{
	if(!getInterface())
		return false;
	bool lost = getInterface()->TestCooperativeLevel() != D3D_OK || m_interfaceDevice.m_reset;
	return lost;
}
bool Device::resetDevice()
{
	//ロスト通知
	std::list<Object*>::iterator it;
	for(it=m_interfaceDevice.m_listObject.begin();it!=m_interfaceDevice.m_listObject.end();++it)
	{
		(*it)->onDeviceLost();
	}
	//getShader()->onDeviceLost();

	bool reset = m_interfaceDevice.resetDevice();
	if(reset)
	{
		std::list<Object*>::iterator it;
		for(it=m_interfaceDevice.m_listObject.begin();it!=m_interfaceDevice.m_listObject.end();++it)
		{
			(*it)->onDeviceRestore();
		}
		//getShader()->onDeviceReset();
	}
	m_interfaceDevice.m_reset = false;
	return reset;
}
void Device::saveStatus()
{
	IDirect3DStateBlock9* pd3dStatus = m_interfaceDevice.getStatusBlock();
	if(pd3dStatus)
	{
		pd3dStatus->Capture();
	}
}
void Device::loadStatus()
{
	IDirect3DStateBlock9* pd3dStatus = m_interfaceDevice.getStatusBlock();
	if(pd3dStatus)
	{
		pd3dStatus->Apply();
	}
}
bool Device::getAdapterLUID(UINT Adapter,LUID *pLUID)
{
#if defined(D3DENUM_NO_DRIVERVERSION)
	if(!m_interfaceDevice.getInterfaceEx())
		return false;
	m_interfaceDevice.getInterfaceEx()->GetAdapterLUID(Adapter,pLUID);
	return true;
#else
	return false;
#endif
}

bool Device::present(HWND hWnd)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	//デバイスがロストしていないかチェック
	if(Device::isLost())
	{
		//ロストデバイスの修復
		if(!Device::resetDevice())
			return false;
	}
	IDirect3DSurface9* s = NULL;
	getInterface()->GetRenderTarget(0,&s);
	if(s)
	{
		D3DSURFACE_DESC desc;
		s->GetDesc(&desc);
		//描画済みサーフェイスを表示
		RECT rect1 = {0,0,desc.Width,desc.Height};
		getInterface()->Present( &rect1, &rect1, hWnd, NULL);
		s->Release();
	}
	return true;
}
VertexShader* Device::getVShader(LPCSTR name)
{
	std::map<String,VertexShader>::iterator it = m_mapVShader.find(name);
	if(it == m_mapVShader.end())
		return NULL;
	return &it->second;
}
void Device::addVShader(LPCSTR name,VertexShader& shader)
{
	m_mapVShader[name] = shader;
}
/*
PixelShader* Device::getPShader(LPCSTR name)
{
	std::map<String,PixelShader>::iterator it = m_mapPShader.find(name);
	if(it == m_mapPShader.end())
		return NULL;
	return &it->second;
}
void D3DDevice::addPShader(LPCSTR name,PixelShader& shader)
{
	m_mapPShader[name] = shader;
}
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Object
// 3Dオブジェクト基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Object::Object()
{
	m_lost = false;;
	m_restore = false;;
	Device::addObject(this);
}
Object::~Object()
{
	Device::delObject(this);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Light
// ライト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Light::Light()
{
	//デフォルトライト
	ZeroMemory((D3DLIGHT9*)this, sizeof(D3DLIGHT9) );
	Type       = D3DLIGHT_DIRECTIONAL;
	Diffuse.r  = 1.0f;
	Diffuse.g  = 1.0f;
	Diffuse.b  = 1.0f;
	Diffuse.a  = 1.0f;
	NVector vect = {0.4f,-0.4f,-0.5f};
	m_enable = true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fog
// フォグ管理用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//-----------------------------------------------
// ---  動作  ---
// コンストラクタ
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
Fog::Fog()
{
	m_enable = true;
	m_fogStart = 1000.0f;
	m_fogEnd = 30000.0f;
	m_fogColor = 0x00a0a0a0;
}
/*
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CameraMAX
// 3DSMAXビュー制御用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CameraMAX::CameraMAX()
{
	m_far = 2000.0f;
	m_near = 1.0f;
	m_fov = 45;
	m_deviceWidth = 0;
	m_deviceHeight = 0;
	m_persp = true;
}
void CameraMAX::setPersp(bool flag)
{
	m_persp = flag;
	m_deviceWidth = 0;
}
void CameraMAX::setDp(float nearZ,float farZ)
{
	m_near = nearZ;
	m_far = farZ;
}
void CameraMAX::setWidth(FLOAT value)
{
	m_width=value;
}

bool CameraMAX::setAngle(UINT uDeviceWidth,UINT uDeviceHeight)
{

	if(m_deviceWidth != uDeviceWidth || m_deviceHeight != uDeviceHeight)
	{
		//視野角度
		FLOAT fViewAngle = m_fov;
		//デバイスサイズの半分を各頂点に
		FLOAT fWidth = (FLOAT)uDeviceWidth/2;
		FLOAT fHeight = (FLOAT)uDeviceHeight/2;
		//アスペクト比(高さを1としたときの幅)
		FLOAT fAspect = (FLOAT)uDeviceWidth/uDeviceHeight;
		//視野をZ=0でデバイスの幅と高さに合わせる
		FLOAT fFovy = fViewAngle*D3DX_PI/180.0f;				
		//奥行き
		FLOAT fDepth = (FLOAT)fHeight/(FLOAT)tan(fFovy/2.0f);					

		float h = m_near*(FLOAT)tan(fFovy/2.0f);
		//奥行きに対する比率を調整
		if(m_persp)
			m_matProjection.setPerspectiveFovRH(fFovy, fAspect,m_near,m_far);
		else
			m_matProjection.setOrthoOrthoRH(m_width,m_width/fAspect,m_near,m_far);
		//m_depth = fDepth;
		m_deviceWidth = uDeviceWidth;
		m_deviceHeight = uDeviceHeight;
	}
	m_matView = m_matExt * m_matViewBase;
	//m_matView = m_matBase3D * m_matExt;
	return true;
}
*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// スワップチェイン
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Screen::Screen()
{
	m_swapChain = NULL;
	m_width = 0;
	m_height = 0;
}
Screen::~Screen()
{
	releaseScreen();
}
bool Screen::releaseScreen()
{
	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = NULL;
	}
	return true;
}
bool Screen::onDeviceLost()
{
	//ロスト時の解放処理
	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = NULL;
		m_lost = true;
		m_restore = false;
	}
	return true;
}
bool Screen::setTarget()
{

	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;
	if(!m_swapChain)
		return false;

	IDirect3DSurface9* backBuffer = NULL;
	m_swapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&backBuffer);
	if(!backBuffer)
		return false;
	Device::setRenderTarget(backBuffer);
	backBuffer->Release();
	return true;
}

bool Screen::onDeviceRestore()
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//ロスト時の復元
	return create(m_width,m_height);
}
HDC Screen::getDC()
{
	HDC hdc = NULL;
	IDirect3DSurface9* surface = NULL;
	if(m_swapChain)
	{
		m_swapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface);
		if(surface)
		{
			surface->GetDC(&hdc);
			surface->Release();
		}
	}
	return hdc;
}
void Screen::releaseDC(HDC hdc)
{
	IDirect3DSurface9* surface = NULL;
	if(m_swapChain)
	{
		m_swapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface);
		if(surface)
		{
			surface->ReleaseDC(hdc);
			surface->Release();
		}
	}
}
INT Screen::getWidth() const
{
	return m_d3dpp.BackBufferWidth;
}
INT Screen::getHeight() const
{
	return m_d3dpp.BackBufferHeight;
}
IDirect3DSurface9* Screen::getSurface() const
{
	IDirect3DSurface9* surface = NULL;
	m_swapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&surface);
	return surface;
}
bool Screen::present(HWND hWnd,bool scale)
{
	//デバイスがロストしていないかチェック
	if(Device::isLost())
	{
		//ロストデバイスの修復
		if(!Device::resetDevice())
			return false;
	}
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	if(m_swapChain)
	{
		//描画済みサーフェイスを表示
		if(scale)
		{
			RECT rect1 = {0,0,m_width,m_height};

			RECT clientRect;
			GetClientRect(hWnd,&clientRect);
			INT x1 = clientRect.right - clientRect.left;
			INT y1 = clientRect.bottom - clientRect.top;

			INT x2 = rect1.right - rect1.left;
			INT y2 = rect1.bottom - rect1.top; 
			FLOAT f1 = (FLOAT)x1 / (FLOAT)y1;
			FLOAT f2 = (FLOAT)x2 / (FLOAT)y2;

			if(f1 < f2)
			{
				INT s = (INT)(x1/f2);
				INT r = (clientRect.bottom - s);
				clientRect.top = r;
				clientRect.bottom = s+r; 
			}
			else
			{
				INT s = (INT)(y1*f2);
				INT r = (clientRect.right - s) / 2;
				clientRect.left = r;
				clientRect.right = s+r;
			}


			m_swapChain->Present(&rect1, &clientRect, hWnd, NULL,D3DPRESENT_DONOTWAIT);
		}
		else
		{
			RECT rect1 = {0,0,m_width,m_height};
			m_swapChain->Present( &rect1, &rect1, hWnd, NULL,D3DPRESENT_DONOTWAIT);
		}
	}
	return true;
}
bool Screen::create(INT width,INT height,D3DFORMAT format)
{
	m_width = width;
	m_height = height;

	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;
	//デフォルトのアダプタの状態を取得
	D3DDISPLAYMODE d3dDisplayMode;
	Device::getAdapterDisplayMode(&d3dDisplayMode);


	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp));				//構造体の初期化
	m_d3dpp.Windowed = true;							//ウインドウ／フルスクリーン切り替えフラグ
	m_d3dpp.BackBufferWidth = width;					//デバイスサイズ幅
	m_d3dpp.BackBufferHeight= height;					//デバイスサイズ高さ
	m_d3dpp.BackBufferCount = 1;						//バックバッファの数
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;			//コピーモード
	if(format == D3DFMT_UNKNOWN)
		m_d3dpp.BackBufferFormat = d3dDisplayMode.Format;	//カラーモードを現在の状態に合わせる
	else
		m_d3dpp.BackBufferFormat = format;					//指定フォーマット

	m_d3dpp.EnableAutoDepthStencil = true;				//Zバッファ使用
	m_d3dpp.AutoDepthStencilFormat  = D3DFMT_D24S8;		//バッファ深度の設定
	m_d3dpp.hDeviceWindow = NULL;						//ウインドウの指定
	m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	releaseScreen();

	pd3dDevice->CreateAdditionalSwapChain(&m_d3dpp,&m_swapChain);
	if(!m_swapChain)
		return false;
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Target
// ターゲット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Target::Target()
{
	m_target = NULL;
	m_width = 0;
	m_height = 0;
}
Target::~Target()
{
	releaseTarget();
}
bool Target::create(INT width,INT height,D3DFORMAT format)
{
	releaseTarget();

	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;

	m_width = width;
	m_height = height;

	pd3dDevice->CreateRenderTarget(width,height,format,D3DMULTISAMPLE_NONE,0,true,&m_target,NULL);
	if(m_target)
		return true;
	pd3dDevice->CreateRenderTarget(width,height,D3DFMT_X8R8G8B8,D3DMULTISAMPLE_NONE,0,true,&m_target,NULL);
	if(m_target)
		return true;
	pd3dDevice->CreateRenderTarget(width,height,D3DFMT_X1R5G5B5,D3DMULTISAMPLE_NONE,0,true,&m_target,NULL);
	if(m_target)
		return true;
	return false;
}
bool Target::releaseTarget()
{
	if(m_target)
	{
		m_target->Release();
		m_target = NULL;
	}
	return true;
}
bool Target::onDeviceLost()
{
	//ロスト時の解放処理
	if(m_target)
	{
		m_target->Release();
		m_target = NULL;
		m_lost = true;
		m_restore = false;
	}
	return true;
}
bool Target::onDeviceRestore()
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//ロスト時の復元
	pd3dDevice->CreateRenderTarget(m_width,m_height,D3DFMT_R8G8B8,D3DMULTISAMPLE_NONE,0,false,&m_target,NULL);
	if(m_target)
		return true;
	pd3dDevice->CreateRenderTarget(m_width,m_height,D3DFMT_X1R5G5B5,D3DMULTISAMPLE_NONE,0,false,&m_target,NULL);
	if(m_target)
		return true;
	return true;
}

IDirect3DSurface9* Target::getSurface() const
{
	if(m_target)
		m_target->AddRef();
	return m_target;
}
bool Target::setTarget()
{

	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;
	if(!m_target)
		return false;

	Device::setRenderTarget(m_target);
	return true;
}
HDC Target::getDC()
{
	HDC hdc = NULL;
	if(m_target)
	{
		m_target->GetDC(&hdc);
	}
	return hdc;
}
void Target::releaseDC(HDC hdc)
{
	if(m_target)
	{
		m_target->ReleaseDC(hdc);
	}
}
INT Target::getWidth() const
{
	return m_width;
}
INT Target::getHeight() const
{
	return m_height;
}
bool Target::lock(D3DLOCKED_RECT* lockRect) const
{
	if(!m_target)
		return NULL;
	return m_target->LockRect(lockRect,NULL,D3DLOCK_DISCARD) == D3D_OK;
}
bool Target::unlock() const
{
	if(m_target)
		return m_target->UnlockRect() == D3D_OK;
	return false;
}
void Target::clear(DWORD color) const
{
	if(m_target)
		Device::getInterface()->ColorFill(m_target,NULL,color);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Index
// 頂点バッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Index::Index()
{
	m_indexBuffer = NULL;;
	ZeroMemory(&m_indexDesc,sizeof(m_indexDesc));
	m_indexSize = 0;

}
Index::~Index()
{
	releaseBuffer();
}
bool Index::createBuffer(INT count,D3DPOOL d3dPool)
{
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//Direct3D9EX対策
	if(Device::getInterfaceEx() && d3dPool == D3DPOOL_MANAGED)
		d3dPool = D3DPOOL_DEFAULT;

	//動的に最適化するか選択
	DWORD usage;
	if(d3dPool == D3DPOOL_MANAGED)
		usage = 0;
	else if(d3dPool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	else
		return false;

	INT size = count * sizeof(WORD);
	//インデックスバッファの確保
	HRESULT hr;
	LPDIRECT3DINDEXBUFFER9 indexBuffer = NULL;
	hr = pd3dDevice->CreateIndexBuffer(size,usage ,D3DFMT_INDEX16,d3dPool,&indexBuffer,NULL);
	if(hr != DD_OK)
		return false;

	//バッファ情報の取得
	indexBuffer->GetDesc(&m_indexDesc);
	if(m_indexBuffer)
		m_indexBuffer->Release();
	m_indexBuffer = indexBuffer;
	
	m_lost = false;
	m_indexSize = size;
	m_indexCount = count;
	return true;

}
bool Index::createBuffer(LPVOID data,INT count,D3DPOOL d3dPool)
{
	if(!createBuffer(count,d3dPool))
		return false;
	return setData(data,count* sizeof(WORD));
}

bool Index::releaseBuffer()
{
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = NULL;
		m_lost = false;
		m_restore = false;
		m_indexSize = 0;
		return true;
	}
	return false;
}

LPVOID Index::lock()
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;

	INT size = getIndexSize();
	return lock(size);
}
LPVOID Index::lock(INT size)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	LPVOID pvoid;
	m_indexBuffer->Lock( 0, size, (LPVOID*)&pvoid, 0 );
	return pvoid;
}
void Index::unlock()
{
	m_indexBuffer->Unlock();
}
bool Index::setData(LPVOID data,INT size)
{
	LPVOID dest = lock(size);
	if(!dest)
		return false;
	CopyMemory(dest,data,size);
	unlock();
	return true;
}
bool Index::onDeviceLost()
{
	//ロスト時の解放処理
	if(getPool() == D3DPOOL_DEFAULT)
	{
		if(m_indexBuffer)
		{
			m_indexBuffer->Release();
			m_indexBuffer = NULL;
			m_lost = true;
			m_restore = false;
		}
	}
	return true;
}
bool Index::onDeviceRestore()
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//ロスト時の復元
	if(m_lost && getPool() == D3DPOOL_DEFAULT)
	{
		HRESULT hr;
		LPDIRECT3DINDEXBUFFER9 indexBuffer = NULL;
		hr = pd3dDevice->CreateIndexBuffer(m_indexDesc.Size,m_indexDesc.Usage ,m_indexDesc.Format,
			m_indexDesc.Pool,&indexBuffer,NULL);
		if(hr != DD_OK)
			return false;
		m_indexBuffer = indexBuffer;
		m_restore = true;
	}
	return true;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// 頂点バッファ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Vertex::Vertex()
{
	m_vertexBuffer = NULL;
	ZeroMemory(&m_vertexDesc,sizeof(m_vertexDesc));
	m_strideSize = 0;
	m_strideCount = 0;
	m_lost = false;
	m_restore = false;

}
Vertex::~Vertex()
{
	if(m_vertexBuffer)
		m_vertexBuffer->Release();

}
bool Vertex::createBuffer(INT strideCount,INT strideSize,DWORD fvf,D3DPOOL d3dPool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//Direct3D9EX対策
	if(Device::getInterfaceEx() && d3dPool == D3DPOOL_MANAGED)
		d3dPool = D3DPOOL_DEFAULT;

	//動的に最適化するか選択
	DWORD usage;
	if(d3dPool == D3DPOOL_MANAGED)
		usage = 0;
	else if(d3dPool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	else
		return false;

	INT size = strideSize * strideCount;
	//頂点バッファの確保
	HRESULT hr;
	LPDIRECT3DVERTEXBUFFER9 pd3dVertexBuffer = NULL;
	hr = pd3dDevice->CreateVertexBuffer(size,usage ,fvf,d3dPool,&pd3dVertexBuffer,NULL);
	if(hr != DD_OK)
		return false;

	//バッファ情報の取得
	pd3dVertexBuffer->GetDesc(&m_vertexDesc);
	if(m_vertexBuffer)
		m_vertexBuffer->Release();
	m_vertexBuffer = pd3dVertexBuffer;
	
	m_strideCount = strideCount;
	m_strideSize = strideSize;
	m_lost = false;

	return true;
}
bool Vertex::createBuffer(LPVOID data,INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool)
{
	if(!createBuffer(count,strideSize,fvf,d3dPool))
		return false;
	return setData(data,count*strideSize);
}

bool Vertex::releaseBuffer()
{
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = NULL;
		m_lost = false;
		m_restore = false;
		return true;
	}
	return false;
}

bool Vertex::onDeviceLost()
{
	//ロスト時の解放処理
	if(getPool() == D3DPOOL_DEFAULT)
	{
		if(m_vertexBuffer)
		{
			m_vertexBuffer->Release();
			m_vertexBuffer = NULL;
			m_lost = true;
			m_restore = false;
		}
	}
	return true;
}
bool Vertex::onDeviceRestore()
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	//ロスト時の復元
	if(m_lost && getPool() == D3DPOOL_DEFAULT)
	{
		HRESULT hr;
		LPDIRECT3DVERTEXBUFFER9 pd3dVertexBuffer = NULL;
		hr = pd3dDevice->CreateVertexBuffer(m_vertexDesc.Size,m_vertexDesc.Usage ,m_vertexDesc.FVF,
			m_vertexDesc.Pool,&pd3dVertexBuffer,NULL);
		if(hr != DD_OK)
			return false;
		m_vertexBuffer = pd3dVertexBuffer;
		m_restore = true;
	}
	return true;
}

LPVOID Vertex::lock(INT size)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;

	LPVOID pvoid;
	if(!size)
		size = getVertexSize();
	m_vertexBuffer->Lock( 0, size, (LPVOID*)&pvoid,0 );
	return pvoid;
}
void Vertex::unlock()
{
	m_vertexBuffer->Unlock();
}
bool Vertex::setData(LPVOID data,INT size)
{
	LPVOID dest = lock(size);
	if(!dest)
		return false;
	CopyMemory(dest,data,size);
	unlock();
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Mesh
// メッシュ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Mesh::Mesh()
{
	m_typePrimitive = D3DPT_TRIANGLELIST;
	m_shadow = false;
}
Mesh::~Mesh()
{
}
void Mesh::getVertexRange(NVector* vect) const
{
	INT i;
	Vertex* vertex = getVertexBuffer();
	if(!vertex)
		return;
	INT count = vertex->getStrideCount();
	INT size = vertex->getStrideSize();

	LPBYTE data = (LPBYTE)vertex->lock();
	for(i=0;i<count;i++)
	{
		D3DVECTOR& vectSrc = *(D3DVECTOR*)data;
		if(vectSrc.x < vect[0].x)
			vect[0].x = vectSrc.x;
		if(vectSrc.y < vect[0].y)
			vect[0].y = vectSrc.y;
		if(vectSrc.z < vect[0].z)
			vect[0].z = vectSrc.z;

		if(vectSrc.x > vect[1].x)
			vect[1].x = vectSrc.x;
		if(vectSrc.y > vect[1].y)
			vect[1].y = vectSrc.y;
		if(vectSrc.z > vect[1].z)
			vect[1].z = vectSrc.z;

		data += size;
	}
	vertex->unlock();
}
void Mesh::setDeclaration(const D3DVERTEXELEMENT9* decl)
{
	m_declaration.create(decl);
}

bool Mesh::createIndexBuffer(INT count,D3DPOOL d3dPool)
{
	Index* index = NEW Index;
	if(index->createBuffer(count,d3dPool))
	{
		m_indexBuffer = SP<Index>(index);
		return true;
	}
	delete index;
	m_indexBuffer = SP<Index>();
	return false;
}
bool Mesh::createIndexBuffer(LPVOID data,INT count,D3DPOOL d3dPool)
{
	Index* index = NEW Index;
	if(index->createBuffer(data,count,d3dPool))
	{
		m_indexBuffer = SP<Index>(index);
		return true;
	}
	delete index;
	m_indexBuffer = SP<Index>();
	return false;
}
bool Mesh::createVertexBuffer(INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool)
{
	Vertex* vertex = NEW Vertex;
	if(vertex->createBuffer(count,strideSize,fvf,d3dPool))
	{
		m_vertexBuffer = SP<Vertex>(vertex);
		return true;
	}
	delete vertex;
	m_vertexBuffer = SP<Vertex>();
	return false;
}
bool Mesh::createVertexBuffer(LPVOID data,INT count,INT strideSize,DWORD fvf,D3DPOOL d3dPool)
{
	Vertex* vertex = NEW Vertex;
	if(vertex->createBuffer(data,count,strideSize,fvf,d3dPool))
	{
		m_vertexBuffer = SP<Vertex>(vertex);
		return true;
	}
	delete vertex;
	m_vertexBuffer = SP<Vertex>();
	return false;
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
bool isOverride(NVector& point1,NVector& point2)
{
	NVector point = (point2 - point1).abs();
	if((NVector3)point < 0.0001f)
		return true;
	return false;
}

Texture* Mesh::getTexture() const
{
	if(m_texture.size())
		return m_texture.front().get();
	return NULL;
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
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// フレーム
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
	std::list<Frame>::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it).getAllFrameCount();
	}
	iCount += getFrameCount();
	return iCount;
}
INT Frame::getAllMeshCount()
{
	INT iCount = 0;
	std::list<Frame>::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it).getAllMeshCount();
	}
	iCount += getMeshCount();
	return iCount;
}



void Frame::createBoundingBox()
{

	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->createBoundingBox();
	}

	if(m_meshes.size())
	{
		NVector vect[2];
		std::list<Mesh>::iterator itMesh;
		itMesh = m_meshes.begin();
		itMesh->getVertexRange(vect);

		for(;itMesh!=m_meshes.end();++itMesh)
		{
			NVector v[2];
			itMesh->getVertexRange(v);
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
	CopyMemory(m_boundingBox,vectBox,sizeof(NVector)*8);
}

void Frame::getBoundingBox(Unit* unit,NMatrix* pTopMatrix,NMatrix* pMatrix,NVector* vect)
{
	NMatrix matrix,matAnimation;
	
	if(unit->getAnimationMatrix(getFrameName(),&matAnimation))
		matrix = matAnimation * *pMatrix;
	else
		matrix = (NMatrix)m_matrix * *pMatrix;

	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->getBoundingBox(unit,pTopMatrix,&matrix,vect);
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


void Frame::getBoundingBox(NVector* vect)
{
	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->getBoundingBox(vect);
	}

	if(m_meshes.size() == 0)
		return;

	INT i;
	for(i=0;i<8;i++)
	{
		NMatrix matrix;//キャッシュ
		NVector vectSrc = m_boundingBox[i].transformCoord(matrix);
	
		vect[0] = vect[0].minimum(vectSrc);
		vect[1] = vect[1].maximum(vectSrc);
	}

}

std::list<Frame>& Frame::getFrameChilds()
{
	return m_frameChilds;
}
std::list<Mesh>& Frame::getMeshes()
{
	return m_meshes;
}



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Texture
// テクスチャ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Texture::Texture()
{
	m_colorMask = 0;
	m_imageWidth = 0;
	m_imageHeight = 0;
	m_texture = NULL;
	m_filter = true;
}
Texture::~Texture()
{
	release();
}
bool Texture::onDeviceLost()
{
	if(m_texture && m_descTexture.Pool == D3DPOOL_DEFAULT)
	{
		setLost();
		m_texture->Release();
		m_texture = NULL;
	}
	return true;
}
bool Texture::onDeviceRestore()
{
	if(m_descTexture.Pool == D3DPOOL_DEFAULT)
	{
		bool ret = createImage(getImageWidth(),getImageHeight(),m_descTexture.Pool);
		if(!ret)
			return false;
		clearLost();
	}
	return true;
}
bool Texture::openImage(LPCSTR fileName,DWORD filter,D3DPOOL pool)
{
	return openImage(UCS2(fileName),filter,pool);
}
bool Texture::openImage(LPCWSTR fileName,DWORD filter,D3DPOOL pool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;

	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放

	//Direct3D9EX対策
	if(Device::getInterfaceEx() && pool == D3DPOOL_MANAGED)
		pool = D3DPOOL_DEFAULT;


	DWORD usage = 0;
	if(pool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC;

	//読み込みパスのカスタム設定を適用
	WString name;
	LPCWSTR path = Device::getTexturePath();
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

	Bitmap* bitmap = Bitmap::FromFile(fileName);
	if(!bitmap)
		return false;
	UINT width = bitmap->GetWidth();
	UINT height = bitmap->GetHeight();
	if(!width || !height)
	{
		delete bitmap;
		return false;
	}
	PixelFormat format = bitmap->GetPixelFormat();
	Gdiplus::BitmapData bitmapData;

	if(!createImage(width,height,pool))
	{
		delete bitmap;
		return false;
	}
	bitmap->LockBits(NULL, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
	
	D3DLOCKED_RECT rect;
	if(lock(&rect))
	{
		LPBYTE dest = (LPBYTE)rect.pBits;
		LPBYTE src = (LPBYTE)bitmapData.Scan0;
		INT i;
		for(i=0;i<(INT)height;i++)
		{
			CopyMemory(dest,src,width*4);
			dest += rect.Pitch;
			src += bitmapData.Stride;
		}
		bitmap->UnlockBits(&bitmapData);
		unlock();
	}

	delete bitmap;
	
	//テクスチャフォーマットの取得
	//m_texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = width;
	m_imageHeight = height;

	//ファイル名の保存
	m_strFileName = fileName;
	return true;
}
bool Texture::createTarget(INT width,INT height,INT format)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放
	
	static INT iSize[] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,0};
	INT i;
	INT iTextureWidth;
	INT iTextureHeight;
	for(i=0;iSize[i] && width > iSize[i];i++);
	iTextureWidth = iSize[i];
	for(i=0;iSize[i] && height > iSize[i];i++);
	iTextureHeight = iSize[i];
	DWORD usage = D3DUSAGE_RENDERTARGET;
	D3DPOOL pool = D3DPOOL_DEFAULT;

	//テクスチャの作成
	LPDIRECT3DTEXTURE9 texture = NULL;
	pd3dDevice->CreateTexture(iTextureWidth,iTextureHeight,1,usage,(D3DFORMAT)format,pool,&texture,NULL);
	if(!texture)
		return false;
	m_texture = texture;

	//テクスチャフォーマットの取得
	texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = width;
	m_imageHeight = height;
	
	//テクスチャのクリア
	if(pool == D3DPOOL_DEFAULT)
		clear(0x00000000);
	return true;
}
bool Texture::createImage(INT width,INT height,INT format,HANDLE handle)
{
	if(!Device::getInterfaceEx())
		return false;
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放
	//テクスチャの作成
	//format = D3DFMT_A8B8G8R8;
	
	LPDIRECT3DTEXTURE9 texture = NULL;
	HRESULT  hr = pd3dDevice->CreateTexture(width,height,1,D3DUSAGE_RENDERTARGET,
		(D3DFORMAT)format,D3DPOOL_DEFAULT,&texture,&handle);

	if(!texture)
		return false;
	m_texture = texture;

	//テクスチャフォーマットの取得
	texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = width;
	m_imageHeight = height;
	//テクスチャのクリア
	//clear(0x00000000);
	return true;
}
bool Texture::createImage(INT width,INT height,D3DPOOL pool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = (IDirect3DDevice9*)Device::getInterface();
	release();		//イメージの解放
	
	//Direct3D9EX対策
	if(Device::getInterfaceEx() && pool == D3DPOOL_MANAGED)
		pool = D3DPOOL_DEFAULT;

	DWORD usage = 0;
	if(pool == D3DPOOL_DEFAULT)
	{
		//動的テクスチャが利用可能か
		if(Device::getCaps()->Caps2 & D3DCAPS2_DYNAMICTEXTURES)
			usage = D3DUSAGE_DYNAMIC;
		else
			pool = D3DPOOL_MANAGED;
	}
	LPDIRECT3DTEXTURE9 texture = NULL;
	//テクスチャの作成
	HRESULT  hr = pd3dDevice->CreateTexture(width,height,1,usage,D3DFMT_A8R8G8B8,pool,&texture,NULL);
	if(!texture)
		return false;
	m_texture = texture;

	//テクスチャフォーマットの取得
	texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = width;
	m_imageHeight = height;
	
	//テクスチャのクリア
	if(pool == D3DPOOL_DEFAULT)
		clear(0x00000000);
	return true;
}
bool Texture::release()
{
	m_imageWidth = 0;
	m_imageHeight = 0;
	ZeroMemory(&m_descTexture,sizeof(D3DSURFACE_DESC));
	if(m_texture)
	{
		m_texture->Release();
		m_texture = NULL;
	}
	return true;
}
bool Texture::lock(D3DLOCKED_RECT* lockRect,DWORD flag) const
{
	if(!m_texture)
		return NULL;
	//return m_texture->LockRect(0,lockRect,NULL,0) == D3D_OK;
	return m_texture->LockRect(0,lockRect,NULL,flag) == D3D_OK;
}
bool Texture::unlock() const
{
	if(m_texture)
		return m_texture->UnlockRect(0) == D3D_OK;
	return false;
}
HDC Texture::getDC() const
{
	if(m_texture)
	{
		IDirect3DSurface9* s;
		if(m_texture->GetSurfaceLevel(0,&s) == S_OK)
		{
			HDC hdc = NULL;
			s->GetDC(&hdc);
			s->Release();
			return hdc;
		}
	}
	return NULL;
}
void Texture::releaseDC(HDC hdc) const
{
	if(m_texture)
	{
		IDirect3DSurface9* s = NULL;
		if(m_texture->GetSurfaceLevel(0,&s) == S_OK)
		{
			s->ReleaseDC(hdc);
			s->Release();
		}
	}
}
bool Texture::setTarget() const
{
	if(m_texture)
	{
		IDirect3DDevice9* pd3dDevice = Device::getInterface();
		if(!pd3dDevice)
			return false;

		IDirect3DSurface9* surface = NULL;
		m_texture->GetSurfaceLevel(0,&surface);
		if(!surface)
			return false;

		Device::setRenderTarget(surface);
	}
	return true;
}
D3DFORMAT Texture::getFormat() const
{
	return m_descTexture.Format;
}
bool  Texture::createText(LPCSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline,INT format)
{
	return createText(UCS2(pText),hFont,colText,colBack,iLimitWidth,mline,format);
}
bool  Texture::createText(LPCWSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline,INT format)
{
	SIZE size;
	WINDOWS::Font::getFontSize(&size,hFont,pText,-1,iLimitWidth,mline);
	createImage(size.cx+1,size.cy+1);
	return drawOutlineText(0,0,pText,hFont,colText,colBack,iLimitWidth,mline);}


bool Texture::createText(LPCSTR pText,INT iSize,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline,INT format)
{
	return createText(UCS2(pText),iSize,colText,colBack,iLimitWidth,mline,format);
}
bool Texture::createText(LPCWSTR pText,INT iSize,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,bool mline,INT format)
{
	WINDOWS::Font font;
	font.setSize(iSize);
	font.createFont();

	SIZE size;
	font.getFontSize(&size,pText,-1,iLimitWidth,mline);
	createImage(size.cx+1,size.cy+1);
	return drawOutlineText(0,0,pText,font,colText,colBack,iLimitWidth,mline);
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

	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
		return false;

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
				drawGlyphOutline(lockRect,iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
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
					drawGlyphOutline2(lockRect,iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
				else
				{
					drawGlyphOutline2(lockRect,iWidth+iX,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(lockRect,iWidth+iX+1,iHeight+iY,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(lockRect,iWidth+iX+2,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(lockRect,iWidth+iX+1,iHeight+iY+2,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(lockRect,iWidth+iX+1,iHeight+iY+1,colText,tm.tmAscent,&metInfo,pbyBitmap);
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
bool Texture::clear(COLORREF colText)
{
	INT i,j;
	DWORD dwColor;
	WORD wColor;

	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect,D3DLOCK_DISCARD))
		return false;
	INT iDestPitch = lockRect.Pitch;
	LPBYTE dest = (LPBYTE)lockRect.pBits;	//書き込み用ポインタ

	BYTE byAlpha = ((LPBYTE)&colText)[3];
	BYTE byRed = ((LPBYTE)&colText)[2];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[0];

	INT format = m_descTexture.Format;
	INT width = m_descTexture.Width;
	INT height = m_descTexture.Height;
	//想定フォーマットごとのα値反転
	switch(format)
	{
	case D3DFMT_X8R8G8B8:
		dwColor = (byRed << 16) + (byGreen << 8) + byBlue;
		for(j=0;j<height;j++)
		{
			for(i=0;i<width;i++)
			{
				*(LPDWORD)(dest+i*4) = dwColor;
			}
			dest += iDestPitch;
		}
		break;
	case D3DFMT_A8R8G8B8:
		dwColor = (byAlpha << 24) + (byRed << 16) + (byGreen << 8) + byBlue;
		for(j=0;j<height;j++)
		{
			for(i=0;i<width;i++)
			{
				*(LPDWORD)(dest+i*4) = dwColor;
			}
			dest += iDestPitch;
		}
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5:
		if(format == D3DFMT_R5G6B5)
			wColor = ((byRed&0xf8) << 8) + ((byGreen&0xfc) << 3) + (byBlue >> 3);
		else if(format == D3DFMT_X1R5G5B5)
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
		else
		{
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
			if(byAlpha)
				wColor |= 0x8000;
		}
		for(j=0;j<height;j++)
		{
			for(i=0;i<width;i++)
			{
				*(LPWORD)(dest+i*2) = wColor;
			}
			dest += iDestPitch;
		}
		break;
	case D3DFMT_A4R4G4B4:
		wColor = ((byAlpha&0xf0)<<8)+((byRed&0xf0) << 4) + (byGreen&0xf0) + (byBlue >> 4);
		for(j=0;j<height;j++)
		{
			for(i=0;i<width;i++)
			{
				*(LPWORD)(dest+i*2) = wColor;
			}
			dest += iDestPitch;
		}
		break;
	default:
		unlock();
		return false;
	}
	unlock();
	return true;
}
bool Texture::drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
		return false;

	bool flag = drawGlyphOutline(lockRect,iX,iY,colText,tmAscent,pmetInfo,pbyBitmap);

	unlock();
	return flag;
}
bool Texture::drawGlyphOutline(D3DLOCKED_RECT& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	INT i,j;
	DWORD dwColor;
	WORD wColor;
	iX += pmetInfo->gmptGlyphOrigin.x;
	iY += tmAscent - pmetInfo->gmptGlyphOrigin.y;
	INT iCountX = pmetInfo->gmBlackBoxX;
	INT iCountY = pmetInfo->gmBlackBoxY;
	INT iSrcPitch = (iCountX + 3) / 4 * 4;

	if(iCountX > getImageWidth() - iX)
		iCountX = getImageWidth() - iX;
	if(iCountY > getImageHeight() - iY)
		iCountY = getImageHeight() - iY;

	INT iDestPitch = lockRect.Pitch;
	LPBYTE pbyDest = (LPBYTE)lockRect.pBits;	//書き込み用ポインタ

	BYTE byAlpha = ((LPBYTE)&colText)[3];
	BYTE byRed = ((LPBYTE)&colText)[2];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[0];

	INT format = m_descTexture.Format;
	//想定フォーマットごとのα値反転
	switch(format)
	{
	case D3DFMT_X8R8G8B8:
		pbyDest += iDestPitch * iY + iX * 4;
		dwColor = (byRed << 16) + (byGreen << 8) + byBlue;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					*(LPDWORD)(pbyDest+i*4) = dwColor;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_A8R8G8B8:
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
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5:
		if(format == D3DFMT_R5G6B5)
			wColor = ((byRed&0xf8) << 8) + ((byGreen&0xfc) << 3) + (byBlue >> 3);
		else if(format == D3DFMT_X1R5G5B5)
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
		else
		{
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
			wColor |= 0x8000;
		}
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					*(LPWORD)(pbyDest+i*2) = wColor;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_A4R4G4B4:
		wColor = ((byRed&0xf0) << 4) + (byGreen&0xf0) + (byBlue >> 4);
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					byAlpha = ((byAlpha-1) << 2) & 0xf0;
					*(LPWORD)(pbyDest+i*2) = wColor;
					pbyDest[i*2+1] |= byAlpha;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	default:
		return false;
	}
	return true;
}
bool Texture::drawGlyphOutline2(D3DLOCKED_RECT& lockRect,INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	INT i,j;
	DWORD dwColor;
	WORD wColor;
	iX += pmetInfo->gmptGlyphOrigin.x;
	iY += tmAscent - pmetInfo->gmptGlyphOrigin.y;
	INT iCountX = pmetInfo->gmBlackBoxX;
	INT iCountY = pmetInfo->gmBlackBoxY;
	INT iSrcPitch = (iCountX+31)/32*4;

	if(iCountX > getImageWidth() - iX)
		iCountX = getImageWidth() - iX;
	if(iCountY > getImageHeight() - iY)
		iCountY = getImageHeight() - iY;

	INT iDestPitch = lockRect.Pitch;
	LPBYTE pbyDest = (LPBYTE)lockRect.pBits;	//書き込み用ポインタ

	BYTE byAlpha = ((LPBYTE)&colText)[3];
	BYTE byRed = ((LPBYTE)&colText)[2];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[0];
	INT format = m_descTexture.Format;
	//想定フォーマットごとのα値反転
	switch(format)
	{
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8R8G8B8:
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
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5:
		if(format == D3DFMT_R5G6B5)
			wColor = ((byRed&0xf8) << 8) + ((byGreen&0xfc) << 3) + (byBlue >> 3);
		else if(format == D3DFMT_X1R5G5B5)
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
		else
		{
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
			wColor |= 0x8000;
		}
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i/8] & (1<<(7-i%8));
				if(byAlpha)
				{
					*(LPWORD)(pbyDest+i*2) = wColor;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_A4R4G4B4:
		wColor = ((byAlpha&0xf0) << 8) + ((byRed&0xf0) << 4) + (byGreen&0xf0) + (byBlue >> 4);
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i/8] & (1<<(7-i%8));
				if(byAlpha)
				{
					*(LPWORD)(pbyDest+i*2) = wColor;
					pbyDest[i*2+1] |= byAlpha;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	default:
		return false;
	}
	return true;
}
bool Texture::drawGlyphOutline2(INT iX,INT iY,COLORREF colText,LONG tmAscent,
	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
		return false;

	bool flag = drawGlyphOutline2(lockRect,iX,iY,colText,tmAscent,pmetInfo,pbyBitmap);

	unlock();
	return flag;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Shader
// シェーダ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Shader::Shader()
{
}
Shader::~Shader()
{
	release();
}
size_t Shader::getSize() const
{
	return m_data->size();
}
LPCVOID Shader::getData() const
{
	return &(*m_data.get())[0];
}
bool Shader::isParam(LPCSTR name) const
{
	std::map<String,INT>::const_iterator it = m_constName.find(name);
	return it != m_constName.end();
}
void Shader::release()
{
	m_constName.clear();
	m_constData.clear();
}
bool Shader::create(LPCSTR cmd,LPCSTR name,LPCSTR target)
{
	release();

#ifdef _AFL_D3DX

	LPD3DXBUFFER output;
	LPD3DXBUFFER error;
	LPD3DXCONSTANTTABLE tables;

	if(D3DXCompileShader(cmd,strlen(cmd),NULL,NULL,name,"vs_2_0",0,&output,&error,&tables) != D3D_OK)
	{
		if(error)
		{
		#if _DEBUG
			OutputDebugStringA((LPCSTR)error->GetBufferPointer());
		#endif
			error->Release();
		}
		return false;
	}
	LPCBYTE data = (LPCBYTE)output->GetBufferPointer();
	m_data = new std::vector<BYTE>;
	m_data->resize(output->GetBufferSize());
	memcpy(&(*m_data.get())[0],output->GetBufferPointer(),output->GetBufferSize());



	output->Release();

	if(tables)
	{
		D3DXCONSTANTTABLE_DESC desc;
		tables->GetDesc(&desc);
		int i;
		for(i=0;i<(INT)desc.Constants;i++)
		{
			D3DXCONSTANT_DESC cdesc;
			D3DXHANDLE handle = tables->GetConstant( NULL, i );
			UINT size = 1;
			tables->GetConstantDesc(handle,&cdesc,&size);
			m_constName[cdesc.Name] = cdesc.RegisterIndex;
			m_constData[cdesc.RegisterIndex].first = cdesc.RegisterSet;
			std::vector<BYTE>& data = m_constData[cdesc.RegisterIndex].second;
			if(cdesc.DefaultValue)
			{
				data.resize(cdesc.Bytes);
				CopyMemory(&data[0],cdesc.DefaultValue,cdesc.Bytes);
			}
		}
		tables->Release();
	}
	return true;
#else
	return false;
#endif

}
bool Shader::create(LPCVOID data,size_t size,LPCSTR target)
{
	release();

	LPCBYTE src = (LPCBYTE)data;
	src += 4;
	INT count = *(LPINT)src;
	src += sizeof(INT);
	INT i;
	for(i=0;i<count;i++)
	{
		INT index = *(LPINT)src;
		src += sizeof(INT);
		INT nameSize = *(LPINT)src;
		src += sizeof(INT);

		m_constName[(LPCSTR)src] = index;
		src += nameSize;

		INT type = *(LPINT)src;
		src += sizeof(INT);

		m_constData[index].first = type;

		INT dataSize = *(LPINT)src;
		src += sizeof(INT);

		if(dataSize)
		{
			std::vector<BYTE>& data = m_constData[index].second;
			data.resize(dataSize);
			CopyMemory(&data[0],src,dataSize);
			src += dataSize;
		}
	}
	INT dataSize = *(LPINT)src;
	src += sizeof(INT);

	
	m_data = new std::vector<BYTE>;
	m_data->resize(dataSize);
	memcpy(&(*m_data.get())[0],src,dataSize);

	return true;
}

bool Shader::open(LPCWSTR fileName,LPCSTR name,LPCSTR target)
{
	File file;
	if(!file.open(fileName))
		return false;
	size_t size = file.getLength();
	LPBYTE data = NEW BYTE[size+1];
	file.read(data,size);
	data[size] = 0;

	bool flag;
	if(*(LPINT)data == *(LPINT)"HLSL")
	{
		flag = create((LPCSTR)data,size);
	}
	else
	{
		flag = create((LPCSTR)data,name);
	}
	delete[] data;
	return flag;
}
bool Shader::open(LPCSTR fileName,LPCSTR name,LPCSTR target)
{
	return open(UCS2(fileName),name);
}
bool Shader::save(LPCWSTR fileName) const
{
	File file;
	if(!file.open(fileName,L"wb"))
		return false;
	file.write("HLSL",4);

	INT count = m_constName.size();
	file.write(&count,sizeof(INT));

	std::map<String,INT>::const_iterator it;
	for(it=m_constName.begin();it!=m_constName.end();++it)
	{
		INT index = it->second;
		file.write(&index,sizeof(INT));
		INT size = it->first.length()+1;
		file.write(&size,sizeof(INT));
		file.write((LPVOID)it->first.c_str(),size);

		std::map<INT,std::pair<INT,std::vector<BYTE> > >::const_iterator it = m_constData.find(index);
		INT type = it->second.first;
		INT dataSize = it->second.second.size();
		file.write(&type,sizeof(INT));
		file.write(&dataSize,sizeof(INT));
		file.write((LPVOID)&it->second.second[0],dataSize);
	}
	INT size = m_data->size();
	file.write(&size,sizeof(INT));
	file.write((LPVOID)&(*m_data.get())[0],size);

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
}
void VertexShader::release()
{
	Shader::release();
	m_vertexShader = NULL;
}
VertexShader::operator IDirect3DVertexShader9*()
{
	if(!m_vertexShader && m_data.get())
	{
		IDirect3DVertexShader9* vertexShader = NULL;
		Device::getInterface()->CreateVertexShader((LPDWORD)(&(*m_data.get())[0]),&vertexShader);
		if(!vertexShader)
			return NULL;
		m_vertexShader = vertexShader;
	}
	return m_vertexShader;
}

void VertexShader::setParam(LPCSTR name,const FLOAT* data,INT count) const
{
	std::map<String,INT>::const_iterator it = m_constName.find(name);
	if(it != m_constName.end())
		Device::getInterface()->SetVertexShaderConstantF(it->second,data,count);
}
void VertexShader::setParam(LPCSTR name,const NMatrix* data,INT count) const
{
	std::map<String,INT>::const_iterator it = m_constName.find(name);
	if(it != m_constName.end())
	{
		INT i;
		for(i=0;i<count;i++)
		{
			NMatrix m;
			m = data[i];
			m.setTranspose();
			Device::getInterface()->SetVertexShaderConstantF(it->second+i*4,(const FLOAT*)&m,4);
		}
	}
}
void VertexShader::setDefaultParam() const
{
	std::map<INT,std::pair<INT,std::vector<BYTE> > >::const_iterator it;
	for(it = m_constData.begin();it!=m_constData.end();++it)
	{
		if(it->second.second.size())
		{
			if(it->second.first == 0)
				Device::getInterface()->SetVertexShaderConstantB(it->first,(const BOOL*)&it->second.second[0],it->second.second.size()/sizeof(BOOL));
			else if(it->second.first == 1)
				Device::getInterface()->SetVertexShaderConstantB(it->first,(const INT*)&it->second.second[0],it->second.second.size()/(sizeof(INT)*4));
			else
				Device::getInterface()->SetVertexShaderConstantF(it->first,(const FLOAT*)&it->second.second[0],it->second.second.size()/(sizeof(FLOAT)*4));
		}
	}
}
void VertexShader::setParam(LPCSTR name,const COLOR4* data,INT count) const
{
	std::map<String,INT>::const_iterator it = m_constName.find(name);
	if(it != m_constName.end())
		Device::getInterface()->SetVertexShaderConstantF(it->second,(const FLOAT*)data,count);
}
void VertexShader::setParam(LPCSTR name,const NVector* data,INT count) const
{
	std::map<String,INT>::const_iterator it = m_constName.find(name);
	if(it != m_constName.end())
		Device::getInterface()->SetVertexShaderConstantF(it->second,(const FLOAT*)data,count);
}

}}
