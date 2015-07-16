//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLXML
//-----------------------------------------------------
#pragma warning( disable : 4786 )	//STLの警告外し

#include <string>
#include <list>
#include <map>
#include <stdio.h>

#include "aflStd.h"

namespace AFL{namespace XML{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Xml
// XMLデータ入出力用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Xml
{
friend class XmlParser;
public:
	Xml();
	bool loadMemory(LPCSTR data);
	bool load(LPCSTR fileName);
	bool save(LPCSTR fileName);
	bool load(LPCWSTR fileName);
	bool save(LPCWSTR fileName);
	bool out(FILE* file=stdout);
	bool get(String& s);
	LPCSTR getName();
	bool isParam(LPCSTR name) const;
	LPCSTR getParam(LPCSTR name) const;
	DWORD getParamX(LPCSTR name) const;
	INT getParamInt(LPCSTR name) const;
	bool getParamBool(LPCSTR name) const;
	FLOAT getParamFloat(LPCSTR name) const;
	void setName(LPCSTR name);
	bool isText()const;
	void setParamX(LPCSTR name,DWORD value);
	void setParam(LPCSTR name,INT value);
	void setParam(LPCSTR name,DWORD value);
	void setParam(LPCSTR name,FLOAT value);
	void setParam(LPCSTR name,LPCSTR value);
	void add(Xml& xml);
	Xml* add(LPCSTR name);
	bool addText(LPCSTR txt);
	Xml* get(Xml* xml=NULL);
	Xml* findName(LPCSTR name)
	{
		std::list<Xml>::iterator it;
		for(it=m_childXml.begin();it!=m_childXml.end();++it)
		{
			if(strcmp(it->getName(),name) == 0)
				return &*it;
		}
		return NULL;
	}

protected:
	std::string m_name;
	std::list<Xml> m_childXml;
	std::map<std::string,std::string> m_option;
	bool m_textFlag;
};

//namespace
};};


//-----------------------------------------------------
#define __INC_AFLXML
#endif	// __INC_AFLXML
//-----------------------------------------------------
