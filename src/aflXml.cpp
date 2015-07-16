#ifdef _WIN32
	#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "aflXml.h"

/*
#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define new new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 
*/

namespace AFL{namespace XML{

#define isSJIS(a) ((unsigned char)a >= 0x81 && (unsigned char)a <= 0x9f || (unsigned char)a >= 0xe0 && (unsigned char)a<=0xfc)


class XmlParser
{
public:
	bool loadMemory(LPCSTR data,Xml& xml)
	{
		read(data,xml);
		return true;
	}
	bool saveFile(LPCSTR fileName,Xml& xml)
	{
		return saveFile(UCS2(fileName),xml);
	}
	bool saveFile(LPCWSTR fileName,Xml& xml)
	{
		FILE* file = _wfopen(fileName,L"wt");
		if(!file)
			return false;
		out(file,xml);
		fclose(file);
		return true;
	}
	bool loadFile(LPCSTR fileName,Xml& xml)
	{
		return loadFile(UCS2(fileName),xml);
	}
	bool loadFile(LPCWSTR fileName,Xml& xml)
	{
		FILE* file = _wfopen(fileName,L"rt");
		if(!file)
			return false;
		fseek(file,0,SEEK_END);
		INT size = ftell(file);
		fseek(file,0,SEEK_SET);
		if(size == 0)
		{
			fclose(file);
			return false;
		}
		LPSTR work = new CHAR[size+1];
		fread(work,size,1,file);
		fclose(file);

		work[size] = 0;
		LPCSTR pt = work;
		read(pt,xml);
		delete[] work;
		return true;
	}
	bool out(String& s,Xml& xml)
	{
		INT level = 0;
		INT textFlag = true;
		
		//構成設定の出力
		s.appendf("<?xml");
		LPCSTR param;

		param = xml.getParam("version");
		if(param)
			s.appendf(" version=\"%s\"",param);
		else
			s.appendf(" version=\"1.0\"");

		param = xml.getParam("encoding");
		if(param)
			s.appendf(" encoding=\"%s\"",param);
		s += "?>\n";

		if(xml.m_childXml.size())
			write(s,xml.m_childXml.front(),0,textFlag);
		s += '\n';
		return true;
	}

	void writeTab(String& s,INT level)
	{
		INT i;
		for(i=0;i<level;i++)
			s += "\t";
	}
	void convertXMLString(String& dest,LPCSTR src)
	{
		INT i;
		for(i=0;src[i];i++)
		{
			switch(src[i])
			{
			case 0xa:
				dest += "&#x0a;";
				break;
			case 0xd:
				dest += "&#x0d;";
				break;
			case '<':
				dest += "&lt;";
				break;
			case '>':
				dest += "&gt;";
				break;
			case '&':
				dest += "&amp;";
				break;
			case '"':
				dest += "&quot;";
				break;
			case '\'':
				dest += "&apos;";
				break;
			default:
				dest += src[i];
				break;			
			}
		}
	}
	bool writeParams(String& s,Xml& xml)
	{
		std::map<std::string,std::string>::iterator it;
		for(it=xml.m_option.begin();it!=xml.m_option.end();++it)
		{
			String name;
			String value;
			convertXMLString(name,it->first.c_str());
			convertXMLString(value,it->second.c_str());
			s.appendf(" %s=\"%s\"",name.c_str(),value.c_str());
		}
		return true;
	}
	bool write(String& s,Xml& xml,INT level,INT& textFlag)
	{
		if(xml.m_textFlag)
		{
			String dest;
			convertXMLString(dest,xml.getName());

			s += dest.c_str();
			textFlag = true;
		}
		else
		{
			if(!textFlag)
			{
				s += "\n";
				writeTab(s,level);
			}
			textFlag = false;
			if(xml.m_childXml.size())
			{
				s.appendf("<%s",xml.getName());
				writeParams(s,xml);
				s += ">";

				std::list<Xml>::iterator it;
				for(it=xml.m_childXml.begin();it!=xml.m_childXml.end();++it)
					write(s,*it,level+1,textFlag);
				if(!textFlag)
				{
					s.appendf("\n");
					writeTab(s,level);
				}
				textFlag = false;
				s.appendf("</%s>",xml.getName());
			}
			else
			{
				s.appendf("<%s",xml.getName());
				writeParams(s,xml);
				s.appendf("/>");
			}
		}
		return true;
	}

	//ファイルへ出力
	bool out(FILE* file,Xml& xml)
	{
		if(!file)
			return false;
		INT level = 0;
		INT textFlag = true;
		
		//構成設定の出力
		fprintf(file,"<?xml");
		LPCSTR param;

		param = xml.getParam("version");
		if(param)
			fprintf(file," version=\"%s\"",param);
		else
			fprintf(file," version=\"1.0\"");

		param = xml.getParam("encoding");
		if(param)
			fprintf(file," encoding=\"%s\"",param);
		fputs("?>\n",file);

		if(xml.m_childXml.size())
			write(file,xml.m_childXml.front(),0,textFlag);
		return true;
	}

