#include <windows.h>
#include <fstream>
#pragma warning(disable : 4995)


#include "aflStd.h"
#include "aflDShow.h"
#include "aflDSound.h"

#if _MSC_VER && !defined(_WIN32_WCE)
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



#ifdef _MSC_VER
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "dxguid.lib")
#endif //_MSC_VER
#pragma comment(lib, "strmiids.lib")


//namespace AFL::DirectSound
namespace AFL{



struct SampleGrabberData
{
	double time;
	BYTE* buff;
	long length;
};
class SampleGrabberCB : public ISampleGrabberCB
{
protected:
	ClassProc m_proc;
	INT m_ref;
public:
	SampleGrabberCB()
	{
		m_ref = 0;
	}
	ULONG WINAPI AddRef()
	{
		return ++m_ref;
	}
	ULONG WINAPI Release()
	{
		if(--m_ref)
			return m_ref;
		delete this;
		return 0;
	}
	HRESULT WINAPI SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}
	HRESULT WINAPI QueryInterface(REFIID refIID, LPVOID* ppUnk)
	{
		return S_OK;
	}

	LONG WINAPI BufferCB(double SampleTime, BYTE* pBuf, long bufLen)
	{
		SampleGrabberData sgd = {SampleTime,pBuf,bufLen};
		m_proc.call(&sgd);
		return S_OK;
	}
	void setCallback(ClassProc& proc)
	{
		m_proc = proc;
	}
};




WaveData::WaveData()
{
	m_waveSize = 0;
	m_waveData = NULL;
	ZeroMemory(&m_waveFormatEx,sizeof(WAVEFORMATEX));

	//データ領域の初期化
	m_waveFormatEx.cbSize = sizeof(m_waveFormatEx);
	m_waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	m_waveFormatEx.nChannels = 2;  
	m_waveFormatEx.nSamplesPerSec = 44100;
	m_waveFormatEx.wBitsPerSample = 16;
	m_waveFormatEx.nBlockAlign = m_waveFormatEx.nChannels * m_waveFormatEx.wBitsPerSample/8;
	m_waveFormatEx.nAvgBytesPerSec = m_waveFormatEx.nSamplesPerSec * m_waveFormatEx.nBlockAlign; 

}

WaveData::~WaveData()
{
	if(m_waveData)
		delete m_waveData;
}

void  WaveData::callback(LPVOID data)
{
	SampleGrabberData* sgd = (SampleGrabberData*)data;
	INT length = sgd->length;
	if(m_waveSize < m_wavePt + sgd->length)
	{
		length = m_waveSize - m_wavePt;
	}
	CopyMemory((((LPBYTE)m_waveData)+m_wavePt),sgd->buff,length);
	m_wavePt += length;
}
BOOL WaveData::open(LPCSTR name,LPCSTR pType)
{
	return open(WString(name),pType);
}
IPin* getPin(IBaseFilter* filter, PIN_DIRECTION direct)
{
	IEnumPins* enumPins;
	filter->EnumPins(&enumPins);
	IPin* pin = NULL;
	PIN_DIRECTION pd;
	while(SUCCEEDED(enumPins->Next(1, &pin, 0)))
	{
		if (SUCCEEDED(pin->QueryDirection(&pd)) && pd == direct)
		{
			break;
		}
		pin->Release();
	}
	enumPins->Release();
	return pin;
}

