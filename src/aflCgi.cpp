#include "aflCgi.h"
#include "aflZip.h"
#include "aflMd5.h"
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>



//----------------------------------------------------
//メモリリークテスト用
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
		#define NEW new
		#define CHECK_MEMORY_LEAK
#endif
//----------------------------------------------------


namespace AFL{ namespace CGI {

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HttpUrl::setUrl(LPCSTR url)
{
	std::string work;
	LPCSTR buff,buff2;

	//プロトコル部の確認
	buff = strchr(url,':');
	if(buff)
	{
		work.assign(url,buff-url);
		url = buff + 1;
	}
	if(work.length())
	{
		m_proto = work;
		work.clear();

		//絶対指定確認
		if(strncmp(url,"//",2))
			return false;
		url += 2;

		//ホスト部
		buff2 = strchr(url,'/');
		buff = strchr(url,':');
		if(buff2 && buff > buff2)
			buff = NULL;
		if(buff)
		{
			work.assign(url,buff-url-1);
			url = buff + 1;

			INT port = 0;
			for(;*url && *url>='0' && *url<='9';url++)
			{
				port *= 10;
				port += *url - '0';
			}
			m_port = port;
		}
		else
		{
			buff = strchr(url,'/');
			if(buff)
			{
				work.assign(url,buff-url);
				url = buff;
			}
			else
				work = url;
			m_port = 80;
			
		}
		m_host = work;
	}
	else
		m_proto = "http";


	for(;*url && *url!=' ' && *url!='?';url++)
		m_path += *url;
	if(m_path.length() == 0)
		m_path = '/';
	if(*url == '?')
	{
		for(++url;*url && *url!=' ';url++)
			m_param += *url;
	}

	return true;
}

MethodFile::MethodFile()
{
	m_size = 0;
	m_file = NULL;
}

bool MethodFile::save(LPCSTR fileName) const
{
	if(!m_file)
		return false;
	FILE* outFile = fopen(fileName,"wb");
	if(!outFile)
		return false;

	CHAR buff[4096];
	INT readSize;
	fseek(m_file,0,SEEK_SET);
	while(readSize = (INT)fread(buff,1,sizeof(buff),m_file))
	{
		fwrite(buff,readSize,1,outFile);
	}
	fclose(outFile);
	return true;
}

MethodBuffer::MethodBuffer()
{
	m_init = false;
	m_clientInst = NULL;
}
MethodBuffer::~MethodBuffer()
{
	//テンポラリファイルの削除
	std::multimap<std::string,MethodFile>::iterator it;
	foreach(it,m_methodFILE)
	{
		if(it->second.m_lname.length())
		{
			fclose(it->second.m_file);
			remove(it->second.m_lname.c_str());
		}
	}

}
void MethodBuffer::init(LPCSTR getData,LPCSTR cookieData,LPCSTR path)
{
	m_init = true;
	if(path)
		m_path = path;

	createENV();
	createGET(getData);
	createCOOKIE(cookieData);

}

void MethodBuffer::init(LPCSTR path)
{
	if(m_init)
		return;
	m_init = true;
	if(path)
		m_path = path;

	if(m_clientInst)
	{
		//Server
		INT i;
		CHAR c;
		ClientInst* clientInst = m_clientInst;
		String buff;
		SOCK::Sock* sock = clientInst->getSock();
		while(1)
		{
			INT size = sock->recv(&c,1,0,1000);
			if(size <= 0)
				break;
			if(c == '\n')
			{
				if(buff.size() == 0)
					break;
				clientInst->setParam(buff);
				buff.clear();
			}
			else if(c != '\r')
			{
				buff += c;
			}
		}
		std::map<std::string,std::string> contentMap;
		createEnvMap2(contentMap,clientInst->getHeader("CONTENT-TYPE:"));

		if(contentMap.find("boundary") == contentMap.end())
		{
			//シングルパートデータ
			INT contentLength = clientInst->getContentLength();
			for(i=0;i<contentLength;i++)
			{
				INT size = sock->recv(&c,1,0,1000);
				if(size <= 0)
					break;
				buff += c;
			}
		}
		else
		{
			//マルチパートデータ
			m_multipartString = "--";
			m_multipartString += contentMap["boundary"];
			INT contentLength = clientInst->getContentLength();
			LPSTR buff = new CHAR[contentLength];
			LPSTR buffWork = buff;
			while(contentLength)
			{
				INT size = sock->recv(buff,contentLength,0,5000);
				if(size <= 0)
					break;
				contentLength -= size;
				buff += size;
			}
			clientInst->setMutipartData(buffWork);
			createMultipart();
			delete[] buffWork;
		}
		m_methodPOSTData.assign(buff);
		init(clientInst->GET(),clientInst->getHeader("COOKIE:"));
		String url;
		LPCSTR hostName = clientInst->getHeader("HOST:");
		url.printf("http://%s%s",hostName,clientInst->getPath());
		m_url = url;

	}
	else
	{
		//CGI
		std::map<std::string,std::string> contentMap;
		createEnvMap(contentMap,"CONTENT_TYPE");
		if(contentMap.find("boundary") == contentMap.end())
			createPOST();
		else
		{
			m_multipartString = "--";
			m_multipartString += contentMap["boundary"];
			createMultipart();
		}
		if(getenv("HTTP_HOST") && getenv("REQUEST_URI"))
		{
			m_url = "http://";
			m_url += getenv("HTTP_HOST");
			LPCSTR uri = getenv("REQUEST_URI");
			LPCSTR q = strchr(uri,'?');
			if(q)
				m_script.append(uri,q-uri);
			else
				m_script += uri;
			m_url += m_script;
		}
		createENV();
		createGET();
		createCOOKIE();
	}
}

bool MethodBuffer::isGET(LPCSTR name) const
{
	std::multimap<std::string,std::string>::const_iterator itMap;
	itMap = m_methodGET.find(name);
	return itMap != m_methodGET.end();
}
bool MethodBuffer::isPOST(LPCSTR name) const
{
	std::multimap<std::string,std::string>::const_iterator itMap;
	itMap = m_methodPOST.find(name);
	return itMap != m_methodPOST.end();
}

bool MethodBuffer::isCOOKIE(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator itMap;
	itMap = m_methodCOOKIE.find(name);
	return itMap != m_methodCOOKIE.end();
}
bool MethodBuffer::isFILE(LPCSTR name) const
{
	std::multimap<std::string,MethodFile>::const_iterator itMap;
	itMap = m_methodFILE.find(name);
	return itMap != m_methodFILE.end();
}
bool MethodBuffer::isENV(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator itMap;
	itMap = m_methodENV.find(name);
	return itMap != m_methodENV.end();
}
LPCSTR MethodBuffer::ENV(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator itMap;
	itMap = m_methodENV.find(name);
	if(itMap != m_methodENV.end())
		return itMap->second.c_str();
	return NULL;
}

LPCSTR MethodBuffer::GET(LPCSTR name,INT index) const
{
	INT i;
	std::multimap<std::string,std::string>::const_iterator itMap;
	itMap = m_methodGET.find(name);
	for(i=0;itMap != m_methodGET.end() && itMap->first == name;++itMap,i++)
	{
		if(i == index)
			return itMap->second.c_str();
	}
	return NULL;
}
LPCSTR MethodBuffer::POST(LPCSTR name,INT index)
{
	if (m_methodPOST.size() == 0)
		createPOST(m_methodPOSTData.c_str());


	INT i;
	std::multimap<std::string,std::string>::const_iterator itMap;
	itMap = m_methodPOST.find(name);
	for(i=0;itMap != m_methodPOST.end() && itMap->first == name;++itMap,i++)
	{
		if(i == index)
			return itMap->second.c_str();
	}
	return NULL;}

LPCSTR MethodBuffer::COOKIE(LPCSTR name) const
{
	std::map<std::string,std::string>::const_iterator itMap;
	itMap = m_methodCOOKIE.find(name);
	if(itMap != m_methodCOOKIE.end())
		return itMap->second.c_str();
	return NULL;
}
const MethodFile* MethodBuffer::FILE(LPCSTR name,INT index) const
{
	INT i;
	std::multimap<std::string,MethodFile>::const_iterator itMap;
	if(name)
		itMap = m_methodFILE.find(name);
	else
		itMap = m_methodFILE.begin();
	for(i=0;itMap != m_methodFILE.end() && (!name || itMap->first == name);++itMap,i++)
	{
		if(i == index)
			return &itMap->second;
	}
	return NULL;
}
bool MethodBuffer::decodeURI(std::string& dest,LPCSTR src)
{
	while(getDecodeChar(dest,src));
	return true;
}
bool MethodBuffer::getDecodeChar(std::string& dest,LPCSTR& data)
{
	char c = *(data++);
	if(!c)
		return false;
	if(c == '+')
		dest += ' ';
	else if(c == '%')
	{
		if(data[0] && data[1])
		{
			int i;
			UCHAR v = 0;
			for(i=0;i<2;i++)
			{
				v *= 16;
				c = *(data++);
				if(c >= '0' && c <= '9')
					v += c - '0';
				else if(c >= 'A' && c <= 'F')
					v += c - 'A' + 10;
				else if(c >= 'a' && c <= 'f')
					v += c - 'a' + 10;
				else
					return false;
			}
			dest += (UCHAR)v;
		}
		else
			return false;
	}
	else
		dest += c;
	return true;
}


bool MethodBuffer::createENV()
{
	//m_methodENV
	if(m_clientInst)
	{
		CHAR buff[128];
		if(m_clientInst->getSock()->getPeerIP(buff))
			m_methodENV["REMOTE_ADDR"] = buff;

		std::map<String,String>& params = m_clientInst->m_params;
		std::map<String,String>::iterator it;
		foreach(it,params)
		{
			if(it->first == "USER-AGENT:")
				m_methodENV["HTTP_USER_AGENT"] = it->second;
			else if(it->first == "REFERER:")
				m_methodENV["HTTP_REFERER"] = it->second;
			else if (it->first == "SEC-WEBSOCKET-KEY:")
				m_methodENV["HTTP_SEC_WEBSOCKET_KEY"] = it->second;
			//printf("%s %s\n",it->first.c_str(),it->second.c_str());
		}

	}
	else
	{
		LPCSTR work;
		if(work = getenv("REMOTE_ADDR"))
			m_methodENV["REMOTE_ADDR"] = work;
		if(work = getenv("HTTP_USER_AGENT"))
			m_methodENV["HTTP_USER_AGENT"] = work;
		if(work = getenv("HTTP_REFERER"))
			m_methodENV["HTTP_REFERER"] = work;
		if (work = getenv("HTTP_SEC_WEBSOCKET_KEY"))
			m_methodENV["HTTP_SEC_WEBSOCKET_KEY"] = work;
	}
	return true;
}
bool MethodBuffer::createPOST()
{
	LPCSTR lengthText = getenv("CONTENT_LENGTH");
	INT length;
	if(!lengthText)
		return false;
	length = atoi(lengthText);

	LPSTR dataSRC = new CHAR[length + 1];
	LPCSTR data = dataSRC;

	if(!data)
		return false;
	fread(dataSRC,length,1,stdin);
	m_methodPOSTData.assign(dataSRC, length);
	//createPOST(data);
	delete[] dataSRC;
	return true;
}
bool MethodBuffer::createPOST(LPCSTR data)
{
	std::string name;
	std::string value;
	name.reserve(512);
	value.reserve(512);
	while(*data)
	{
		name.clear();
		value.clear();
		while(*data && *data!='=' && *data!='&')
			getDecodeChar(name,data);
		if(!*data)
			break;
		if(*data == '=')
		{
			data++;
			while(*data && *data!='&')
				getDecodeChar(value,data);
		}
		else if(*data == '&')
			data++;

		m_methodPOST.insert(std::pair<std::string,std::string>(name,value));
	}
	return true;
}

bool MethodBuffer::createEnvMap(std::map<std::string,std::string>& map,LPCSTR envName)
{
	LPCSTR data = getenv(envName);
	if(!data)
		return false;
	return createEnvMap2(map,data);
}
bool MethodBuffer::createEnvMap2(std::map<std::string,std::string>& map,LPCSTR data)
{
	std::string name;
	std::string value;
	name.reserve(512);
	value.reserve(512);
	while(*data)
	{
		name.clear();
		value.clear();

		//先頭スペースの除去
		for(;*data == ' ';data++);
		while(*data && *data!='=' && *data!=';')
		{
			getDecodeChar(name,data);
		}
		if(!*data)
			break;
		if(*data == '=')
		{
			data++;
			while(*data && *data!=';')
				getDecodeChar(value,data);
		}
		else if(*data == ';')
			data++;
		if(map.find(name) == map.end())
			map[name] = value;
	}
	return true;	
}
bool MethodBuffer::createGET()
{
	LPCSTR data = getenv("QUERY_STRING");
	if(!data)
		return false;
	return createGET(data);
}
bool MethodBuffer::createGET(LPCSTR data)
{
	std::string name;
	std::string value;
	name.reserve(512);
	value.reserve(512);
	while(*data)
	{
		name.clear();
		value.clear();
		while(*data && *data!='=' && *data!='&')
			getDecodeChar(name,data);
		if(!*data)
			break;
		if(*data == '=')
		{
			data++;
			while(*data && *data!='&')
				getDecodeChar(value,data);
		}
		if(*data)
			++data;

		m_methodGET.insert(std::pair<std::string,std::string>(name,value));
	}
	return true;

}
bool MethodBuffer::createCOOKIE(LPCSTR data)
{
	return createEnvMap2(m_methodCOOKIE,data);
}

bool MethodBuffer::createCOOKIE()
{
	return createEnvMap(m_methodCOOKIE,"HTTP_COOKIE");
}
INT MethodBuffer::isMultipartString(std::vector<CHAR>& buff,INT& length)
{
	CHAR c;
	INT i;
	INT multipartLength;
	LPCSTR multipartString; 
	multipartLength = (INT)m_multipartString.length();
	multipartString = m_multipartString.c_str();

	if(length == 0)
		return -2;
	for(i=0;i<multipartLength;i++)
	{
		if(length == 0)
			return 0;
		length--;
		c = getChar();

		buff.push_back(c);
		if(c != multipartString[i])
			return 0;

	}
	c = getChar();
	if(c == '-')
		return 0;
	if(c == '\r')
		getChar();

	return 1;
}
bool MethodBuffer::readHeader(
	std::map<std::string,std::map<std::string,std::string> >&map,INT& length)
{
	CHAR c;
	std::string buff;
	std::map<std::string,std::string> params;
	while(1)
	{
		buff.clear();
		params.clear();
		for(;length;length--)
		{
			c = getChar();
			if(c == '\r')
			{
				c = getChar();
				break;
			}
			if(c == '\n')
				break;
			buff += c;
		}
		if(length < 2)
			return false;
		if(c != '\n')
			return false;
		if(buff.length() == 0)
			break;

		INT i;
		std::string header;
		LPCSTR lineString = buff.c_str();
		for(i=0;lineString[i] && lineString[i]!=':';i++);
		if(lineString[i]!=':')
			continue;
		header.assign(lineString,i);
		convertHtmlToBin(header);

		for(i++;lineString[i];i++)
		{
			std::string name,value;
			//スペースの除去
			for(i++;lineString[i]==' ';i++);

			bool flag;
			//項目名の読み出し
			for(flag=false;lineString[i] && (flag || (lineString[i] !='=' && lineString[i] !=';'));i++)
			{
				if(lineString[i] == '"')
					flag = !flag;
				else
					name += lineString[i];
			}
			//値の読み出し
			if(lineString[i] =='=')
			{
				for(flag=false,i++;lineString[i] && (flag || lineString[i] !=';');i++)
				{
					if(lineString[i] == '"')
						flag = !flag;
					else
						value += lineString[i];
				}
			}
			//convertHtmlToBin(name);
			//convertHtmlToBin(value);
			
			params[name] = value;
			if(!lineString[i])
				break;
		}
		map[header] = params;
	}
	return true;
}
void MethodBuffer::convertHtmlToBin(std::string& string)
{
	INT i;
	WORD value;
	std::string dest;
	LPCSTR src = string.c_str();
	for(i=0;src[i];i++)
	{
		if(src[i] == '&')
		{
			value = 0;
			for(i+=2;src[i] && src[i] !=';';i++)
			{
				value *= 10;
				value += (WORD)src[i] - '0';
			}
			dest += (CHAR)(value & 0xff);
			dest += (CHAR)(value >> 8);
		}
		else
			dest += src[i];
	}
	string = dest;
}

bool MethodBuffer::readMultipartPost(INT& length,LPCSTR name)
{
	std::vector<CHAR> buff;
	std::string value;
	INT ret;
	while((ret = isMultipartString(buff,length)) == 0)
	{
		value.append(&buff[0],buff.size());
		buff.clear();
	}
	if(ret == -2)
		return false;
	value += (char)0;
	m_methodPOST.insert(std::pair<std::string,std::string>(name,&value[0]));
	return true;
}



bool MethodBuffer::readMultipartFile(INT& length,LPCSTR name,LPCSTR fileName)
{
	static INT localID = 0;

	MethodFile methodFile;

	::FILE* file;
	if(m_path.length())
	{
		//ディレクトリの設定
		std::string localName = m_path;
		CHAR lc = localName[localName.length()-1];
		if(lc != '\\' || lc != '/')
			localName += '/';

		//プロセスIDからファイル名の生成
		INT pid = getpid();
		String s;
		s.printf("temp%05d.%d",pid,localID++);

		localName += s;
		file = fopen(localName.c_str(),"wb+");
		if(file)
			methodFile.m_lname = localName;

	}
	else
		file = tmpfile();
	if(!file)
		return false;

	INT multipartLength = (INT)m_multipartString.length();
	LPCSTR multipartString = m_multipartString.c_str(); 

	INT i;
	INT fileNameLength = (INT)strlen(fileName);
	for(i=fileNameLength-1;i>=0;i--)
	{
		if(fileName[i] == '/' || fileName[i] == '\\')
			break;
	}
	methodFile.m_name = fileName + i+1;

	CHAR c;
	CHAR* buff = new CHAR[multipartLength];
	if(m_clientInst)
		m_clientInst->read(buff,multipartLength);
	else
		fread(buff,multipartLength,1,stdin);
	length -= multipartLength;
	while(length)
	{
		if(strncmp(buff,multipartString,multipartLength) == 0)
			break;
		fputc(buff[0],file);
		memcpy(buff,buff+1,multipartLength-1);
		buff[multipartLength-1] = getChar();
		length--;
	}
	delete[] buff;

	methodFile.m_size = (INT)ftell(file);
	methodFile.m_file = file;

	m_methodFILE.insert(std::pair<std::string,MethodFile>(name,methodFile));

	if(length)
	{
		c = getChar();
		if(c == '-')
		{
			getChar();
			c = getChar();
		}
		if(c == '\r')
			c = getChar();
		if(c != '\n')
			return false;
	}
	return true;
}

bool MethodBuffer::createMultipart()
{
	if(!m_clientInst)
		setBinary(stdin);

	INT length;
	if(m_clientInst)
		length = m_clientInst->getContentLength();
	else
	{
		LPCSTR lengthText = getenv("CONTENT_LENGTH");
		if(!lengthText)
			return false;
		length = atoi(lengthText);
	}
	INT point = 0;
	std::vector<CHAR> buff;

	INT multipartLength = (INT)m_multipartString.length();
	LPCSTR multipartString = m_multipartString.c_str(); 

	if(length < multipartLength)
		return false;
	CHAR* buffPart = new CHAR[multipartLength];
	bool error;
	if(m_clientInst)
		error = m_clientInst->read(buffPart,multipartLength)!=multipartLength || strncmp(multipartString,buffPart,multipartLength) != 0;
	else
		error = fread(buffPart,multipartLength,1,stdin)!=1 || strncmp(multipartString,buffPart,multipartLength) != 0;
	delete[] buffPart;
	
	if(error)
		return false;

	length -= multipartLength;

	CHAR c = getChar();
	if(c == '\r')
	{
		getChar();
		m_multipartString.insert(0,"\r\n");
		length -= 2;
	}
	else
	{
		m_multipartString.insert(0,"\n");
		length--;
	}

	bool flag = true;
	while(flag)
	{
		std::map<std::string,std::map<std::string,std::string> > headerMap;
		if(!readHeader(headerMap,length))
			return false;

		std::map<std::string,std::map<std::string,std::string> >::iterator itHeader;
		itHeader = headerMap.find("Content-Disposition");
		if(itHeader == headerMap.end())
			return false;

		std::map<std::string,std::string>::iterator itName,itFileName;
		itFileName = itHeader->second.find("filename");
		itName = itHeader->second.find("name");
		if(itName != itHeader->second.end())
		{
			if(itFileName == itHeader->second.end())
				flag = readMultipartPost(length,itName->second.c_str());
			else
				flag = readMultipartFile(length,itName->second.c_str(),itFileName->second.c_str());
		}
	}
	return true;
}



bool Method::isGET(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].isGET(name);
}
bool Method::isCOOKIE(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].isCOOKIE(name);
}
bool Method::isPOST(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].isPOST(name);
}
bool Method::isFILE(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].isFILE(name);
}
bool Method::isENV(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].isENV(name);
}
LPCSTR Method::ENV(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].ENV(name);
}