	void writeTab(FILE* file,INT level)
	{
		INT i;
		for(i=0;i<level;i++)
			fputc('\t',file);
	}

	bool writeParams(FILE* file,Xml& xml)
	{
		std::map<std::string,std::string>::iterator it;
		for(it=xml.m_option.begin();it!=xml.m_option.end();++it)
		{
			String name;
			String value;
			convertXMLString(name,it->first.c_str());
			convertXMLString(value,it->second.c_str());
			fprintf(file," %s=\"%s\"",name.c_str(),value.c_str());
		}
		return true;
	}
	bool write(FILE* file,Xml& xml,INT level,INT& textFlag)
	{
		if(xml.m_textFlag)
		{
			String dest;
			convertXMLString(dest,xml.getName());
			fputs(dest.c_str(),file);
			textFlag = true;
		}
		else
		{
			if(!textFlag)
			{
				fputc('\n',file);
				writeTab(file,level);
			}
			textFlag = false;
			if(xml.m_childXml.size())
			{
				fprintf(file,"<%s",xml.getName());
				writeParams(file,xml);
				fputs(">",file);

				std::list<Xml>::iterator it;
				for(it=xml.m_childXml.begin();it!=xml.m_childXml.end();++it)
					write(file,*it,level+1,textFlag);
				if(!textFlag)
				{
					fputc('\n',file);
					writeTab(file,level);
				}
				textFlag = false;
				fprintf(file,"</%s>",xml.getName());
			}
			else
			{
				fprintf(file,"<%s",xml.getName());
				writeParams(file,xml);
				fputs("/>",file);
			}
		}
		return true;
	}