BOOL WaveData::open(LPCWSTR name,LPCSTR pType)
{
	AutoCom<IGraphBuilder> graphBuilder;
	AutoCom<IMediaControl> mediaControl;
	AutoCom<IMediaFilter> mediaFilter;
	AutoCom<IMediaEvent> mediaEvent;
	AutoCom<IMediaSeeking> mediaSeeking;
	AutoCom<IBaseFilter> sampleGrabberFilter;
	AutoCom<ISampleGrabber> sampleGrabber;
	AutoCom<IBaseFilter> baseFilterNull;
	AM_MEDIA_TYPE am_media_type = {0};

	//COMを初期化
	CoInitialize(NULL);

	CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC,IID_IGraphBuilder,graphBuilder);
	CoCreateInstance(CLSID_NullRenderer,NULL,CLSCTX_INPROC,IID_IBaseFilter,baseFilterNull);
	CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC,IID_IBaseFilter,sampleGrabberFilter);

	//FilterからISampleGrabberインターフェースを取得します
	sampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (LPVOID *)&sampleGrabber);
	//GraphにSampleGrabber Filterを追加
	graphBuilder->AddFilter(sampleGrabberFilter, L"Sample Grabber");

	//コールバッククラスの設定
	SampleGrabberCB* sampleGrabberCB = new SampleGrabberCB();
	sampleGrabber->SetCallback(sampleGrabberCB, 1);

	//ウェーブに変換させる
	am_media_type.majortype = MEDIATYPE_Audio;
	am_media_type.subtype = MEDIASUBTYPE_PCM;
	am_media_type.formattype = FORMAT_WaveFormatEx;
	sampleGrabber->SetMediaType(&am_media_type);


	//MediaControlインターフェース取得
	graphBuilder->QueryInterface(IID_IMediaControl,mediaControl);
	graphBuilder->QueryInterface(IID_IMediaFilter,mediaFilter);
	graphBuilder->QueryInterface(IID_IMediaEvent,mediaEvent);
	graphBuilder->QueryInterface(IID_IMediaSeeking,mediaSeeking);

	//ファイルをロードする
	AutoCom<IBaseFilter> baseFilter; 
	if(graphBuilder->AddSourceFilter(name,name,baseFilter)!=S_OK)
		return false;

	//読み込んだデータのフィルターを接続
	IPin* out = getPin(baseFilter, PINDIR_OUTPUT);
	IPin* in = getPin(sampleGrabberFilter, PINDIR_INPUT);
	graphBuilder->Connect(out,in);
	in->Release();
	out->Release();

	//出力先のNullにしてデコードだけさせる
	graphBuilder->AddFilter(baseFilterNull, L"Null Renderer");
	out = getPin(sampleGrabberFilter, PINDIR_OUTPUT);
	in = getPin(baseFilterNull, PINDIR_INPUT);
	graphBuilder->Connect(out,in);

	//デコードフィルターの設定
	graphBuilder->Render(out);
	out->Release();
	in->Release();

	//演奏時間の取得
	LONGLONG playTime = 0;
	mediaSeeking->GetDuration(&playTime);

	//メディアタイプの確認
	sampleGrabber->GetConnectedMediaType(&am_media_type);
	m_waveFormatEx = *(WAVEFORMATEX*)am_media_type.pbFormat;

	//同期させず全て書き出す
	mediaFilter->SetSyncSource(NULL);


	//データサイズの確定
	m_waveSize = (INT)((LONGLONG)m_waveFormatEx.nAvgBytesPerSec * playTime / 10000000); 
	//データ領域の確保
	if(m_waveData)
		delete m_waveData;
	m_waveData = NEW BYTE[m_waveSize];
	sampleGrabberCB->setCallback(CLASSPROC(this,WaveData,callback));
	m_wavePt = 0;

	//再生開始
	mediaControl->Run();

	//書き出しが終わるまで待つ
	LONG code;
	mediaEvent->WaitForCompletion(INFINITE,&code);

	//停止処理
	mediaControl->Stop();

	return TRUE;
}

//////////////////////////////////////////////////////////
//Sound

//--------------------------------
// コンストラクタ
//--------------------------------
Sound::Sound()
{
	m_nCount = 0;
	m_ppDSBuffer = NULL;
	m_hWnd = 0;

	::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	::CoCreateInstance(CLSID_DirectSound,NULL, CLSCTX_ALL, IID_IDirectSound, (LPVOID*)&m_pDirectSound);
	if(m_pDirectSound)
	{
		IDirectSound_Initialize(m_pDirectSound, NULL);
		setCoopWnd();
	}

}


//--------------------------------
// デストラクタ
//--------------------------------
Sound::~Sound()
{
	release();
	if(m_pDirectSound)
		m_pDirectSound->Release();
	//::CoUninitialize();

}


//--------------------------------
// ウインドウの関連づけ
//--------------------------------
BOOL Sound::setCoopWnd(HWND hWnd)
{
	if(!m_pDirectSound || (hWnd && (m_hWnd == hWnd)))
		return FALSE;
	
	if(!hWnd)
	{
		hWnd = ::GetActiveWindow();
		if(hWnd)
			m_hWnd = hWnd;
	}
	else
		m_hWnd = hWnd;

	return (m_pDirectSound->SetCooperativeLevel(m_hWnd,DSSCL_NORMAL)==DS_OK);
}


