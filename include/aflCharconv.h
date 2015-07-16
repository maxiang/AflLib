#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLCHARCONV
#include "aflStd.h"

class CharaTable
{
	friend class CharaConvert;
private:
	CharaTable();
	~CharaTable();
	void init();

	void createUniTable();
	void createUniTable(int high,int* codeCount);
	int UniToSjis(char* mbChar,unsigned short wCode) const;
	unsigned short* m_UNItoSJIS_TABLE;
	static unsigned short m_SJIStoUNI_TABLE[];
	bool m_init;
};

class CharaConvert
{
public:
	static int UniToSJIS(char* mbChar,unsigned short wCode);
	static void UniToSJIS(AFL::String& dest,LPCWSTR src);
protected:
	static CharaTable m_charaTable;
};


#define __INC_AFLCHARCONV
#endif