LPCSTR Method::GET(LPCSTR name,INT index)
{
	return m_methodBuffer[getCurrentThreadID()].GET(name,index);
}
LPCSTR Method::POST(LPCSTR name,INT index)
{
	return m_methodBuffer[getCurrentThreadID()].POST(name,index);
}
LPCSTR Method::POST()
{
	return m_methodBuffer[getCurrentThreadID()].POST();
}
INT Method::getPostLength()
{
	return m_methodBuffer[getCurrentThreadID()].getPostLength();
}

static std::string& POST();

LPCSTR Method::COOKIE(LPCSTR name)
{
	return m_methodBuffer[getCurrentThreadID()].COOKIE(name);
}
LPCSTR Method::URL()
{
	return m_methodBuffer[getCurrentThreadID()].m_url.c_str();
}
LPCSTR Method::SCRIPT()
{
	return m_methodBuffer[getCurrentThreadID()].m_script.c_str();
}
const MethodFile* Method::FILE(LPCSTR name,INT index)
{
	return m_methodBuffer[getCurrentThreadID()].FILE(name,index);
}
const MethodFile* Method::FILE(INT index,std::string& name)
{
	INT i;
	std::multimap<std::string,MethodFile>::const_iterator itMap;
	itMap = m_methodBuffer[getCurrentThreadID()].m_methodFILE.begin();
	for(i=0;itMap != m_methodBuffer[getCurrentThreadID()].m_methodFILE.end();++itMap,i++)
	{
		if(i == index)
		{
			name = itMap->first;
			return &itMap->second;
		}
	}
	return NULL;
}

