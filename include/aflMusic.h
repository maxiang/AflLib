#ifndef __INC_AFLMUSIC

#ifndef _INC_MMSYSTEM
	#include <mmsystem.h>
#endif
#include <stdio.h>
#include "aflStd.h"
#include "aflWinTool.h"


struct IDirectMusic;
struct IDirectMusicPort;
struct IDirectMusicBuffer;
struct IReferenceClock;
struct IDirectMusicDownloadedInstrument;


//namespace AFL::DirectX::Music
namespace AFL{namespace DirectMusic{

#define MARK_LOOP "L"	//ループ用マーク

//------------------------------------------------------------
// MTimer
// マルチメディアタイマー代替用
//------------------------------------------------------------
class MTimer
{
public:
	MTimer();
	~MTimer();
	DWORD callProcess(LPVOID pvData);
	bool stop();
	bool start(DWORD dwTime);

	virtual void procTimer();
	bool isTimer()const;

protected:
	Thread m_thread;
	ClassProc* m_paflClassCallBack;
	AFL::WINDOWS::AdviseTimer m_timerAdvise;
	volatile bool m_bEnable;
	volatile bool m_bActive;
};

//------------------------------------------------------------
// MusicDevice
// MIDIデバイス制御用
//------------------------------------------------------------
#define MIDI_DIRECTMUSIC     ((UINT)-2)
class MusicDevice
{
public:
	MusicDevice();
	~MusicDevice();
	bool open(UINT uDeviceID = MIDI_MAPPER);
	bool close();
	bool out(LPMIDIHDR pMidiHdr)const;
	bool out(DWORD dwData)const;
	bool out(BYTE byData1,BYTE byData2,BYTE byData3)const;

	void getDeviceName(INT nIndex,LPSTR pString);
	INT getDeviceCount()const;
	bool isDevice()const;

	void setIdelTime(DWORD dwTime);
	bool setDLS(LPDWORD pData,INT nCount);
protected:
	bool openDirectMusic();
	bool closeDirectMusic();

	HMIDIOUT m_hMidi;
	UINT m_uDevice;
	DWORD m_dwIdelTime;

	IDirectMusic* m_pDirectMusic;
	IDirectMusicPort* m_pDirectMusicPort;
	IDirectMusicBuffer* m_pDirectMusicBufferShort;
	IReferenceClock *m_pReferenceClock;
	IDirectMusicDownloadedInstrument* m_pDirectMusicDownloadedInstrument;

};

/////////////////////////////////////////////////////////////////////////////
//     MExclusive
//---------------------------------------------------------
class MExclusive
{
public:
	MExclusive();
	~MExclusive();
	void set(INT nCount,LPBYTE pData);
	LPMIDIHDR get();
protected:
	MIDIHDR m_midiHDR;
};

//---------------------------------------------------------
//     SGpMidiCode
union UGpMidiData
{
	DWORD dwMsg;
	BYTE bbData[4];
};

//---------------------------------------------------------
//     SGpPlayInfo
//---------------------------------------------------------
struct SGpPlayInfo
{
	DWORD dwNowStep;
	WORD wRCount;
	DWORD dwTempo;
	LPCSTR pTitle;
	struct Channel
	{
		UGpMidiData midiMsg;
		BYTE bbNote[16];
		BYTE bbPC;
		BYTE bbCC;
		BYTE bbPan;
		BYTE bbRev;
		BYTE bbChos;
		BYTE bbVolume;
		BYTE bbVolume2;
	}Chn[16];
};

//---------------------------------------------------------
struct SGpMidiCode
{
	DWORD dwWait;
	UGpMidiData Mdf;
};
/////////////////////////////////////////////////////////////////////////////
//     MPlayData
//---------------------------------------------------------

class MPlayData
{
	struct STrackHeader{DWORD dwCheck;DWORD dwBytes;};
	struct SMidiHeader{DWORD dwCheck,dwBytes;WORD wFormat,wTracks;WORD wCount;};
public:
	/////////////////////////////////////////////////////////////////////////////
	//     CTrackData
	//---------------------------------------------------------
	class CTrackData
	{
	public:
		CTrackData();
		BYTE getStat();
		BYTE getData();
		DWORD setHeader(STrackHeader* pTHeader);
		DWORD getNextCount();
		void next();
		void setStep(INT nStep);
		bool isPlay()const;
		DWORD getWait();
		void setSkip();
		void setSkip(DWORD dwSkip);
		void init();
		DWORD getStep()const;
		DWORD getEStep()const;
		DWORD getMarkerSize()const;
		LPCSTR getString()const;
		LPBYTE getAdr();
	protected:	
		STrackHeader* m_pTHeader;
		DWORD m_dwWCount;
		BYTE m_bbStatBack;
		DWORD m_dwNextCount;
		LPBYTE m_pByte,m_pLastByte; 
		INT m_nStep,m_nEStep;
		INT m_nMarker;			//マーカー文字数
	};
public:
	MPlayData();
	~MPlayData();
	void releaseData();
	void init();
	bool read(const LPBYTE pData,INT nSize);
	LPCSTR getTitle()const;
	WORD getRCount()const;
	DWORD getNextCount()const;
	BYTE getCode()const;
	DWORD getDWord()const;
	BYTE getByte(INT iIndex)const;
	LPMIDIHDR getEData();
	WORD getDataWord()const;
	bool isPlay()const;
	void Next();
	void incCount();
	DWORDLONG getMarkLoopstart()const;
	DWORDLONG getNowCount()const;
	DWORDLONG getAllCount()const;
	DWORD getAllTime()const;
	bool isData()const;
	LPSTR getMarker(INT nPoint);

