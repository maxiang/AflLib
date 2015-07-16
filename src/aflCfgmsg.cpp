#if defined(_WIN32) | defined(_WIN32_WCE)
	#include <windows.h>
#endif


#include <stdio.h>
#include <string.h>
#include "aflCfgmsg.h"

namespace AFL{namespace CFGMSG{


CPObject::CPObject()
{
}

char* CPObject::StrKeep(const char* pScr)
{
	char* pDest;

	if(pScr != NULL)
	{
		pDest = new char[strlen(pScr)+1];
		strcpy(pDest,pScr);
	}
	else
	{
		pDest = NULL;
	}
	return pDest;
}

BOOL CPObject::StrcmpNumber(const char* pString1,
							const char* pString2,int Number)
{
	int i;
	int Data;

	if(Number < 0)
	{
		if(strcmp(pString1,pString2) == 0)
			return TRUE;
		else
			return FALSE;
	}

	if(strstr(pString1,pString2)==pString1)
	{
	
		i = (int)strlen(pString2);
		if(sscanf(pString1 + i,"%d",&Data) != 0)
			if(Data == Number)
				return TRUE;
	}
	return FALSE;
}

//========================================================================
//CConfigFileクラス

///////////////////////////////////////////////////////////////////////
//パブリックメンバ

CPCfgFile::CPCfgFile()
{
	m_pFileName = NULL;
	m_pHederName = NULL;
	SetDefault();
}
CPCfgFile::CPCfgFile(const char* pFileName)
{
	m_pFileName = new char[strlen(pFileName)+1];
	strcpy(m_pFileName,pFileName);
	SetDefault();
}
CPCfgFile::~CPCfgFile()
{
	Destroy();
}
BOOL CPCfgFile::Init(const char* pFileName)
{
	Destroy();
	m_pFileName = new char[strlen(pFileName)+1];
	strcpy(m_pFileName,pFileName);
	SetDefault();
	if(Open()==FALSE)
		return FALSE;
	else
	{
		Close();
		return TRUE;
	}
}

void CPCfgFile::SetBuffSize(int BuffSize)
{
	delete[] m_pBuff;
	m_BuffSize = BuffSize;
	m_pBuff = new char[m_BuffSize + 1];
}

const char* CPCfgFile::ReadHeder()
{
	int i;
	long FileSeek;

	if(m_pHederName != NULL)
	{
		delete[] m_pHederName;
		m_pHederName = NULL;
	}
	while(m_FileEof)
	{
		FileSeek = m_FileSeek;
		Read();
		if(*m_pBuff == '[')
		{
			for(i=0;m_pBuff[i]!=0;i++)
			{
				if(m_pBuff[i] == ']' || m_pBuff[i] == '\n')
				{
					m_pBuff[i] = 0;
					break;
				}
			}
			m_pHederName = new char[strlen(m_pBuff)];
			strcpy(m_pHederName,m_pBuff+1);
			return m_pHederName;
		}
	}
	return NULL;
}

const char* CPCfgFile::ReadName()
{
	long FileSeek;
	char* pBuff;
	char* pBuff2;
	int i;

	m_DataSeek = -1;
	while(m_FileEof)
	{
		FileSeek = m_FileSeek;
		Read();
		if(*m_pBuff == '[')
		{
			m_FileSeek = FileSeek;
			break;
		}
		pBuff=StrchrCode(m_pBuff,'=');
		pBuff2=StrchrCode(m_pBuff,'\"');
		if(pBuff && (pBuff < pBuff2 || !pBuff2))
		{
			m_DataSeek = FileSeek + (long)(pBuff - m_pBuff + 1);
			for(i=0;m_pBuff[i]!=0;i++)
			{
				if(m_pBuff[i] == m_FCode)
					i++;
				else
				{
					if(m_pBuff[i] == '\t'||	m_pBuff[i] == '=' ||
						m_pBuff[i] == ' ')
					{
						m_pBuff[i] = 0;
						break;
					}
				}
			}
			return m_pBuff;
		}
	}
	return NULL;
}

char* CPCfgFile::ReadData(long Seek)
{
	long FileSeek;
	char* pWorkBuff;

	FileSeek = m_FileSeek;
	pWorkBuff = new char[1];
	*pWorkBuff = 0;
	m_FileSeek = Seek;
	m_FileEof = 1;
	while(m_FileEof)
	{
		Read();
		if(*m_pBuff == '[')
		{
			break;
		}
		if(SetStrData(m_pBuff) == FALSE)
			break;
		pWorkBuff = StrcatData(pWorkBuff,m_pBuff);
	}
	strcpy(m_pBuff,pWorkBuff);
	delete[] pWorkBuff;
	m_FileSeek = FileSeek;
	return m_pBuff;

}


char* CPCfgFile::StrchrCode(char* pString,char Code)
{
char* pBuff;
pBuff = strchr(pString,Code);
if(pBuff == NULL || (pBuff != pString && *(pBuff-1) == m_FCode))
	return NULL;
return pBuff;
}
///////////////////////////////////////////////////////////////////////
//プロテクトメンバ

void CPCfgFile::Destroy()
{
	if(m_pFileName != NULL)
		delete[] m_pFileName;
	if(m_pHederName != NULL)
		delete[] m_pHederName;
	if(m_pBuff != NULL)
		delete[] m_pBuff;
}

void CPCfgFile::SetDefault()
{
	m_DataSeek = -1;
	m_FileEof = 1;
	m_FileSeek = 0;
	m_pFile = NULL;
	m_BuffSize = 10000;
	m_FCode = '\\';
	m_pBuff = new char[m_BuffSize + 1];
}

BOOL CPCfgFile::Open()
{
	m_pFile = fopen(m_pFileName,"rb");
	if(m_pFile == NULL)
		return FALSE;
	else
	{
		m_FileEof = 1;
		fseek(m_pFile,m_FileSeek,0);
	}
	return TRUE;
}
void CPCfgFile::Close()
{
	fclose(m_pFile);
	m_pFile = NULL;
}
void CPCfgFile::Read()
{
	int i;
	Open();
	fgets(m_pBuff,m_BuffSize,m_pFile);
	m_FileSeek = ftell(m_pFile);
	if(feof(m_pFile) || m_pFile==NULL)
	{
		*m_pBuff = 0;
		m_FileEof = 0;
	}
	else
	{
		bool bFlag = false;
		for(i=0;m_pBuff[i]!=0;i++)
		{
			if(m_pBuff[i] == '"')
			{
				if(i == 0 || m_pBuff[i-1] != m_FCode)
				{
					bFlag = !bFlag;
				}
			}
			if(m_pBuff[i] == ';')
			{
				if(i == 0 || m_pBuff[i-1] != m_FCode && !bFlag)
				{
					m_pBuff[i] = 0;
					break;
				}
			}
		}
	}
	Close();
}

char* CPCfgFile::StrcatData(char* dest,const char* scr)
{
	char* pBuff;
	pBuff = new char[strlen(dest) + strlen(scr) + 1];
	strcpy(pBuff,dest);
	strcat(pBuff,scr);
	delete[] dest;
	return pBuff;
}

BOOL CPCfgFile::SetStrData(const char* scr)
{
	int i,j;
	int Flag=0;
	for(i=j=0;scr[i] != 0 && scr[i] != '\r';i++)
	{
		if(scr[i] == m_FCode)
		{
			i++;
			switch(scr[i])
			{
			case 'n':
				m_pBuff[j] = '\n';
				break;
			case 't':
				m_pBuff[j] = '\t';
				break;
			case 'e':
				m_pBuff[j] = '\x1b';
				break;
			case 's':
				m_pBuff[j] = ' ';
			default:
				m_pBuff[j] = scr[i];
				break;
			}
			j++;
		}
		else
		{
			if(scr[i] == '\"')
			{
				Flag ^= 1;
				continue;
			}
			if(Flag == 0)
			{
				if(scr[i] == ' ' || scr[i] == '\t' || scr[i] =='\n')
					continue;
				if(scr[i] == '=')
					return FALSE;
			}
			if((unsigned)scr[i] >= 0x80 && (unsigned)scr[i] <= 0x9f || (unsigned)scr[i] >= 0xE0)
			{
				m_pBuff[j] = scr[i];
				i++;j++;
			}
			m_pBuff[j] = scr[i];
			j++;
		}
	}
	m_pBuff[j] = 0;
	return TRUE;
}


//========================================================================
//CPCfgDataクラス

CPCfgData::CPCfgData(const char* pHeder,const char* pName,long Seek)
{
	m_pHeder = StrKeep(pHeder);
	m_pName = StrKeep(pName);
	m_Seek = Seek;
}
CPCfgData::~CPCfgData()
{
	delete[] m_pHeder;
	delete[] m_pName;
}


//========================================================================
//CfgMsgクラス

///////////////////////////////////////////////////////////////////////
//パブリックメンバ

CfgMsg::CfgMsg():CPCfgFile()
{
	m_ppCfgData = NULL;
	m_DataCount = 0;
	m_DataCur = 0;
}

CfgMsg::~CfgMsg()
{
	int i;
	if(m_ppCfgData != NULL)
	{
		for(i=0;i < m_DataCount;i++)
			delete m_ppCfgData[i];
		delete m_ppCfgData;
	}
}

bool CfgMsg::open(LPCSTR fileName)
{
	if(!CPCfgFile::Init(fileName))
		return false;
	MakeList();
	return true;
}

void CfgMsg::MakeList()
{
	const char* pHederText;
	const char* pNameText;
	long Seek;
	while(1)
	{
		pHederText = ReadHeder();
		if(pHederText == NULL)
			break;
		while(1)
		{
			pNameText = ReadName();
			if(pNameText == NULL)
				break;
			Seek = GetDataSeek();
			Add(pHederText,pNameText,Seek);
		}
	}
}

int CfgMsg::searchSet(const char* pHeder,int HNo)
{
	int i;
	for(i=m_DataCur;i < m_DataCount;i++)
	{
		if(StrcmpNumber(m_ppCfgData[i]->GetHeder(),pHeder,HNo)==1)
		{
			m_DataCur = i;
			return 1;
		}
	}
	return 0;
}

char* CfgMsg::search(const char* pHeder,int HNo,
									const char* pName,int NNo)
{
	return StrKeep(searched(pHeder,HNo,pName,NNo));
}

char* CfgMsg::searched(const char* pHeder,
							int HNo,const char* pName,int NNo)
{
	int i;
	for(i=m_DataCur;i < m_DataCount;i++)
	{
		if(StrcmpNumber(m_ppCfgData[i]->GetHeder(),pHeder,HNo)==1 &&
				StrcmpNumber(m_ppCfgData[i]->GetName(),pName,NNo)==1)
			return ReadData(m_ppCfgData[i]->GetSeek());
	}
	return NULL;
}


char* CfgMsg::search(const char* pName,int NNo)
{
	int i;
	for(i=m_DataCur;i < m_DataCount;i++)
	{
		if(	StrcmpNumber(m_ppCfgData[i]->GetName(),pName,NNo)==0)
		{
			m_DataCur = i+1;
			return StrKeep(ReadData(m_ppCfgData[i]->GetSeek()));
		}
	}
	return NULL;
}

char* CfgMsg::GetHeder()
{
	if(m_DataCur >= m_DataCount)
		return NULL;
	return m_ppCfgData[m_DataCur]->GetHeder();
}
char* CfgMsg::GetName()
{
	if(m_DataCur >= m_DataCount)
		return NULL;
	return m_ppCfgData[m_DataCur]->GetName();
}
char* CfgMsg::GetData()
{
	if(m_DataCur >= m_DataCount)
		return NULL;
	return ReadData(m_ppCfgData[m_DataCur]->GetSeek());
}

CPCfgData* CfgMsg::GetDataAdr()
{
	if(m_DataCur >= m_DataCount)
		return NULL;
	m_DataCur++;
	return m_ppCfgData[m_DataCur-1];
}

///////////////////////////////////////////////////////////////////////
//プロテクトメンバ

void CfgMsg::Add(const char* pHeder,const char* pName,long Seek)
{

	int i;
	CPCfgData** ppCfgData;

	ppCfgData = new (CPCfgData* [m_DataCount+1]);
	for(i=0;i < m_DataCount;i++)
	{
		ppCfgData[i] = m_ppCfgData[i];
	}
	ppCfgData[i] = new CPCfgData(pHeder,pName,Seek);
	delete[] m_ppCfgData;
	m_ppCfgData = ppCfgData;
	m_DataCount++;
}

}}
