#include <windows.h>
#include <tchar.h>

#include "aflDirectMedia.h"

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
// UnitVideo
// DirectX - 動画再生用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitVideo::UnitVideo()
{
	m_dataPtr = NULL;
	m_dataSize = 0;
	m_initImage = false;
}
UnitVideo::~UnitVideo()
{
	releaseBuffer();
}
void UnitVideo::releaseBuffer()
{
	stop();
	m_critical.lock();
	if(m_dataPtr)
	{
		delete[] m_dataPtr;
		m_dataSize = 0;
		m_dataPtr = NULL;
	}
	m_critical.unlock();
}
void UnitVideo::onImageInit()
{
	m_critical.lock();
	INT width = MediaSampler::getImageWidth();
	INT height = MediaSampler::getImageHeight();

	INT size = (width * 24/8 + 3)/4*4 * height;
	if(m_dataSize != size)
	{
		delete[] m_dataPtr;
		m_dataSize = size;
		m_dataPtr = new BYTE[size];
	}
	m_initImage = true;
	m_critical.unlock();
}
bool UnitVideo::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag() && m_dataPtr && getStat() == MEDIA_PLAY)
	{
		m_critical.lock();
		if(m_initImage)
		{
			INT width = MediaSampler::getImageWidth();
			INT height = MediaSampler::getImageHeight();
			setSize(width,height,::D3DFMT_R8G8B8);
			m_initImage = false;
		}

		INT width = MediaSampler::getImageWidth();
		INT height = MediaSampler::getImageHeight();
		INT pitch = (width * 24/8 + 3)/4*4;
		Gdiplus::Bitmap bitmap(width,height,pitch,PixelFormat24bppRGB,(LPBYTE)m_dataPtr);
		Gdiplus::Graphics* g = getGraphics();
	
		g->DrawImage(&bitmap,0,height-1,width,-height);
		m_critical.unlock();
	}
	return UnitGdip::onRender(world,x,y,z);
}
void UnitVideo::onImageDraw(LPVOID data,DWORD size)
{
	m_critical.lock();
	if(m_dataSize == size)
	{
		CopyMemory(m_dataPtr,data,size);
		resetRenderFlag();
	}
	m_critical.unlock();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// TargetRecoder
// DirectX - ターゲット取り込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
TargetRecoder::TargetRecoder()
{
	m_soundRecoder.setCallback(CLASSPROC(this,TargetRecoder,captureSound));

	m_videoFPS = 29.976f;
	m_audioBitrate = 63*1000;
	m_videoBitrate = 512*1024;
	m_writer = new WMWriter;
	m_buffer = NULL;
	m_network = false;
	m_networkPort = 8080;
	m_videoQuality = 30;
	m_videoWidth = 640;
	m_videoHeight = 480;
	m_file = false;
	m_recode = false;
	m_fileName = L"output.wmv";
}
TargetRecoder::~TargetRecoder()
{
	stop();
	delete m_writer;
}
bool TargetRecoder::start()
{
	stop();
	if(!m_target.create(m_videoWidth,m_videoHeight,::D3DFMT_X8R8G8B8))
		return false;
	m_pitch = (m_videoWidth * 32/8 + 3)/4*4;

	AutoCom<IWMProfile> wmProfile;
	AutoCom<IWMProfileManager> profileManager;
	WMCreateProfileManager(profileManager );
	//空プロファイルの作成
	profileManager->CreateEmptyProfile(WMT_VER_9_0,wmProfile);
	wmProfile->SetName(L"WMV");

	//オーティオストリームの追加
	IWMStreamConfig* soundConfig;
	soundConfig = getAudioConfig(m_audioBitrate,2,48000,16);
	if(soundConfig)
	{
		soundConfig->SetStreamNumber(1);
		wmProfile->AddStream(soundConfig);
		soundConfig->Release();
		m_soundRecoder.setSamples(48000);
	}

	//ビデオストリームの追加
	AutoCom<IWMStreamConfig> videoConfig;
	wmProfile->CreateNewStream(WMMEDIATYPE_Video,videoConfig);
	wmProfile->AddStream(videoConfig);
	//ビデオストリームの設定
	AutoCom<IWMVideoMediaProps> props;
	videoConfig->QueryInterface(IID_IWMVideoMediaProps,props);
	videoConfig->SetBitrate(m_videoBitrate);
	videoConfig->SetStreamName(L"VIDEO");
	videoConfig->SetConnectionName(L"V");

	//videoConfig->SetBufferWindow(10000);
	props->SetQuality(m_videoQuality);
	props->SetMaxKeyFrameSpacing(5*10*1000*1000);

	WM_MEDIA_TYPE* mediaType = NULL;
	DWORD mediaSize = 0;
	props->GetMediaType(NULL,&mediaSize);
	mediaType = (WM_MEDIA_TYPE*)new BYTE[mediaSize];
	props->GetMediaType(mediaType,&mediaSize);

	INT width = getVideoWidth();
	INT height = getVideoHeight();
	if(mediaType->formattype == WMFORMAT_VideoInfo)
	{
		WMVIDEOINFOHEADER* videoInfo = (WMVIDEOINFOHEADER*)mediaType->pbFormat; 
		BITMAPINFOHEADER* bitmapInfo = &videoInfo->bmiHeader;
		mediaType->bTemporalCompression = true;

		if(isVideoCodec(WMMEDIASUBTYPE_WMV1))
			mediaType->subtype = WMMEDIASUBTYPE_WMV1;
		else if(isVideoCodec(WMMEDIASUBTYPE_WMV2))
			mediaType->subtype = WMMEDIASUBTYPE_WMV2;
		else if(isVideoCodec(WMMEDIASUBTYPE_WMV3))
			mediaType->subtype = WMMEDIASUBTYPE_WMV3;

		videoInfo->dwBitRate = m_videoBitrate;
		videoInfo->AvgTimePerFrame = (LONGLONG)(1000.0*1000.0*10.0/m_videoFPS);
		bitmapInfo->biBitCount = 32;
		bitmapInfo->biWidth = width;
		bitmapInfo->biHeight = height;	
		bitmapInfo->biSizeImage = 0;
		bitmapInfo->biCompression = mediaType->subtype.Data1;
	}
	props->SetMediaType(mediaType);
	delete[] mediaType;
	wmProfile->ReconfigStream(videoConfig);

	m_writer->setProfile(wmProfile);
	m_writer->init(m_file?m_fileName:NULL,m_network?m_networkPort:0);

	if(!m_writer->start())
		return false;
	m_timer = timeGetTime();
	m_soundRecoder.start();
	m_buffer = m_writer->createBuffer(m_pitch*getVideoHeight());
	m_recode = true;
	return true;
}
bool TargetRecoder::stop()
{
	if(m_recode)
	{
		m_recode = false;
		m_critical.lock();
		if(m_buffer)
		{
			m_buffer->Release();
			m_buffer = NULL;
		}
		m_critical.unlock();
		m_soundRecoder.stop();
		m_writer->stop();
	}
	return true;
}
INT TargetRecoder::getVideoHeight() const
{
	return m_videoHeight;
}
INT TargetRecoder::getVideoWidth() const
{
	return m_videoWidth;
}
void TargetRecoder::setVideoWidth(INT value)
{
	if(value < 16)
		m_videoWidth = 16;
	else if(value > 2000)
		m_videoWidth = 2000;
	else
		m_videoWidth = value;
}
void TargetRecoder::setVideoHeight(INT value)
{
	if(value < 16)
		m_videoHeight = 16;
	else if(value > 2000)
		m_videoHeight = 2000;
	else
		m_videoHeight = value;
}

void TargetRecoder::setSize(INT width,INT height)
{
	m_videoWidth = width;
	m_videoHeight = height;
}
void TargetRecoder::draw(Screen* screen)
{
	if(!m_buffer)
		return;
	IDirect3DSurface9* src = screen->getSurface();
	if(src)
	{
		draw(src);
		src->Release();
	}
}
void TargetRecoder::draw(Target* screen)
{
	if(!m_buffer)
		return;
	IDirect3DSurface9* src = screen->getSurface();
	if(src)
	{
		draw(src);
		src->Release();
	}
}
void TargetRecoder::draw(IDirect3DSurface9* src)
{
	m_critical.lock();
	if(!m_buffer)
		return;

	D3DSURFACE_DESC descSrc;
	src->GetDesc(&descSrc);
	INT srcWidth = descSrc.Width;
	INT srcHeight = descSrc.Height;

	if(srcWidth == getVideoWidth() && srcHeight == getVideoHeight())
	{
		D3DLOCKED_RECT rect;
		if(src->LockRect(&rect,NULL,0) == S_OK)
		{
			INT height = srcHeight;
			INT lineSize  = rect.Pitch < m_pitch?rect.Pitch:m_pitch;
			LPBYTE srcData = (LPBYTE)rect.pBits + rect.Pitch * (height-1);
			LPBYTE destData;
			m_buffer->GetBuffer(&destData);
			INT i;
			for(i=0;i<height;i++)
			{
				CopyMemory(destData,srcData,lineSize);
				srcData -= rect.Pitch;
				destData += m_pitch;
			}
			src->UnlockRect();

			m_writer->write(1,(QWORD)timeGetTime()*1000*10,m_buffer);
			m_critical.unlock();
			return;
		}
	}

	IDirect3DSurface9* dest = m_target.getSurface();
	IDirect3DDevice9* device = Device::getInterface();


	INT destWidth = m_target.getWidth();
	INT destHeight = m_target.getHeight();

	INT lastWidth = destWidth;
	INT lastHeight = destHeight;
	FLOAT aspect = (FLOAT)srcWidth / (FLOAT)srcHeight;
	FLOAT aspect2 = (FLOAT)destWidth / (FLOAT)destHeight;
	if(aspect > aspect2)
	{
		lastHeight = (INT)(destWidth / aspect);
	}
	else if(aspect < aspect2)
	{
		lastWidth = (INT)(destHeight * aspect);
	}
	RECT rectDest;
	rectDest.left = (destWidth-lastWidth)/2;
	rectDest.top = (destHeight-lastHeight)/2;
	rectDest.right = rectDest.left + lastWidth;
	rectDest.bottom = rectDest.top + lastHeight;


	device->ColorFill(dest,NULL,0);

	device->StretchRect(src,NULL,dest,&rectDest,D3DTEXF_LINEAR);

	D3DLOCKED_RECT rect;
	if(dest->LockRect(&rect,NULL,0) == S_OK)
	{
		INT height = m_target.getHeight();
		INT lineSize  = rect.Pitch < m_pitch?rect.Pitch:m_pitch;
		LPBYTE srcData = (LPBYTE)rect.pBits + rect.Pitch * (height-1);
		LPBYTE destData;
		m_buffer->GetBuffer(&destData);
		INT i;
		for(i=0;i<height;i++)
		{
			CopyMemory(destData,srcData,lineSize);
			srcData -= rect.Pitch;
			destData += m_pitch;
		}
		dest->UnlockRect();

		m_writer->write(1,(QWORD)timeGetTime()*1000*10,m_buffer);
	}

	dest->Release();
	m_critical.unlock();
}
DWORD TargetRecoder::captureSound(LPVOID* data)
{
	LPBYTE adr = (LPBYTE)data[0];
	DWORD size = *(LPDWORD)data[1];
	m_writer->write(0,((QWORD)m_timer+m_soundRecoder.getTime())*10*1000,adr,size);
	return 0;
}
DWORD TargetRecoder::getAudioBitrate() const
{
	return m_audioBitrate;
}
DWORD TargetRecoder::getVideoBitrate() const
{
	return m_videoBitrate;
}
void TargetRecoder::setAudioBitrate(DWORD value)
{
	m_audioBitrate = value;
}
void TargetRecoder::setVideoBitrate(DWORD value)
{
	//ビットレート範囲の調整
	if(value < 4*1024)
		m_videoBitrate = 4*1024;
	else if(value > 20*1024*1024)
		m_videoBitrate = 20*1024*1024;
	else
		m_videoBitrate = value;
}
FLOAT TargetRecoder::getVideoFPS() const
{
	return m_videoFPS;
}
void TargetRecoder::setVideoFPS(FLOAT value)
{
	if(value < 0.001f)
		m_videoFPS = 0.001f;
	else if(value > 72.0f)
		m_videoFPS = 72.0f;
	else
		m_videoFPS = value;
}
bool TargetRecoder::isNetwork() const
{
	return m_network;
}
void TargetRecoder::setNetwork(bool flag)
{
	m_network = flag;
}
INT TargetRecoder::getNetworkPort() const
{
	return m_networkPort;
}
void TargetRecoder::setNetworkPort(INT value)
{
	m_networkPort = value;
}
LPCWSTR TargetRecoder::getFileName() const
{
	return m_fileName;
}
void TargetRecoder::setFileName(LPCWSTR fileName)
{
	m_fileName = fileName;
}
bool TargetRecoder::isFile() const
{
	return m_file;
}
void TargetRecoder::setFile(bool flag)
{
	m_file = flag;
}
void TargetRecoder::setVideoQuality(INT value)
{
	if(value < 0)
		m_videoQuality = 0;
	else if(value > 100)
		m_videoQuality = 100;
	else
		m_videoQuality = value;
}
INT TargetRecoder::getVideoQuality() const
{
	return m_videoQuality;
}
bool TargetRecoder::isRecode() const
{
	return m_recode;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitScreenCapture
// DirectX - スクリーンキャプチャ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitScreenCapture::UnitScreenCapture()
{
	m_screen.left = 0;
	m_screen.top = 0;
	m_screen.right = GetSystemMetrics(SM_CXSCREEN);
	m_screen.bottom = GetSystemMetrics(SM_CYSCREEN);
	m_index = 0;
//	m_thread.setIdle(CLASSPROC(this,UnitScreenCapture,onIdle));		//動作制御用

	add(&m_image[0]);
	add(&m_image[1]);
}
void UnitScreenCapture::setScreenRect(RECT* rect)
{
	m_screen = *rect;
}
bool UnitScreenCapture::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	INT px = m_screen.left;
	INT py = m_screen.top;
	INT width = m_screen.right - m_screen.left;
	INT height = m_screen.bottom - m_screen.top;
	if(getImageWidth() != width || getImageHeight() != height)
	{
		createImage(width,height,::D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT);
	}
	//HDC hdc = GetDC(NULL);
	//HDC hdc = CreateDCW(L"DISPLAY", NULL, NULL, NULL); 

	HDC hdc = GetDCEx(NULL,NULL,DCX_CACHE|DCX_LOCKWINDOWUPDATE);
	HDC destDC = getDC();
	BitBlt(destDC,0,0,width,height,hdc,px,py,SRCCOPY);
	releaseDC(destDC);
	ReleaseDC(NULL,hdc);
	//DeleteDC(hdc);


//	m_i11.captureScreen(px,py,width,height);
	return true;
}
}}
