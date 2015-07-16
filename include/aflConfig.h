//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLCONFIG
//-----------------------------------------------------

#include <map>
#include <string>

#include "aflStd.h"

namespace AFL{ namespace CONFIG{

class Config
{
typedef std::multimap<std::string,std::string> CONFIGVALUE;
typedef std::multimap<std::string,CONFIGVALUE> CONFIGDATA;
public:
	Config();
	bool load(LPCSTR fileName);
	bool save(LPCSTR fileName) const;
	void addData(LPCSTR header,CONFIGVALUE& data);

	LPCSTR getData(LPCSTR header,LPCSTR name,INT index=0) const;
	LPCSTR getData(LPCSTR header,INT index1,LPCSTR name,INT index2=0) const;
	INT getCount(LPCSTR header=NULL,LPCSTR name=NULL) const;

	INT findHeader(LPCSTR header,LPCSTR name,LPCSTR value) const;
protected:
	void outValue(FILE* file,LPCSTR value) const;

	CONFIGDATA m_configData;
	CHAR m_quat;
	CHAR m_comment;
};
}}

#define __INC_AFLCONFIG
#endif
