#pragma once
#include "aflStd.h"
#include <list>

enum {
	ASIOSTInt16MSB   = 0,
	ASIOSTInt24MSB   = 1,		// used for 20 bits as well
	ASIOSTInt32MSB   = 2,
	ASIOSTFloat32MSB = 3,		// IEEE 754 32 bit float
	ASIOSTFloat64MSB = 4,		// IEEE 754 64 bit double float
	ASIOSTInt32MSB16 = 8,		// 32 bit data with 16 bit alignment
	ASIOSTInt32MSB18 = 9,		// 32 bit data with 18 bit alignment
	ASIOSTInt32MSB20 = 10,		// 32 bit data with 20 bit alignment
	ASIOSTInt32MSB24 = 11,		// 32 bit data with 24 bit alignment
	ASIOSTInt16LSB   = 16,
	ASIOSTInt24LSB   = 17,		// used for 20 bits as well
	ASIOSTInt32LSB   = 18,
	ASIOSTFloat32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	ASIOSTFloat64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
	ASIOSTInt32LSB16 = 24,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB18 = 25,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB20 = 26,		// 32 bit data with 20 bit alignment
	ASIOSTInt32LSB24 = 27,		// 32 bit data with 24 bit alignment
	ASIOSTDSDInt8LSB1   = 32,		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
	ASIOSTDSDInt8MSB1   = 33,		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
	ASIOSTDSDInt8NER8	= 40,		// DSD 8 bit data, 1 sample per byte. No Endianness required.
	ASIOSTLastEntry

};

enum
{
	ASE_OK = 0,
	ASE_SUCCESS = 0x3f4847a0,
	ASE_NotPresent = -1000, 
	ASE_HWMalfunction, 
	ASE_InvalidParameter, 
	ASE_InvalidMode, 
	ASE_SPNotAdvancing, 
	ASE_NoClock,  
	ASE_NoMemory 
};
typedef long ASIOBool;
typedef long ASIOError;
typedef double ASIOSampleRate;
typedef long ASIOSampleType;

typedef struct ASIOTimeStamp
{
	unsigned long hi;
	unsigned long lo;
}ASIOTimeStamp;

typedef struct ASIOSample
{
	unsigned long hi;
	unsigned long lo;
}ASIOSamples;

typedef struct AsioTimeInfo
{
	double          speed;
	ASIOTimeStamp   systemTime; 
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;
	unsigned long flags;
	char reserved[12];
} AsioTimeInfo;

typedef struct ASIOTimeCode
{       
	double          speed;
	ASIOSamples     timeCodeSamples;
	unsigned long   flags;
	char future[64];
} ASIOTimeCode;

typedef struct ASIOTime
{
	long reserved[4];
	struct AsioTimeInfo     timeInfo;
	struct ASIOTimeCode     timeCode;
} ASIOTime;

typedef struct ASIOClockSource
{
	long index;
	long associatedChannel;
	long associatedGroup;
	ASIOBool isCurrentSource;
	char name[32];
} ASIOClockSource;

typedef struct ASIOChannelInfo
{
	long channel;
	ASIOBool isInput;
	ASIOBool isActive;
	long channelGroup;
	ASIOSampleType type;
	char name[32];
} ASIOChannelInfo;

typedef struct ASIOCallbacks
{
	void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);
	void (*sampleRateDidChange) (ASIOSampleRate sRate);
	long (*asioMessage) (long selector, long value, void* message, double* opt);
	ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
} ASIOCallbacks;

typedef struct ASIOBufferInfo
{
	ASIOBool isInput;
	long channelNum;	
	void *buffers[2];
} ASIOBufferInfo;

interface IASIO : public IUnknown
{

	virtual ASIOBool init(void *sysHandle) = 0;
	virtual void getDriverName(char *name) = 0;	
	virtual long getDriverVersion() = 0;
	virtual void getErrorMessage(char *string) = 0;	
	virtual ASIOError start() = 0;
	virtual ASIOError stop() = 0;
	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels) = 0;
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency) = 0;
	virtual ASIOError getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity) = 0;
	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate) = 0;
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	virtual ASIOError setClockSource(long reference) = 0;
	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info) = 0;
	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
		long bufferSize, ASIOCallbacks *callbacks) = 0;
	virtual ASIOError disposeBuffers() = 0;
	virtual ASIOError controlPanel() = 0;
	virtual ASIOError future(long selector,void *opt) = 0;
	virtual ASIOError outputReady() = 0;
};
struct OUTBUFF
{
	INT size;
	LPVOID data;
};
struct OUTCHANNEL
{
	INT position;
};
class AsioOutBuff
{
public:
	AsioOutBuff();
	~AsioOutBuff();
	void out(LPVOID data,INT size);
	INT getData(LPVOID destData,INT destSize);
	INT getSize()
	{
		INT size = 0;
		std::list<OUTBUFF>::iterator it;
		foreach(it,m_outList)
		{
			size += it->size;
		}
		return size;
	}
protected:
	INT m_position;
	std::list<OUTBUFF> m_outList;
};
class Asio
{
public:
	Asio();
	~Asio();
	INT getDeviceCount();
	INT getPlayDeviceCount();
	INT getRecordDeviceCount();
	bool getDeviceName(INT index,std::wstring& name);
	bool getDeviceClassID(INT index,CLSID& clsid);
	bool selectDevice(INT index = 0);
	bool selectDevice(CLSID& clsid);
	bool deleteDevice();
	bool createRecordBuffer(INT count=1,INT index=0,...);
	bool createPlayBuffer(INT count=1,INT index=0,...);
	bool start();
	bool stop();
	void bufferSwitch(long index, ASIOBool processNow);


	void setCallbackSwitch(AFL::ClassProc& classProc);
	bool getChannelInfo(ASIOChannelInfo* info);
	INT getSampleRate();
	bool setSampleRate(double rate);
	INT getChannelCount();
	LPVOID getBuffer(INT index);
	INT getBufferSize();
	bool out(INT index,LPVOID data,INT size);
	INT getType();
	INT getSampleBit();
protected:
	bool sendOutData();
	static ASIOTime* _bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
	static long _asioMessage(long selector, long value, void* message, double* opt);
	static void _recordSwitch(long index, ASIOBool processNow);
	static void _playSwitch(long index, ASIOBool processNow);
	static void _sampleRateChange(ASIOSampleRate sRate);

	bool m_record;
	INT m_switchIndex;
	INT m_channelCount;
	ASIOBufferInfo* m_info;
	LONG m_bufferSize;
	IASIO* m_asio;
	INT m_bitSize;
	DWORD m_threadID;
	AFL::Critical m_critical;

	AFL::ClassProc m_switch;

	std::map<INT,AsioOutBuff> m_outData;	//出力用データバッファ
	std::map<INT,INT> m_outPosition;		//出力用データ位置

};
