#ifndef __INC_AFLSOUND

#ifndef _INC_MMSYSTEM
	#include "mmsystem.h"
#endif
#ifndef __DSOUND_INCLUDED__
#include "dsound.h"
#endif
#include  "aflStd.h"
#include  <queue>
//namespace AFL::DirectSound
namespace AFL{namespace DSOUND{

class WaveData
{
public:
	WaveData();
	~WaveData();
	BOOL open(LPCSTR pName,LPCSTR pType);
	BOOL open(LPCWSTR pName,LPCSTR pType);
	bool isData(){return m_waveData != NULL;}
	LPVOID getWaveData(){return m_waveData;}
	LPWAVEFORMATEX getFormat(){return &m_waveFormatEx;}
	DWORD getDataSize(){return m_waveSize;}
protected:
	void callback(LPVOID data);

	DWORD m_waveSize;
	LPVOID m_waveData;
	DWORD m_wavePt;
	WAVEFORMATEX m_waveFormatEx;
};

/////////////////////////////////////////////////////////////////////////////
// Sound
class Sound
{
public:
	Sound();
	~Sound();
	BOOL setWindow(HWND hWnd=0){return setCoopWnd(hWnd);}
	BOOL setCoopWnd(HWND hWnd=0);
	BOOL open(LPCSTR pName,LPCSTR pType=NULL);
	BOOL play(int nPan=0);
	void createList(int nCount,int bFlag = 0);
	WaveData* getWaveData(){return &m_waveData;}
protected:
	void release();
	LPDIRECTSOUNDBUFFER creatBuffer();
	
	LPDIRECTSOUND m_pDirectSound;
	HWND m_hWnd;
	int m_nCount;
	LPDIRECTSOUNDBUFFER* m_ppDSBuffer;
	WaveData m_waveData;
};


class SoundPlayer
{
public:
	SoundPlayer();
	~SoundPlayer();
	bool createBuffer(LPGUID guid,WAVEFORMATEX* waveFormat=NULL);
	bool stop();
	bool play();
	DWORD getPlayWaitTime();
	DWORD getPlayWaitSize();
	DWORD getWriteBufferSize();
	DWORD write(LPVOID dataAdr,DWORD dataSize);
	void setVolume(INT volume);
	void setPan(INT pan);
protected:
	void onBuffer(LPVOID data);

	LPDIRECTSOUND8 m_sound;
	LPDIRECTSOUNDBUFFER8 m_soundBuffer;
	DSBUFFERDESC m_descBuffer;
	INT m_currentPos;
	DWORD m_currentPlay;

	INT m_pan;
	INT m_volume;
	INT m_bufferMSec;
	INT m_bufferRate;
	INT m_bufferBit;
	INT m_bufferChannel;
	INT m_bufferSizePerSec;
	DWORD m_bufferSize;
	Thread m_thread;
	volatile bool m_playing;
	Critical m_critical;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// SoundRecoder
// WAVÉfÅ[É^ÇÃéÊìæ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class SoundRecoder
{
public:
	SoundRecoder();
	~SoundRecoder();
	bool start();
	bool stop();
	bool createBuffer(LPGUID guid=NULL);
	void routingSound(LPVOID data);
	void setCallback(ClassProc& callback);
	ULONGLONG getTime() const
	{
		return m_streamSize*1000 / (m_bufferRate*m_bufferBit*m_bufferChannel/8);
	}
	void setSamples(DWORD value)
	{
		m_bufferRate = value;
	}
protected:
	INT m_notifyCount;
	DSBPOSITIONNOTIFY* m_positionNotify;
	LPDIRECTSOUNDCAPTURE8 m_soundCapture;
	LPDIRECTSOUNDCAPTUREBUFFER8 m_soundCaptureBuffer;
	Thread m_thread;
	volatile bool m_enable;

	INT m_bufferMSec;
	INT m_bufferRate;
	INT m_bufferBit;
	INT m_bufferChannel;
	INT m_bufferSize;

	ULONGLONG m_streamSize;

	INT m_latency;
	ClassProc m_callback;
};

//namespace
}}


#define __INC_AFLSOUND
#endif	// __INC_AFLSOUND