//--------------------------------
// WAVファイルを読む
//--------------------------------
BOOL Sound::open(LPCSTR pName,LPCSTR pType)
{
	getWaveData()->open(pName,pType);
	createList(3);
	return TRUE;
}

//--------------------------------
// 同時発音用バッファの確保
//--------------------------------
void Sound::createList(int nCount,int bFlag)
{
	int i;
	if(!getWaveData()->isData())
		return;
	release();
	m_ppDSBuffer = NEW LPDIRECTSOUNDBUFFER[nCount];
	m_nCount = nCount;
//	if(bFlag == 0)
	{
		m_ppDSBuffer[0] = creatBuffer();
		for(i=1;i<m_nCount;i++)
			m_pDirectSound->DuplicateSoundBuffer(m_ppDSBuffer[0],&m_ppDSBuffer[i]);
	}

}

//--------------------------------
// 効果音の再生
//--------------------------------
BOOL Sound::play(int nPan)
{
	int i;
	if(m_ppDSBuffer && m_ppDSBuffer[0] && setCoopWnd())
	{
		DWORD dwWord;
		for(i=0;i<m_nCount;i++)
		{
			if(m_ppDSBuffer[i])
			{
				m_ppDSBuffer[i]->GetStatus(&dwWord);
				if(dwWord != DSBSTATUS_PLAYING)
				{
					m_ppDSBuffer[i]->SetPan(nPan);
					return (m_ppDSBuffer[i]->Play(0,0,0)==DS_OK);
				}
			}
		}
		if(m_ppDSBuffer[m_nCount-1])
		{
			m_ppDSBuffer[m_nCount-1]->Stop();
			m_ppDSBuffer[m_nCount-1]->SetPan(nPan);
			m_ppDSBuffer[m_nCount-1]->SetCurrentPosition(0);
			return (m_ppDSBuffer[m_nCount-1]->Play(0,0,0)==DS_OK);
		}
	}
	return FALSE;
}


//--------------------------------
// データの解放
//--------------------------------
void Sound::release()
{
	int i;
	if(m_ppDSBuffer)
	{
		for(i=0;i<m_nCount;i++)
		{
			if(m_ppDSBuffer[i])
				m_ppDSBuffer[i]->Release();
		}
		delete m_ppDSBuffer;
		m_ppDSBuffer=NULL;
		m_nCount = 0;
	}
}


//--------------------------------
// DirectSound用バッファの確保
//--------------------------------
LPDIRECTSOUNDBUFFER Sound::creatBuffer()
{
	LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
	if(m_pDirectSound)
	{
		DSBUFFERDESC Desc;
		ZeroMemory(&Desc,sizeof(DSBUFFERDESC));
		Desc.dwSize = sizeof(DSBUFFERDESC);
		Desc.dwFlags = DSBCAPS_STATIC|DSBCAPS_LOCSOFTWARE|DSBCAPS_GLOBALFOCUS;
		Desc.dwBufferBytes = getWaveData()->getDataSize();
		Desc.lpwfxFormat = getWaveData()->getFormat();
		m_pDirectSound->CreateSoundBuffer(&Desc,&pDSBuffer,NULL);
		if(pDSBuffer)
		{
			LPVOID pData1;
			DWORD dwBytes1;
			pDSBuffer->Lock(0,getWaveData()->getDataSize(),&pData1,&dwBytes1,NULL,NULL,0);
			CopyMemory(pData1,getWaveData()->getWaveData(),dwBytes1);
			pDSBuffer->Unlock(pData1,dwBytes1,NULL,0);
		}
	}
	return pDSBuffer;
}


