#include <windows.h>
#include "aflAsio.h"

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

static const WCHAR ASIO_NAME[] = L"software\\asio";

AsioOutBuff::AsioOutBuff()
{
	m_position = 0;
}
AsioOutBuff::~AsioOutBuff()
{
	std::list<OUTBUFF>::iterator it;
	foreach(it,m_outList)
	{
		delete it->data;
	}
}
void AsioOutBuff::out(LPVOID data,INT size)
{
	OUTBUFF buff;
	buff.data =new BYTE[size];
	buff.size = size;
	CopyMemory(buff.data,data,size);
	m_outList.push_back(buff);
}
INT AsioOutBuff::getData(LPVOID destData,INT destSize)
{
	LPBYTE data = (LPBYTE)destData;
	INT size = destSize;
	while(m_outList.size() && size)
	{
		OUTBUFF& outbuff = m_outList.front();
		INT srcSize = outbuff.size - m_position;
		if(size < srcSize)
		{
			CopyMemory(data,outbuff.data,size);
			m_position += size;
			size = 0;
		}
		else
		{
			CopyMemory(data,outbuff.data,srcSize);
			m_position = 0;
			data += srcSize;
			size -= srcSize;
			delete outbuff.data;
			m_outList.erase(m_outList.begin());
		}
	}
	return destSize - size;
}



static AFL::Critical g_critical;
static Asio* g_callbackPtr;
static std::map<DWORD,Asio*> g_playMap;
static std::map<DWORD,Asio*> g_recordMap;

