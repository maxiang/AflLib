#ifndef	_INC_CFGMSG

#include "aflStd.h"


#ifndef	UINT
	typedef unsigned int UINT;
#endif

#ifndef	BOOL
	#define BOOL UINT
#endif

#ifndef	TRUE
	#define TRUE 1
#endif

#ifndef	FALSE
	#define FALSE 0
#endif
//namespace Config
namespace AFL{
namespace CFGMSG{

class CPObject
{
protected:
	char* StrKeep(const char* pScr);
	BOOL StrcmpNumber(const char* pString1,const char* pString2,int Number);
	CPObject();

};



class CPCfgFile : public CPObject
{
public:
	CPCfgFile();
	CPCfgFile(const char* pFileName);
	~CPCfgFile();
	BOOL Init(const char* pFileName);
	void SetBuffSize(int BuffSize);
	const char* ReadHeder();
	const char* ReadName();
	char* ReadData(long Seek);
	inline long GetDataSeek(){return m_DataSeek;}
	inline void ChengeCode(char FCode){m_FCode = FCode;}

protected:
	char* StrchrCode(char* pString,char Code);
	void Destroy();
	void SetDefault();
	BOOL Open();
	void Close();
	void Read();
	char* StrcatData(char* dest,const char* scr);
	BOOL SetStrData(const char* scr);

	FILE* m_pFile;
	int m_FileEof;
	char* m_pFileName;
	char* m_pBuff;
	char* m_pHederName;
	int m_BuffSize;
	long m_FileSeek;
	long m_DataSeek;
	char m_FCode;
};


class CPCfgData : public CPObject
{
public:
	CPCfgData(const char* pHeder,const char* pName,long Seek);
	~CPCfgData();

	inline char* GetHeder(){return m_pHeder;}
	inline char* GetName(){return m_pName;}
	inline long GetSeek(){return m_Seek;}

protected:
	char* m_pHeder;
	char* m_pName;
	long m_Seek;
};

class CfgMsg : public CPCfgFile
{
public:
	CfgMsg();
	~CfgMsg();
	bool open(LPCSTR fileName);
	void MakeList();

	int searchSet(const char* pHeder,int HNo);
	inline int searchSet(const char* pHeder){return searchSet(pHeder,-1);}

	char* searched(const char* pHeder,int HNo,const char* pName,int NNo);
	char* search(const char* pHeder,int HNo,const char* pName,int NNo);
	char* search(const char* pName,int NNo);
	inline char* search(const char* pHeder,const char* pName)
		{return search(pHeder,-1,pName,-1);}
	inline char* search(const char* pHeder,int HNo,const char* pName)
		{return search(pHeder,HNo,pName,-1);}
	inline char* search(const char* pHeder,const char* pName,int NNo)
		{return search(pHeder,-1,pName,NNo);}
	inline char* search(const char* pName){return search(pName,-1);}

	inline char* searched(const char* pHeder,const char* pName)
		{return searched(pHeder,-1,pName,-1);}
	inline char* searched(const char* pHeder,int HNo,const char* pName)
		{return searched(pHeder,HNo,pName,-1);}
	inline char* searched(const char* pHeder,const char* pName,int NNo)
		{return searched(pHeder,-1,pName,NNo);}

	inline int GetDataCount(){return m_DataCount;}
	inline void SetFirst(){m_DataCur = 0;}
	inline void SetNext(){m_DataCur++;}
	inline int GetSearchCur(){return m_DataCur;}

	char* GetHeder();
	char* GetName();
	char* GetData();
	CPCfgData* GetDataAdr();

protected:
	void Add(const char* pHeder,const char* pName,long Seek);
	CPCfgData** m_ppCfgData;
	int m_DataCount;
	int m_DataCur;
};

//namespace
}}
#define	_INC_CFGMSG
#endif	//_INC_CFGMSG