	void read(LPCSTR& src,Xml& xml)
	{
		std::string text;
		while(read(src,text))
		{
			if(isData(text))
			{
				Xml xmlWork;
				INT level = readTag(text.c_str(),xmlWork);
				if(level == -1)
					break;
				if(level == -2)
				{
					xml = xmlWork;
				}
				else
				{
					xml.m_childXml.push_back(xmlWork);
				
					if(level == 1)
						read(src,xml.m_childXml.back());
				}
			}
		}
	}
	void decodeText(std::string& dest,LPCSTR src)
	{
		std::string text;
		CHAR c;
		for(;c = *src;src++)
		{
			if(c != '&')
				text += c;
			else
			{
				std::string work;
				for(src++;(c = *src) && c != ';';src++)
				{
					work += c;
				}
				if(work[0] == '#')
				{
					if(work[1] == 'x' || work[1] == 'X')
					{
						INT n;
						sscanf(work.c_str()+2,"%x",&n);
						text += (UCHAR)n;
					}
					else
						text += (UCHAR)atoi(work.c_str()+1);
				}
				else if(work == "amp")
					text += "&";
				else if(work == "lt")
					text += "<";
				else if(work == "gt")
					text += ">";
				else if(work == "quot")
					text += "\"";
				else if(work == "apos")
					text += "\'";
			}
		}
		dest = text;
	}
	INT readTag(LPCSTR text,Xml& xml)
	{
		INT i,j;
		//名前の取り出し
		//プレーンなテキストデータ
		std::string name;
		if(text[0] != '<')
		{
			name = text;
			xml.m_textFlag = true;
			return 0;
		}
		//終了タグか判定
		if(text[1] == '/')
			i=2;
		//構成要素
		else if(text[1] == '?')
			i=2;
		else
			i=1;
		//スペース除去
		for(;text[i] && text[i] != '>' && text[i] != ' ' && text[i] != '\t';i++);
		name.append(&text[1],i-1);
		decodeText(xml.m_name,name.c_str());

		while(text[i])
		{
			//スペース等の除去
			for(;text[i] && text[i] == ' ' || text[i] == '\t';i++);
			if(text[i] == '>')
				break;
			for(j=i;text[j] && text[j] != '>' && text[j] != '=' && text[j] != ' ' && text[j] != '\t';j++);
			std::string paramData;
			std::string paramName(&text[i],j-i);
			i=j;

			if(text[i] == '>')
			{
				if(paramName == "/")
					return 0;
				if(paramName == "?")
					return -2;
			}

			//スペース等の除去
			for(;text[i] && text[i] == ' ' || text[i] == '\t';i++);

			if(text[i] == '=')
			{
				//データの読み出し
				//スペース等の除去
				for(i++;text[i] && text[i] == ' ' || text[i] == '\t';i++);
				if(text[i] != '>')
				{
					UCHAR flag = 0;
					if(text[i] == '\'' || text[i] == '"')
					{
						flag = text[i++];
						for(;text[i] && text[i]!=flag;i++)
						{
							paramData += text[i];
						}
						i++;
					}
					else
					{
						for(;text[i] && text[i] != ' ' && text[i] != '\t';i++)
							paramData += text[i];
					}
				}
			}
			decodeText(paramName,paramName.c_str());
			decodeText(paramData,paramData.c_str());
			xml.m_option[paramName] = paramData;
		}
		if(text[1] == '/')
			return -1;
		return 1;
	}
	bool read(LPCSTR& src,std::string& text)
	{
		text.clear();
		text.reserve(2000);
		UCHAR flag = 0;
		UCHAR c;

		c = *src;
		if(!c)
			return false;
		src++;

		CHAR endChar;
		text += c;
		if(c == '<')
			endChar = '>';
		else
			endChar = '<';

		for(;(c = *src) && c != endChar;src++)
			text += c;

		if(c == '>')
		{
			src++;
			text += c;
		}
		return true;
	}


protected:
	bool isData(std::string& text)
	{
		INT i;
		LPCSTR data = text.c_str();
		for(i=0;data[i];i++)
		{
			switch(data[i])
			{
			case '\t':
			case ' ':
			case '\r':
			case '\n':
				break;
			default:
				return true;
			}
		}
		return false;
	}
};


Xml::Xml()
{
	m_textFlag = false;
}
bool Xml::loadMemory(LPCSTR data)
{
	XmlParser parser;
	return parser.loadMemory(data,*this);
}
bool Xml::load(LPCSTR fileName)
{
	XmlParser parser;
	return parser.loadFile(fileName,*this);
}
bool Xml::save(LPCSTR fileName)
{
	XmlParser parser;
	return parser.saveFile(fileName,*this);
}
bool Xml::load(LPCWSTR fileName)
{
	XmlParser parser;
	return parser.loadFile(fileName,*this);
}
bool Xml::save(LPCWSTR fileName)
{
	XmlParser parser;
	return parser.saveFile(fileName,*this);
}
bool Xml::out(FILE* file)
{
	XmlParser parser;
	return parser.out(file,*this);
}
bool Xml::get(String& s)
{
	XmlParser parser;
	return parser.out(s,*this);
}
LPCSTR Xml::getName()
{
	return m_name.c_str();
}
bool Xml::isParam(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator it = m_option.find(name);
	return it != m_option.end();
}
LPCSTR Xml::getParam(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator it = m_option.find(name);
	if(it != m_option.end())
		return it->second.c_str();
	return NULL;
}
DWORD Xml::getParamX(LPCSTR name) const
{
	LPCSTR value = getParam(name);
	DWORD data = 0;
	sscanf(value,"%x",&data);
	return data;
}
INT Xml::getParamInt(LPCSTR name) const
{
	LPCSTR value = getParam(name);
	if(value)
		return atoi(value);
	return 0;

}
FLOAT Xml::getParamFloat(LPCSTR name) const
{
	LPCSTR value = getParam(name);
	if(value)
		return (FLOAT)atof(value);
	return 0;

}
bool Xml::getParamBool(LPCSTR name) const
{
	LPCSTR value = getParam(name);
	if(value)
		return atoi(value) != false;
	return false;
}

void Xml::setName(LPCSTR name)
{
	m_name = name;
}
bool Xml::isText()const 
{
	return m_textFlag;
}
void Xml::setParamX(LPCSTR name,DWORD value)
{
	CHAR buff[100];
	sprintf(buff,"%X",value);
	m_option[name] = buff;
}
void Xml::setParam(LPCSTR name,DWORD value)
{
	CHAR buff[100];
	sprintf(buff,"%u",value);
	m_option[name] = buff;
}
void Xml::setParam(LPCSTR name,INT value)
{
	CHAR buff[100];
	sprintf(buff,"%d",value);
	m_option[name] = buff;
}
void Xml::setParam(LPCSTR name,FLOAT value)
{
	CHAR buff[100];
	sprintf(buff,"%f",value);
	m_option[name] = buff;
}
void Xml::setParam(LPCSTR name,LPCSTR value)
{
	m_option[name] = value;
}
void Xml::add(Xml& xml)
{
	m_childXml.push_back(xml);
}
bool Xml::addText(LPCSTR text)
{
	m_childXml.resize(m_childXml.size()+1);
	m_childXml.back().setName(text);
	m_childXml.back().m_textFlag = true;
	return true;
}
Xml* Xml::add(LPCSTR name)
{
	m_childXml.resize(m_childXml.size()+1);
	m_childXml.back().setName(name);
	return &m_childXml.back();
}
Xml* Xml::get(Xml* xml)
{
	if(m_childXml.size() == 0)
		return NULL;
	if(!xml)
		return &m_childXml.front();
	std::list<Xml>::iterator it;
	for(it=m_childXml.begin();it!=m_childXml.end();++it)
	{
		if(xml == &*it)
		{
			++it;
			if(it!=m_childXml.end())
				return &*it;
			else
				return NULL;
		}
	}	
	return NULL;
}
//namespace
};};

