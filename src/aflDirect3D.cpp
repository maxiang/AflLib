#include <windows.h>

#include "aflDirect3DUnit.h"

#pragma comment(lib, "d3dx9.lib")


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
	m_pd3dStatus = NULL;

	//Direct3DCreate8をDLLから参照
	//COM使えず、またLoadLibraryの時代に逆戻り
	m_hLibrary = LoadLibraryW(L"d3d9.dll");
	if(!m_hLibrary)
		return;

	//DLLから関数のアドレスを取得
	IDirect3D9* (WINAPI *pd3dCreate9)(UINT) = 
		(IDirect3D9* (WINAPI *)(UINT))GetProcAddress(m_hLibrary,"Direct3DCreate9");
	if(pd3dCreate9)	//取得成功なら
		m_pd3d = pd3dCreate9(D3D_SDK_VERSION);


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

//-----------------------------------------------
//bool InterfaceDevice::isDirectX() const
// ---  動作  ---
// DirectGraphicsの存在チェック
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
	m_hWnd = hWnd;
	//デフォルトのアダプタの状態を取得
	D3DDISPLAYMODE d3dDisplayMode;
	getInterface()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3dDisplayMode );

	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp));				//構造体の初期化
	//サイズの設定
	if(deviceWidth)
		m_d3dpp.BackBufferWidth = deviceWidth;
	if(deviceHeight)
		m_d3dpp.BackBufferHeight = deviceHeight;
	
	m_screenFull = screenFull;

	m_d3dpp.Windowed = !m_screenFull;					//ウインドウ／フルスクリーン切り替えフラグ
	//m_d3dpp.BackBufferWidth = m_deviceWidth;			//デバイスサイズ幅
	//m_d3dpp.BackBufferHeight= m_deviceHeight;			//デバイスサイズ高さ
	m_d3dpp.BackBufferCount = 1;						//バックバッファの数
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;			//コピーモード
	m_d3dpp.BackBufferFormat = d3dDisplayMode.Format;	//カラーモードを現在の状態に合わせる
	m_d3dpp.EnableAutoDepthStencil = true;				//Zバッファ使用
	m_d3dpp.AutoDepthStencilFormat  = D3DFMT_D24S8;		//バッファ深度の設定
	m_d3dpp.hDeviceWindow = hWnd;						//ウインドウの指定

	if(videoSync)	//リフレッシュレートと同期に設定
		m_d3dpp.PresentationInterval = 0;
	else
		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//デバイスが既に存在していたら解放
	if(m_pd3dDevice)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	D3DCAPS9 caps;
	m_pd3d->GetDeviceCaps(0,D3DDEVTYPE_HAL,&caps);


	//デバイスの作成

	if(caps.VertexShaderVersion>=D3DVS_VERSION(1,1))
	{
		m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
                              D3DCREATE_MULTITHREADED|D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE,
                              &m_d3dpp, &m_pd3dDevice );
	}
	else
	{
		m_pd3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL , hWnd,
							D3DCREATE_MULTITHREADED|D3DCREATE_SOFTWARE_VERTEXPROCESSING,
							&m_d3dpp, &m_pd3dDevice );
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
D3D_DEVICESTAT Device::getStat()
{
	return m_interfaceDevice.getStat();
}
IDirect3DDevice9* Device::getInterface()
{
	return m_interfaceDevice.getDeviceInterface();
}
D3DCAPS9 const* Device::getCaps()
{
	return m_interfaceDevice.getCaps();
}
UINT Device::getTargetWidth()
{
	if(getInterface())
	{
		IDirect3DSurface9* backBuffer;
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
		IDirect3DSurface9* backBuffer;
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

Shader* Device::getShader()
{
	return &m_shader;
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


Shader::Shader()
{
	m_defaultEffect = NULL;
	m_defaultHandle = NULL;
}
Shader::~Shader()
{
	release();
}
bool Shader::setMaterial(Material* material,bool shadow)
{
	m_defaultEffect = material->getEffect();
	if(shadow)
		m_defaultHandle = material->getTechniqueShadow();
	else
		m_defaultHandle = material->getTechnique();
	m_defaultEffect->SetTechnique(m_defaultHandle);
	return true;
}
bool Shader::loadMemory(const DWORD* data,size_t size,LPCSTR shaderName)
{
	ID3DXEffect* effect;
	if(D3DXCreateEffect(Device::getInterface(),data,(INT)size,NULL,NULL,0,NULL,&effect,NULL) != D3D_OK)
		return false;
	m_shader[shaderName] = effect;
	
	D3DXHANDLE handle = NULL;
	for(handle=NULL;effect->FindNextValidTechnique(handle,&handle)==D3D_OK;)
	{
		D3DXTECHNIQUE_DESC desc;
		effect->GetTechniqueDesc(handle,&desc);
		m_shaderHandle[std::pair<std::string,std::string>(shaderName,desc.Name)] = handle;
	}
	return true;
}

bool Shader::load(LPCSTR fileName,LPCSTR shaderName)
{
	ID3DXEffect* effect;
	if(D3DXCreateEffectFromFileA(Device::getInterface(),fileName,NULL,NULL,0,NULL,&effect,NULL) != D3D_OK)
		return false;
	m_shader[shaderName] = effect;
	
	D3DXHANDLE handle = NULL;
	for(handle=NULL;effect->FindNextValidTechnique(handle,&handle)==D3D_OK;)
	{
		D3DXTECHNIQUE_DESC desc;
		effect->GetTechniqueDesc(handle,&desc);
		m_shaderHandle[std::pair<std::string,std::string>(shaderName,desc.Name)] = handle;
	}
	return true;
}
bool Shader::release(LPCSTR shaderName)
{
	if(shaderName)
	{
		std::map<std::string,ID3DXEffect*>::iterator it;
		it = m_shader.find(shaderName);
		if(it == m_shader.end())
			return false;
		it->second->Release();
	}
	else
	{
		std::map<std::string,ID3DXEffect*>::iterator it;
		for(it=m_shader.begin();it!=m_shader.end();++it)
		{
			it->second->Release();
		}
	}
	return true;
}
bool Shader::setParam(LPCSTR paramName,D3DVECTOR* value)
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->SetValue( paramName, value,sizeof(D3DVECTOR));
	return true;
}

bool Shader::setParam(LPCSTR paramName,D3DCOLORVALUE* value)
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->SetVector( paramName, (D3DXVECTOR4*)value);
	return true;
}

bool Shader::setParam(LPCSTR paramName,D3DMATRIX* matrix,INT count)
{
	if(m_defaultEffect)
	{
		if(count>0)
			m_defaultEffect->SetMatrixArray( paramName, (D3DXMATRIX*)matrix,count);
		else
			m_defaultEffect->SetMatrix( paramName, (D3DXMATRIX*)matrix);
		return true;
	}
	return false;
}
bool Shader::setParam(LPCSTR paramName,const PFLOAT value,INT count)
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->SetFloatArray ( paramName, value,count);
	return true;
}
bool Shader::setParam(LPCSTR paramName,INT value)
{
	if(m_defaultEffect)
	{
		m_defaultEffect->SetInt( paramName, value);
		return true;
	}
	return false;
}
bool Shader::setEffect(LPCSTR tecName,LPCSTR shaderName)
{
	std::map<std::pair<std::string,std::string>,D3DXHANDLE >::iterator itHandle = m_shaderHandle.begin();
	if(itHandle == m_shaderHandle.end())
		return false;
	m_defaultEffect = m_shader[itHandle->first.first];
	m_defaultHandle = itHandle->second;
	return true;
}
bool Shader::begin(LPUINT pass)
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->Begin( pass, D3DXFX_DONOTSAVESTATE);
	return true;
}
bool Shader::beginPass(UINT pass)
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->BeginPass(pass);
	return true;
}
bool Shader::endPass()
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->EndPass();
	return true;
}
bool Shader::end()
{
	if(!m_defaultEffect)
		return false;
	m_defaultEffect->End();
	return true;
}

