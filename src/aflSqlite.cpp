#if defined(_WIN32) | defined(_WIN32_WCE)
	#include <windows.h>
#endif
#include <stdio.h>
#include "aflSqlite.h"

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
		#define CHECK_MEMORY_LEAK
		#define NEW new
	#endif //_DEBUG
#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
#endif 


namespace AFL{ namespace SQLITE {


INT SQLiteResult::getFeilds() const
{
	return (INT)m_feildName.size();
}
INT SQLiteResult::getRows() const
{
	return (INT)m_arrayData.size();
}
LPCSTR SQLiteResult::getFeildName(INT index) const
{
	return m_feildName[index].c_str();
}
LPCSTR SQLiteResult::getColumn(INT rowIndex,INT colIndex) const
{
	return m_arrayData[rowIndex][colIndex];
}
LPCSTR SQLiteResult::getColumn(INT index)
{
	if(m_columnNow < 0)
		m_columnNow = 0;
	if((INT)m_arrayData.size() <= m_columnNow)
		return NULL;
	return m_arrayData[m_columnNow][index].c_str();
}
LPCSTR SQLiteResult::getColumnFromName(LPCSTR name)
{
	if(m_columnNow < 0)
		m_columnNow = 0;
	return m_arrayData[m_columnNow][m_feildReverse[name]].c_str();
}
LPCSTR SQLiteResult::getColumnFromName(INT index,LPCSTR name)
{
	return m_arrayData[index][m_feildReverse[name]].c_str();
}
void SQLiteResult::setFirst()
{
	m_columnNow = -1;
}
bool SQLiteResult::next()
{
	++m_columnNow;
	return m_columnNow < getRows();
}
bool SQLiteResult::isError()const
{
	return m_error;
}
LPCSTR SQLiteResult::getMessage()const
{
	return m_message.c_str();
}



SQLite::SQLite()
{
	m_sqlite = NULL;
	m_debugFile = NULL;
	m_sjis = false;
}
SQLite::~SQLite()
{
	close();
	if(m_debugFile)
		fclose(m_debugFile);
}
bool SQLite::setDebugFile(LPCSTR fileName)
{
	//デバッグ用ファイルの作成
	if(m_debugFile)
	{
		fclose(m_debugFile);
		m_debugFile = NULL;
	}
	FILE* file = fopen(fileName,"at");
	if(!file)
		return false;
	m_debugFile = file;
	return true;
}
bool SQLite::openSJIS(LPCSTR fileName)
{
	WString s;
	s.printf(L"%hs",fileName);
	String utfFile;
	UCS2toUTF8(utfFile,s);

	//すでに開いているデータベースを閉じる
	close();
	//データベースの開始処理
	sqlite3* db;
	INT rc = sqlite3_open(utfFile, &db);
	if(rc)
	{
		m_errorMessage = sqlite3_errmsg(db);
		sqlite3_close(db);
		return false;
	}
	m_sqlite = db;
	m_fileName = fileName;
	return true;
}
#ifdef _WIN32
bool SQLite::open(LPCWSTR fileName)
{
	String utfFile;
	UCS2toUTF8(utfFile,fileName);

	//すでに開いているデータベースを閉じる
	close();
	//データベースの開始処理
	sqlite3* db;
	INT rc = sqlite3_open(utfFile, &db);
	if(rc)
	{
		m_errorMessage = sqlite3_errmsg(db);
		sqlite3_close(db);
		return false;
	}
	m_sqlite = db;
	m_fileName = fileName;
	return true;
}
#endif
bool SQLite::open(LPCSTR fileName)
{
	//すでに開いているデータベースを閉じる
	close();
	//データベースの開始処理
	sqlite3* db;
	INT rc = sqlite3_open(fileName, &db);
	if(rc)
	{
		m_errorMessage = sqlite3_errmsg(db);
		sqlite3_close(db);
		if(m_debugFile)
			fprintf(m_debugFile,"ERROR [%s]\n",m_errorMessage.c_str());
		return false;
	}
	if(m_debugFile)
		fprintf(m_debugFile,"OPEN [%s]\n",fileName);
	sqlite3_busy_timeout(db,3000);
	m_sqlite = db;
	m_fileName = fileName;
	return true;
}
bool SQLite::dumpSQL(String* dest)
{
	if(!m_sqlite)
		return false;
	String sql;
	sql.printf("select name,sql from sqlite_master;");
	SQRes res = exec(sql);
	*dest = "begin;";
	while(res->next())
	{
		dest->appendf("%s;\n",res->getColumn(1));
		sql.printf("select * from %s;",SQLSTRING(res->getColumn(0)));
		SQRes data = exec(sql);

		String s;
		INT c,r;
		INT cols = data->getFeilds();
		INT rows = data->getRows();
	
		for(r=0;r<rows;r++)
		{
			String work;
			for(c=0;c<cols;c++)
			{
				if(c)
					work += ',';
				work.appendf("'%s'",SQLSTRING(data->getColumn(r,c)));
			}
			s.appendf("INSERT INTO %s values(%s);\n",SQLSTRING(res->getColumn(0)),work.c_str());
		}
		*dest += s;
	}		
	*dest += "commit;";
	return true;
}

bool SQLite::dumpTable(String* dest,LPCSTR tableName)
{
	if(!m_sqlite || !isTable(tableName))
		return false;

	String s;
	String sql;

	sql.printf("select sql from sqlite_master where name=\"%s\";",SQLSTRING(tableName));
	SQRes res = exec(sql);
	dest->appendf("%s;\n",res->getColumn(0));


	sql.printf("select * from %s;",SQLSTRING(tableName));
	SQRes data = exec(sql);


	INT c,r;
	INT cols = data->getFeilds();
	INT rows = data->getRows();
	
	for(r=0;r<rows;r++)
	{
		String work;
		for(c=0;c<cols;c++)
		{
			if(c)
				work += ',';
			work.appendf("'%s'",SQLSTRING(data->getColumn(r,c)));
		}
		s.appendf("INSERT INTO %s values(%s);\n",SQLSTRING(tableName),work.c_str());
	}
	*dest += s;

	return true;
}

bool SQLite::close()
{
	if(!m_sqlite)
		return false;

	sqlite3_close(m_sqlite);
	m_sqlite = NULL;
	m_fileName = "";
	return false;
}
SQRes SQLite::exec(LPCSTR sql) const
{
	SQRes sqres;
	SQLiteResult* result = NEW SQLiteResult();
	sqres.set(result);
	result->m_columnNow = -1;

	result->m_error = false;
	if(!m_sqlite)
	{
		result->m_error = true;
		result->m_message = "Not DB.";
		return sqres;
	}
	//SQL文が入っているかチェック
	if(!sql)
		return sqres;

	String work;
	if(m_sjis)
	{
#ifdef _WIN32
		SJIStoUTF8(work,sql);
#endif
		sql = work;
	}

	sqlite3_stmt *stmt = NULL;
	INT rc = -1;
	while(sql[0])
	{
		if(stmt)
		{
			while(sqlite3_step(stmt) == SQLITE_ROW);
			sqlite3_finalize(stmt);
		}
		rc = sqlite3_prepare(m_sqlite, sql, -1, &stmt, &sql);
		if(rc != SQLITE_OK)
		{
			result->m_error = true;
			result->m_message = sqlite3_errmsg(m_sqlite);
			break;
		}
		INT cols = sqlite3_column_count(stmt);
		if(cols)
		{
			INT i;
			bool first = true;
			result->m_error = false;
			std::list<std::vector<String> > columnData;
			while(1)
			{
				int w;
				for(w=0;w<100;w++)
				{
					rc = sqlite3_step(stmt);
					if(rc != SQLITE_BUSY && rc != SQLITE_LOCKED)
						break;
					Sleep(100);
				}
				if(w == 100)
				{
					result->m_error = true;
					result->m_message = sqlite3_errmsg(m_sqlite);
					if (m_debugFile)
					{
						fprintf(m_debugFile, "QUARY [%s]\n%s\n", sql, result->m_message.c_str());
					}
					break;
				}
				if(first)
				{
					first = false;
					result->m_feildName.resize(cols);
					for(i=0;i<cols;i++)
					{
						if(m_sjis)
							UTF8toSJIS(result->m_feildName[i],sqlite3_column_name(stmt, i));
						else
							result->m_feildName[i] = sqlite3_column_name(stmt, i);

						result->m_feildReverse[result->m_feildName[i]] = i;
					}
				}
				if(rc != SQLITE_ROW)
					break;
				std::vector<String> data(cols);
				for(i=0;i<cols;i++)
				{
					LPCSTR value = (LPCSTR)sqlite3_column_text(stmt,i);
					if(value)
					{
						if(m_sjis)
							UTF8toSJIS(data[i],value);
						else
							data[i] = value;
					}
				}
				columnData.push_back(data);
			}
			result->m_arrayData.reserve(columnData.size());
			std::list<std::vector<String> >::iterator it;
			foreach(it,columnData)
			{
				result->m_arrayData.push_back(*it);
			}
		}
	}
	//領域の開放
	if(stmt)
	{
		while(sqlite3_step(stmt) == SQLITE_ROW);
		sqlite3_finalize(stmt);
	}
	//デバッグ情報の出力
	if(m_debugFile)
	{
		if(sqres->isError())
			fprintf(m_debugFile,"ERROR [%s]\n",sqres->getMessage());
	}
	return sqres;		

}

bool SQLite::isTable(LPCSTR name) const
{
	if(!m_sqlite)
		return false;
	if(name)
	{
		String sql;
		sql.printf("select 0 from sqlite_master where name='%s';",name);
		SQRes res = exec(sql);
		if(res->getRows())
			return true;
	}
	return false;
}


}}
