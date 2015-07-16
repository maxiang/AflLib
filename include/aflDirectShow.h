#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDSHOW

#pragma warning(disable:4819;disable:4996;disable:4995)


#include <tchar.h>

#include <dshow.h>
#include <wmsdk.h>
#define __D3DRM_H__  
#define _D3DRMOBJ_H_ 

//qedit.hの仕様変更対策
//ただし、必要な物だけ宣言するように変更したためコメントアウト
/*
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>
*/
#ifndef __qedit_h__
EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const IID IID_ISampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB( 
		double SampleTime,
		IMediaSample *pSample) = 0;
		
	virtual HRESULT STDMETHODCALLTYPE BufferCB( 
		double SampleTime,
		BYTE *pBuffer,
		long BufferLen) = 0;
		
};

class ISampleGrabber : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long *pBufferSize,long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample **ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB *pCallback,long WhichMethodToCallback) = 0;
};


#endif



#include "aflImage.h"
#include "aflWindow.h"
#include "aflDirectSound.h"

using namespace AFL::WINDOWS;



enum MEDIA_STAT
{
	MEDIA_IDEL,
	MEDIA_CONNECTTING,
	MEDIA_CONNECTED,
	MEDIA_BUFFER,
	MEDIA_ERROR,
	MEDIA_PLAY,
	MEDIA_STOP
};

template<class T> class AutoCom
{
public:
	AutoCom(){m_class = NULL;}
	~AutoCom(){if(m_class)m_class->Release();}
	operator LPVOID*() {return (LPVOID*)&m_class;}
	operator T*() const{return m_class;}
	operator T**() {return &m_class;}
	T* operator->() const{return m_class;}
	bool isInit()const{return m_class!=NULL;}
protected:
	T* m_class;
};
IWMStreamConfig* getAudioConfig(DWORD bitrate,WORD channel,DWORD samplesPerSec,WORD bitsPerSample);
bool isVideoCodec(GUID guid);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WMWriter
// WMA/WMV書き込み
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WMWriter
{
public:
	WMWriter();
	~WMWriter();
	bool init(LPCWSTR fileName=NULL,DWORD port=0);
	bool setSource(IWMReader* wmReader);
	bool start()const;
	void stop()const;
	void flush()const
	{
		m_wmWriter->Flush();
	}
	bool setProfile(IWMProfile* wmProfile);
	IWMWriterAdvanced* getWriter()const;
	void write(INT index,QWORD sampleTime,LPVOID data,INT size);
	bool getCodecInfo(IWMHeaderInfo3* info,WORD wIndex,WORD* pwStreamNum,
		AFL::WString& name,AFL::WString& desc,WMT_CODEC_INFO_TYPE*  pType,std::vector<BYTE>& codecInfo);
	INSSBuffer* createBuffer(INT size);
	void write(INT index,QWORD sampleTime,INSSBuffer* buff);

protected:
	bool getAttributeByIndex(IWMHeaderInfo* info,WORD wIndex,WORD* pwStreamNum,
		AFL::WString& name,WMT_ATTR_DATATYPE*  pType,std::vector<BYTE>& value);

	AutoCom<IWMWriter> m_wmWriter;
	AutoCom<IWMWriterAdvanced> m_writerAdvanced;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaReader
// WMA/WMV読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MediaSampler : public IWMReaderCallback,IWMReaderCallbackAdvanced
{
public:
	MediaSampler();
	virtual ~MediaSampler();
	bool stop();
	bool open(LPCWSTR url=NULL);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject );
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	DWORD getBufferPercent()const;
	DWORD getBufferSize()const;

	INT getImageWidth()const;
	INT getImageHeight()const;
	MEDIA_STAT getStat()const;
	DWORD getPlayTime()const;
	void setCallDraw(AFL::ClassProc& proc);
	void setVolume(INT volume)
	{
		m_sound.setVolume(volume);
	}
	bool isStream() const;
	bool isPlay()const
	{
		return m_play;
	}
protected:
	void initDevice();
	virtual HRESULT STDMETHODCALLTYPE OnSample(DWORD dwOutputNum,QWORD cnsSampleTime,
		QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer *pSample,void *pvContext);
	virtual HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,
		QWORD cnsSampleDuration,DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext);
	HRESULT STDMETHODCALLTYPE OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,
		BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext );
	virtual HRESULT STDMETHODCALLTYPE OnTime(QWORD qwCurrentTime,void __RPC_FAR * pvContext);
	virtual HRESULT STDMETHODCALLTYPE OnStreamSelection(
		WORD wStreamCount,WORD __RPC_FAR * pStreamNumbers,
		WMT_STREAM_SELECTION __RPC_FAR * pSelections,void __RPC_FAR * pvContext);
	virtual HRESULT STDMETHODCALLTYPE OnOutputPropsChanged(DWORD dwOutputNum,
		WM_MEDIA_TYPE __RPC_FAR * pMediaType,void __RPC_FAR * pvContext );
	virtual HRESULT STDMETHODCALLTYPE AllocateForOutput(DWORD dwOutputNum,
		DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
		void __RPC_FAR * pvContext);
	virtual HRESULT STDMETHODCALLTYPE AllocateForStream(WORD wStreamNum,
		DWORD cbBuffer,INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,void __RPC_FAR * pvContext);
	virtual void onStatChange();
	virtual void onImageInit();
	virtual void onImageDraw(LPVOID data,DWORD size);

	IWMReaderAdvanced2* m_wmRaderAdvanced;
	IWMReader* m_wmReader;
	LONG m_refCount;
	MEDIA_STAT m_mediaStat;
	INT m_audioNumber;
	INT m_videoNumber;
	QWORD m_playTimeVideo;
	QWORD m_playTimeAudio;
	volatile QWORD m_playTimeBuffer;
	bool m_streamFlag;
	WAVEFORMATEX m_waveFormat;

	AFL::ClassProc m_callDrawProc;
	INT m_imageWidth;
	INT m_imageHeight;

	AFL::DSOUND::SoundPlayer m_sound;
	AFL::WString m_metaText;
	bool m_play;
	AFL::WString m_url;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaStreamRecoder
// WMA/WMV ストリーム保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MediaStreamRecoder : public MediaSampler
{
public:
	MediaStreamRecoder();

	bool setFile(LPCWSTR file);
	DWORD getStreamSize() const;
protected:
	HRESULT STDMETHODCALLTYPE OnStatus(WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,
		BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext );

	HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,
		DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext);
	WMWriter m_writer;

	DWORD m_streamSize;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaReader
// WMA/WMV読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MediaReader : public MediaSampler
{
public:
	MediaReader();
	AFL::WINDOWS::DIB* getDIB();
	bool isRedraw();
protected:
	virtual void onImageInit();
	virtual void onImageDraw(LPVOID data,DWORD size);

	bool m_redraw;
	AFL::DSOUND::SoundPlayer m_sound;
	AFL::WINDOWS::DIB m_dib;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MediaRecoder
// WMA/WMV保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MediaRecoder : public MediaReader
{
public:
	MediaRecoder();
	~MediaRecoder();
	bool record(LPCWSTR descFileName,LPCWSTR srcFileName);
	bool record(LPCWSTR descFileName);
	bool start();
	bool stop();
	DWORD getStreamSize() const;
	void writeImage(QWORD t,HDC hdc=NULL);
	void writeImage(HDC hdc);
	void setImageSize(INT width,INT height);
	INT getImageWidth() const;
	INT getImageHeight() const;
	DWORD captureSound(LPVOID* data);
	void setVideoRate(INT rate);
	void setAudioRate(INT rate);
	void setVideoFPS(INT fps);
protected:
	HRESULT STDMETHODCALLTYPE OnStatus( 
		WMT_STATUS Status,HRESULT hr,WMT_ATTR_DATATYPE dwType,BYTE __RPC_FAR *pValue,void __RPC_FAR *pvContext);
	HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum,QWORD cnsSampleTime,QWORD cnsSampleDuration,
		DWORD dwFlags,INSSBuffer __RPC_FAR * pSample,void __RPC_FAR * pvContext);
	WMWriter* m_writer;
	DWORD m_streamSize;
	AFL::WINDOWS::DIB m_dib;
	AFL::DSOUND::SoundRecoder m_soundRecoder;
	DWORD m_timer;
	DWORD m_audioBitrate;
	DWORD m_videoBitrate;
	DWORD m_fps;

};

class WindowVideo : public Window
{
public:
	WindowVideo();
	~WindowVideo();
	void setDrainWnd(HWND hwnd);
	bool play(LPCWSTR name);
	bool play(LPCSTR name);
	void stop();
	LONG getBufferPercent() const;
	DWORD getPlayTime()const;
	DWORD getVideoWidth() const;
	DWORD getVideoHeight() const;
	void setFullScreen(bool flag) const;
	bool getFullScreen() const;
	MEDIA_STAT getStat()const;
protected:
	void release();
	bool init();
	void onSize(Message* m);
	void onPlay(LPVOID data);
	void onDraw(LPVOID data);


	AFL::WString m_url;
	AFL::Thread m_thread;
	volatile  bool m_play;				//状態
	HWND m_drainWnd;
	MediaReader m_mediaReader;
};
class TimeCount
{
public:
	void setTime(DWORD count)
	{
		m_count = GetTickCount()+count;
	}
	bool isTimeout() const
	{
		if(GetTickCount() > m_count)
			return true;
		return false;
	}
protected:
	volatile DWORD m_count;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CameraDevice
// カメラからのデータ取得
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class CameraDevice
{
public:
	CameraDevice();
	~CameraDevice();
	bool start();
	void stop();
	INT getWidth() const
	{
		return m_imageWidth;
	}
	INT getHeight() const
	{
		return m_imageHeight;
	}
	INT getPitch() const
	{
		return m_imagePitch;
	}
	LPBYTE CameraDevice::getImage() const;
protected:
	AutoCom<IMediaControl> m_mediaControl;
	AutoCom<ISampleGrabber> m_sampleGrabber;
	INT m_imageWidth;
	INT m_imageHeight;
	INT m_imagePitch;
	AFL::SP<BYTE> m_imageData;
};


#define __INC_AFLDSHOW
#endif