void Shader::onDeviceLost()
{
	std::map<std::string,ID3DXEffect*>::iterator it;
	for(it=m_shader.begin();it != m_shader.end();++it)
	{
		it->second->OnLostDevice();
	}
}
void Shader::onDeviceReset()
{
	std::map<std::string,ID3DXEffect*>::iterator it;
	for(it=m_shader.begin();it != m_shader.end();++it)
	{
		it->second->OnResetDevice();
	}
}
bool Shader::isTechnique(LPCSTR tecName,LPCSTR effectName)
{
	return	m_shaderHandle.find(std::pair<std::string,std::string>(tecName,effectName)) != m_shaderHandle.end();
}
bool Shader::isEffect(LPCSTR effectName)
{
	return getEffect(effectName) != NULL;
}
ID3DXEffect* Shader::getEffect(LPCSTR effectName)
{
	std::map<std::string,ID3DXEffect*>::iterator it = m_shader.find(effectName);
	if(it == m_shader.end())
		return NULL;
	return it->second;
}
D3DXHANDLE Shader::getTechnique(LPCSTR tecName,LPCSTR effectName)
{
	std::map<std::pair<std::string,std::string>,D3DXHANDLE >::iterator it =
		m_shaderHandle.find(std::pair<std::string,std::string>(effectName,tecName));
	if(it == m_shaderHandle.end())
		return NULL;
	return it->second;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device
// DirectX - Direct3Dデバイス操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
InterfaceDevice Device::m_interfaceDevice;
AFL::DirectDraw::Screen Device::m_screen;

Shader  Device::m_shader;
bool Device::setClipper(FLOAT x1,FLOAT y1,FLOAT z1,FLOAT x2,FLOAT y2,FLOAT z2,FLOAT x3,FLOAT y3,FLOAT z3)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	DWORD dwClip = D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2 | D3DCLIPPLANE3;
	FLOAT x01,y01,z01;
	FLOAT x02,y02,z02;
	D3DXVECTOR3 vect1;
	FLOAT length,length2;
	//X
	vect1.x = x1-x2;
	vect1.y = y1-y2;
	vect1.z = z1-z2;
	D3DXVec3Normalize(&vect1,&vect1);
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
	D3DXVec3Normalize(&vect1,&vect1);
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
	getShader()->onDeviceLost();

	bool reset = m_interfaceDevice.resetDevice();
	if(reset)
	{
		std::list<Object*>::iterator it;
		for(it=m_interfaceDevice.m_listObject.begin();it!=m_interfaceDevice.m_listObject.end();++it)
		{
			(*it)->onDeviceRestore();
		}
		getShader()->onDeviceReset();
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
	D3DXVec3Normalize( (D3DXVECTOR3*)&Direction, &D3DXVECTOR3(0.4f,-0.4f,-0.5f));
	//D3DXVec3Normalize( (D3DXVECTOR3*)&Direction, &D3DXVECTOR3(0.0f,-1.0f,0.0f));
	//D3DXVec3Normalize( (D3DXVECTOR3*)&Direction, &D3DXVECTOR3(0.1f,0.5f,-0.5f));

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
			D3DXMatrixPerspectiveFovRH( &m_matProjection,fFovy, fAspect,m_near,m_far );
		else
			D3DXMatrixOrthoRH(&m_matProjection,m_width,m_width/fAspect,m_near,m_far);
		//m_depth = fDepth;
		m_deviceWidth = uDeviceWidth;
		m_deviceHeight = uDeviceHeight;
	}
	m_matView = m_matExt * m_matViewBase;
	//m_matView = m_matBase3D * m_matExt;
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Camera3D
// 3Dカメラ系
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Camera3D::Camera3D()
{
	m_depth = 0.0f;
	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
	m_rotX = 0.0f;
	m_rotY = 0.0f;
	m_rotZ = 0.0f;
	m_deviceWidth = 0;
	m_deviceHeight = 0;

}
void Camera3D::resetPoint()
{
	setAngle(Device::getTargetWidth(),Device::getTargetHeight());
	//m_x = (FLOAT)Device::getWidth() * 0.5f;
	m_y = m_depth;
	//m_z = -(FLOAT)Device::getHeight() * 0.5f;

}

void Camera3D::getVector(D3DVECTOR* pVect,FLOAT fX,FLOAT fY)
{
	D3DXMATRIX mat;
	D3DXMatrixInverse(&mat,NULL,&m_matView);
	D3DXVECTOR3 vect(fX,fY,getBaseDepth());
	D3DXVec3TransformNormal(&vect,&vect,&mat);
	D3DXVec3Normalize((D3DXVECTOR3*)pVect,&vect);
}


bool Camera3D::setAngle(UINT uDeviceWidth,UINT uDeviceHeight)
{	
	D3DXMATRIX matRotation;
	*getExt() = *D3DXMatrixTranslation(&matRotation,-m_x,-m_y,-m_z);
	*getExt() *= *D3DXMatrixRotationZ(&matRotation,D3DX_PI * -m_rotZ / 180.0f);
	*getExt() *= *D3DXMatrixRotationY(&matRotation,D3DX_PI * -m_rotY / 180.0f);
	*getExt() *= *D3DXMatrixRotationX(&matRotation,D3DX_PI * -m_rotX / 180.0f);
	
	if(m_deviceWidth != uDeviceWidth || m_deviceHeight != uDeviceHeight)
	{
		//視野角度
		FLOAT fViewAngle = 45;
		//デバイスサイズの半分を各頂点に
		FLOAT fWidth = (FLOAT)uDeviceWidth/2;
		FLOAT fHeight = (FLOAT)uDeviceHeight/2;
		//アスペクト比(高さを1としたときの幅)
		FLOAT fAspect = (FLOAT)uDeviceWidth/uDeviceHeight;
		//視野をZ=0でデバイスの幅と高さに合わせる
		FLOAT fFovy = fViewAngle*D3DX_PI/180.0f;				
		//奥行き
		FLOAT fDepth = (FLOAT)fHeight/(FLOAT)tan(fFovy/2.0f);					

		//ビューの設定
		D3DXMatrixLookAtLH( &m_matBase3D, 
			&D3DXVECTOR3( 0.0f, 0.0f,0),
			&D3DXVECTOR3( 0.0f, -fDepth,0),
			&D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) );

		D3DXMATRIX matrix;
		D3DXMatrixLookAtLH( &m_matViewBase, 
			&D3DXVECTOR3( fWidth,fHeight,fDepth),
			&D3DXVECTOR3( fWidth,fHeight, 0.0f),
			&D3DXVECTOR3( 0.0f, -1.0f, 0.0f ) );

		//奥行きに対する比率を調整
		D3DXMatrixPerspectiveFovLH( &m_matProjection,fFovy, fAspect,50.0f,20000.0f );

		m_depth = fDepth;
		m_deviceWidth = uDeviceWidth;
		m_deviceHeight = uDeviceHeight;
	}
	m_matView = m_matExt * m_matBase3D;
	//m_matView = m_matBase3D * m_matExt;
	return true;
}
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
bool Screen::present(HWND hWnd)
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
	if(m_swapChain)
	{
		//描画済みサーフェイスを表示
		RECT rect1 = {0,0,m_width,m_height};
		m_swapChain->Present( &rect1, &rect1, hWnd, NULL,D3DPRESENT_DONOTWAIT);
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

	//動的に最適化するか選択
	DWORD usage;
	if(d3dPool == D3DPOOL_MANAGED)
		usage = 0;
	else if(d3dPool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY ;
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
}
Mesh::~Mesh()
{
}
void Mesh::getVertexRange(D3DVECTOR* vect) const
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
bool isOverride(D3DXVECTOR3& point1,D3DXVECTOR3& point2)
{
	D3DXVECTOR3 point = point2 - point1;
	if(abs(point.x) < 0.0001f && abs(point.y) < 0.0001f && abs(point.z) < 0.0001f)
		return true;
	return false;
}
bool Mesh::createShadow(Mesh& shadow) const
{
	INT i,j,k,l;
	if(!getIndexBuffer())
		return false;
	const INT indexCount = getIndexBuffer()->getCount();
	if(!indexCount)
		return false;
	const INT strideSize = getVertexBuffer()->getStrideSize();
	const LPBYTE vb = (const LPBYTE)getVertexBuffer()->lock();
	LPWORD indexBuffer = NEW WORD[indexCount];
	CopyMemory(indexBuffer,(const LPWORD)getIndexBuffer()->lock(),indexCount*sizeof(WORD));
	const LPWORD ib = indexBuffer;

	std::list<WORD> shadowIndex;
	INT c = 0;
	for(j=0;j<indexCount - 3;j+=3)
	{
		D3DXVECTOR3 point1[3] =
		{
			*(D3DXVECTOR3*)(vb+strideSize*ib[j+0]),
			*(D3DXVECTOR3*)(vb+strideSize*ib[j+1]),
			*(D3DXVECTOR3*)(vb+strideSize*ib[j+2])
		};
		for(i=j+3;i<indexCount;i+=3)
		{
			D3DXVECTOR3 point2[3] =
			{
				*(D3DXVECTOR3*)(vb+strideSize*ib[i+0]),
				*(D3DXVECTOR3*)(vb+strideSize*ib[i+1]),
				*(D3DXVECTOR3*)(vb+strideSize*ib[i+2])
			};
			bool flag[3][3];
			for(k=0;k<3;k++)
			{
				for(l=0;l<3;l++)
				{
					flag[k][l] = isOverride(point1[k],point2[l]);
				}
			}
			static INT index[] = {0,1,1,2,2,0};
			for(k=0;k<6;k+=2)
			{
				for(l=0;l<6;l+=2)
				{
					if(flag[index[k]][index[l]] && flag[index[k+1]][index[l+1]])
					{
						shadowIndex.push_back(ib[j+index[k]]);
						shadowIndex.push_back(ib[j+index[k+1]]);
						shadowIndex.push_back(ib[i+index[l]]);

					}
					else if(flag[index[k]][index[l+1]] && flag[index[k+1]][index[l]])
					{
						shadowIndex.push_back(ib[j+index[k]]);
						shadowIndex.push_back(ib[j+index[k+1]]);
						shadowIndex.push_back(ib[i+index[l]]);

						shadowIndex.push_back(ib[i+index[l+1]]);
						shadowIndex.push_back(ib[i+index[l]]);
						shadowIndex.push_back(ib[j+index[k+1]]);
						break;
						//
						//shadowIndex.push_back(ib[j+index[k]]);
						//shadowIndex.push_back(ib[i+index[l]]);
						//shadowIndex.push_back(ib[j+index[k+1]]);

						//shadowIndex.push_back(ib[i+index[l+1]]);
						//shadowIndex.push_back(ib[j+index[k+1]]);
						//shadowIndex.push_back(ib[i+index[l]]);
						//shadowIndex.push_back(i+index[l+1]);
						
						//shadowIndex.push_back(j+index[k+1]);
						//shadowIndex.push_back(j+index[l]);
						//shadowIndex.push_back(i+index[l+1]);
					}
				}
			}
		}
	}


	getIndexBuffer()->unlock();
	getVertexBuffer()->unlock();

	INT shadowCount = (INT)shadowIndex.size();
	if(shadowCount)
	{
		getIndexBuffer()->createBuffer(shadowCount+indexCount);
		LPWORD destBuffer = (LPWORD)getIndexBuffer()->lock();
		CopyMemory(destBuffer,indexBuffer,sizeof(WORD)*indexCount);
		//ZeroMemory(destBuffer,sizeof(WORD)*indexCount);

		std::list<WORD>::iterator it;
		for(i=0,it=shadowIndex.begin();it!=shadowIndex.end();++it,i++)
		{
			destBuffer[indexCount + i] = *it;
		}
		getIndexBuffer()->unlock();
	}

	delete[] indexBuffer;
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// フレーム
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Frame::Frame()
{
	D3DXMatrixIdentity((D3DXMATRIX*)&m_matrix);
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
	m_meshs.push_back(*mesh);
}
void Frame::addShadow(Mesh* mesh)
{
	m_meshsShadow.push_back(*mesh);
}
void Frame::setMatrix(D3DMATRIX* matrix)
{
	m_matrix = *matrix;
}
INT Frame::getFrameCount()
{
	return (INT)m_frameChilds.size();
}
INT Frame::getMeshCount()
{
	return (INT)m_meshs.size();
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

bool Frame::createShadow()
{
	bool flag = false;
	size_t count = m_meshs.size();
	if(count)
	{
		m_meshsShadow.clear();
		m_meshsShadow.resize(m_meshs.size());

		std::list<Mesh>::iterator itMesh;
		std::list<Mesh>::iterator itMeshShadow;
		for(itMesh=m_meshs.begin(),itMeshShadow=m_meshsShadow.begin();
			itMesh!=m_meshs.end();++itMesh,++itMeshShadow)
		{
			itMesh->createShadow(*itMeshShadow);
		}
		flag = true;
	}
	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		flag |= itFrame->createShadow();
	}

	return flag;
}
void Frame::createBoundingBox()
{

	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->createBoundingBox();
	}

	D3DVECTOR vect[2] = {10000,10000,10000,-10000,-10000,-10000};
	bool bFlag = false;


	std::list<Mesh>::iterator itMesh;
	foreach(itMesh,m_meshs)
	{
		itMesh->getVertexRange(vect);
		bFlag = true;
	}

	if(bFlag)
	{
		m_bounding[0] = vect[0];
		m_bounding[1] = vect[1];
	}
	else
	{
		ZeroMemory(m_bounding,sizeof(m_bounding));
	}

	D3DVECTOR vectBox[8] = 
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
	CopyMemory(m_boundingBox,vectBox,sizeof(D3DVECTOR)*8);
}

void Frame::getBoundingBox(Unit* unit,D3DMATRIX* pTopMatrix,D3DMATRIX* pMatrix,D3DVECTOR* vect)
{
	D3DXMATRIX matrix,matAnimation;
	
	if(unit->getAnimationMatrix(getFrameName(),&matAnimation))
		matrix = matAnimation * *pMatrix;
	else
		matrix = (D3DXMATRIX)m_matrix * *pMatrix;

	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->getBoundingBox(unit,pTopMatrix,&matrix,vect);
	}

	if(m_meshs.size() == 0)
		return;

	m_matrixCache = matrix * *pTopMatrix;

	INT i;
	for(i=0;i<8;i++)
	{
		D3DVECTOR vectSrc;
		D3DXVec3TransformCoord((D3DXVECTOR3*)&vectSrc,(D3DXVECTOR3*)&m_boundingBox[i],
			(D3DXMATRIX*)&m_matrixCache);
	
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
	}

}