void Method::POST(std::vector<LPCSTR>& dest,LPCSTR name)
{
	dest.reserve(m_methodBuffer[getCurrentThreadID()].m_methodPOST.size());
	std::multimap<std::string,std::string>::iterator it;
	for(it=m_methodBuffer[getCurrentThreadID()].m_methodPOST.find(name);
		it!=m_methodBuffer[getCurrentThreadID()].m_methodPOST.end() && it->first == name;++it)
	{
		dest.push_back(it->second.c_str());
	}
}

INT Method::numPOST(LPCSTR name)
{
	if(!name)
		return (INT)m_methodBuffer[getCurrentThreadID()].m_methodPOST.size();
	std::multimap<std::string,std::string>::iterator it;
	INT i = 0;
	foreach(it,m_methodBuffer[getCurrentThreadID()].m_methodPOST)
	{
		if(it->first != name)
			break;
		i++;
	}
	return i;
}
INT Method::numGET(LPCSTR name)
{
	if(!name)
		return (INT)m_methodBuffer[getCurrentThreadID()].m_methodGET.size();
	std::multimap<std::string,std::string>::iterator it;
	INT i = 0;
	foreach(it,m_methodBuffer[getCurrentThreadID()].m_methodGET)
	{
		if(it->first != name)
			break;
		i++;
	}
	return i;
}
INT Method::numFILE(LPCSTR name)
{
	if(!name)
		return (INT)m_methodBuffer[getCurrentThreadID()].m_methodFILE.size();
	std::multimap<std::string,MethodFile>::iterator it;
	INT i = 0;
	foreach(it,m_methodBuffer[getCurrentThreadID()].m_methodFILE)
	{
		if(it->first != name)
			break;
		i++;
	}
	return i;
}


