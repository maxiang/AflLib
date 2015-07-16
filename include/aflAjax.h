//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLAJAX
//-----------------------------------------------------

#include "aflCgi.h"
#include "aflSqlite.h"
#include "aflMd5.h"

void decodeArrays(std::list<std::map<AFL::String,AFL::String> >& dest,LPCSTR src);
void decodeArray(std::list<AFL::String>& dest,LPCSTR src);
void splitOption(std::map<AFL::String,AFL::String>& dest,LPCSTR option);
class Time
{
public:
	static void getDateTime(std::string& dest);
	static void getDate(std::string& dest);
};

class AccessLog
{
public:
	AccessLog();
	bool init(AFL::CGI::Stream& s,LPCSTR fileName);
	bool outLog(LPCSTR cmd,INT param);
	bool outLog(LPCSTR cmd,LPCSTR param);
	AFL::SQLITE::SQRes getLog(INT count);
	AFL::SQLITE::SQLite* getDB(){return &m_db;}
protected:
	AFL::SQLITE::SQLite m_db;
	AFL::String m_ucode;
	INT m_userID;
};


class SystemLink
{
public:
	bool init(AFL::SQLITE::SQLite* db);
	AFL::SQLITE::SQRes getData(INT id=-1);
	bool delData(INT id);
	bool delFromParent(INT id);
	bool setData(INT id,INT mid,bool enable,LPCSTR name,LPCSTR url);
	void getListID(std::map<AFL::String,INT>& dest,INT id=-1);
	bool getURL(AFL::String& dest,INT id);

protected:
	AFL::SQLITE::SQLite* m_db;
};
class SystemFile
{
public:
	SystemFile();
	void setDataDir(LPCSTR value){m_dataDir=value;}
	bool init(AFL::SQLITE::SQLite* db,AFL::SQLITE::SQLite* dbLog=NULL);
	bool tranceFile(AFL::CGI::Stream& s,INT id);
	bool getLocalPath(AFL::String& dest,INT id);
	bool saveFile(const AFL::CGI::MethodFile* file,INT pid);
	bool saveFile(LPCSTR fileName,INT pid);
	bool removeFile(INT id);
	bool renameFile(INT id,LPCSTR fileName);
	bool makeDir(INT pid,LPCSTR name);
	bool getFilePath(AFL::String& dest,INT id);
	void outFileList(AFL::CGI::Stream* s);
	void outDirList(AFL::CGI::Stream* s);

	INT getParentID(INT id);
	INT getKind(INT id);
	bool moveFile(INT pid,INT id);

	AFL::JsonArray* getDirList(int pid,int level=0);


protected:
	AFL::SQLITE::SQLite* m_db;
	AFL::SQLITE::SQLite* m_dbLog;
	AFL::String m_dataDir;

};

class SystemUser
{
public:
	SystemUser();
	void init(AFL::SQLITE::SQLite* db);
	bool isAdmin();
	INT getUserID(LPCSTR userName);
	INT getUserID(LPCSTR userName,LPCSTR userPass);
	bool getUserNick(AFL::String& dest,INT id);
	bool delUser(INT id);
	bool setUser(INT id,LPCSTR name,LPCSTR password,LPCSTR nickname);
protected:
	AFL::SQLITE::SQLite* m_db;
};
//-----------------------------------------------------
#define __INC_AFLAJAX
#endif	// __INC_AFLAJAX
//-----------------------------------------------------
