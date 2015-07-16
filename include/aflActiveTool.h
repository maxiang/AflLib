#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLACTIVETOOL


#pragma warning( disable : 4786 )	//STL�̌x���O��
#include <map>
#include <list>

namespace AFL{namespace Game{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflActiveCounter
// �s���J�E���^�p�N���X
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-----------------------------------------------
//�J�E���^�p�\����
typedef struct tagActiveCounter
{
	DWORD dwSpeed;
	DWORD dwCount;
	bool bCallback;
}ACTIVECOUNTER,*LPACTIVECOUNTER,*PACTIVECOUNTER;

//-----------------------------------------------
class AflActiveCounter
{
public:
	AflActiveCounter();
	virtual ~AflActiveCounter();
	void addCounter(DWORD dwCount);
	bool isCounter(DWORD dwID);
	void clearCounter(DWORD dwID);
	void createCounter(DWORD dwID,DWORD dwSpeed,bool bCallback=false);
	void setSpeed(DWORD dwID,DWORD dwSpeed);

protected:
	virtual void onCounterCallback(DWORD dwID){}
	std::map<DWORD,LPACTIVECOUNTER> m_mapCounter;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// AflActiveList
// �s���J�E���^����p���X�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class AflActiveList
{
public:
	AflActiveList();
	void increment();
	void addList(AflActiveCounter* pActiveCount);
	void delList(AflActiveCounter* pActiveCount);

protected:
	DWORD m_dwActiveCount;
	std::list<AflActiveCounter*> m_listActiveCount;
};

};};

#define __INC_AFLACTIVETOOL
#endif
