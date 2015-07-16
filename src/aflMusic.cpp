#include <windows.h>
#include <mmsystem.h>
#include <fstream>

#include "aflMusic.h"

/*
#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define new new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtsetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 
*/
  
#ifdef _USE_DMUSIC
	#include <dmusicc.h>
	#include <dmusici.h>
#endif //_USE_DMUSIC




#ifdef _MSC_VER
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "dxguid.lib")
#endif //_MSC_VER

//namespace AFL::DirectMusic
namespace AFL{namespace DirectMusic{

//---------------------------------------------------------
//     ダブルワード変換
//---------------------------------------------------------
inline static DWORD protConvDW(DWORD dwData)
{
	DWORD dwWork;
	((char*)&dwWork)[0] = ((char*)&dwData)[3];
	((char*)&dwWork)[1] = ((char*)&dwData)[2];
	((char*)&dwWork)[2] = ((char*)&dwData)[1];
	((char*)&dwWork)[3] = ((char*)&dwData)[0];
	return dwWork;
}
inline static DWORD protgetWaitCount(LPBYTE* ppData)
{
	BYTE bbData;
	DWORD dwData = 0;
	do
	{
		bbData = **ppData;
		++*ppData;
		dwData = (dwData<<7) + (bbData&0x7f);
	}while(bbData&0x80);
	return dwData;
}

//---------------------------------------------------------
//     ワード変換
//---------------------------------------------------------
inline static WORD protConvW(WORD wData)
{
	WORD wWork;
	((char*)&wWork)[0] = ((char*)&wData)[1];
	((char*)&wWork)[1] = ((char*)&wData)[0];
	return wWork;
}


//------------------------------------------------------------
// MTimer
// マルチメディアタイマー代替用
//------------------------------------------------------------
MTimer::MTimer()
{
	m_bActive = false;
	m_bEnable = true;
	m_thread.startThread(CLASSPROC(this,MTimer,callProcess),0);
}
MTimer::~MTimer()
{
	m_bEnable = false;
	m_timerAdvise.release();
}
DWORD MTimer::callProcess(LPVOID pvData)
{
	SetThreadPriority(m_thread.getThreadHandle(),THREAD_PRIORITY_TIME_CRITICAL);
	SetThreadAffinityMask(m_thread.getThreadHandle(),1);
	while(m_bEnable)
	{
		m_timerAdvise.waitTimer();
		procTimer();
	}
	return 0;
}
bool MTimer::stop()
{
	m_bActive = false;
	return m_timerAdvise.stopTimer();
}
bool MTimer::start(DWORD dwTime)
{
	m_timerAdvise.startTimer(dwTime*10000);
	m_bActive = true;
	return 0;
}
void MTimer::procTimer()
{
}
bool MTimer::isTimer()const
{
	return m_bActive;
}


/////////////////////////////////////////////////////////////////////////////
//     MExclusive
//---------------------------------------------------------
MExclusive::MExclusive()
{
	ZeroMemory(&m_midiHDR,sizeof(MIDIHDR));
}
MExclusive::~MExclusive()
{
	if(m_midiHDR.lpData)
		delete m_midiHDR.lpData;
}
void MExclusive::set(INT iCount,LPBYTE pData)
{
	m_midiHDR.lpData = new char[iCount+1];
	m_midiHDR.lpData[0] = (char)0xf0;
	CopyMemory(&m_midiHDR.lpData[1],pData,iCount);
	m_midiHDR.dwFlags = 0;
	m_midiHDR.dwBufferLength = iCount+1;
}
LPMIDIHDR MExclusive::get()
{
	return &m_midiHDR;
}

/////////////////////////////////////////////////////////////////////////////
// MusicDevice
// MIDIデバイス制御用
//------------------------------------------------------------
bool MusicDevice::closeDirectMusic()
{
#ifdef _USE_DMUSIC
	if(!m_pDirectMusic)
		return false;

	if(m_pDirectMusicBufferShort)
		m_pDirectMusicBufferShort->Release();
	if(m_pReferenceClock)
		m_pReferenceClock->Release();
	if(m_pDirectMusicDownloadedInstrument)
		m_pDirectMusicDownloadedInstrument->Release();
	if(m_pDirectMusicPort)
		m_pDirectMusicPort->Release();
	if(m_pDirectMusic)
		m_pDirectMusic->Release();
	m_pDirectMusic = NULL;
	m_pDirectMusicPort = NULL;
	m_pDirectMusicDownloadedInstrument = NULL;
	m_pDirectMusicBufferShort = NULL;
	m_pReferenceClock = NULL;

//	::CoUninitialize();
	return true;
#else
	return false;
#endif
}
bool MusicDevice::openDirectMusic()
{
#ifdef _USE_DMUSIC
	if(m_pDirectMusic)
	{
		m_hMidi = (HMIDIOUT)-1;
		return true;
	}
	//COMを利用可能に
	::CoInitializeEx(NULL,COINIT_MULTITHREADED);
	//IDirectMusicを利用可能に
	m_pDirectMusic = NULL;
	if(::CoCreateInstance(CLSID_DirectMusic,NULL,CLSCTX_INPROC_SERVER,
		IID_IDirectMusic,(LPVOID*)&m_pDirectMusic) != S_OK)
		return false;
	 

	if(m_pDirectMusic)
	{
		//DLS用DirectSoundの設定
		m_pDirectMusic->SetDirectSound( NULL,NULL);

		//デフォルトポートの取得
		DMUS_PORTCAPS portCaps;
		m_pDirectMusic->GetDefaultPort( &portCaps.guidPort );
		
		//ポートの作成
		DMUS_PORTPARAMS portParams;
		ZeroMemory( &portParams , sizeof( DMUS_PORTPARAMS ) );
		portParams.dwSize = sizeof( DMUS_PORTPARAMS );
		m_pDirectMusic->CreatePort( portCaps.guidPort , &portParams , &m_pDirectMusicPort , NULL ); 
		//バッファの作成
		DMUS_BUFFERDESC descBuff;
		descBuff.dwSize = sizeof( DMUS_BUFFERDESC );
		descBuff.dwFlags = 0;
		descBuff.guidBufferFormat = GUID_NULL;
		descBuff.cbBuffer = sizeof(DMUS_EVENTHEADER) + 4;
		m_pDirectMusic->CreateMusicBuffer( &descBuff , &m_pDirectMusicBufferShort , NULL ); 
		
		//ポートクロックの取得と動作許可
		if(m_pDirectMusicPort)	
		{
			m_pDirectMusicPort->GetLatencyClock( &m_pReferenceClock ); 
			m_pDirectMusic->Activate(true);
		}
	}
		m_hMidi = (HMIDIOUT)-1;
	return true;
#else
	return false;
#endif
}
bool MusicDevice::setDLS(LPDWORD pData,INT iCount)
{
#ifdef _USE_DMUSIC
	if(openDirectMusic())
	{
		//PINT pData  必要な音色リスト
		//INT  iCount 音色リストの数

		//コレクションから音色の読み出し
		IDirectMusicLoader* pDirectMusicLoader;
		IDirectMusicCollection* pDirectMusicCollection;
		
		//IDirectMusicLoaderを利用可能に
		::CoCreateInstance( CLSID_DirectMusicLoader , NULL , CLSCTX_INPROC_SERVER , IID_IDirectMusicLoader , (LPVOID*)&pDirectMusicLoader);
		
		if(pDirectMusicLoader)
		{
			//デフォルト音色の設定
			DMUS_OBJECTDESC desc; 
			ZeroMemory(&desc, sizeof(desc));
			desc.dwSize        = sizeof(DMUS_OBJECTDESC);
			desc.guidObject    = GUID_DefaultGMCollection;
			desc.guidClass     = CLSID_DirectMusicCollection;
			desc.dwValidData   = (DMUS_OBJ_CLASS | DMUS_OBJ_OBJECT);
			pDirectMusicLoader->GetObject(&desc, IID_IDirectMusicCollection,
				(void **)&pDirectMusicCollection);

			if(pDirectMusicCollection)
			{
				INT i;
				IDirectMusicInstrument* pDirectMusicInstrument;
				
				//通常音色の読み出し
				for(i=0;i<iCount;i++)
				{
					//DLSから音色の読み出し
					pDirectMusicInstrument = NULL;
					pDirectMusicCollection->GetInstrument(pData[i], &pDirectMusicInstrument);
					//音色が存在しなかったら
					if(!pDirectMusicInstrument)
					{
						//バリエーションをマスクして、代替音色の設定
						pDirectMusicCollection->GetInstrument(pData[i]&0xFF0000FF,
							&pDirectMusicInstrument);
						if(!pDirectMusicInstrument)
							continue;
						//代替用に音色番号の変更
						pDirectMusicInstrument->SetPatch(pData[i]);
					}
					//ポートに音色を関連づけ
					m_pDirectMusicPort->DownloadInstrument(pDirectMusicInstrument,
						&m_pDirectMusicDownloadedInstrument,NULL,0); 
					pDirectMusicInstrument->Release();
				}
				//念のためデフォルトのドラムセットだけは強制設定
				pDirectMusicCollection->GetInstrument(0x80000000, &pDirectMusicInstrument);
				m_pDirectMusicPort->DownloadInstrument(pDirectMusicInstrument,
					&m_pDirectMusicDownloadedInstrument,NULL,0); 
				
				//使い終わったコンポーネントの解放
				//pDirectMusicInstrument->Release();
				pDirectMusicCollection->Release();
				//音色用のメモリを最適化
				m_pDirectMusicPort->Compact();
			}
			pDirectMusicLoader->Release();
		}
	}
	return true;
#else
	return false;
#endif
}

//-------------------------------
//MIDI Device コンストラクタ
MusicDevice::MusicDevice()
{
	m_hMidi = 0;
	m_uDevice = 0;

	m_pDirectMusic = NULL;
	m_pDirectMusicPort = NULL;
	m_pDirectMusicDownloadedInstrument = NULL;
	m_pDirectMusicBufferShort = NULL;
	m_pReferenceClock = NULL;

	m_dwIdelTime = 0;
}
//-------------------------------
//MIDI Device デストラクタ
MusicDevice::~MusicDevice()
{
	close();
	closeDirectMusic();
}
//-------------------------------
//MIDI Device 開く
bool MusicDevice::open(UINT uDeviceID)
{
	if(m_hMidi && m_uDevice == uDeviceID)
		return true;
	close();
	m_uDevice = uDeviceID;

	if(uDeviceID == MIDI_DIRECTMUSIC)
		return openDirectMusic();
	else
		return midiOutOpen(&m_hMidi,uDeviceID,NULL,NULL,CALLBACK_NULL) == MMSYSERR_NOERROR;
}
//-------------------------------
//MIDI Device 閉じる
bool MusicDevice::close()
{
	if(!m_hMidi)
		return false;
	if(m_hMidi == (HMIDIOUT)-1)
	{}//closeDirectMusic();
	else
	{
		midiOutReset(m_hMidi);
		if(m_hMidi)
			midiOutClose(m_hMidi);
	}
	m_hMidi = 0;
	return true;
}
//-------------------------------
//MIDI Device ロングメッセージの送信
bool MusicDevice::out(LPMIDIHDR pMidiHdr) const
{
	if(!m_hMidi)
		return false;
	if(m_hMidi == (HMIDIOUT)-1)
	{
#ifdef _USE_DMUSIC
		REFERENCE_TIME refTime;
		m_pReferenceClock->GetTime( &refTime );
	
		IDirectMusicBuffer* pDirectMusicBufferLong;
		DMUS_BUFFERDESC descBuff;
		descBuff.dwSize = sizeof( DMUS_BUFFERDESC );
		descBuff.dwFlags = 0;
		descBuff.guidBufferFormat = GUID_NULL;
		descBuff.cbBuffer = sizeof(DMUS_EVENTHEADER) + pMidiHdr->dwBufferLength;
		m_pDirectMusic->CreateMusicBuffer(&descBuff,(IDirectMusicBuffer**)&pDirectMusicBufferLong,NULL); 
		pDirectMusicBufferLong->PackUnstructured(refTime,0,pMidiHdr->dwBufferLength,(PBYTE)pMidiHdr->lpData); 
		m_pDirectMusicPort->PlayBuffer(pDirectMusicBufferLong);
		pDirectMusicBufferLong->Flush(); 
		pDirectMusicBufferLong->Release();
		return true;
#else
		return false;
#endif
	}
	else
	{
		bool bFlag;
		midiOutPrepareHeader(m_hMidi,pMidiHdr,sizeof(MIDIHDR));
		bFlag = midiOutLongMsg(m_hMidi,pMidiHdr,sizeof(MIDIHDR)) == MMSYSERR_NOERROR;
		midiOutUnprepareHeader(m_hMidi,pMidiHdr,sizeof(MIDIHDR));
		return bFlag;
	}
}
//-------------------------------
//MIDI Device ショートメッセージの送信
bool MusicDevice::out(DWORD dwData) const
{
	if(!m_hMidi)
		return false;
	if(m_hMidi == (HMIDIOUT)-1)
	{
#ifdef _USE_DMUSIC
		//m_pReferenceClock IDirectMusicPortから取得した延滞付きクロック
		//m_pDirectMusicBufferShort IDirectMusicBufferで4バイト確保したもの
		//m_pDirectMusicPort IDirectMusicPortの出力先

		//延滞付き時間の取得
		REFERENCE_TIME refTime;
		m_pReferenceClock->GetTime( &refTime );
		//バッファにメッセージを設定
		m_pDirectMusicBufferShort->PackStructured( refTime + m_dwIdelTime, 0 , dwData); 
		//ポートにバッファのデータを送信
		m_pDirectMusicPort->PlayBuffer( m_pDirectMusicBufferShort );
		//バッファ内のデータをフラッシュ
		m_pDirectMusicBufferShort->Flush(); 
		return true;
#else
		return false;
#endif
	}
	else
	{
		return midiOutShortMsg(m_hMidi,dwData) == MMSYSERR_NOERROR;
	}
}
//-------------------------------
//MIDI Device ショートメッセージの送信
bool MusicDevice::out(BYTE byData1,BYTE byData2,BYTE byData3) const
{
	UGpMidiData OutWork;
	OutWork.bbData[0] = byData1;
	OutWork.bbData[1] = byData2;
	OutWork.bbData[2] = byData3;
	OutWork.bbData[3] = 0;
	return out(OutWork.dwMsg) == MMSYSERR_NOERROR;
}
//-------------------------------
//MIDI Device デバイス名を得る
void MusicDevice::getDeviceName(INT nIndex,LPSTR pString)
{
	MIDIOUTCAPSA midiCaps;
	midiOutGetDevCapsA(nIndex,&midiCaps,sizeof(midiCaps));
	strcpy(pString,midiCaps.szPname);
}

INT MusicDevice::getDeviceCount()const
{
	return midiOutGetNumDevs();
}
bool MusicDevice::isDevice()const
{
	return m_hMidi!=0;
}

void MusicDevice::setIdelTime(DWORD dwTime)
{
	m_dwIdelTime=dwTime;
}

/////////////////////////////////////////////////////////////////////////////
// MPlayData::CTrackData
// .midファイル読み出し用
MPlayData::CTrackData::CTrackData()
{
	m_dwWCount = 0;
	m_nStep = 0;
}
BYTE MPlayData::CTrackData::getData()
{
	BYTE bbByte = *m_pByte;
	m_pByte++;
	return bbByte;
}
DWORD MPlayData::CTrackData::getNextCount()
{
	return m_dwNextCount;
}
void MPlayData::CTrackData::next()
{
	m_dwNextCount += getWait();
}
void MPlayData::CTrackData::setStep(INT nStep)
{
	m_dwNextCount-=nStep;
}
bool MPlayData::CTrackData::isPlay()const
{
	return m_pByte<m_pLastByte-1;
}
DWORD MPlayData::CTrackData::getWait()
{
	return protgetWaitCount(&m_pByte);
}
void MPlayData::CTrackData::setSkip()
{
	m_pByte += getWait();
}
void MPlayData::CTrackData::setSkip(DWORD dwSkip)
{
	m_pByte += dwSkip;
}
void MPlayData::CTrackData::init()
{
	m_dwNextCount = 0;
	m_pByte = (LPBYTE)m_pTHeader+8;
	m_pLastByte=m_pByte+protConvDW(m_pTHeader->dwBytes);
}
DWORD MPlayData::CTrackData::getStep()const
{
	return m_nStep;
}
DWORD MPlayData::CTrackData::getEStep()const
{
	return m_nEStep;
}
DWORD MPlayData::CTrackData::getMarkerSize()const
{
	return m_nMarker;
}
LPCSTR MPlayData::CTrackData::getString() const
{
	return (LPCSTR)m_pByte;
}
LPBYTE MPlayData::CTrackData::getAdr()
{
	return m_pByte;
}




//-----------------------------------------------------
//	ステータス読みだし
//-----------------------------------------------------
BYTE MPlayData::CTrackData::getStat()
{
	BYTE bbStat = *m_pByte;
	if(bbStat < (BYTE)0x80)
		bbStat = m_bbStatBack;
	else
	{
		m_bbStatBack = bbStat;
		++m_pByte;
	}
	return bbStat;
}

//-----------------------------------------------------
//	ヘッダー読みだし
//-----------------------------------------------------
DWORD MPlayData::CTrackData::setHeader(STrackHeader* pTHeader)
{	
	BYTE bbStat;
	m_nStep = 0;
	m_nEStep = 0;
	m_dwWCount = 0;
	m_nMarker = 0;
	m_pTHeader = pTHeader;
	init();
	
	while(isPlay())
	{
		m_dwWCount += getWait();
		bbStat = getStat();
		if((bbStat&0xf0) == 0xc0 || (bbStat&0xf0) == 0xd0)
			m_pByte++;
		else if(bbStat == 0xfe)
			continue;
		else if(bbStat == 0xff)
		{
			if(*m_pByte == 0x51)
				m_nStep+=2;
			else if(*m_pByte == 0x06)
				m_nMarker += m_pByte[1] + 1;
			m_pByte++;
			m_pByte += getWait();
		}
		else if(bbStat == 0xf0)
		{
			m_nEStep++;
			m_pByte += getWait();
		}
		else
			m_pByte += 2;
		m_nStep++;
	}
	return m_dwWCount;
}


/////////////////////////////////////////////////////////////////////////////
// MPlayData
// 演奏データ用

//-----------------------------------------------------
//	コンストラクタ
//-----------------------------------------------------
MPlayData::MPlayData()
{
	m_pMCode = NULL;
	m_pECode = NULL;
	m_pTitle = NULL;
	m_pMarker = NULL;
	m_dwPoint = 0;
	m_dwlMarkLoop = 0;
	m_pDLSList = NULL;
	m_nDLSCount = 0;
}
//-----------------------------------------------------
//	デストラクタ
//-----------------------------------------------------
MPlayData::~MPlayData()
{
	releaseData();
}

//-----------------------------------------------------
//	データの解放
//-----------------------------------------------------
void MPlayData::releaseData()
{
	m_dwPoint = 0;
	m_wRCount = 0;
	if(m_pTitle)
		delete m_pTitle;
	m_pTitle = NULL;
	if(m_pMCode)
		delete[] m_pMCode;
	m_pMCode = NULL;
	if(m_pECode)
		delete[] m_pECode;
	m_pECode = NULL;
	if(m_pMarker)
		delete[] m_pMarker;
	m_pMarker = NULL;
	if(m_pDLSList)
		delete[] m_pDLSList;
	m_pDLSList = NULL;
	m_nDLSCount = 0;
}

//-----------------------------------------------------
//	データファイルの読み出し
//-----------------------------------------------------
bool MPlayData::read(const LPBYTE pData,INT nSize)
{
	INT i;
	const SMidiHeader* pFormat;
	INT nHSize;
	INT wTracks;

	releaseData();								//既存のデータの解放

	pFormat = (const SMidiHeader*)pData;		//フォーマットの設定
	nHSize =  protConvDW(pFormat->dwBytes);
	wTracks = protConvW(pFormat->wTracks);
	m_wRCount = protConvW(pFormat->wCount);
	m_dwTempoCount = 0;
	m_dwlAllCount = 0;
	m_dwlWorkCount = 0;
	STrackHeader** ppTHeader = new STrackHeader*[wTracks];
	ppTHeader[0] = (STrackHeader*)(pData + 8 + nHSize);

	DWORD dwData;
	for(i=1;i<wTracks;i++)
	{
		dwData = protConvDW(ppTHeader[i-1]->dwBytes)+8;
		if((INT)dwData > nSize)					//途中でデータの終端が来たら異常終了
		{
			delete[] ppTHeader;
			return false;
		}
		//データ領域の設定
		ppTHeader[i] = (STrackHeader*)((LPBYTE)ppTHeader[i-1] + dwData);
	}
	
	dwData = protConvDW(ppTHeader[i-1]->dwBytes)+8;
	if((INT)dwData > nSize)
	{
		delete[] ppTHeader;
		return false;
	}
	
	track(ppTHeader,wTracks);
	delete[] ppTHeader;
	return true;
}

//-----------------------------------------------------
//	トラックごとの読み出し
//-----------------------------------------------------
void MPlayData::track(STrackHeader** ppTHeader,INT nTCount)
{
	INT i;
	CTrackData* pTData = new CTrackData[nTCount];
	for(i=0;i<nTCount;i++)
		pTData[i].setHeader(ppTHeader[i]);
	setTracks(pTData,nTCount);
	delete[] pTData;
}

//-----------------------------------------------------
//	トラックデータの併合
//-----------------------------------------------------
void MPlayData::setTracks(CTrackData* pTData,INT nTracks)
{
	INT i;
	bool bFlag = true;;
	DWORD dwStep = 0;
	DWORD dwExStep = 0;
	DWORD dwNowStep = 0;
	DWORD dwNowEStep = 0;
	INT nMarkerSize = 0;
	INT nMarkerPoINT = 0;

	for(i=0;i<nTracks;i++)
	{
		pTData[i].init();
		pTData[i].next();
		dwStep += pTData[i].getStep();				//通常データのサイズ
		dwExStep += pTData[i].getEStep();			//エクスクルーシブのサイズ
		nMarkerSize += pTData[i].getMarkerSize();	//マーカの文字サイズ
	}
	m_dwAllTime = 0;
	m_dwTempoCount = 60000000/120;
	m_dwStep = dwStep;
	if(dwExStep)
		m_pECode = new MExclusive[dwExStep];
	m_pMCode = new SGpMidiCode[dwStep+1000];
	if(nMarkerSize)
		m_pMarker = new CHAR[nMarkerSize];
	ZeroMemory(m_pMCode,sizeof(SGpMidiCode)*dwStep);

	BYTE byCCL[16],byCCH[16];
	ZeroMemory(byCCL,sizeof(byCCL));
	ZeroMemory(byCCH,sizeof(byCCH));

	INT nDLSCount = 2;
	PINT pDLS = new INT[10000];	//音色バッファ
	ZeroMemory(pDLS,sizeof(INT)*10000);
	pDLS[1] = 0x80000000;

	while(bFlag)
	{
		bFlag = false;
		DWORD dwNextCount = 0xffffffff;
		//Trackから次に取り出すデータまでのカウンタを取得
		for(i=0;i<nTracks;i++)
		{
			if(pTData[i].isPlay())
			{
				bFlag = true;
				if(pTData[i].getNextCount() < dwNextCount)
					dwNextCount = pTData[i].getNextCount();
			}
		}
		m_pMCode[dwNowStep].dwWait = dwNextCount;	//カウンタを入れる
		if(dwNextCount != 0xffffffff)
			m_dwlAllCount += dwNextCount;
		
		//Trackデータを併合
		for(i=0;i<nTracks;i++)
		{
			if(pTData[i].isPlay())
			{
				if(dwNextCount == pTData[i].getNextCount())
				{
					BYTE bbStat = pTData[i].getStat();
					m_pMCode[dwNowStep].Mdf.bbData[0] = bbStat; 
					if((bbStat&0xf0) == 0xc0 || (bbStat&0xf0) == 0xd0)
					{
						m_pMCode[dwNowStep].Mdf.bbData[1] = pTData[i].getData();
						
						if((bbStat&0xf0) == 0xc0)	//音色抽出用
						{
							INT nCH = bbStat&0x0f;
							INT nDLS = (byCCH[nCH]<<16) + (byCCL[nCH]<<8) + m_pMCode[dwNowStep].Mdf.bbData[1];
							if(nCH == 9)
								nDLS += 0x80000000;
							INT j;
							for(j=0;j<nDLSCount && pDLS[j] != nDLS && j<10000;j++);
							if(j == nDLSCount)
							{
								pDLS[j] = nDLS;
								nDLSCount++;
							}
						}
						
					}
					else if(bbStat == 0xfe)
						continue;
					else if(bbStat == 0xf0)
					{
						DWORD dwCount = pTData[i].getWait();
						m_pECode[dwNowEStep].set(dwCount,pTData[i].getAdr());
						*((LPWORD)&m_pMCode[dwNowStep].Mdf.bbData[1]) = (WORD)dwNowEStep;
						pTData[i].setSkip(dwCount);
						dwNowEStep++;
					}
					else if(bbStat == 0xff)
					{
						BYTE bbData = pTData[i].getData();
						DWORD dwCount = pTData[i].getWait();
						m_pMCode[dwNowStep].Mdf.bbData[1] = bbData; 
						if(bbData==0x51)			//テンポ
						{
							DWORD dwTCount=0;
							for(INT j=0;j<(INT)dwCount;j++)
								dwTCount += (dwTCount<<8)+pTData[i].getData();
							m_dwTempoCount = dwTCount;
							dwNowStep++;
							*((LPWORD)&m_pMCode[dwNowStep].Mdf.bbData[1]) = ((LPWORD)&dwTCount)[0];
							dwNowStep++;
							*((LPWORD)&m_pMCode[dwNowStep].Mdf.bbData[1]) = ((LPWORD)&dwTCount)[1];
							m_dwAllTime += DWORD((m_dwlAllCount - m_dwlWorkCount)*m_dwTempoCount/(1000*getRCount()));
							m_dwlWorkCount = m_dwlAllCount;
						}
						else
						{
							m_pMCode[dwNowStep].Mdf.bbData[0] = 0;	//SkipData
							if(i == 0)
							{
								if(bbData == 3)						//タイトル
								{
									if(m_pTitle)
										delete m_pTitle;
									m_pTitle = new CHAR[dwCount+1];
									strncpy(m_pTitle,(LPCSTR)pTData[i].getString(),dwCount);
									m_pTitle[dwCount] = 0;
								}
								else if(bbData == 6)				//マーカ
								{
									strncpy(m_pMarker+nMarkerPoINT,(LPCSTR)pTData[i].getString(),dwCount);
									m_pMarker[nMarkerPoINT + dwCount] = 0;

									if(!strcmp(m_pMarker+nMarkerPoINT,MARK_LOOP))
										m_dwlMarkLoop = m_dwlAllCount;
									m_pMCode[dwNowStep].Mdf.bbData[0] = 0xff;
									m_pMCode[dwNowStep].Mdf.bbData[1] = 6;
									*(LPWORD)(m_pMCode[dwNowStep].Mdf.bbData +2) = (WORD)nMarkerPoINT;
									nMarkerPoINT += dwCount + 1;
								}
							}
							
							pTData[i].setSkip(dwCount);
						}
					}
					else
					{
						m_pMCode[dwNowStep].Mdf.bbData[1] = pTData[i].getData();
						m_pMCode[dwNowStep].Mdf.bbData[2] = pTData[i].getData();
						
						if((bbStat&0xf0) == 0xb0)	//音色抽出用
						{
							if(m_pMCode[dwNowStep].Mdf.bbData[1] == 0)
								byCCH[bbStat&0x0f] = m_pMCode[dwNowStep].Mdf.bbData[2];
							else if(m_pMCode[dwNowStep].Mdf.bbData[1] == 0x20)
								byCCL[bbStat&0x0f] = m_pMCode[dwNowStep].Mdf.bbData[2];
						}
					}
					dwNowStep++;
					if(pTData[i].isPlay())
						pTData[i].next();
				}
				pTData[i].setStep(dwNextCount);
			}
		}
	}
	
	//音色配列の設定
	m_pDLSList = new DWORD[nDLSCount];
	CopyMemory(m_pDLSList,pDLS,sizeof(INT)*nDLSCount);
	m_nDLSCount = nDLSCount;
	delete[] pDLS;

	m_dwStep = dwNowStep;
	m_dwAllTime += DWORD((m_dwlAllCount - m_dwlWorkCount)*m_dwTempoCount/(1000*getRCount()));
}

//---------------------------------------------------------
//     
//---------------------------------------------------------
void MPlayData::init()
{
	m_dwlCount=0;
	m_dwPoint = 0;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
LPCSTR MPlayData::getTitle()const
{
	return m_pTitle;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD MPlayData::getRCount()const
{
	return m_wRCount;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORD MPlayData::getNextCount()const
{
	return m_pMCode[m_dwPoint].dwWait;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
BYTE MPlayData::getCode()const
{
	return m_pMCode[m_dwPoint].Mdf.bbData[0];
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORD MPlayData::getDWord()const
{
	return m_pMCode[m_dwPoint].Mdf.dwMsg;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
BYTE MPlayData::getByte(INT iIndex)const
{
	return m_pMCode[m_dwPoint].Mdf.bbData[iIndex];
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
LPMIDIHDR MPlayData::getEData()
{
	return m_pECode[getDataWord()].get();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD MPlayData::getDataWord()const
{
	return *(LPWORD)(&m_pMCode[m_dwPoint].Mdf.bbData[1]);
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool MPlayData::isPlay()const
{
	return (m_dwPoint < m_dwStep);
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
void MPlayData::incCount()
{
	m_dwlCount++;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
void MPlayData::Next()
{
	m_dwPoint++;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG MPlayData::getMarkLoopstart()const
{
	return m_dwlMarkLoop;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG MPlayData::getNowCount()const
{
	return m_dwlCount;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG MPlayData::getAllCount()const
{
	return m_dwlAllCount;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORD MPlayData::getAllTime()const
{
	return m_dwAllTime;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool MPlayData::isData()const
{
	return m_pMCode!=0;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
LPSTR MPlayData::getMarker(INT nPoint)
{
	return m_pMarker + nPoint;
}

//---------------------------------------------------------
//     
//---------------------------------------------------------
INT MPlayData::getDLSCount()const
{
	return m_nDLSCount;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
LPDWORD MPlayData::getDLS()
{
	return m_pDLSList;
}

/////////////////////////////////////////////////////////////////////////////
// Music

//---------------------------------------------------------
//     コンストラクタ
//---------------------------------------------------------
Music::Music()
{
	m_uDevice = MIDI_MAPPER;
	m_bFadeIn = m_bFadeOut = false;
	m_bPlayOut = false;
	m_bLoop = true;
	m_bThrough = false;
	ZeroMemory(&m_Relative,sizeof(m_Relative));

	m_dwLoopSearch = 0;
	m_dwlLoopSearch = 0;
	m_Relative.cKey = 0;
	m_dwResetCount = 0;
}
//---------------------------------------------------------
//     デストラクタ
//---------------------------------------------------------
Music::~Music()
{
	stop();
}

//---------------------------------------------------------
//     ファイル読み出し
//---------------------------------------------------------
bool Music::open(LPCSTR pName,LPCSTR pType)
{
	stop();
	std::fstream file;

	file.open(pName,std::ios_base::in|std::ios_base::binary);
	if(!file.is_open())
		return false;
	file.seekg(0,std::ios::end);
	INT nLength = (INT)file.tellg();
	if(!nLength)
		return false;
	file.seekg(0);
	LPBYTE pData = new BYTE[nLength];
	file.read((PCHAR)pData,nLength);
	bool bFlag = stdMidi(pData,nLength);
	delete[] pData;
	
	if(!bFlag)
		return false;

	ZeroMemory(&m_playInfo,sizeof(SGpPlayInfo));
	m_playInfo.pTitle = getTitle();
	m_playInfo.dwTempo = 250;
	m_dwTempParam = 60000000 / m_playInfo.dwTempo;
	m_bFadeIn = m_bFadeOut = false;

	m_dwlLoopSearch = m_MidiData.getMarkLoopstart();

	if(m_uDevice == MIDI_DIRECTMUSIC)
		m_MDevice.setDLS(m_MidiData.getDLS(),m_MidiData.getDLSCount());

	return true;
}

//---------------------------------------------------------
//     データの解放
//---------------------------------------------------------
bool Music::close()
{
	if(!isData())
		return false;
	stop();
	m_MidiData.releaseData();
	return true;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::isData()
{
	return m_MidiData.isData();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::sendReset(INT nMode)
{
	static BYTE byGs[] = {0x41,0x10,0x42,0x12,0x40,0x00,0x7f,0x00,0x41,0xf7};
	static BYTE byGm[] = {0x7e,0x7f,0x09,0x01,0xf7};
	static BYTE byGm2[] = {0x7e,0x7f,0x09,0x03,0xf7};
	stop();
	MExclusive Exclusive;
	m_MDevice.open(m_uDevice);
	switch(nMode)
	{
	case RESET_GS:
		Exclusive.set(10,byGs);
		break;
	case RESET_GM:
		Exclusive.set(5,byGm);
		break;
	case RESET_GM2:
		Exclusive.set(5,byGm2);
		break;
	}
	m_MDevice.out(Exclusive.get());
	m_MDevice.close();
	m_dwResetCount = timeGetTime();
	return true;
}

//---------------------------------------------------------
//     演奏前の初期化
//---------------------------------------------------------
void Music::insideInit()
{
	INT i;
	ZeroMemory(&m_playInfo,sizeof(SGpPlayInfo));
	for(i=0;i<16;i++)
	{
		m_playInfo.Chn[i].bbVolume = 100;
		m_playInfo.Chn[i].bbPan = 64;
	}
	m_playInfo.pTitle = getTitle();
	m_playInfo.dwTempo = 120;
	m_dwTempParam = 60000000 / m_playInfo.dwTempo;
	m_bFadeIn = m_bFadeOut = false;
	m_dwResetCount = 0;
	m_dwNowTime = 0;
	m_dwlCountTime = 0;
	m_nEventFlag = 0;
}
//---------------------------------------------------------
//     演奏開始
//---------------------------------------------------------
bool Music::play()
{
	if(!isData())
		return false;
	stop();
	m_bStopFlag = false;


	insideInit();
	m_MidiData.init();
	m_MDevice.open(m_uDevice);
	m_dwResetCount = timeGetTime()+50;
	insideSetTimer();
	return true;
}

//---------------------------------------------------------
//     頭出し
//---------------------------------------------------------
bool Music::play(DWORDLONG dwlCount)
{
	if(!isData())
		return false;

	insideInit();
	m_MidiData.init();
	insideOpenDevice();
	m_bThrough = true;

	
	while(m_MidiData.isPlay() && m_MidiData.getNowCount() < dwlCount)
		insideMidiout();

	m_bThrough = false;
	m_dwResetCount = timeGetTime()+50;
	cont();
	return true;
}

//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::playTime(DWORD dwTime)
{
	if(!isData())
		return false;
	insideInit();
	m_MidiData.init();
	insideOpenDevice();
	m_bThrough = true;
	while(m_MidiData.isPlay() && getPlayTime() < dwTime)
	{
		m_playInfo.dwNowStep = 0;
		insideMidiout();
	}
	m_bThrough = false;
	m_dwResetCount = timeGetTime()+50;
	cont();
	return true;
}
//---------------------------------------------------------
//     演奏再開
//---------------------------------------------------------
bool Music::cont()
{
	if(!isData())
		return false;
	m_bStopFlag = false;
	m_Relative.fVolume = 0;
	if(!m_MDevice.isDevice())
		insideOpenDevice();
	insideComeback();
	return insideSetTimer();
}

//---------------------------------------------------------
//     演奏停止
//---------------------------------------------------------
bool Music::stop()
{
	if(isPlay())
	{
		INT i;
		m_bStopFlag = true;
		MTimer::stop();
		while(isPlayout())
			Sleep(1);
		insideSetVolume(-128);
		for(i=0;i<16;i++)
			m_MDevice.out(0xb0+i,0x7b,0x00);

		m_MDevice.close();
		m_bFadeIn = false;
		m_bFadeOut = false;

		for(i=0;i<16;i++)
			ZeroMemory(m_playInfo.Chn[i].bbNote,sizeof(BYTE[16]));
	}
	return true;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::isPlay()const
{
	return isTimer();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::outMidiData(DWORD dwData)
{
	return m_MDevice.out(dwData);
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::outMidiData(BYTE byData1,BYTE byData2,BYTE byData3)
{
	return m_MDevice.out(byData1,byData2,byData3);
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
LPCSTR Music::getTitle()const
{
	return m_MidiData.getTitle();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
SGpPlayInfo* Music::getPlayInfo()
{
	return &m_playInfo;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD Music::getTimeBase()const
{
	return m_MidiData.getRCount();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD Music::getRCount()const
{
	return getTimeBase();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD Music::getTempo()const
{
	return (WORD)m_playInfo.dwTempo;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
WORD Music::getTemp()const
{
	return getTempo();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG Music::getAllCount()const
{
	return m_MidiData.getAllCount();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG Music::getPlayCount()const
{
	return m_MidiData.getNowCount();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG Music::getAllTime()const
{
	return m_MidiData.getAllTime();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORDLONG Music::getPlayTime()const
{
	return m_dwNowTime + 
		DWORD((getPlayCount()-m_dwlCountTime)*m_dwTempParam/(1000*getRCount()));
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
INT Music::getEventFlag()
{
	return m_nEventFlag;
}

//---------------------------------------------------------
//     
//---------------------------------------------------------
INT Music::getDeviceCount()
{
	return m_MDevice.getDeviceCount();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
void Music::getDeviceName(INT nIndex,LPSTR pString)
{
	m_MDevice.getDeviceName(nIndex,pString);
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::openDevice()
{
	return insideOpenDevice();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::closeDevice()
{
	return m_MDevice.close();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
UINT Music::getTimerID()const
{
	return (UINT)isTimer();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
void Music::setTimeCount()
{
	m_timePlay = ::timeGetTime() - getTimeCountBase();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORD Music::getTimeCount()
{
	return m_timePlay;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
void Music::setTimeCountBase()
{
	m_timePlayBase = ::timeGetTime();
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
DWORD Music::getTimeCountBase()
{
	return m_timePlayBase;
}

//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::isPlayout()const
{
	return m_bPlayOut;
}
//---------------------------------------------------------
//     
//---------------------------------------------------------
bool Music::stdMidi(const LPBYTE pData,INT nSize)
{
	return m_MidiData.read(pData,nSize);
}

//---------------------------------------------------------
//     フェードイン
//---------------------------------------------------------
bool Music::fadeIn(INT nTime,INT nVolume)
{
	if(!isData())
		return false;
	m_nFadeVolume = nVolume;
	m_Relative.fVolume = (FLOAT)nVolume;
	m_nFadeCount = nTime;
	m_nFadeCountWork = 0;
	m_bFadeOut = false;
	m_bFadeIn = true;
	cont();
	return true;
}

//---------------------------------------------------------
//     フェードアウト
//---------------------------------------------------------
bool Music::fadeOut(INT nTime)
{
	if(!isData())
		return false;
	m_Relative.fVolume = (FLOAT)0;
	m_nFadeCount = nTime/m_nTimeCycle;
	m_nFadeCountWork=0;
	m_bFadeIn = false;
	m_bFadeOut = true;
	return true;
}

//---------------------------------------------------------
//     デバイスの選択
//---------------------------------------------------------
bool Music::setDevice(INT nIndex)
{
	if(m_uDevice != (UINT)nIndex && nIndex == MIDI_DIRECTMUSIC && isData())
		m_MDevice.setDLS(m_MidiData.getDLS(),m_MidiData.getDLSCount());

	m_uDevice = nIndex;
	if(isPlay())
	{
		stop();
		DWORDLONG dwlCount = getPlayCount();
		play(dwlCount);
	}
	return true;
}

//---------------------------------------------------------
//     テンポを設定
//---------------------------------------------------------
void Music::setTemp(INT nRelativeTempo)
{
	m_Relative.nTemp = nRelativeTempo;
	if(isPlay())
		insideSetTimer();
}
void Music::setLoop(bool bLoop)
{
	m_bLoop=bLoop;
}
void Music::setLoopTop(DWORDLONG dwlLoopSearch)
{
	m_dwlLoopSearch = dwlLoopSearch;
}
void Music::setLoopTopTime(DWORD dwLoopSearch)
{
	m_dwLoopSearch = dwLoopSearch;
}
void Music::setRelativeTemp(INT nRelativeTempo)
{
	setTemp(nRelativeTempo);
}
void Music::setRelativeKey(char cKey)
{
	m_Relative.cKey = cKey;
}
void Music::setRelativeVelocity(INT nVelo)
{
	m_Relative.nVelocity = nVelo;
}
void Music::setRelativeVolume(INT nVolume)
{
	if(m_Relative.nVolume != nVolume)
	{
		m_Relative.nVolume = nVolume;insideSetVolume(m_Relative.nVolume);
	}
}
void Music::setRelativePan(INT nPan)
{
	if(m_Relative.nPan != nPan)
	{
		m_Relative.nPan = nPan;insideSetPan(m_Relative.nPan);
	}
}
void Music::setRelativeChorus(INT nChorus)
{
	if(m_Relative.nChorus != nChorus)
	{
		m_Relative.nChorus = nChorus;insideSetChorus(m_Relative.nChorus);
	}
}
void Music::setRelativeReverb(INT nReverb)
{
	if(m_Relative.nReverb != nReverb)
	{
		m_Relative.nReverb = nReverb;
		insideSetReverb(m_Relative.nReverb);
	}
}

//---------------------------------------------------------
//     MIDIデバイスの初期化
//---------------------------------------------------------
bool Music::insideOpenDevice()
{
	bool bFlag = m_MDevice.open(m_uDevice);
//	insideSetNoteOff();
	return bFlag;
}

//---------------------------------------------------------
//     MIDI音源に設定を伝える
//---------------------------------------------------------
void Music::insideComeback()
{
	INT i;
	SGpMidiCode midiData;
	m_Relative.fVolume = (FLOAT)0;

	//プログラムナンバー復帰
	midiData.Mdf.bbData[2] = 0;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = BYTE(0xc0 + i);
		midiData.Mdf.bbData[1] = m_playInfo.Chn[i].bbPC;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	//コントロールナンバー復帰
	midiData.Mdf.bbData[1] = 0;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = BYTE(0xb0 + i);
		midiData.Mdf.bbData[2] = m_playInfo.Chn[i].bbCC;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	//パン復帰
	midiData.Mdf.bbData[1] = 0x0a;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = BYTE(0xb0 + i);
		midiData.Mdf.bbData[2] = m_playInfo.Chn[i].bbPan;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	//リバーブ復帰
	midiData.Mdf.bbData[1] = 0x5b;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = BYTE(0xb0 + i);
		midiData.Mdf.bbData[2] = m_playInfo.Chn[i].bbRev;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	//コーラス復帰
	midiData.Mdf.bbData[1] = 0x5d;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = BYTE(0xb0 + i);
		midiData.Mdf.bbData[2] = m_playInfo.Chn[i].bbChos;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	//ボリューム復帰
	insideSetVolume();
}

//---------------------------------------------------------
//     Reverb設定
//---------------------------------------------------------
void Music::insideSetReverb(INT nSData)
{
	INT i;
	SGpMidiCode midiData;
	if(m_bThrough)
		return;
	midiData.Mdf.bbData[1] = (BYTE)0x5b;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = (BYTE)(0xb0 + i);
		INT nData = m_playInfo.Chn[i].bbRev+nSData;
		if(nData < 0)
			nData = 0;
		else if(nData > 127)
			nData = 127;
		midiData.Mdf.bbData[2] = (BYTE)nData;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
	

}

//---------------------------------------------------------
//     Chorus設定
//---------------------------------------------------------
void Music::insideSetChorus(INT nSData)
{
	INT i;
	SGpMidiCode midiData;
	if(m_bThrough)
		return;
	midiData.Mdf.bbData[1] = (BYTE)0x5d;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = (BYTE)(0xb0 + i);
		INT nData = m_playInfo.Chn[i].bbChos+nSData;
		if(nData < 0)
			nData = 0;
		else if(nData > 127)
			nData = 127;
		midiData.Mdf.bbData[2] = (BYTE)nData;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
}
//---------------------------------------------------------
//     PAN設定
//---------------------------------------------------------
void Music::insideSetPan(INT nSData)
{
	INT i;
	SGpMidiCode midiData;
	if(m_bThrough)
		return;
	midiData.Mdf.bbData[1] = (BYTE)0x0a;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = (BYTE)(0xb0 + i);
		INT nData = m_playInfo.Chn[i].bbPan+nSData;
		if(nData < 0)
			nData = 0;
		else if(nData > 127)
			nData = 127;
		midiData.Mdf.bbData[2] = (BYTE)nData;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
}
//---------------------------------------------------------
//     ボリューム設定
//---------------------------------------------------------
void Music::insideSetVolume(INT nSVolume)
{
	INT i;
	SGpMidiCode midiData;
	if(m_bThrough)
		return;
	midiData.Mdf.bbData[1] = (BYTE)0x07;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = (BYTE)(0xb0 + i);
		INT nData = m_playInfo.Chn[i].bbVolume+(INT)m_Relative.fVolume+nSVolume;
		if(nData < 0)
			nData = 0;
		else if(nData > 127)
			nData = 127;
		midiData.Mdf.bbData[2] = (BYTE)nData;
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
}
//---------------------------------------------------------
//     ボリューム設定
//---------------------------------------------------------
void Music::insideSetNoteOff()
{
	INT i;
	SGpMidiCode midiData;
	if(m_bThrough)
		return;
	midiData.Mdf.bbData[1] = (BYTE)0x78;
	midiData.Mdf.bbData[2] = 0;
	midiData.Mdf.bbData[3] = 0;
	for(i=0;i<16;i++)
	{
		midiData.Mdf.bbData[0] = (BYTE)(0xb0 + i);
		m_MDevice.out(midiData.Mdf.dwMsg);
	}
}

//---------------------------------------------------------
//     タイマー設定
//---------------------------------------------------------
bool Music::insideSetTimer()
{
	if(m_bStopFlag)
		return false;

	m_nTimeCycle = 1;
	m_playInfo.dwTempo = 60000000 / m_dwTempParam;
	if(m_Relative.nTemp)
		m_nWaitCycle = (DWORD)((DWORDLONG)getTimeBase()*(m_Relative.nTemp+m_playInfo.dwTempo))/60000;
	else
		m_nWaitCycle = getTimeBase()*1000;
	setTimeCountBase();
	setTimeCount();
	m_dwTempCount = m_nWaitCycle/m_dwTempParam;
	m_dwTempChangeCount = 0;
	if(m_bThrough)
		return true;
	return start(m_nTimeCycle);
}

bool Music::isResetWait()
{
	//GSリセット50ms待機
	if(m_dwResetCount)
	{
		if((INT)timeGetTime() - (INT)m_dwResetCount < 50)
		{
			return true;
		}
		setTimeCountBase();
		m_dwResetCount = 0;
	}
	return false;
}
//---------------------------------------------------------
//     タイマー呼び出し部分
//---------------------------------------------------------
void Music::procTimer()
{
	if(m_bStopFlag)
		MTimer::stop();
	else if(!isResetWait())
	{
		setTimeCount();
		if(!m_bPlayOut)
		{
			m_bPlayOut = true;
			insidePlay();
			m_bPlayOut = false;
		}
	}
}
//---------------------------------------------------------
//     マーカのコールバック
//---------------------------------------------------------
void Music::callbackMarkerEvent(LPCSTR pString)
{
	INT i;
	if(pString[0] == 'M')
	{
		for(i=0;pString[i] && isdigit(pString[i]);i++);
		if(!pString[i])
			m_nEventFlag = atoi(pString+1);
	}
}

//---------------------------------------------------------
//     演奏出力一歩手前
//---------------------------------------------------------
void Music::insidePlay()
{	
	//フェードアウト
	if(m_bFadeOut)
	{
		if(m_nFadeCountWork < m_nFadeCount)
		{
			m_Relative.fVolume -= (FLOAT)90.0 / m_nFadeCount;
			if((INT)m_Relative.fVolume != m_Relative.nVolumeWork)
				insideSetVolume();
			m_Relative.nVolumeWork = (INT)m_Relative.fVolume;
			m_nFadeCountWork++;
		}
		else
		{
			m_Relative.fVolume = (FLOAT)0;
			m_bFadeOut = false;
			m_bPlayOut = false;
			stop();
			return;
		}
	}
	// フェードイン
	if(m_bFadeIn)
	{
		if(m_nFadeCountWork < m_nFadeCount)
		{
			m_nFadeCountWork++;
			m_Relative.fVolume = FLOAT(m_nFadeVolume-m_nFadeVolume*m_nFadeCountWork / m_nFadeCount);
			if((INT)m_Relative.fVolume != m_Relative.nVolumeWork)
				insideSetVolume();
			m_Relative.nVolumeWork = (INT)m_Relative.fVolume;
		}
		else
		{
			m_Relative.fVolume = (FLOAT)0;
			m_bFadeIn = false;
			insideSetVolume();
		}
	}
	if(m_MidiData.isPlay())
	{
		INT i;
		for(i=0;/*i<100;*/;i++,m_dwTempChangeCount++)
		{
			if(m_Relative.nTemp)
				m_dwTempCount = (DWORD)(((DWORDLONG)m_timePlay*getTimeBase()*(m_Relative.nTemp+m_playInfo.dwTempo))/60000);
			else
				m_dwTempCount = (DWORD)((DWORDLONG)m_timePlay*m_nWaitCycle/m_dwTempParam);
			if(m_dwTempChangeCount > m_dwTempCount)
				break;
			if(m_dwTempChangeCount+200<=m_dwTempCount)
			{
				if(!m_bThroughSkip)
				{
					insideSetNoteOff();
					m_bThroughSkip = true;
				}
			}
			else
			{
				if(m_bThroughSkip)
				{
					insideComeback();
					m_bThroughSkip = false;
				}
			}
			insideMidiout();
		}
	}
	else
	{	
		if(m_bLoop)
		{
			MTimer::stop();
			insideInit();
			m_MidiData.init();
	
			if(m_dwlLoopSearch)
			{
				m_bThrough = true;
				while(m_MidiData.isPlay() && getPlayCount() < m_dwlLoopSearch)
					insideMidiout();
				m_bThrough = false;
			}
			else if(m_dwLoopSearch)
			{
				m_bThrough = true;
				while(m_MidiData.isPlay() && getPlayTime() < m_dwLoopSearch)
					insideMidiout();
				m_bThrough = false;
			}
			cont();
		}
		else
		{
			m_bPlayOut = false;
			stop();
		}
	}
}

//---------------------------------------------------------
//     実演奏処理
//---------------------------------------------------------
void Music::insideMidiout()
{
	INT nWork;
	BYTE bbStat,bbStat2,bbStat3;
	UGpMidiData MData;
	while(m_playInfo.dwNowStep == 0 && m_MidiData.isPlay())
	{
		MData.dwMsg = m_MidiData.getDWord();
		bbStat = MData.bbData[0];
		bbStat2 = MData.bbData[0] &  (BYTE)0xf0;
		bbStat3 = MData.bbData[0] &  (BYTE)0x0f;
		if(bbStat == 0xf0)	//エクスクルーシブ
		{
			m_MDevice.out(m_MidiData.getEData());
		}
		else if(bbStat == 0xff)	//テンポ
		{
			if(MData.bbData[1] == 0x51)
			{
				m_dwNowTime += DWORD((getPlayCount()-m_dwlCountTime)*m_dwTempParam/(1000*getTimeBase()));	//現在の時間の保存
				m_dwlCountTime = getPlayCount();
				m_MidiData.Next();
				((LPWORD)&m_dwTempParam)[0] = m_MidiData.getDataWord();
				m_MidiData.Next();
				((LPWORD)&m_dwTempParam)[1] = m_MidiData.getDataWord();
				insideSetTimer();
			}
			else if(MData.bbData[1] == 6)	//マーク
			{	
				callbackMarkerEvent(m_MidiData.getMarker(*(LPWORD)(MData.bbData + 2)));
			}
		}
		else if(bbStat >= 0x80)				//通常データ
		{
			m_playInfo.Chn[bbStat3].midiMsg.dwMsg = MData.dwMsg;
			switch(bbStat2)	//ステータスの判定
			{
			case 0x90:						//ノートオン
				if(MData.bbData[2])
				{
					MData.bbData[1] += m_Relative.cKey;
					m_playInfo.Chn[bbStat3].bbNote[MData.bbData[1]>>3] |=
						(BYTE)(1<<(MData.bbData[1]&0x07));
					INT nVelo = MData.bbData[2] + m_Relative.nVelocity;
					if(nVelo < 0)
						nVelo = 0;
					else if(nVelo > 127)
						nVelo = 127;
					MData.bbData[2] = (BYTE)nVelo;
					break;
				}
			case 0x80:						//ノートオフ
				MData.bbData[1] += m_Relative.cKey;
				m_playInfo.Chn[bbStat3].bbNote[MData.bbData[1]>>3] &=
						(BYTE)~(1<<(MData.bbData[1]&7));
				break;
			case 0xc0:						//プログラムチェンジ
				m_playInfo.Chn[bbStat3].bbPC = MData.bbData[1];
				break;
			case 0xb0:
				switch(MData.bbData[1])
				{
				case 0x00:					//コントロールチェンジ
					m_playInfo.Chn[bbStat3].bbCC = MData.bbData[2];
					break;
				case 0x07:					//ボリューム
					m_playInfo.Chn[bbStat3].bbVolume = MData.bbData[2];
					nWork = MData.bbData[2] + m_Relative.nVolume + (INT)m_Relative.fVolume;
					if(nWork < (BYTE)0)
						nWork = 0;
					else if(nWork > 127)
						nWork = 127;
					m_playInfo.Chn[bbStat3].bbVolume2 = (BYTE)nWork;
					MData.bbData[2] = (BYTE)nWork;
					break;
				case 0x5b:					//リバーブ
					nWork = MData.bbData[2] + m_Relative.nReverb;
					if(nWork < 0)		nWork = 0;
					else if(nWork > 127)	nWork = 127;
					MData.bbData[2] = (BYTE)nWork;
					m_playInfo.Chn[bbStat3].bbRev = MData.bbData[2];
					break;
				case 0x5d:					//コーラス
					nWork = MData.bbData[2] + m_Relative.nChorus;
					if(nWork < 0)		nWork = 0;
					else if(nWork > 127)	nWork = 127;
					MData.bbData[2] = (BYTE)nWork;
					m_playInfo.Chn[bbStat3].bbChos = MData.bbData[2];
					break;
				case 0x0a:					//パン
					nWork = MData.bbData[2] + m_Relative.nPan;
					if(nWork < 0)		nWork = 0;
					else if(nWork > 127)	nWork = 127;
					MData.bbData[2] = (BYTE)nWork;
					m_playInfo.Chn[bbStat3].bbPan = MData.bbData[2];
					break;
				}
			}
			if(!m_bThrough && !(m_bThroughSkip && bbStat2 == 0x90))
				m_MDevice.out(MData.dwMsg);
		}
		m_MidiData.Next();
		m_playInfo.dwNowStep = m_MidiData.getNextCount();
	}
	m_MidiData.incCount();
	m_playInfo.dwNowStep--;
}

//namespace
}}