void Method::init(LPCSTR path)
{
	m_methodBuffer[getCurrentThreadID()].init(path);
}
void Method::setClientInst(ClientInst* clientInst)
{
	if(clientInst)
		m_methodBuffer[getCurrentThreadID()].setClientInst(clientInst);
	else
		m_methodBuffer.erase(getCurrentThreadID());
}

std::map<INT,MethodBuffer> Method::m_methodBuffer;



Stream::Stream(bool enable)
{
	m_compress = false;
	m_socket = NULL;
	m_enable = enable;
	m_level = 0;
	m_output = false;
	m_fileError = false;
	m_length = 0;
	m_header["Content-type"] = "text/html; charset=utf-8";
	m_stream.reserve(50000);
}
Stream::~Stream()
{
	if(m_enable)
		outBody();
}
void Stream::setWebSocketKey(LPCSTR key)
{
	String work;
	work.printf("%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);
	UINT8 sha1[20];
	SHA1::parse(sha1, work);
	String rkey;
	BASE64(rkey, sha1, 20);
	setHeader("Upgrade", "websocket");
	setHeader("Connection", "Upgrade");
	setHeader("Sec-WebSocket-Accept", rkey);
}
void Stream::setHeader(LPCSTR name,LPCSTR value)
{
	m_header[name] = value;
}
void convert(String& dest,LPCSTR src)
{
	INT i;
	for(i=0;src[i];i++)
	{
		char c = src[i];
		if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') ||
			c=='*' || c=='-' || c=='.' || c=='@' || c=='_')
		{
			dest += src[i];
		}
		else if(c == ' ')
		{
			dest += '+';
		}
		else
			dest.appendf("%%%02X",(UCHAR)c);
	}
}
void Stream::setFile(LPCSTR fileName,LPCSTR localName,INT size,LPCSTR type)
{
	if(localName)
	{
		m_fileName = fileName;
		m_localName = localName;
		m_fileSize = size;
		m_fileType = type;
	}
	else
	{
		struct _stat s;
		if(::_stat(fileName,&s) == 0)
		{
			LPCSTR w = strrchr(fileName,'/');
			if(w)
				m_fileName = w+1;
			else
				m_fileName = fileName;

			w = strrchr(m_fileName,'.');
			if(w)
			{
				m_fileType = w + 1;
				m_fileType.toUpper();
			}
			m_localName = fileName;
			m_fileSize = s.st_size;
			m_fileError = false;
		}
		else
			m_fileError = true;
	}
}
bool Stream::outHeader()
{
	//ヘッダの出力確認
	if(m_output)
		return false;
	String dest;
	setBinary(stdout);

	if(m_socket)
	{
		if(m_fileError)
		{
			m_stream = "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY>Not Found.</BODY></HTML>";
			dest = "HTTP/1.1 404 Not Found\r\n";
		}
		else if (m_header.find("Upgrade") != m_header.end())
		{
			dest = "HTTP/1.1 101 Switching Protocols\r\n";
		}
		else if (m_header.find("Location") != m_header.end())
			dest = "HTTP/1.1 301 Moved Permanently\r\n";
		else
			dest = "HTTP/1.1 200 OK\r\n";
	}
	else
	{
		if (m_header.find("Upgrade") != m_header.end())
		{
			dest = "HTTP/1.1 101 Switching Protocols\r\n";
		}
	}

	//ファイル転送処理
	if(m_localName.length())
	{
		m_length = m_fileSize;
		if(m_fileType == "HTML" || m_fileType == "HTM" || m_fileError)
			setHeader("Content-type","text/html");
		else if(m_fileType == "TXT")
			setHeader("Content-type","text/plain");
		else if(m_fileType == "CSS")
			setHeader("Content-type","text/css");
		else if(m_fileType == "GIF")
			setHeader("Content-type","image/gif");
		else if(m_fileType == "PNG")
			setHeader("Content-type","image/png");
		else if(m_fileType == "JPEG" || m_fileType == "JPG")
			setHeader("Content-type","image/jpeg");
		else if(m_fileType == "ICO")
			setHeader("Content-type","image/x-icon");
		else if(m_fileType == "CGI")
			setHeader("Content-type","application/pdf");
		else
			setHeader("Content-type","application/octet-stream");
	}
	if(m_fileName.length())
	{
		String work;
		work.printf("filename=\"%s\"",m_fileName.c_str());
		setHeader("Content-Disposition",work.c_str());
	}

	//独自ヘッダが設定されているか確認
	std::map<std::string,std::string>::iterator itHeader;
	for(itHeader=m_header.begin();itHeader!=m_header.end();++itHeader)
	{
		dest.appendf("%s: %s\r\n",itHeader->first.c_str(),itHeader->second.c_str());
	}
#ifdef ZipWriter
	if(m_compress && !m_localName.length())
	{
		dest += "Content-Encoding: gzip\r\n";
		//m_header.erase("Content-Length");
	}
#endif
	itHeader = m_header.find("Content-Length");
	if(itHeader == m_header.end())
	{
		if(m_length)
			dest.appendf("%s%d\r\n","Content-Length: ",m_length);
	}




	m_output = true;


	//クッキーの出力
	if(m_cookie.size())
	{
		std::map<std::string,Cookie>::iterator it;
		for(it=m_cookie.begin();it!=m_cookie.end();++it)
		{
			tm ntime;
			time_t t;
			String name;
			String param;
			convert(name,it->first.c_str());
			convert(param,it->second.value.c_str());
			
			if(it->second.expires == 0)
			{
				dest.appendf("%s %s=%s; PATH=%s;\r\n",
					"Set-Cookie:",
					name.c_str(),
					param.c_str(),
					it->second.path.c_str());
			}
			else
			{
				if(it->second.expires == -1)
				{
					time(&t);
					t += 60*60*24*30;
				}
				else
					t = it->second.expires;
				ntime = *gmtime(&t);

				static LPCSTR mon[] =
				{
					"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
				};
				static LPCSTR wday[] =
				{
					"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
				};


				dest.appendf("%s %s=%s; PATH=%s; expires=%s, %02d-%s-%04d %02d:%02d:%02d GMT;\r\n",
					"Set-Cookie:",
					name.c_str(),
					param.c_str(),
					it->second.path.c_str(),
					wday[ntime.tm_wday],
					ntime.tm_mday,
					mon[ntime.tm_mon],
					ntime.tm_year+1900,
					ntime.tm_hour,
					ntime.tm_min,
					ntime.tm_sec
					);
			}
		}
	}
	dest.appendf("\r\n");


	if(m_socket)
	{
		while(m_socket->send(dest.c_str(),(INT)dest.length()) == 0)
			Sleep(1);
	}
	else
	{
		fwrite(dest.c_str(), dest.length(), 1, stdout);
		fflush(stdout);
	}
	return true;
}
bool Stream::outTab()
{
	INT i;
	for(i=0;i<m_level;i++)
	{
		m_stream += '\t';
	}
	return true;
}
bool Stream::outBody()
{

	if(m_localName.length())
	{
		//ヘッダの出力
		outHeader();

		INT size;
		CHAR buff[10000];
		FILE* file = fopen(m_localName.c_str(),"rb");
		if(file)
		{
			while(size = (INT)fread(buff,1,sizeof(buff),file))
			{
				if(m_socket)
				{
					while(m_socket->send(buff,size) == 0)
						Sleep(10);
				}
				else
					fwrite(buff,size,1,stdout);
			}
			fflush(stdout);
			fclose(file);
		}
	}
	else
	{
		m_length = (INT)m_stream.length();
#ifdef ZipWriter
		//本文の出力
		if(m_compress)
		{
			BinaryStream bs;
			ZipWriter z;
			z.open(&bs);
			z.write(m_stream.c_str(),m_length);
			z.close();

			m_length = bs.getSize();
			outHeader();
			if(m_socket)
			{

				m_socket->send2(bs.getData(),bs.getSize());
				m_socket->close();
			}
			else
			{
				fwrite(bs.getData(),m_length,1,stdout);
				fflush(stdout);
			}
		}
		else
#endif	
		{

			outHeader();
			m_length = (INT)m_stream.length();
			if(m_socket)
			{
				if(m_length)
				{
					while(m_socket->send(m_stream.c_str(),m_length) == 0)
						Sleep(10);
				}
				m_socket->close();
			}
			else
			{
				fwrite(m_stream.c_str(),m_length,1,stdout);
				fflush(stdout);
			}
		}
	}
	if(m_socket)
	{
		m_socket->waitSend();
		m_socket->close();
	}
	return true;
}
bool Stream::setCookie(LPCSTR name,LPCSTR value,LPCSTR path,time_t expires)
{
	Cookie cookie = {value,path,expires};
	m_cookie[name] = cookie;
	return true;
}
void Stream::block()
{
	INT length = (INT)m_stream.length();
	if(length && m_stream[length-1] != '\n')
	{
		m_stream += "\n";
		outTab();
	}
}
bool Stream::send(LPCVOID src)
{
	return send((LPVOID)src, strlen((char*)src), false);
}
bool Stream::send(LPCVOID src, INT length, bool binary)
{
	//ヘッダの作成
	BYTE head;
	if (binary)
		head = 0x82;
	else
		head = 0x81;
	if (m_socket)
		m_socket->send(&head, 1);
	else
		fwrite(&head, 1, 1, stdout);

	//データ長の作成
	if (length >= 0x10000)
	{
		BYTE len[8];
		int64_t src = length;
		for (int i = 0; i < 8; i++)
		{
			len[i] = ((LPBYTE)&src)[8 - i];
		}
		if (m_socket)
			m_socket->send(len, 8);
		else
			fwrite(&len, 8, 1, stdout);
	}
	else if (length >= 126)
	{
		BYTE len[2];
		WORD src = (WORD)length;
		len[0] = ((LPBYTE)&src)[1];
		len[1] = ((LPBYTE)&src)[0];
		if (m_socket)
			m_socket->send(len, 2);
		else
			fwrite(&len, 2, 1, stdout);
	}
	else
	{
		BYTE len = (BYTE)length;
		if (m_socket)
			m_socket->send(&len, 1);
		else
			fwrite(&len, 1, 1, stdout);
	}
	//データ本体の送信
	if (m_socket)
		m_socket->send(src, length);
	else
		fwrite(src, length, 1, stdout);
	return true;
}
bool Stream::getByte(LPBYTE value)
{
	if (m_socket)
	{
		int size;
		while ((size = m_socket->recv(value, 1)) == 0)
			Sleep(1);
		if (size == -1)
			return false;
	}
	else
	if (fread(value, 1, 1, stdin) <= 0)
		return false;
	return true;
}
bool Stream::read(LPVOID dest, int size)
{
	int pt = 0;
	int length = size;
	if (m_socket)
	{
		while (pt < size)
		{
			int ret;
			while ((ret = m_socket->recv((LPBYTE)dest + pt, length)) == 0)
				Sleep(1);
			if (ret == -1)
				return false;
			length -= ret;
			pt += ret;
		}
	}
	else
	{
		if (fread(dest, size, 1, stdin) < 1)
			return false;
	}
	return true;
}
INT Stream::printf(LPCSTR format,...)
{
	va_list args;
	va_start(args,format);


	std::string dest;
	INT r = strprintf(dest,format,args);
	va_end(args);

	INT length = (INT)m_stream.length();
	if(length && m_stream[length-1] == '\n')
	{
		outTab();
		m_stream += dest;
//		m_stream += "\r\n";
	}
	else
	{
		m_stream += dest;
	}
	return r;
}
INT Stream::puts(LPCSTR string)
{
	INT length = (INT)m_stream.length();
	if(length && m_stream[length-1] == '\n')
	{
		outTab();
	}
	m_stream += string;
	m_stream += "\n";
	return 0;
}
INT Stream::out(LPCSTR string)
{
	INT length = (INT)m_stream.length();
	if(length && m_stream[length-1] == '\n')
	{
		outTab();
	}
	INT i;
	for(i=0;string[i];i++)
	{
		m_stream += string[i];
		if(string[i] == '\n' && string[i+1])
			outTab();
	}
	return 0;
}

