#if defined(_WIN32) | defined(_WIN32_WCE)
	#include <windows.h>
#endif

#include "aflConfig.h"

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
		#define CHECK_MEMORY_LEAK _CrtsetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 
*/


namespace AFL{ namespace CONFIG{

Config::Config()
{
	m_quat = '"';
	m_comment = '#';
}
bool Config::load(LPCSTR fileName)
{
	FILE* file = fopen(fileName,"rt");
	if(!file)
		return false;

	CHAR c;
	std::string header;
	std::string name;
	std::string value;
	std::string work;
	CHAR quot = 0;

	INT mode = 0;
	CONFIGVALUE data;

	while((c = fgetc(file)) != EOF)
	{
		if(quot == 0)
		{
			//コメント処理
			if(m_comment == c)
			{
				while((c = fgetc(file)) != EOF)
					if(c == '\n' || c == '\r')
						break;
			}
			if(c == '\'' || c=='\"')
			{
				//クオート処理
				quot = c;
				continue;
			}
			else
			{
				//クオート外の無視キャラクタ
				if(c == '\n' || c == '\r')
				{
					value += work;
					work.clear();
					continue;
				}
				if(c=='\t' || c==' ')
					continue;
			}
		}
		else
		{
			//クオートの解除
			if(c == quot)
			{
				quot = 0;
				continue;
			}
		}

		if(mode == 0)
		{
			if(c == '[' && work.length()==0  && quot==0)
			{
				//データの追加
				if(header.length() && name.length())
				{
					data.insert(std::pair<std::string,std::string>(name,value));
					addData(header.c_str(),data);
					data.clear();
				}
				//ヘッダー読み取り状態
				header.clear();
				name.clear();
				value.clear();
				work.clear();
				mode = 1;
				continue;
			}
			else if(c == '=' && quot==0)
			{
				if(header.length() && name.length())
				{
					data.insert(std::pair<std::string,std::string>(name,value));
				}
				value.clear();
				name = work;
				work.clear();
			}
			else if(c == '\\')
			{
				c = fgetc(file);
				if(c == EOF)
					break;
				if(c == 't')
					work += '\t';
				else if(c == 'n')
					work += '\n';
				else if(c == '\\')
					work += '\\';
				else if(c == '"' || c == '\'')
					work += c;
			}
			else
			{
				work += c;
			}

		}
		else if(mode == 1)
		{
			//ヘッダの構成
			if(c == ']')
				mode = 0;
			else
				header += c;
			continue;
		}

	}
	if(header.length() && name.length())
	{
		data.insert(std::pair<std::string,std::string>(name,value));
		addData(header.c_str(),data);
	}
	fclose(file);
	return true;
}
bool Config::save(LPCSTR fileName) const
{
	FILE* file = fopen(fileName,"wt");
	if(!file)
		return false;

	CONFIGDATA::const_iterator itHeader;
	CONFIGVALUE::const_iterator itValue;
	
	foreach(itHeader,m_configData)
	{
		fprintf(file,"[%s]\n",itHeader->first.c_str());
		foreach(itValue,itHeader->second)
		{
			fprintf(file,"%s =",itValue->first.c_str());
			outValue(file,itValue->second.c_str());
		}
		fprintf(file,"\n");
	}
	return true;
}
void Config::addData(LPCSTR header,CONFIGVALUE& data)
{
	m_configData.insert(std::pair<std::string,CONFIGVALUE>(header,data));
}
void Config::outValue(FILE* file,LPCSTR value) const
{
	INT i;

	//数値か確認
	for(i=0;value[i] && isdigit(value[i]);i++);
	if(value[i] == 0 && i)
		fprintf(file," %s\n",value);
	else
	{
		bool ent = false;
		bool entStart = true;
		for(i=0;value[i] && value[i] != '\n';i++);
		if(value[i] == '\n')
		{
			ent = true;
			fputc('\n',file);
		}

		for(i=0;value[i];i++)
		{
			if(entStart)
			{
				if(ent)
					fprintf(file,"\t%c",m_quat);
				else
					fprintf(file,"%c",m_quat);
				entStart = false;
			}

			if(value[i] == '\n')
			{
				entStart = true;
				fprintf(file,"\\n%c\n",m_quat);
			}
			else if(value[i] == '\t')
			{
				fputs("\\t",file);
			}
			else if(value[i] == '\\')
			{
				fputs("\\\\",file);
			}
			else if(value[i] == m_quat)
			{
				fprintf(file,"\\%c",m_quat);
			}
			else
                fprintf(file,"%c",value[i]);
		}
		if(!entStart)
			fprintf(file,"%c\n",m_quat);
	}
}
INT Config::findHeader(LPCSTR header,LPCSTR name,LPCSTR value) const
{
	INT j;
	CONFIGDATA::const_iterator itHeader = m_configData.find(header);
	for(j=0;itHeader != m_configData.end() && itHeader->first == header;++j,++itHeader)
	{
		CONFIGVALUE::const_iterator itValue = itHeader->second.find(name);
		for(;itValue != itHeader->second.end() && itValue->first == name;++itValue)
			if(itValue->second == value)
				return j;
	}
	return -1;
}

LPCSTR Config::getData(LPCSTR header,INT index1,LPCSTR name,INT index2) const
{
	INT i,j;
	CONFIGDATA::const_iterator itHeader = m_configData.find(header);
	for(j=0;itHeader != m_configData.end() && itHeader->first == header;++j,++itHeader)
	{
		if(j==index1)
		{
			CONFIGVALUE::const_iterator itValue = itHeader->second.find(name);
			for(i=0;itValue != itHeader->second.end() && itValue->first == name;i++,++itValue)
			if(i == index2)
				return itValue->second.c_str();
		}
	}
	return NULL;
}

LPCSTR Config::getData(LPCSTR header,LPCSTR name,INT index) const
{
	CONFIGDATA::const_iterator itHeader = m_configData.find(header);
	if(itHeader == m_configData.end())
		return NULL;

	INT i;
	CONFIGVALUE::const_iterator itValue = itHeader->second.find(name);
	for(i=0;itValue != itHeader->second.end() && itValue->first == name;i++,++itValue)
		if(i == index)
			return itValue->second.c_str();
	return NULL;
}
INT Config::getCount(LPCSTR header,LPCSTR name) const
{
	if(!header)
		return (INT)m_configData.size();
	CONFIGDATA::const_iterator itHeader = m_configData.find(header);
	if(itHeader == m_configData.end())
		return 0;
	if(!name)
		return (INT)itHeader->second.size();

	INT i;
	CONFIGVALUE::const_iterator itValue = itHeader->second.find(name);
	for(i=0;itValue != itHeader->second.end() && itValue->first == name;i++,++itValue);
	return i;

}
}}