SoundPlayer::SoundPlayer()
{
	m_pan = 0;
	m_volume = 0;
	m_bufferMSec = 500;
	m_bufferRate = 48000;
	m_bufferBit = 16;
	m_bufferChannel = 2;
	m_bufferSizePerSec = 0;
	m_sound = NULL;
	m_soundBuffer = NULL;
}
SoundPlayer::~SoundPlayer()
{
	stop();
	if(m_soundBuffer)
		m_soundBuffer->Release();
}
bool SoundPlayer::createBuffer(LPGUID guid,WAVEFORMATEX* waveFormat)
{
	stop();
	if(m_soundBuffer)
	{
		m_soundBuffer->Release();
		m_soundBuffer = NULL;
	}
	//デバイスの作成
	if(m_sound)
	{
		m_sound->Release();
		m_sound = NULL;
	}

	HMODULE library = LoadLibraryW(L"dsound.dll");
	if(!library)
		return false;

	HRESULT (WINAPI *DirectSoundCreate8)(LPCGUID,LPDIRECTSOUND8*,LPUNKNOWN) =
		(HRESULT (WINAPI *)(LPCGUID,LPDIRECTSOUND8*,LPUNKNOWN))
		GetProcAddress(library,"DirectSoundCreate8");

	DirectSoundCreate8(guid,&m_sound,NULL);
	FreeLibrary(library);

	//作成できなかったら終了
	if(!m_sound)
		return false;
	m_sound->SetCooperativeLevel(GetDesktopWindow(),DSSCL_PRIORITY );



	
	if(waveFormat)
	{
		m_bufferChannel = waveFormat->nChannels;
		m_bufferRate = waveFormat->nSamplesPerSec;
		m_bufferBit = waveFormat->wBitsPerSample;

		//CopyMemory(&wfx,waveFormat,sizeof(WAVEFORMATEX));
	}
	WAVEFORMATEX wfx = 
	{
		WAVE_FORMAT_PCM,
		m_bufferChannel,
		m_bufferRate,
		m_bufferRate*m_bufferChannel*m_bufferBit/8,
		m_bufferChannel*m_bufferBit/8,
		m_bufferBit, 0
	};
	if(waveFormat)
		CopyMemory(&wfx,waveFormat,sizeof(WAVEFORMATEX));


	m_bufferSizePerSec = wfx.nSamplesPerSec;

	m_descBuffer.dwSize = sizeof(DSBUFFERDESC);
	m_descBuffer.dwFlags = DSBCAPS_STICKYFOCUS|DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN;
	m_descBuffer.dwBufferBytes = wfx.nAvgBytesPerSec*m_bufferMSec/1000;
	m_descBuffer.dwReserved = 0;
	m_descBuffer.lpwfxFormat = &wfx;
	


	LPDIRECTSOUNDBUFFER soundBuffer = NULL;
	m_sound->CreateSoundBuffer(&m_descBuffer, &soundBuffer, NULL);
	if(soundBuffer)
	{
		soundBuffer->QueryInterface(IID_IDirectSoundBuffer8,(LPVOID*)&m_soundBuffer);
		soundBuffer->Release();  
	}
	m_currentPos = -1;
	return true;
}
bool SoundPlayer::play()
{
	if(!m_soundBuffer)
		return false;
	stop();

	LPVOID adr1,adr2;
	DWORD size1,size2;
	m_soundBuffer->Lock(NULL,m_descBuffer.dwBufferBytes,&adr1,&size1,&adr2,&size2,DSBLOCK_ENTIREBUFFER);

	if(adr1)
		ZeroMemory(adr1,m_descBuffer.dwBufferBytes);
	m_soundBuffer->Unlock(adr1,size1,adr2,size2);
	m_soundBuffer->SetVolume(m_volume);
	m_soundBuffer->SetPan(m_pan);

	m_playing = true;
	m_thread.startThread(CLASSPROC(this,SoundPlayer,onBuffer),NULL);
	m_currentPos = -1;

	return true;
}
bool SoundPlayer::stop()
{
	m_playing = false;
	if(!m_soundBuffer)
		return false;
	while(m_thread.isActiveThread())
		Sleep(1);
	m_soundBuffer->Stop();
	return false;
}
void SoundPlayer::onBuffer(LPVOID data)
{
	while(m_playing)
	{
		if(getPlayWaitTime() < 200)
		{
			m_soundBuffer->Stop();
			m_currentPos = -1;
		}
		else
		{
			m_soundBuffer->Play(0,0,DSBPLAY_LOOPING);
		}
		Sleep(100);
	}
}

