#include <windows.h>
#include "aflActiveTool.h"

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

namespace AFL{namespace Game{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflActiveCounter
// 行動カウンタ用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflActiveCounter::AflActiveCounter()
{
}
//-----------------------------------------------
AflActiveCounter::~AflActiveCounter()
{
	std::map<DWORD,LPACTIVECOUNTER>::iterator it;
	for(it=m_mapCounter.begin();it!=m_mapCounter.end();it++)
	{
		LPACTIVECOUNTER pCounter = it->second;
		if(pCounter)
			delete pCounter;
	}
}
//-----------------------------------------------
void AflActiveCounter::addCounter(DWORD dwCount)
{
	std::map<DWORD,LPACTIVECOUNTER>::iterator it;
	for(it=m_mapCounter.begin();it!=m_mapCounter.end();it++)
	{
		LPACTIVECOUNTER pCounter = it->second;
		if(pCounter)
		{
			pCounter->dwCount += dwCount;
			if(pCounter->bCallback)
			{
				if(pCounter->dwCount > pCounter->dwSpeed)
				{
					pCounter->dwCount -= pCounter->dwSpeed;
					onCounterCallback(it->first);
				}
			}
		}
	}
}
//-----------------------------------------------
bool AflActiveCounter::isCounter(DWORD dwID)
{
	LPACTIVECOUNTER pCounter = m_mapCounter[dwID];
	if(!pCounter)
		return false;
	if(pCounter->dwCount > pCounter->dwSpeed)
	{
		pCounter->dwCount -= pCounter->dwSpeed;
		return true;
	}
	return false;
}
//-----------------------------------------------
void AflActiveCounter::clearCounter(DWORD dwID)
{
	LPACTIVECOUNTER pCounter = m_mapCounter[dwID];
	if(pCounter)
		pCounter->dwCount = 0;
}
//-----------------------------------------------
void AflActiveCounter::createCounter(DWORD dwID,DWORD dwSpeed,bool bCallback)
{
	LPACTIVECOUNTER pCounter = m_mapCounter[dwID];
	if(pCounter)
		delete pCounter;
	pCounter = NEW ACTIVECOUNTER;
	pCounter->dwCount = 0;
	pCounter->dwSpeed = dwSpeed;
	pCounter->bCallback = bCallback;
	m_mapCounter[dwID] = pCounter;
}
//-----------------------------------------------
void AflActiveCounter::setSpeed(DWORD dwID,DWORD dwSpeed)
{
	LPACTIVECOUNTER pCounter = m_mapCounter[dwID];
	if(pCounter)
		pCounter->dwSpeed = dwSpeed;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflActiveList
// 行動カウンタ制御用リスト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
AflActiveList::AflActiveList()
{
	m_dwActiveCount = 1000;
}
//-----------------------------------------------
void AflActiveList::increment()
{
	INT dwActiveCount = m_dwActiveCount;
	std::list<AflActiveCounter*>::iterator it;
	for(it=m_listActiveCount.begin();it!=m_listActiveCount.end();it++)
	{
		(*it)->addCounter(dwActiveCount);
	}
}
//-----------------------------------------------
void AflActiveList::addList(AflActiveCounter* pActiveCount)
{
	m_listActiveCount.push_back(pActiveCount);
}
//-----------------------------------------------
void AflActiveList::delList(AflActiveCounter* pActiveCount)
{
	m_listActiveCount.remove(pActiveCount);
}
//-----------------------------------------------

};};
