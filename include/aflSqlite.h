//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLSQLITE
//-----------------------------------------------------

#include "aflStd.h"
#if defined(_WIN32) | defined(_WIN32_WCE)
	#include "../LibOther/sqlite/sqlite3.h"
#else
	#include "sqlite/sqlite3.h"
#endif
#include <vector>
#include <list>
#include <string>
#include <map>

namespace AFL{ namespace SQLITE {


#define SQLSTRING(a) ((LPCSTR)AFL::SQLITE::SQLString(a))
class SQLString
{
public:
	SQLString(LPCSTR src)
	{
		if(!src)
			return;
		//“ÁŽê•¶Žš‚Ì•ÏŠ·
		INT i;
		for(i=0;src[i];i++)
		{
			if(src[i] == '\'')
				m_string += "''";
			else
				m_string += src[i];
		}
	}
	operator LPCSTR() const {return m_string.c_str();}
protected:
	String m_string;
};


class SQLiteResult
{
	friend class SQLite;
public:
	INT getFeilds()const;
	INT getRows()const;
	INT getNowColumn()const
	{
		return m_columnNow;
	}
	LPCSTR getFeildName(INT index=0)const;
	LPCSTR getColumn(INT rowIndex,INT colIndex)const;
	LPCSTR getColumn(INT index=0);
	LPCSTR getColumnFromName(LPCSTR name);
	LPCSTR getColumnFromName(INT index,LPCSTR name);
	bool next();
	void setFirst();
	bool isError()const;
	LPCSTR getMessage()const;
protected:
	std::vector<String> m_feildName;
	std::map<String,INT> m_feildReverse;
	std::vector<std::vector<String> > m_arrayData;
	INT m_columnNow;
	std::string m_message;
	bool m_error;
};
typedef SP<SQLiteResult> SQRes;

class SQLite
{
public:
	SQLite();
	~SQLite();
	bool open(LPCSTR fileName);
	bool open(LPCWSTR fileName);
	bool openSJIS(LPCSTR fileName);
	void setSJIS(bool flag){m_sjis=flag;}
	bool close();
	SQRes exec(LPCSTR sql) const;
	bool isTable(LPCSTR name) const;
	LPCSTR getFileName()const{return m_fileName;}
	bool isDB()const{return m_sqlite!=NULL;}
	bool setDebugFile(LPCSTR fileName);
	bool dumpSQL(String* dest);
	bool dumpTable(String* dest,LPCSTR tableName);
protected:
	static INT callback(LPVOID result,INT argc,LPCSTR argv[],LPCSTR azColName[]);
	sqlite3* m_sqlite;
	std::string m_errorMessage;
	String m_fileName;
	FILE* m_debugFile;
	bool m_sjis;

};
}}

//-----------------------------------------------------
#define __INC_AFLSQLITE
#endif	// __INC_AFLSQLITE
//-----------------------------------------------------