void Frame::getBoundingBox(D3DVECTOR* vect)
{
	std::list<Frame>::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		itFrame->getBoundingBox(vect);
	}

	if(m_meshs.size() == 0)
		return;

	INT i;
	for(i=0;i<8;i++)
	{
		D3DVECTOR vectSrc;
		D3DXVec3TransformCoord((D3DXVECTOR3*)&vectSrc,(D3DXVECTOR3*)&m_boundingBox[i],
			(D3DXMATRIX*)&m_matrixCache);
	
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
	}

}
void Frame::setShader(bool flag)
{
	std::list<Frame>::iterator itFrame;
	foreach(itFrame,m_frameChilds)
	{
		itFrame->setShader(flag);
	}
	std::list<Mesh>::iterator itMesh;
	foreach(itMesh,m_meshs)
	{
		itMesh->setShader(false);
	}
}
D3DMATRIX* Frame::getMatrix()
{
	return &m_matrix;
}
std::list<Frame>* Frame::getFrameChilds()
{
	return &m_frameChilds;
}
std::list<Mesh>* Frame::getMeshs()
{
	return &m_meshs;
}
std::list<Mesh>* Frame::getShadows()
{
	return &m_meshsShadow;
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
		bool ret = createImage(getImageWidth(),getImageHeight(),
			m_descTexture.Format,m_descTexture.Pool);
		if(!ret)
			return false;
		clearLost();
	}
	return true;
}
bool Texture::openImage(LPCSTR fileName,INT format,DWORD filter,D3DPOOL pool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放

	DWORD usage = 0;
	if(pool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC;

	LPDIRECT3DTEXTURE9 texture = NULL;
	D3DXIMAGE_INFO infoImage;
	HRESULT hr = D3DXCreateTextureFromFileExA(pd3dDevice,fileName,
						 D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,usage,
						 (D3DFORMAT)format,pool,filter,1,
						 m_colorMask,&infoImage,NULL,&texture);

	if(!texture)
		return false;
    m_texture = texture;
    
	//テクスチャフォーマットの取得
	texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = infoImage.Width;
	m_imageHeight = infoImage.Height;

	//ファイル名の保存
	m_strFileName = fileName;
	return true;
}
bool Texture::openImage(LPCWSTR fileName,INT format,DWORD filter,D3DPOOL pool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放

	DWORD usage = 0;
	if(pool == D3DPOOL_DEFAULT)
		usage = D3DUSAGE_DYNAMIC;

	LPDIRECT3DTEXTURE9 texture = NULL;
	D3DXIMAGE_INFO infoImage;
	HRESULT hr = D3DXCreateTextureFromFileExW(pd3dDevice,fileName,
						 D3DX_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,usage,
						 (D3DFORMAT)format,pool,filter,1,
						 m_colorMask,&infoImage,NULL,&texture);

	if(!texture)
		return false;
    m_texture = texture;
    
	//テクスチャフォーマットの取得
	texture->GetLevelDesc(0,&m_descTexture);

	//イメージサイズの取得
	m_imageWidth = infoImage.Width;
	m_imageHeight = infoImage.Height;

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
	D3DXCreateTexture(pd3dDevice,iTextureWidth,iTextureHeight,1,usage,(D3DFORMAT)format,pool,&texture);
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
bool Texture::createImage(INT width,INT height,INT format,D3DPOOL pool)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return NULL;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	release();		//イメージの解放
	

	DWORD usage = 0;
	if(pool == D3DPOOL_DEFAULT)
	{
		//動的テクスチャが利用可能か
		if(Device::getCaps()->Caps2 & D3DCAPS2_DYNAMICTEXTURES)
			usage = D3DUSAGE_DYNAMIC;
		else
			pool = D3DPOOL_MANAGED;
	}
	//テクスチャの作成
	LPDIRECT3DTEXTURE9 texture = NULL;
	D3DXCreateTexture(pd3dDevice,width,height,1,usage,(D3DFORMAT)format,pool,&texture);
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
bool Texture::lock(D3DLOCKED_RECT* lockRect) const
{
	if(!m_texture)
		return NULL;
	return m_texture->LockRect(0,lockRect,NULL,D3DLOCK_DISCARD) == D3D_OK;
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
	createImage(size.cx+1,size.cy+1,format);
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
				drawGlyphOutline(iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
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
				//drawGlyphOutline(iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
				if(colBack == -2)
					drawGlyphOutline2(iWidth+iX,iHeight+iY,colText,tm.tmAscent,&metInfo,pbyBitmap);
				else
				{
					drawGlyphOutline2(iWidth+iX,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(iWidth+iX+1,iHeight+iY,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(iWidth+iX+2,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(iWidth+iX+1,iHeight+iY+2,colBack,tm.tmAscent,&metInfo,pbyBitmap);
					drawGlyphOutline2(iWidth+iX+1,iHeight+iY+1,colText,tm.tmAscent,&metInfo,pbyBitmap);
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
	return true;
}
bool Texture::clear(COLORREF colText)
{
	INT i,j;
	DWORD dwColor;
	WORD wColor;

	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
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

	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
		return false;
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
		unlock();
		return false;
	}
	unlock();
	return true;
}
bool Texture::drawGlyphOutline2(INT iX,INT iY,COLORREF colText,LONG tmAscent,
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

	D3DLOCKED_RECT lockRect;
	if(!lock(&lockRect))
		return false;
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
		unlock();
		return false;
	}
	unlock();
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Material
// マテリアル
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Material::Material()
{
	m_effect = NULL;
	m_tecHandle = NULL;
	m_normalAuto = false;

	m_matView = NULL;
	m_matProj = NULL;
	m_matViewProj = NULL;
	m_matWorld = NULL;
	m_matWorldView = NULL;
	m_matWorldViewProj = NULL;

}
void Material::setTexture(SP<Texture>& texture)
{
	m_texture = texture;
}
Texture* Material::getTexture() const
{
	return m_texture.get();
}
bool Material::isShader() const
{
	return m_effect && m_tecHandle;
}
bool Material::setShader(LPCSTR technique,LPCSTR effect)
{
	m_effect = Device::getShader()->getEffect(effect);
	if(!m_effect)
		return false;
	m_tecHandle = Device::getShader()->getTechnique(technique,effect);

	m_matView = m_effect->GetParameterByName(NULL,"View");
	m_matProj = m_effect->GetParameterByName(NULL,"Proj");
	m_matWorld = m_effect->GetParameterByName(NULL,"World");
	m_matViewProj = m_effect->GetParameterByName(NULL,"ViewProj");
	m_matWorldView = m_effect->GetParameterByName(NULL,"WorldView");
	m_matWorldViewProj = m_effect->GetParameterByName(NULL,"WorldViewProj");
	return m_effect && m_tecHandle;
}
bool Material::setShaderShadow(LPCSTR technique,LPCSTR effect)
{
	m_effect = Device::getShader()->getEffect(effect);
	m_tecShadow = Device::getShader()->getTechnique(technique,effect);
	return m_effect && m_tecHandle;
}
ID3DXEffect* Material::getEffect() const
{
	return m_effect;
}
D3DXHANDLE Material::getTechnique() const
{
	return m_tecHandle;
}
D3DXHANDLE Material::getTechniqueShadow() const
{
	return m_tecShadow;
}

D3DXHANDLE Material::getView() const
{
	return m_matView;
}
D3DXHANDLE Material::getProj() const
{
	return m_matProj;
}
D3DXHANDLE Material::getViewProj() const
{
	return m_matViewProj;
}
D3DXHANDLE Material::getWorld() const
{
	return m_matWorld;
}
D3DXHANDLE Material::getWorldView() const
{
	return m_matWorldView;
}
D3DXHANDLE Material::getWorldViewProj() const
{
	return m_matWorldViewProj;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Animation
// コンストラクタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Animation::Animation()
{
	m_animationTime = 0;
	m_animationTimeStart = 0;
	m_animationTimeLength = 0;
	m_animationTimeChange = 0;
	m_animationTimeChangeWork = 0;
	m_loop = true;
}


//-----------------------------------------------
// ---  動作  ---
// アニメーションのキーを取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
INT Animation::getAnimationKey(LPCSTR pFrameName,D3DMATRIX* matrix,D3DVECTOR* position,D3DVECTOR* scale,D3DXQUATERNION* rotation)
{
	std::map<std::string,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return 0;
	DWORD dwAnimationCount = m_animationTime;
	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationKey(dwAnimationCount,matrix,position,scale,rotation,m_loop) + 1;
}
INT Animation::getAnimationKey2(LPCSTR pFrameName,D3DMATRIX* matrix,D3DVECTOR* position,D3DVECTOR* scale,D3DXQUATERNION* rotation)
{
	std::map<std::string,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return 0;
	DWORD dwAnimationCount = m_animationTime;

	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationKey2(dwAnimationCount,matrix,position,scale,rotation,m_loop) + 1;
}
//-----------------------------------------------
// ---  動作  ---
// アニメーションの行列を取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
bool Animation::getAnimationMatrix(LPCSTR pFrameName,D3DMATRIX* pMatrix)
{
	std::map<std::string,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return false;

	DWORD dwAnimationCount = m_animationTime;
	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationMatrix(dwAnimationCount,pMatrix,m_loop);
}

//-----------------------------------------------
// ---  動作  ---
// フレームのキーデータを取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
AnimationKey* Animation::getAnimationKey(LPCSTR pFrameName)
{
	std::map<std::string,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return NULL;
	return &(*it).second;

}

//-----------------------------------------------
// ---  動作  ---
// フレームのキーの数を取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
INT Animation::getAnimationCount() const
{
	return (INT)m_animationKey.size();
}

//-----------------------------------------------
// ---  動作  ---
// アニメーションリストからまとめて転送
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
void Animation::setAnimation(std::string name,std::map<std::string,AnimationKey>& animationKey)
{
	m_name = name;
	m_animationKey = animationKey;
	m_animationTimeLast = 0;
	std::map<std::string,AnimationKey>::const_iterator it;
	for(it=m_animationKey.begin();it!=m_animationKey.end();++it)
	{
		DWORD last = it->second.getAllCount();
		if(last > m_animationTimeLast)
			m_animationTimeLast = last;
	}
}
void Animation::setTimeCount(DWORD dwCount)
{
	m_animationTime = dwCount;
}
void Animation::setTimeStart(DWORD dwCount)
{
	m_animationTimeStart = dwCount;
}
void Animation::setTimeLength(DWORD dwCount)
{
	m_animationTimeLength = dwCount;
}
DWORD Animation::getTimeCount() const
{
	return m_animationTime;
}
DWORD Animation::getTimeStart() const
{
	return m_animationTimeStart;
}
DWORD Animation::getTimeLength() const
{
	return m_animationTimeLength;
}
void Animation::setTimeChange(DWORD count)
{
	m_animationTimeChange = count;
}
DWORD Animation::getTimeChange() const
{
	return m_animationTimeChange;
}
void Animation::setTimeChangeWork(DWORD count)
{
	m_animationTimeChangeWork = count;
}
DWORD Animation::getTimeChangeWork() const
{
	return m_animationTimeChangeWork;
}
LPCSTR Animation::getName() const
{
	return m_name.c_str();
}
void Animation::setLoop(bool loop)
{
	m_loop = loop;
}
bool Animation::isAnimation() const
{
	if(m_loop)
		return true;
	if(m_animationTime >=  (DWORD)getLastTime())
		return false;
	return true;
}
DWORD Animation::getLastTime() const
{
	return m_animationTimeLast;
}





}}