DWORD SoundPlayer::getPlayWaitTime()
{
	if(m_bufferSizePerSec)
	{
		DWORD size = getPlayWaitSize();
		return size*1000 / m_bufferSizePerSec;
	}
	return 0;
}
DWORD SoundPlayer::getWriteBufferSize()
{
	if(!m_soundBuffer)
		return 0;
	DWORD size;
	DWORD playPos=0,writePos = 0;
	m_soundBuffer->GetCurrentPosition(&playPos,&writePos);
	if(m_currentPos == -1)
		m_currentPos = writePos;
	if((INT)playPos < m_currentPos)
	{
		size = m_descBuffer.dwBufferBytes - m_currentPos + playPos;
	}
	else
	{
		size = playPos - m_currentPos;
	}
	return size;
}

DWORD SoundPlayer::getPlayWaitSize()
{
	if(!m_soundBuffer)
		return 0;
	DWORD size;
	DWORD playPos=0,writePos = 0;
	m_soundBuffer->GetCurrentPosition(&playPos,&writePos);
	if(m_currentPos == -1)
		m_currentPos = writePos;
	if(m_currentPos < (INT)playPos)
	{
		size = m_descBuffer.dwBufferBytes - playPos + m_currentPos;
	}
	else
	{
		size = m_currentPos - playPos;
	}
	return size;
}