size_t Stream::outFile(LPCSTR fileName)
{
	FILE* file = fopen(fileName,"r");
	if(!file)
		return 0;
	size_t size;
	size_t allSize = 0;
	CHAR buff[1024];
	while(size = fread(buff,1,sizeof(buff),file))
	{
		m_stream.append(buff,size);
		allSize += size;
	}
	fclose(file);
	return allSize;
}
void Stream::write(LPVOID data,INT size)
{
	m_stream.append((LPCSTR)data,size);
}
void TextToHtml(String& dest,LPCSTR src)
{
	Sleep(10);
	INT i;
	dest.clear();
	for(i=0;src[i];i++)
	{
		if(src[i] == '\t')
			dest += "&nbsp;&nbsp;&nbsp;&nbsp;";
		else if(src[i] == ' ')
			dest += "&nbsp;";
		else if(src[i] == '&')
			dest += "&amp;";
		else if(src[i] == '<')
			dest += "&lt;";
		else if(src[i] == '>')
			dest += "&gt;";
		else if(src[i] == '\r' || src[i] == '\n')
		{
			dest += "<BR>\n";
			if(src[i] == '\n')
				i++;
		}
		else
			dest += src[i];
	}
}
//------------------------------------------------------------
// ClientInst
// クライアントインスタンス管理
//------------------------------------------------------------
ClientInst::ClientInst(SOCK::Sock* sock)
{
	m_sock = sock;
	m_contentLength = 0;
	m_multipartData = NULL;
	m_multipartPoint = 0;
}
ClientInst::~ClientInst()
{
	if(m_sock)
		delete m_sock;
}
SOCK::Sock* ClientInst::getSock()const
{
	return m_sock;
}
CHAR ClientInst::getChar()
{
	if(!m_multipartData || m_multipartPoint >= m_contentLength)
		return 0;
	return  m_multipartData[m_multipartPoint++];
}
INT ClientInst::read(LPSTR dest,INT size)
{
	INT l = m_contentLength - m_multipartPoint;
	if(size > l)
		size = l;
	memcpy(dest,m_multipartData+m_multipartPoint,size);
	m_multipartPoint += size;
	return size;
}
void ClientInst::setParam(LPCSTR string)
{
	INT i;
	String header;
	String data;
	//ヘッダーの抽出
	for(i=0;string[i] && i<1024 && string[i] != ' ';i++)
		header += toupper(string[i]);
	//スペースの除去
	for(;string[i] && i<1024 && string[i] == ' ';i++);
	//データの抽出
	for(;string[i] && i<1024;i++)
		data += string[i];
	
	if(header == "GET" || header == "POST" || header == "HEAD")
	{
		m_method = header;
		setUrl(data.c_str());
	}
	else if(header == "CONTENT-LENGTH:")
	{
		m_contentLength = data.toInt();
		//printf("[%s] [%s]\n",header.c_str(),data.c_str());
	}
	else
	{
		m_params[header] = data;
		//printf("[%s] [%s]\n",header.c_str(),data.c_str());
	}
}
void ClientInst::setUrl(LPCSTR url)
{
	HttpUrl httpUrl;
	httpUrl.setUrl(url);
	m_path = httpUrl.getPath();
	m_get = httpUrl.getParam();
	printf("PATH --> %s\n",m_path.c_str());
	printf("DATA --> %s\n",httpUrl.getParam());
}
Thread* ClientInst::getThread()
{
	return &m_thread;
}
INT ClientInst::getContentLength()const
{
	return m_contentLength;
}
LPCSTR ClientInst::GET()const
{
	return m_get.c_str();
}
LPCSTR ClientInst::getHeader(LPCSTR name)
{
	return m_params[name].c_str();
}
LPCSTR ClientInst::getMultipartData()const
{
	return m_multipartData;
}
void ClientInst::setMutipartData(LPSTR data)
{
	m_multipartData = data;
}
//------------------------------------------------------------
// HttpServer
// 実験用仮想サーバ用
//------------------------------------------------------------
HttpServer::HttpServer()
{
	m_documentRoot = ".";
	m_port = 8088;
	m_serverActive = false;

}
HttpServer::~HttpServer()
{
	m_serverActive = false;
	while(m_serverThread.isActiveThread())
		Sleep(10);
}
bool HttpServer::start()
{
	if(!m_listener.create(m_port))
		return false;
	
	m_listener.listen(50);
	m_serverActive = true;
	m_serverThread.startThread(CLASSPROC(this,HttpServer,procServer),NULL);

	return true;
}
DWORD HttpServer::procServer(LPVOID pvoid)
{
	SOCK::Sock* sock;
	
	while(m_serverActive)
	{
		while(sock = m_listener.accept(1000))
		{
			ClientInst* clientInst = NEW ClientInst(sock);
			connect(clientInst);
		}

		if(m_clientInstEnd.size())
		{
			m_critical.lock();
			std::list<ClientInst*>::iterator it;
			foreach(it,m_clientInstEnd)
			{
				m_clientInst.remove(*it);
				delete *it;
			}
			m_clientInstEnd.clear();
			m_critical.unlock();
		}
	}
	return true;
}


