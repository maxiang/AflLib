#include <windows.h>
#include <tchar.h>

#include "aflD3DMedia.h"

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

namespace AFL{


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
bool UnitVideo::onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag() && m_dataPtr && getStat() == MEDIA_PLAY)
	{
		m_critical.lock();
		if(m_initImage)
		{
			INT width = MediaSampler::getImageWidth();
			INT height = MediaSampler::getImageHeight();
			setSize(width,height);
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

}