DWORD SoundPlayer::write(LPVOID dataAdr,DWORD dataSize)
{
	if(!m_soundBuffer)
		return 0;
	DWORD ptSize1,ptSize2;
	LPVOID ptAdr1,ptAdr2;

	INT currentPos = m_currentPos;
	if(currentPos == -1)
		m_soundBuffer->GetCurrentPosition(0,(LPDWORD)&currentPos);

	m_soundBuffer->Lock(currentPos,dataSize,&ptAdr1,&ptSize1,&ptAdr2,&ptSize2,0);

	if(dataSize <= ptSize1)
	{
		CopyMemory(ptAdr1,dataAdr,dataSize);
		currentPos += dataSize;
	}
	else
	{
		CopyMemory(ptAdr1,dataAdr,ptSize1);
		dataSize -= ptSize1;
		m_currentPos += ptSize1;
		if(dataSize <= ptSize2)
		{
			CopyMemory(ptAdr2,(LPBYTE)dataAdr+ptSize1,dataSize);
			currentPos += dataSize;
		}
		else
		{
			CopyMemory(ptAdr2,(LPBYTE)dataAdr+ptSize1,ptSize2);
			currentPos += ptSize2;
		}
	}
	m_soundBuffer->Unlock(ptAdr1,ptSize1,ptAdr2,ptSize2);

	m_currentPos =  currentPos % m_descBuffer.dwBufferBytes;

	return dataSize;
}
void SoundPlayer::setVolume(INT volume)
{
	m_volume = volume;
	if(m_soundBuffer)
		m_soundBuffer->SetVolume(volume);
}
void SoundPlayer::setPan(INT pan)
{
	m_pan = pan;
	if(m_soundBuffer)
		m_soundBuffer->SetPan(pan);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SoundRecoder
// WAVデータの取得
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
SoundRecoder::SoundRecoder()
{
	m_bufferMSec = 3000;
	m_bufferRate = 44100;
	m_bufferBit = 16;
	m_bufferChannel = 2;
	m_notifyCount = 20;
	m_positionNotify = NEW DSBPOSITIONNOTIFY[m_notifyCount];
	INT i;
	for(i=0;i<m_notifyCount;i++)
		m_positionNotify[i].hEventNotify = CreateEvent(NULL, false, false, NULL);

	m_soundCapture = NULL;
	m_soundCaptureBuffer = NULL;

}
SoundRecoder::~SoundRecoder()
{
	stop();
	if(m_soundCaptureBuffer)
		m_soundCaptureBuffer->Release();

	INT i;
	for(i=0;i<m_notifyCount;i++)
	{
		CloseHandle(m_positionNotify[i].hEventNotify);
	}
	delete[] m_positionNotify;
}

bool SoundRecoder::start()
{
	if(!m_soundCapture)
		createBuffer();
	if(!m_soundCaptureBuffer)
		return false;
	if(m_soundCaptureBuffer->Start(DSCBSTART_LOOPING) != S_OK)
		return false;

	m_streamSize = 0;
	m_enable = true;
	m_thread.startThread(CLASSPROC(this,SoundRecoder,routingSound),NULL);
	return true;
}
bool SoundRecoder::stop()
{
	m_enable = false;
	while(m_thread.isActiveThread())
		Sleep(1);
	return true;
}
bool SoundRecoder::createBuffer(LPGUID guid)
{
	//デバイスの作成
	HMODULE library = LoadLibraryA("dsound.dll");
	if(!library)
		return false;

	HRESULT (WINAPI *DirectSoundCaptureCreate8)(LPCGUID,LPDIRECTSOUNDCAPTURE8*,LPUNKNOWN) =
		(HRESULT (WINAPI *)(LPCGUID,LPDIRECTSOUNDCAPTURE8*,LPUNKNOWN))
		GetProcAddress(library,"DirectSoundCaptureCreate8");

	DirectSoundCaptureCreate8(guid,&m_soundCapture,NULL);

	FreeLibrary(library);

	//デバイスが作成できなかったら終了
	if(!m_soundCapture)
		return false;

	if(m_soundCaptureBuffer)
	{
		m_soundCaptureBuffer->Release();
		m_soundCaptureBuffer = NULL;
	}
	DSCBUFFERDESC dscbd;
	WAVEFORMATEX wfx = 
	{
		WAVE_FORMAT_PCM,
		m_bufferChannel,
		m_bufferRate,
		m_bufferRate*m_bufferChannel*m_bufferBit/8,
		m_bufferChannel*m_bufferBit/8,
		m_bufferBit, 0
	};

	m_bufferSize =  wfx.nAvgBytesPerSec*m_bufferMSec/1000;

	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = 0;
	dscbd.dwBufferBytes = m_bufferSize;
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &wfx;
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;
	
	m_latency = m_bufferSize / m_notifyCount;


	LPDIRECTSOUNDCAPTUREBUFFER soundBuffer = NULL;
	m_soundCapture->CreateCaptureBuffer(&dscbd, &soundBuffer, NULL);
	if(!soundBuffer)
		return false;
	soundBuffer->QueryInterface(IID_IDirectSoundCaptureBuffer8,(LPVOID*)&m_soundCaptureBuffer);
	soundBuffer->Release();

	if(!m_soundCaptureBuffer)
		return false;

	INT i;
	for(i=0;i<m_notifyCount;i++)
	{
		m_positionNotify[i].dwOffset = (dscbd.dwBufferBytes /m_notifyCount)*i ;
	}

	LPDIRECTSOUNDNOTIFY8 dsNotify;
	m_soundCaptureBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&dsNotify);
	dsNotify->SetNotificationPositions(m_notifyCount, m_positionNotify);
	dsNotify->Release();

	return true;
}
void SoundRecoder::routingSound(LPVOID data)
{
	DWORD currentPos = 0;
	HANDLE* event = NEW HANDLE[m_notifyCount];
	INT i;
	for(i=0;i<m_notifyCount;i++)
		event[i] = m_positionNotify[i].hEventNotify;

	DWORD capturePos,readPos;
	m_soundCaptureBuffer->GetCurrentPosition(&capturePos,&readPos);
	currentPos = readPos;
	while(m_enable)
	{
		if(WaitForMultipleObjects(m_notifyCount,event,false,1000) != WAIT_TIMEOUT)
		{
			INT buffSize;

			m_soundCaptureBuffer->GetCurrentPosition(&capturePos,&readPos);
			if(currentPos > readPos)
				buffSize = m_bufferSize - currentPos + readPos;
			else
				buffSize = readPos - currentPos; 

			DWORD ptSize1,ptSize2;
			LPVOID ptAdr1,ptAdr2;
			m_soundCaptureBuffer->Lock(currentPos,buffSize,&ptAdr1,&ptSize1,&ptAdr2,&ptSize2,0);
			if(ptSize1)
			{
				LPVOID data1[2] = {ptAdr1,&ptSize1};
				m_callback.call(data1);
				m_streamSize += ptSize1;
			}
			if(ptSize2)
			{
				LPVOID data2[2] = {ptAdr2,&ptSize2};
				m_callback.call(data2);
				m_streamSize += ptSize2;
			}
			m_soundCaptureBuffer->Unlock(ptAdr1,ptSize1,ptAdr2,ptSize2);
			currentPos += buffSize;
			currentPos %= m_bufferSize;

		}
	}
	delete[] event;
}

void SoundRecoder::setCallback(ClassProc& callback)
{
	m_callback = callback;
}

//namespace
}