void HttpServer::connect(ClientInst* clientInst)
{
	m_critical.lock();
	m_clientInst.push_back(clientInst);
	m_critical.unlock();
	clientInst->getThread()->startThread(
		CLASSPROC2(this,HttpServer,proc,ClientInst*),clientInst);
}
bool Stream::recvWebSocket(LPVOID destData, INT* destLength)
{
	BYTE head;
	if (!read(&head, 1))
		return false;

	BYTE len;
	if (!read(&len, 1))
		return false;
	bool flagMask = (len & 0x80) != 0;
	len &= 0x7f;
	int length;
	if (len == 126)
	{
		unsigned short len2;
		if (!read(&len2, 2))
			return false;
		length = (len2 >> 8) | ((len2 & 0xff) << 8);
	}
	else if (len == 127)
	{
		BYTE len2[8];
		if (!read(len2, 8))
			return false;
		int i;
		length = 0;
		for (i = 0; i < 8; i++)
		{
			length = (length << 8) | len2[i];
		}
	}
	else
	{
		length = len;
	}
	int mask;
	if (flagMask)
	{
		if (!read(&mask, 4))
			return false;
	}
	LPBYTE data = new BYTE[(length + 3) / 4 * 4];
	if (data == NULL)
		return false;
	if (!read(data, length))
	{
		delete[] data;
		return false;
	}
	if (flagMask)
	{
		int i;
		for (i = 0; i < (length + 3) / 4; i++)
		{
			((int*)data)[i] ^= mask;
		}

	}
	//WEBSockData wsd = { &s, data, length };

	*(LPVOID*)destData = data;
	*destLength = length;
	//delete[] data;
	return true;
}

DWORD HttpServer::proc(ClientInst* clientInst)
{
	m_critical.lock();
	Method::setClientInst(clientInst);
	m_critical.unlock();
	Method::init();

	Stream s;
	s.setSock(clientInst->getSock());


	if(strlen(clientInst->getPath()) > 1)
	{
		String fileName;
		fileName.printf("%s%s",m_documentRoot.c_str(),clientInst->getPath());
		s.setFile(fileName);
	}
	else
	{
		if (m_proc.isAddress())
		{
			m_proc.call(&s);
		}
		else
		{
			call(&s);
		}
	}
	s.outBody();
	clientInst->getSock()->close();


	m_critical.lock();
	Method::setClientInst(NULL);
	m_clientInstEnd.push_back(clientInst);
	m_critical.unlock();

	return 0;
}
void HttpServer::call(Stream* s)
{
}


}}