Asio::Asio()
{
	CoInitialize(NULL);
	m_asio = NULL;
	m_info = NULL;
	m_channelCount = 0;
	m_switchIndex = -1;
}
Asio::~Asio()
{
	deleteDevice();
	CoUninitialize();
	if(m_info)
		delete[] m_info;
}
INT Asio::getDeviceCount()
{
	//ASIOデバイスの数をレジストリから取得
	DWORD count = 0;
	HKEY hKey = NULL;
	if(RegOpenKeyW(HKEY_LOCAL_MACHINE,ASIO_NAME,&hKey) == ERROR_SUCCESS)
	{
		RegQueryInfoKeyW(hKey,NULL,NULL,NULL,&count,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
		RegCloseKey(hKey);
	}
	return count;
}
INT Asio::getPlayDeviceCount()
{
	if(!m_asio)
		return 0;
	INT i;
	ASIOChannelInfo in;
	for(i=0;;i++)
	{
		in.channel = i;
		in.isInput = false;
		if(!getChannelInfo(&in))
			break;
	}
	return i;
}
INT Asio::getRecordDeviceCount()
{
	if(!m_asio)
		return 0;
	INT i;
	ASIOChannelInfo in;
	for(i=0;;i++)
	{
		in.channel = i;
		in.isInput = true;
		if(!getChannelInfo(&in))
			break;
	}
	return i;
}
bool Asio::getDeviceName(INT index,std::wstring& name)
{
	//デバイス名の取得
	bool flag = false;
	WCHAR keyName[256];
	HKEY hKey = NULL;
	if(RegOpenKeyW(HKEY_LOCAL_MACHINE,ASIO_NAME,&hKey) == ERROR_SUCCESS)
	{
		if(RegEnumKeyW(hKey,index,keyName,sizeof(keyName)) == ERROR_SUCCESS)
		{
			name = keyName;
			flag = true;
		}
		RegCloseKey(hKey);
	}
	return flag;
}
bool Asio::getDeviceClassID(INT index,CLSID& clsid)
{
	//デバイスクラスIDの取得
	bool flag = false;
	WCHAR keyName[256];
	HKEY hKey = NULL;
	if(RegOpenKeyW(HKEY_LOCAL_MACHINE,ASIO_NAME,&hKey) == ERROR_SUCCESS)
	{
		if(RegEnumKeyW(hKey,index,keyName,sizeof(keyName)) == ERROR_SUCCESS)
		{
			HKEY hSubKey = NULL;
			if(RegOpenKeyExW(hKey,keyName,0,KEY_READ,&hSubKey) == ERROR_SUCCESS)
			{
				DWORD dataType = REG_SZ;
				BYTE dataBuff[256];
				DWORD dataSize = sizeof(dataBuff);

				if(RegQueryValueExW(hSubKey,L"clsid",0,&dataType,dataBuff,&dataSize) == ERROR_SUCCESS)
				{
					CLSIDFromString((LPOLESTR)dataBuff,&clsid);
					flag = true;
				}
				RegCloseKey(hSubKey);
			}
		}

		RegCloseKey(hKey);
	}
	return flag;
}
bool Asio::selectDevice(CLSID& clsid)
{
	//デバイスを解放
	deleteDevice();

	//デバイスの作成
	if(CoCreateInstance(clsid,0,CLSCTX_INPROC_SERVER,clsid,(LPVOID*)&m_asio) == S_OK)
	{
		m_asio->init(NULL);
		return true;
	}
	return false;
}

bool Asio::selectDevice(INT index)
{
	//デバイスを解放
	deleteDevice();

	//デバイスのクラスIDを取得
	CLSID clsid;
	if(getDeviceClassID(index,clsid))
	{
		return selectDevice(clsid);
	}
	return false;
}
bool Asio::deleteDevice()
{
	if(m_asio)
	{
		g_critical.lock();
		m_asio->stop();
		m_asio->disposeBuffers();
		m_asio->Release();
		m_asio = NULL;
		g_critical.unlock();
		return true;
	}
	return false;
}
bool Asio::createRecordBuffer(INT count,INT index,...)
{
	if(!m_asio || count<1)
		return false;
	m_asio->disposeBuffers();
	m_record = true;

	INT i;
	va_list list;
	va_start( list, count );

	m_channelCount = count;
	//バッファサイズの設定
	LONG minSize;
	LONG maxSize;
	LONG preferredSize;
	LONG granularity;
	m_asio->getBufferSize(&minSize,&maxSize,&preferredSize,&granularity);
	//preferredSize = 10*(48000/8*2)/1000;
	//preferredSize*=2;
	m_bufferSize = preferredSize;

	if(m_info)
		delete[] m_info;
	m_info = new ASIOBufferInfo[count];
	for(i=0;i<count;i++)
	{
		m_info[i].isInput = true;
		m_info[i].channelNum = va_arg(list, int);
		m_info[i].buffers[0] = NULL;
		m_info[i].buffers[1] = NULL;
	}
	va_end(list);

	ASIOCallbacks callback;
	callback.asioMessage = _asioMessage;
	callback.bufferSwitch = _recordSwitch;
	callback.bufferSwitchTimeInfo = _bufferSwitchTimeInfo;
	callback.sampleRateDidChange = _sampleRateChange;
	if(m_asio->createBuffers(m_info,count,preferredSize,&callback) == ASE_OK)
		return true;
	return false;
}
bool Asio::createPlayBuffer(INT count,INT index,...)
{
	if(!m_asio || count<1)
		return false;
	m_asio->disposeBuffers();

	m_record = false;

	INT i;
	va_list list;
	va_start( list, count );

	m_channelCount = count;
	//バッファサイズの設定
	LONG minSize;
	LONG maxSize;
	LONG preferredSize;
	LONG granularity;
	m_asio->getBufferSize(&minSize,&maxSize,&preferredSize,&granularity);
	preferredSize = 50*(getSampleRate())/1000;
	//preferredSize /= 4;
	m_bufferSize = preferredSize;

	if(m_info)
		delete[] m_info;
	m_info = new ASIOBufferInfo[count];
	for(i=0;i<count;i++)
	{
		m_info[i].isInput = false;
		m_info[i].channelNum = va_arg(list, int);
		m_info[i].buffers[0] = NULL;
		m_info[i].buffers[1] = NULL;
		m_outPosition[i] = 0;
	}
	va_end(list);

	ASIOCallbacks callback;
	callback.asioMessage = _asioMessage;
	callback.bufferSwitch = _playSwitch;
	callback.bufferSwitchTimeInfo = _bufferSwitchTimeInfo;
	callback.sampleRateDidChange = _sampleRateChange;
	if(m_asio->createBuffers(m_info,count,preferredSize,&callback) == ASE_OK)
		return true;
	return false;
}
bool Asio::start()
{
	if(!m_asio)
		return false;
	m_switchIndex = -1;
	g_critical.lock();
	g_callbackPtr = this;
	if(m_asio->start() == ASE_OK)
	{
		while(g_callbackPtr)
			Sleep(1);
		return true;
	}
	g_callbackPtr = NULL;
	g_critical.unlock();
	return false;
}
bool Asio::stop()
{
	if(!m_asio)
		return false;
	if(m_asio->stop() == ASE_OK)
	{
		g_critical.lock();
		g_playMap.erase(m_threadID);
		g_recordMap.erase(m_threadID);
		m_outData.clear();
		g_critical.unlock();
		return true;
	}
	return false;
}
void Asio::bufferSwitch(long index, ASIOBool processNow)
{
	m_switchIndex = index;
	if(m_record)
	{
		g_critical.lock();
		m_switch.call(NULL);
		g_critical.unlock();
	}
	else
	{
		sendOutData();
	}
}

ASIOTime* Asio::_bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{
	return 0;
}
long Asio::_asioMessage(long selector, long value, void* message, double* opt)
{
	return 0;
}
void Asio::_playSwitch(long index, ASIOBool processNow)
{
	DWORD id = GetCurrentThreadId();

	Asio* asio = g_playMap[id];
	if(!asio)
	{
		asio = g_callbackPtr;
		asio->m_threadID = id;
		g_playMap[id] = g_callbackPtr;
		g_callbackPtr = NULL;
		g_critical.unlock();
	}
	//if(asio)
/*	g_critical.lock();
	FILE* file = fopen("test.txt","at");
	fprintf(file,"%d %d %d\n",id,index,asio->m_record);
	fclose(file);
	asio->bufferSwitch(index,processNow);
	g_critical.unlock();
	asio->m_asio->outputReady();
*/
}
void Asio::_recordSwitch(long index, ASIOBool processNow)
{
	DWORD id = GetCurrentThreadId();

	Asio* asio = g_recordMap[id];
	if(!asio)
	{
		asio = g_callbackPtr;
		asio->m_threadID = id;
		g_recordMap[id] = g_callbackPtr;
		g_callbackPtr = NULL;
		g_critical.unlock();
	}

}
void Asio::_sampleRateChange(ASIOSampleRate sRate)
{
}
void Asio::setCallbackSwitch(AFL::ClassProc& classProc)
{
	m_switch = classProc; 
}
bool Asio::getChannelInfo(ASIOChannelInfo* info)
{
	if(!m_asio)
		return false;
	return m_asio->getChannelInfo(info) == S_OK;
}
INT Asio::getSampleRate()
{
	if(!m_asio)
		return 0;
	ASIOSampleRate rate = 0;
	m_asio->getSampleRate(&rate);
	return (INT)rate;
}
bool Asio::setSampleRate(double rate)
{
	if(!m_asio)
		return false;
	if(m_asio->setSampleRate(rate) == S_OK)
		return true;
	return false;
}
INT Asio::getChannelCount()
{
	return m_channelCount;
}
LPVOID Asio::getBuffer(INT index)
{
	if(index >= m_channelCount)
		return NULL;
	return m_info[index].buffers[m_switchIndex];
}
INT Asio::getBufferSize()
{
	return m_bufferSize*getSampleBit()/8;
}
bool Asio::out(INT index,LPVOID data,INT size)
{
	if(index >= m_channelCount)
		return false;
	m_critical.lock();
	m_outData[index].out(data,size);
	m_critical.unlock();
	//sendOutData();
	return true;
}
bool Asio::sendOutData()
{
	if(m_switchIndex != 0 && m_switchIndex != 1)
		return false;
	INT i;
	m_critical.lock();
	for(i=0;i<m_channelCount;i++)
	{
		if(m_outData[i].getSize() < getBufferSize())
			continue;
		INT position = 0;//m_outPosition[i];
		INT size = m_outData[i].getData(*(LPBYTE*)&m_info[i].buffers[m_switchIndex]+position,getBufferSize()-position);
	/*	if(getBufferSize() - position - size == 0)
		{
			position = 0;
		}
		else
		{
			position += size;
		}
		m_outPosition[i] = position;
	*/
	}
	m_critical.unlock();
	m_asio->outputReady();
	return true;
}
INT Asio::getType()
{
	ASIOChannelInfo in;
	in.channel = 0;
	in.isInput = m_record;
	if(!getChannelInfo(&in))
		return -1;
	return in.type;
}
INT Asio::getSampleBit()
{
	INT type = getType();
	if(type == 16)
		return 16;
	else if(type == 18)
		return 32;
	return 0;
}