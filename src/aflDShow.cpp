#include <windows.h>
#define INITGUID
#include <tchar.h>
#include <math.h>
#include <stdlib.h>
#include "aflDShow.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "WMVCORE.lib")

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
		#define NEW new
		#define CHECK_MEMORY_LEAK
#endif

using namespace AFL;
using namespace WINDOWS;
void MBtoUCS2(WString& dest,LPCSTR src)
{
	LPWSTR buff = new WCHAR[strlen(src)+1];
	swprintf(buff,L"%hs",src);
	dest = buff;
	delete[] buff;
}
bool isVideoCodec(GUID guid)
{
	//条件にあったストリームを検索

	AutoCom<IWMProfileManager> profileManager;
	WMCreateProfileManager(profileManager);

	AutoCom<IWMCodecInfo> codecInfo;
	profileManager->QueryInterface(IID_IWMCodecInfo,codecInfo);

	INT i,j;
	bool flag = true;
	DWORD codecCount;
	codecInfo->GetCodecInfoCount(WMMEDIATYPE_Video,&codecCount);
	for(j=0;j<(INT)codecCount && flag;j++)
	{
		DWORD formatCount;
		codecInfo->GetCodecFormatCount(WMMEDIATYPE_Video,j,&formatCount);

		for(i=0;i<(INT)formatCount;i++)
		{
			AutoCom<IWMStreamConfig> config;
			codecInfo->GetCodecFormat(WMMEDIATYPE_Video,j,i,config);

			AutoCom<IWMMediaProps> audioProps;
			config->QueryInterface(IID_IWMMediaProps,audioProps);

			WM_MEDIA_TYPE* mediaType = NULL;
			DWORD mediaSize = 0;
			//メディア情報の領域確保
			audioProps->GetMediaType(NULL,&mediaSize);
			mediaType = (WM_MEDIA_TYPE*)new BYTE[mediaSize];
			//メディア情報の取得
			audioProps->GetMediaType(mediaType,&mediaSize);

			GUID subtype = mediaType->subtype;
			delete[] mediaType;
			if(subtype == guid)
				return true;
		}
	}
	return false;
}
IWMStreamConfig* getAudioConfig(DWORD bitrate,WORD channel,DWORD samplesPerSec,WORD bitsPerSample)
{
	//条件にあったストリームを検索

	AutoCom<IWMProfileManager> profileManager;
	WMCreateProfileManager(profileManager);

	AutoCom<IWMCodecInfo> codecInfo;
	profileManager->QueryInterface(IID_IWMCodecInfo,codecInfo);

	INT i,j;
	bool flag = true;
	DWORD codecCount;
	codecInfo->GetCodecInfoCount(WMMEDIATYPE_Audio,&codecCount);
	for(j=0;j<(INT)codecCount && flag;j++)
	{
		DWORD formatCount;
		codecInfo->GetCodecFormatCount(WMMEDIATYPE_Audio,j,&formatCount);

		INT nearBitrate = -1;
		INT nearIndex = -1;
		for(i=0;i<(INT)formatCount;i++)
		{
			AutoCom<IWMStreamConfig> config;
			codecInfo->GetCodecFormat(WMMEDIATYPE_Audio,j,i,config);

			AutoCom<IWMMediaProps> audioProps;
			config->QueryInterface(IID_IWMMediaProps,audioProps);

			WM_MEDIA_TYPE* mediaType = NULL;
			DWORD mediaSize = 0;
			//メディア情報の領域確保
			audioProps->GetMediaType(NULL,&mediaSize);
			mediaType = (WM_MEDIA_TYPE*)new BYTE[mediaSize];
			//メディア情報の取得
			audioProps->GetMediaType(mediaType,&mediaSize);
			if(mediaType->subtype == WMMEDIASUBTYPE_WMAudioV8)
			{
				flag = true;

				WAVEFORMATEX* pwfex = (WAVEFORMATEX*)mediaType->pbFormat;
				if(pwfex->wBitsPerSample == bitsPerSample &&
					pwfex->nSamplesPerSec == samplesPerSec &&
					pwfex->nChannels == channel)
				{
					DWORD rate;
					config->GetBitrate(&rate);
					if(nearBitrate == -1 || abs(static_cast<int>(rate-bitrate))<abs(static_cast<int>(nearBitrate-bitrate)))
					{
						nearBitrate = rate; 
						nearIndex = i;
					}
				}
			}
			delete[] mediaType;
		}
		if(nearIndex >= 0)
		{
			IWMStreamConfig* config;
			codecInfo->GetCodecFormat(WMMEDIATYPE_Audio,j,nearIndex,&config);
			return config;
		}
	}
	return NULL;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WMWriter
// WMA/WMV書き込み
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
WMWriter::WMWriter()
{
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	HRESULT hr = WMCreateWriter( NULL, m_wmWriter );
	if(FAILED(hr))
		return;
	m_wmWriter->QueryInterface( IID_IWMWriterAdvanced, (void **)&m_writerAdvanced );
}
WMWriter::~WMWriter()
{
	stop();
	CoUninitialize();
}
bool WMWriter::init(LPCWSTR fileName,DWORD port)
{
	if(!m_wmWriter.isInit())
	{
		HRESULT hr = WMCreateWriter( NULL, m_wmWriter );
		if(FAILED(hr))
			return false;
		m_wmWriter->QueryInterface( IID_IWMWriterAdvanced, (void **)&m_writerAdvanced );
	}

	INT i;
	DWORD count;
	m_writerAdvanced->GetSinkCount(&count);
	for(i=0;i<(INT)count;i++)
	{
		IWMWriterSink* sink;
		m_writerAdvanced->GetSink(i,&sink);
		m_writerAdvanced->RemoveSink(sink);
	}

	if(fileName)
	{
		IWMWriterFileSink* fileSink;
		WMCreateWriterFileSink(&fileSink);
		m_writerAdvanced->AddSink(fileSink);
		fileSink->Open(fileName);
		fileSink->Release();
	}

	if(port)
	{
		IWMWriterNetworkSink* netSink;
		WMCreateWriterNetworkSink(&netSink);
		m_writerAdvanced->AddSink(netSink);
		netSink->Open(&port);
		netSink->Release();
	}
	return true;

}
bool WMWriter::setProfile(IWMProfile* wmProfile)
{
	HRESULT hr;

	INT i;
	hr = m_wmWriter->SetProfile( wmProfile );

	DWORD streamCount;
	wmProfile->GetStreamCount(&streamCount);
	for(i=0;i<(INT)streamCount;i++)
	{
		WORD streamNumber;
		IWMStreamConfig* streamConfig = NULL;
		hr = wmProfile->GetStream( i, &streamConfig );
		streamConfig->GetStreamNumber(&streamNumber);
	}
	return true;
}

bool WMWriter::setSource(IWMReader* wmReader)
{
	INT i;
	if(!wmReader)
		return false;
	AutoCom<IWMHeaderInfo3> readerInfo;
	AutoCom<IWMProfile> readerProfile;
	AutoCom<IWMHeaderInfo3> writeInfo;
	AutoCom<IWMReaderAdvanced2> wmRaderAdvanced;


	HRESULT hr;
	hr = wmReader->QueryInterface( IID_IWMHeaderInfo3, (void **)readerInfo);
	hr = wmReader->QueryInterface( IID_IWMProfile, (void **)readerProfile );
	hr = wmReader->QueryInterface( IID_IWMReaderAdvanced2, (void **)wmRaderAdvanced );
	hr = m_wmWriter->QueryInterface( IID_IWMHeaderInfo3, (void **)writeInfo);
	hr = m_wmWriter->SetProfile( readerProfile );

	if(FAILED(hr))
		return false;
	WCHAR name[1024];
	DWORD s = 1024;
	readerProfile->GetName(name,&s);


	DWORD streamCount;
	readerProfile->GetStreamCount(&streamCount);
	for(i=0;i<(INT)streamCount;i++)
	{
		WORD streamNumber;
		IWMStreamConfig* streamConfig = NULL;
		readerProfile->GetStream( i, &streamConfig );
		streamConfig->GetStreamNumber(&streamNumber);
		wmRaderAdvanced->SetReceiveStreamSamples(streamNumber, true );
		streamConfig->Release();
	}

	start();
	return true;
}
bool WMWriter::start()const
{
	stop();
	if(m_wmWriter.isInit())
	{
		m_writerAdvanced->SetLiveSource(true);
		HRESULT hr = m_wmWriter->BeginWriting();
		return hr == S_OK;
	}
	return false;
}
void WMWriter::stop()const
{
	if(m_wmWriter.isInit())
	{
/*		INT i;
		DWORD count;
		m_writerAdvanced->GetSinkCount(&count);
		for(i=0;i<(INT)count;i++)
		{
			IWMWriterSink* sink;
			m_writerAdvanced->GetSink(i,&sink);
			m_writerAdvanced->RemoveSink(sink);
		}
*/
		m_wmWriter->EndWriting();
	}
}
IWMWriterAdvanced* WMWriter::getWriter() const
{
	return m_writerAdvanced;
}
void WMWriter::write(INT index,QWORD sampleTime,LPVOID data,INT size)
{
	AutoCom<INSSBuffer> buff;
	if(m_wmWriter->AllocateSample(size,buff) == S_OK)
	{
		LPBYTE dest = NULL;
		buff->GetBuffer(&dest);
		if(dest)
		{
			CopyMemory(dest,data,size);
		}
		m_wmWriter->WriteSample(index,sampleTime,0,buff);
	}
}
INSSBuffer* WMWriter::createBuffer(INT size)
{
	INSSBuffer* buff = NULL;
	HRESULT hr = m_wmWriter->AllocateSample(size,&buff);
	if(hr != S_OK)
		return NULL;
	return buff;
}
void WMWriter::write(INT index,QWORD sampleTime,INSSBuffer* buff)
{
	m_wmWriter->WriteSample(index,sampleTime,0,buff);
}
bool WMWriter::getAttributeByIndex(IWMHeaderInfo* info,WORD wIndex,WORD* pwStreamNum,
	WString& name,WMT_ATTR_DATATYPE*  pType,std::vector<BYTE>& value)
{
	WORD nameLength;
	WORD valueLength;
	info->GetAttributeByIndex(wIndex,pwStreamNum,NULL,&nameLength,pType,NULL,&valueLength);

	WCHAR* nameWork = NEW WCHAR[nameLength];
	value.resize(valueLength);
	info->GetAttributeByIndex(wIndex,pwStreamNum,nameWork,&nameLength,pType,&value[0],&valueLength);
	name = nameWork;
	delete[] nameWork;

	return true;
}
bool WMWriter::getCodecInfo(IWMHeaderInfo3* info,WORD wIndex,WORD* pwStreamNum,
	WString& name,WString& desc,WMT_CODEC_INFO_TYPE*  pType,std::vector<BYTE>& codecInfo)
{
	WORD nameLength;
	WORD descLength;
	WORD infoLength;
	info->GetCodecInfo(wIndex,&nameLength,NULL,&descLength,NULL,pType,&infoLength,NULL);

	WCHAR* nameWork = NEW WCHAR[nameLength];
	WCHAR* descWork = NEW WCHAR[descLength];
	codecInfo.resize(infoLength);
	info->GetCodecInfo(wIndex,&nameLength,nameWork,&descLength,descWork,pType,&infoLength,&codecInfo[0]);
	name = nameWork;
	desc = descWork;
	delete[] nameWork;
	delete[] descWork;

	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaSampler
// WMA/WMV読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MediaSampler::MediaSampler()
{
	::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	WMCreateReader( NULL, WMT_RIGHT_PLAYBACK, &m_wmReader );
	m_wmReader->QueryInterface( IID_IWMReaderAdvanced2, (void **)&m_wmRaderAdvanced );
	m_refCount = 1;
	m_playTimeVideo = 0;
	m_playTimeAudio = 0;
	m_imageWidth = 0;
	m_imageHeight = 0;
	m_play = false;
	m_streamFlag = false;
}
MediaSampler::~MediaSampler()
{
	stop();
	m_wmRaderAdvanced->Release();
	m_wmReader->Release();
	//CoUninitialize();
}
INT MediaSampler::getImageWidth() const
{
	return m_imageWidth;
}
INT MediaSampler::getImageHeight() const
{
	return m_imageHeight;
}
MEDIA_STAT MediaSampler::getStat() const
{
	return m_mediaStat;
}
void MediaSampler::onStatChange()
{
}
void MediaSampler::onImageInit()
{
}
void MediaSampler::onImageDraw(LPVOID data,DWORD size)
{
	if(m_callDrawProc.isAddress())
		m_callDrawProc.call(this);
}
DWORD MediaSampler::getPlayTime() const
{
//	if(m_playTimeAudio > m_playTimeVideo)
//		return (DWORD)(m_playTimeAudio / 1000 / 1000 / 10);
//	else
		return (DWORD)(m_playTimeVideo / 1000 / 1000 / 10);
}	
bool MediaSampler::isStream() const
{
	return m_streamFlag;
}

void MediaSampler::setCallDraw(AFL::ClassProc& proc)
{
	m_callDrawProc = proc;
}
bool MediaSampler::stop()
{
	m_play = false;
	if(!m_wmReader)
		return false;
	m_wmReader->Stop();
	m_wmReader->Close();
	m_sound.stop();
	return true;
}
bool MediaSampler::open(LPCWSTR url)
{
	if(!m_wmReader)
		return false;
	if(url)
		m_url = url;
	stop();
	m_playTimeVideo = 0;
	m_playTimeAudio = 0;
	m_playTimeBuffer = 0;
	m_play = true;
	m_mediaStat = MEDIA_IDEL;
	m_wmReader->Open(m_url,this,NULL);
	return true;
}

DWORD MediaSampler::getBufferPercent() const
{
	DWORD  percent;
	QWORD  buffering;
	m_wmRaderAdvanced->GetBufferProgress(&percent,&buffering);
	return percent;
}
DWORD MediaSampler::getBufferSize() const
{
	DWORD  percent;
	QWORD  buffering;
	m_wmRaderAdvanced->GetBufferProgress(&percent,&buffering);
	return (DWORD)buffering;
}

HRESULT STDMETHODCALLTYPE MediaSampler::OnSample(
	DWORD dwOutputNum,QWORD cnsSampleTime,
	QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer *pSample,void *pvContext)
{
	BYTE        *pData = NULL;
	DWORD       cbData = 0;
	HRESULT     hr = S_OK;

	if( dwOutputNum == m_audioNumber )
	{
		m_playTimeAudio = cnsSampleTime - (QWORD)m_sound.getPlayWaitTime()*1000*10;
		hr = pSample->GetBufferAndLength( &pData, &cbData );
		m_sound.write(pData,cbData);
		m_playTimeBuffer = m_playTimeAudio + (QWORD)m_sound.getPlayWaitTime()*1000*10;
		
	}
	else if( dwOutputNum == m_videoNumber )
	{
		if(m_playTimeVideo == 0)
			m_sound.play();
		if(m_audioNumber >= 0)
		{
			if(m_playTimeVideo > m_playTimeBuffer - (QWORD)m_sound.getPlayWaitTime()*1000*10 + (QWORD)5000*1000*10)
				m_sound.play();
		}
		m_playTimeVideo = cnsSampleTime;
		hr = pSample->GetBufferAndLength( &pData, &cbData );
		onImageDraw(pData,cbData);
	}
	return S_OK;
}
HRESULT STDMETHODCALLTYPE MediaSampler::OnStreamSample(
	WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,
	DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext)
{
	m_playTimeAudio = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	m_playTimeVideo = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE MediaSampler::OnStatus( WMT_STATUS Status,
									HRESULT hr,
									WMT_ATTR_DATATYPE dwType,
									BYTE __RPC_FAR *pValue,
									void __RPC_FAR *pvContext )
{
	MEDIA_STAT stat = m_mediaStat;
	switch( Status )
	{
	case WMT_OPENED:
		if(FAILED(hr))
			m_mediaStat = MEDIA_ERROR;
		else
		{
			initDevice();
			m_mediaStat = MEDIA_CONNECTED;
			m_wmReader->Start(0, 0, 1.0, NULL );
		}
		break;
	case WMT_RECONNECT_START:
	case WMT_LOCATING:
	case WMT_CONNECTING:
		m_sound.stop();
		m_mediaStat = MEDIA_CONNECTTING;
		break;
	case WMT_BUFFERING_START:
		m_sound.stop();
		m_mediaStat = MEDIA_BUFFER;
		break;
	case WMT_BUFFERING_STOP:
		if(m_play)
		{
			m_mediaStat = MEDIA_PLAY;
			if(!m_streamFlag)
				m_sound.play();
		}
		break;
	case WMT_STARTED:
		m_mediaStat = MEDIA_PLAY;
		if(m_audioNumber >= 0)
		{
			if(!m_streamFlag)
			{
				m_sound.createBuffer(NULL,&m_waveFormat);
				m_sound.play();
			}
		}
		break;
	case WMT_ERROR:
	case WMT_CLOSED:
	case WMT_STOPPED:
		if(!m_play)
		{
			m_mediaStat = MEDIA_STOP;
		}
		else
		{
			m_sound.stop();
			m_mediaStat = MEDIA_ERROR;
		}
		break;
	case WMT_NEW_METADATA:
		m_metaText = (LPCWSTR)pValue;
		break;
	default:
		return S_OK;
	}
	if(stat != m_mediaStat)
		onStatChange();
	return S_OK;
}

void MediaSampler::initDevice()
{
	DWORD outputCount = 0;
	m_wmReader->GetOutputCount(&outputCount );

	m_videoNumber = -1;
	m_audioNumber = -1;
	INT i;
	for(i=0;i<(INT)outputCount;i++)
	{
		IWMOutputMediaProps* props = NULL;
		m_wmReader->GetOutputProps( i, &props);
		if(props)
		{
			ULONG mediaSize = 0;
			props->GetMediaType( NULL, &mediaSize );
			WM_MEDIA_TYPE* mediaType = (WM_MEDIA_TYPE*)new BYTE[mediaSize];
			props->GetMediaType( mediaType, &mediaSize );

			if(mediaType->majortype == WMMEDIATYPE_Audio)
			{
				m_audioNumber = i;
				CopyMemory(&m_waveFormat,mediaType->pbFormat,sizeof(WAVEFORMATEX));
			}
			else if(mediaType->majortype == WMMEDIATYPE_Video)
			{
				WMVIDEOINFOHEADER* wmvVideoInfo = (WMVIDEOINFOHEADER*)mediaType->pbFormat;
				m_videoNumber = i;
				m_imageWidth = wmvVideoInfo->bmiHeader.biWidth;
				m_imageHeight = wmvVideoInfo->bmiHeader.biHeight;
				onImageInit();
			}
			delete[] mediaType;
			props->Release();
		}
	}

	AutoCom<IWMProfile> readerProfile;
	m_wmReader->QueryInterface( IID_IWMProfile, (void **)readerProfile );
}


HRESULT STDMETHODCALLTYPE MediaSampler::OnTime(
	QWORD qwCurrentTime,void __RPC_FAR * pvContext)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE MediaSampler::OnStreamSelection(WORD wStreamCount,
													WORD __RPC_FAR * pStreamNumbers,
													WMT_STREAM_SELECTION __RPC_FAR * pSelections,
													void __RPC_FAR * pvContext){return S_OK;}


HRESULT STDMETHODCALLTYPE MediaSampler::OnOutputPropsChanged(DWORD dwOutputNum,
													   WM_MEDIA_TYPE __RPC_FAR * pMediaType,
													   void __RPC_FAR * pvContext ){return S_OK;}


HRESULT STDMETHODCALLTYPE MediaSampler::AllocateForOutput(DWORD dwOutputNum,
													DWORD cbBuffer,
													INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
													void __RPC_FAR * pvContext){return S_OK;}

HRESULT STDMETHODCALLTYPE MediaSampler::AllocateForStream(WORD wStreamNum,
													DWORD cbBuffer,
													INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
													void __RPC_FAR * pvContext){return S_OK;}


HRESULT STDMETHODCALLTYPE MediaSampler::QueryInterface(REFIID riid,
													 void __RPC_FAR *__RPC_FAR *ppvObject ) 
{
	if( ( IID_IWMReaderCallback == riid ) ||
		( IID_IUnknown == riid ) )
	{

		AddRef();
		return( S_OK );
	}
	else if( riid == IID_IWMReaderCallbackAdvanced )
	{
		*ppvObject = static_cast< IWMReaderCallbackAdvanced * >( this );
		AddRef();
		return( S_OK );
	}
	else if( riid == IID_IWMStatusCallback )
	{
		*ppvObject = static_cast< IWMStatusCallback * >( this );
		AddRef();
		return( S_OK );
	}


	*ppvObject = NULL;
	return( E_NOINTERFACE );
}

ULONG STDMETHODCALLTYPE MediaSampler::AddRef()
{
	return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE MediaSampler::Release()
{
	if( 0 == InterlockedDecrement(&m_refCount))
	{
		delete this;
		return  0;
	}
	return m_refCount;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaStreamRecoder
// WMA/WMV ストリーム保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MediaStreamRecoder::MediaStreamRecoder()
{
	m_streamFlag = true;
}

bool MediaStreamRecoder::setFile(LPCWSTR file)
{
	return m_writer.init(file);
}


DWORD MediaStreamRecoder::getStreamSize() const
{
	return m_streamSize;
}

HRESULT STDMETHODCALLTYPE MediaStreamRecoder::OnStatus(
	WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext )
{
	switch( Status )
	{
	case WMT_STARTED:
		m_streamSize = 0;
		m_writer.setSource(m_wmReader);
		break;
	}
	return MediaSampler::OnStatus(Status,hr,dwType,pValue,pvContext);
}

HRESULT STDMETHODCALLTYPE MediaStreamRecoder::OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,
	DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext)
{
	m_playTimeAudio = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	m_playTimeVideo = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	DWORD length = 0;
	pSample->GetLength(&length);
	m_streamSize += length;
	m_writer.getWriter()->WriteStreamSample(
		wStreamNum,cnsSampleTime,0,cnsSampleDuration,dwFlags,pSample );

	return S_OK;
}	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaReader
// WMA/WMV読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MediaReader::MediaReader()
{
	m_redraw = false;
}
AFL::WINDOWS::DIB* MediaReader::getDIB()
{
	return &m_dib;
}
bool MediaReader::isRedraw()
{
	bool ret = m_redraw;
	m_redraw = false;
	return ret;
}
void MediaReader::onImageInit()
{
	m_dib.createDIB(m_imageWidth,m_imageHeight,AFL::WINDOWS::D3DFMT_R8G8B8);
}
void MediaReader::onImageDraw(LPVOID data,DWORD size)
{
	INT pitch = m_dib.getPitch();
	LPBYTE dest = (LPBYTE)m_dib.getImage();
	LPBYTE src = &((LPBYTE)data)[pitch*(m_dib.getHeight()-1)];

	INT i;
	for(i=0;i<m_dib.getHeight();i++)
		CopyMemory(dest+i*pitch,src-pitch*i,pitch);
	m_redraw = true;
	MediaSampler::onImageDraw(data,size);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaRecoder
// WMA/WMV保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MediaRecoder::MediaRecoder()
{
	m_dib.setReverse(false);
	m_fps = 30;
	m_audioBitrate = 63*1000;
	m_videoBitrate = 512*1024;
	m_writer = NEW WMWriter;
	m_streamSize = 0;
	m_soundRecoder.setCallback(CLASSPROC(this,MediaRecoder,captureSound));
}
MediaRecoder::~MediaRecoder()
{
	stop();
	delete m_writer;
}
bool MediaRecoder::record(LPCWSTR descFileName,LPCWSTR srcFileName)
{
	stop();
	m_writer->init(descFileName);
	open(srcFileName);
	return true;
}


bool MediaRecoder::record(LPCWSTR descFileName)
{
	stop();

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
	props->SetQuality(30);
	props->SetMaxKeyFrameSpacing(4*10*1000*1000);

	WM_MEDIA_TYPE* mediaType = NULL;
	DWORD mediaSize = 0;
	props->GetMediaType(NULL,&mediaSize);
	mediaType = (WM_MEDIA_TYPE*)new BYTE[mediaSize];
	props->GetMediaType(mediaType,&mediaSize);

	INT width = getImageWidth();
	INT height = getImageHeight();
	if(mediaType->formattype == WMFORMAT_VideoInfo)
	{
		WMVIDEOINFOHEADER* videoInfo = (WMVIDEOINFOHEADER*)mediaType->pbFormat; 
		BITMAPINFOHEADER* bitmapInfo = &videoInfo->bmiHeader;
		mediaType->bTemporalCompression = true;
		mediaType->subtype = WMMEDIASUBTYPE_WMV1;
		videoInfo->dwBitRate = m_videoBitrate;
		videoInfo->AvgTimePerFrame = 1000*1000*10/m_fps;
		bitmapInfo->biBitCount = 24;
		bitmapInfo->biWidth = width;
		bitmapInfo->biHeight = height;	
		bitmapInfo->biSizeImage = 0;
		bitmapInfo->biCompression = WMMEDIASUBTYPE_WMV1.Data1;
	}
	props->SetMediaType(mediaType);
	delete[] mediaType;
	wmProfile->ReconfigStream(videoConfig);

	m_writer->setProfile(wmProfile);
	m_writer->init(descFileName);

	start();
	return true;
}
void MediaRecoder::writeImage(QWORD t,HDC hdc)
{
	if(hdc)
		m_dib.bitBlt(hdc);
	m_writer->write(1,t,m_dib.getImage(),m_dib.getSize());
}
void MediaRecoder::writeImage(HDC hdc)
{
	if(hdc)
		m_dib.bitBlt(hdc);
}
void MediaRecoder::setImageSize(INT width,INT height)
{
	m_dib.createDIB(width,height,AFL::WINDOWS::D3DFMT_R8G8B8);
}
INT MediaRecoder::getImageWidth() const
{
	return m_dib.getWidth();
}
INT MediaRecoder::getImageHeight() const
{
	return m_dib.getHeight();
}
DWORD MediaRecoder::captureSound(LPVOID* data)
{
	LPBYTE adr = (LPBYTE)data[0];
	DWORD size = *(LPDWORD)data[1];
	m_writer->write(0,(m_timer+m_soundRecoder.getTime())*10*1000,adr,size);
	return 0;
}
HRESULT STDMETHODCALLTYPE MediaRecoder::OnStatus( WMT_STATUS Status,
									HRESULT hr,
									WMT_ATTR_DATATYPE dwType,
									BYTE __RPC_FAR *pValue,
									void __RPC_FAR *pvContext )
{
	switch( Status )
	{
	case WMT_OPENED:
		if(FAILED(hr))
			m_mediaStat = MEDIA_ERROR;
		else
		{
			m_mediaStat = MEDIA_CONNECTED;
			m_wmReader->Start( 0, 0, 1.0, NULL );
		}
		break;
	case WMT_LOCATING:
	case WMT_CONNECTING:
		m_mediaStat = MEDIA_CONNECTTING;
		break;
	case WMT_BUFFERING_START:
		m_mediaStat = MEDIA_BUFFER;
		break;
	case WMT_BUFFERING_STOP:
		break;
	case WMT_STARTED:
		m_mediaStat = MEDIA_PLAY;
		initDevice();
		m_streamSize = 0;
		m_writer->setSource(m_wmReader);
		break;
	case WMT_STOPPED:
		m_mediaStat = MEDIA_ERROR;
		break;
	case WMT_CLOSED:
		m_mediaStat = MEDIA_ERROR;
		break;

	}
	return S_OK;
}
HRESULT STDMETHODCALLTYPE MediaRecoder::OnStreamSample(
	WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,
	DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext)
{
	m_playTimeAudio = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	m_playTimeVideo = (DWORD)(cnsSampleTime / 1000 / 1000 / 10);
	DWORD length = 0;
	pSample->GetLength(&length);
	m_streamSize += length;
	m_writer->getWriter()->WriteStreamSample( wStreamNum,
										   cnsSampleTime,
										   0,
										   cnsSampleDuration,
										   dwFlags,
										   pSample );

	return S_OK;
}
bool MediaRecoder::start()
{
	m_timer = timeGetTime();
	m_writer->start();
	m_soundRecoder.start();
	return true;
}
bool MediaRecoder::stop()
{
	MediaSampler::stop();
	m_writer->stop();
	m_soundRecoder.stop();

	return true;
}
DWORD MediaRecoder::getStreamSize() const
{
	return m_streamSize;
}
void  MediaRecoder::setVideoRate(INT rate)
{
	m_videoBitrate = rate;
}
void  MediaRecoder::setAudioRate(INT rate)
{
	m_audioBitrate = rate;
}
void  MediaRecoder::setVideoFPS(INT fps)
{
	m_fps = fps;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowVideo
// ビデオウインドウ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

WindowVideo::WindowVideo()
{
	m_mediaReader.setCallDraw(CLASSPROC(this,WindowVideo,onDraw));
	addMessage(WM_SIZE,CLASSPROC(this,WindowVideo,onSize));
	setBackColor(0);
	m_drainWnd = NULL;
	init();
}
WindowVideo::~WindowVideo()
{

	stop();
}
void WindowVideo::setDrainWnd(HWND hwnd)
{
	m_drainWnd = hwnd;
}
bool WindowVideo::play(LPCWSTR name)
{
	stop();
	m_url = name;
	m_play = true;
	m_mediaReader.open(name);

	//m_thread.startThread(CLASSPROC(this,WindowVideo,onPlay),NULL);		
	return true;
}
bool WindowVideo::play(LPCSTR name)
{
	return play(UCS2(name));
}
void WindowVideo::stop()
{
	m_play = false;
	while(m_thread.isActiveThread())
		Sleep(100);
}
MEDIA_STAT WindowVideo::getStat() const
{
	return m_mediaReader.getStat();
}
LONG WindowVideo::getBufferPercent() const
{
	return m_mediaReader.getBufferPercent();
}
DWORD WindowVideo::getPlayTime()const
{
	return m_mediaReader.getPlayTime();
}
DWORD WindowVideo::getVideoWidth() const
{
	return m_mediaReader.getImageWidth();
}
DWORD WindowVideo::getVideoHeight() const
{
	return m_mediaReader.getImageHeight();

}
void WindowVideo::setFullScreen(bool flag) const
{
//	if(!m_videoWindow)
//		return;
//	m_videoWindow->put_FullScreenMode(flag);
}
bool WindowVideo::getFullScreen() const
{
//	if(!m_videoWindow)
//		return false;
//	LONG l;
//	m_videoWindow->get_FullScreenMode(&l);
//	return l!=0;
	return false;
}
void WindowVideo::release()
{
}

bool WindowVideo::init()
{

	return true;
}

void WindowVideo::onSize(Message* m)
{

}


void WindowVideo::onPlay(LPVOID data)
{
	init();

	m_mediaReader.open(m_url);
}

void WindowVideo::onDraw(LPVOID data)
{
	DIB* dib = m_mediaReader.getDIB();

	INT width = dib->getWidth();
	INT height = dib->getHeight();

	RECT rect;
	getClientRect(&rect);
	INT clientWidth = rect.right;
	INT clientHeight = rect.bottom;
	
	if(width && height)
	{
		FLOAT aspect1 = (FLOAT)width/height;
		FLOAT aspect2 = (FLOAT)clientWidth / clientHeight;
		if(aspect1 > aspect2)
		{
			width = clientWidth;
			height = (INT)(clientWidth / aspect1);
		}
		else if(aspect1 < aspect2)
		{
			width = (INT)(clientHeight * aspect1);
			height = clientHeight;
		}
		//m_videoWindow->SetWindowPosition(
		//	(clientWidth-width)/2, (clientHeight-height)/2, width,height);
		HDC hdc = getDC();
		if(hdc)
		{
			dib->stretchBlt(hdc,(clientWidth-width)/2, (clientHeight-height)/2, width,height);
			//dib->bitBlt(hdc);
			releaseDC(hdc);
		}
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CameraDevice
// カメラからのデータ取得
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
CameraDevice::CameraDevice()
{
	::CoInitializeEx(NULL,COINIT_MULTITHREADED);
}
CameraDevice::~CameraDevice() 
{
	stop();
	//CoUninitialize();
}
void getDeviceType(IMoniker* moniker,WString& name)
{
	LPOLESTR displayName;
	IBindCtx* pbcfull = NULL;
	CreateBindCtx( 0, &pbcfull );
	moniker->GetDisplayName(pbcfull,NULL,&displayName);

	WCHAR buff[1024];
	swscanf(displayName,L"%*[^?]?\\%[^#]",buff);
	name = buff;
	pbcfull->Release();
}
bool getDeviceList()
{
	AutoCom<ICreateDevEnum> devEnum;
	if(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,IID_ICreateDevEnum, devEnum)!=S_OK)
		return false;
	AutoCom<IEnumMoniker> enumMoniker;
	if(devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, enumMoniker, 0)!=S_OK)
		return false;

	IMoniker* moniker;		
	while(enumMoniker->Next(1, &moniker,NULL) == S_OK)
	{
		WString deviceType;
		getDeviceType(moniker,deviceType);

		AutoCom<IPropertyBag2> propertyBag;
		moniker->BindToStorage(0, 0, IID_IPropertyBag2,propertyBag);

		HRESULT result;
		PROPBAG2 progbag;
		VARIANT var;
		var.vt = VT_BSTR;
		while(propertyBag->Read(0,&progbag,NULL,&var, &result) == S_OK)
		{
			VariantClear(&var);
		}
		moniker->Release();
	}
	return true;
}
bool CameraDevice::start()
{
	AutoCom<ICaptureGraphBuilder2 > captureBuilder;
	AutoCom<IMoniker> moniker;		
	AutoCom<IBaseFilter> videoFilter;
	AutoCom<IBaseFilter> baseFilter;
	AutoCom<IGraphBuilder> graphBuilder;
	AutoCom<ICreateDevEnum> devEnum;
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,IID_ICreateDevEnum, devEnum);

	AutoCom<IEnumMoniker> enumMoniker;

	CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC,IID_IGraphBuilder, graphBuilder);
	graphBuilder->QueryInterface( IID_IMediaControl, m_mediaControl);
	CoCreateInstance( CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,IID_ICaptureGraphBuilder2, captureBuilder);
	captureBuilder->SetFiltergraph( graphBuilder );


	devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, enumMoniker, 0);

	if(!enumMoniker.isInit())
		return false;

	bool dvFlag = false;
	enumMoniker->Next(1, moniker, NULL);

	WString deviceType;
	getDeviceType(moniker,deviceType);


	moniker->BindToObject( 0, 0, IID_IBaseFilter, baseFilter );


	graphBuilder->AddFilter( baseFilter, L"Video Capture");

	CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, videoFilter);
	graphBuilder->AddFilter(videoFilter, L"SamGra");
	videoFilter->QueryInterface( IID_ISampleGrabber, m_sampleGrabber);

	AutoCom<IBaseFilter> soundFilter;
	CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, soundFilter);
	graphBuilder->AddFilter(soundFilter, L"Sound Out");
	AutoCom<ISampleGrabber> soundGrabber;
	soundFilter->QueryInterface( IID_ISampleGrabber, soundGrabber);

	FILTER_INFO filterInfo;
	soundFilter->QueryFilterInfo(&filterInfo);


	AM_MEDIA_TYPE amt;
	ZeroMemory(&amt, sizeof(AM_MEDIA_TYPE));
	amt.majortype  = MEDIATYPE_Video;
	amt.subtype    = MEDIASUBTYPE_RGB32;
	amt.formattype = FORMAT_VideoInfo; 
	m_sampleGrabber->SetMediaType( &amt );


	if(deviceType == L"avc")
	{
		//DVならスプリッタを利用して音声を取得
		AutoCom<IBaseFilter> dvFilter;
		CoCreateInstance( CLSID_DVSplitter , NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, dvFilter);
		graphBuilder->AddFilter( dvFilter, L"Splitter");
		captureBuilder->RenderStream(NULL,&MEDIATYPE_Interleaved,baseFilter,NULL,dvFilter);
		captureBuilder->RenderStream(NULL,&MEDIATYPE_Video,dvFilter, NULL, videoFilter);
		captureBuilder->RenderStream(NULL,&MEDIATYPE_Audio,dvFilter,NULL,soundFilter);
	}
	else	
		//音声無し
		captureBuilder->RenderStream(NULL,&MEDIATYPE_Video,baseFilter, NULL, videoFilter);

	m_sampleGrabber->GetConnectedMediaType( &amt ); 
	if(amt.pbFormat == NULL)
		return false;
	VIDEOINFOHEADER *videoHeader = (VIDEOINFOHEADER*)amt.pbFormat;
	BITMAPINFOHEADER* bitmapInfo = &videoHeader->bmiHeader;

	m_imageWidth = bitmapInfo->biWidth;
	m_imageHeight = bitmapInfo->biHeight;
	m_imagePitch = bitmapInfo->biWidth*4;
	m_imageData = NEW BYTE[bitmapInfo->biSizeImage];
	m_sampleGrabber->SetBufferSamples(TRUE);

	m_mediaControl->Run();
	return true;
}
void CameraDevice::stop()
{
	if(m_mediaControl.isInit())
		m_mediaControl->Stop();
}
LPBYTE CameraDevice::getImage() const
{
	if(m_sampleGrabber.isInit())
	{
		LONG s = 0;
		m_sampleGrabber->GetCurrentBuffer(&s,NULL);
		m_sampleGrabber->GetCurrentBuffer(&s,(PLONG)m_imageData.get());
	}
	return m_imageData.get();
}