	INT getDLSCount()const;
	LPDWORD getDLS();
protected:
	void track(STrackHeader** ppTHeader,INT nTCount);
	void setTracks(CTrackData* pTData,INT nTracks);
	WORD m_wRCount;

	LPSTR m_pTitle;
	LPSTR m_pMarker;
	DWORD m_dwStep;
	SGpMidiCode* m_pMCode;
	MExclusive* m_pECode;
	DWORD m_dwPoint;
	DWORDLONG m_dwlCount;
	DWORDLONG m_dwlAllCount;
	DWORDLONG m_dwlWorkCount;
	DWORDLONG m_dwlMarkLoop;
	DWORD m_dwAllTime;
	DWORD m_dwTempoCount;

	LPDWORD m_pDLSList;
	INT m_nDLSCount;
};



/////////////////////////////////////////////////////////////////////////////
// Music
class Music : public MTimer
{
public:
	struct SRelativeData
	{
		FLOAT fVolume;
		INT nVolumeWork;
		INT nVolume;
		INT nTemp;
		INT nPan;
		INT nVelocity;
		INT nReverb;
		INT nChorus;
		CHAR cKey;
	};
	enum
	{
		RESET_GS,RESET_GM,RESET_GM2
	};
	//---------------------------
	// コンスト／デスト
	Music();
	~Music();
	
	//---------------------------
	// データ系
	bool open(LPCSTR pFileName,LPCSTR pType=NULL);
	bool close();
	bool isData();
	
	//---------------------------
	// 演奏系
	bool fadeIn(INT nTime,INT nVolume=-70);
	bool fadeOut(INT nTime);
	bool playTime(DWORD dwTime);
	bool play(DWORDLONG dwlCount);
	bool play();
	bool cont();
	bool stop();
	bool isPlay()const;
	bool outMidiData(DWORD dwData);
	bool outMidiData(BYTE byData1,BYTE byData2,BYTE byData3);

	//---------------------------
	// 演奏制御系
	void setLoop(bool bLoop);
	void setLoopTop(DWORDLONG dwlLoopSearch);
	void setLoopTopTime(DWORD dwLoopSearch);
	void setTemp(INT nRelativeTempo);
	void setRelativeTemp(INT nRelativeTempo);
	void setRelativeKey(CHAR cKey);
	void setRelativeVelocity(INT nVelo);
	void setRelativeVolume(INT nVolume);
	void setRelativePan(INT nPan);
	void setRelativeChorus(INT nChorus);
	void setRelativeReverb(INT nReverb);

	//---------------------------
	// 演奏状態系
	LPCSTR getTitle()const;
	SGpPlayInfo* getPlayInfo();
	WORD getTimeBase()const;
	WORD getRCount()const;
	WORD getTempo()const;
	WORD getTemp()const;
	DWORDLONG getAllCount()const;
	DWORDLONG getPlayCount()const;
	DWORDLONG getAllTime()const;
	DWORDLONG getPlayTime()const;
	INT getEventFlag();

	virtual void callbackMarkerEvent(LPCSTR pString);

	//---------------------------
	// デバイス設定系
	bool setDevice(INT nIndex=MIDI_MAPPER);
	INT getDeviceCount();
	void getDeviceName(INT nIndex,LPSTR pString);
	bool sendReset(INT nMode=RESET_GS);
	bool openDevice();
	bool closeDevice();
protected:
	bool insideOpenDevice();
	void procTimer();
	bool isResetWait();
	UINT getTimerID()const;
	void setTimeCount();
	DWORD getTimeCount();
	void setTimeCountBase();
	DWORD getTimeCountBase();

	bool isPlayout()const;
	bool stdMidi(const LPBYTE pData,INT nSize);
	bool insideSetTimer();
	void insideInit();
	void insideComeback();
	void insideSetNoteOff();
	void insideSetVolume(INT nVolume=0);
	void insideSetPan(INT nSData=64);
	void insideSetReverb(INT nSData=0);
	void insideSetChorus(INT nSData=0);
	void insidePlay();
	void insideMidiout();

	volatile bool m_bPlayOut;
	volatile bool m_bStopFlag;
	INT m_nTimeCycle;
	INT m_nWaitCycle;
	DWORD m_timePlay;
	DWORD m_timePlayBase;
	DWORD m_dwTempCount;
	DWORD m_dwTempChangeCount;
	DWORD m_dwNowTime;
	DWORDLONG m_dwlCountTime;

	INT m_nEventFlag;
	DWORD m_dwResetCount;
	DWORD m_dwTempParam;
	INT m_nFadeVolume;

	SRelativeData m_Relative;
	SGpPlayInfo m_playInfo;

	INT m_nFadeCount;
	INT m_nFadeCountWork;
	bool m_bFadeIn,m_bFadeOut;
	bool m_bThrough;
	bool m_bThroughSkip;
	bool m_bLoop;
	DWORDLONG m_dwlLoopSearch;
	DWORD m_dwLoopSearch;
	UINT m_uDevice;
	MPlayData m_MidiData;
	MusicDevice m_MDevice;
};

//namespace
}}

#define __INC_AFLMUSIC
#endif	// __INC_AFLMUSIC
