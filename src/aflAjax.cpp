#include "aflAjax.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

using namespace AFL;
using namespace AFL::CGI;
using namespace AFL::SQLITE;
void splitOption(std::map<String,String>& dest,LPCSTR option)
{
	LPCSTR work = option;
	INT i;
	for(i=0;work[i];i++)
	{
		String name;
		String data;
		for(;work[i] && work[i] != '=';i++)
			name += work[i];
		if(work[i] != '=')
			break;
		for(++i;work[i] && work[i] != ',';i++)
			data += work[i];
		dest[name] = data;
		if(work[i] != ',')
			break;
	}
}
void decodeArray(std::list<String>& dest,LPCSTR src)
{
	INT i;
	LPCSTR work = src;
	for(i=0;work[i];i++)
	{
		String data;
		for(;work[i] && work[i] != ',';i++)
			data += work[i];
		String d;
		Method::decodeURI(d,data);
		dest.push_back(d);
		if(!work[i])
			break;
	}
}
void decodeArrays(std::list<std::map<String,String> >& dest,LPCSTR src)
{
	INT i;
	std::map<String,String> arrays;
	for(i=0;src[i];i++)
	{
		String name;
		String data;
		for(;src[i] && src[i] != '=';i++)
			name += src[i];
		if(src[i] != '=')
			break;
		for(i++;src[i] && src[i] != '&' && src[i] != '/';i++)
			data += src[i];
		String nameD,dataD;
		Method::decodeURI(nameD,name.c_str());
		Method::decodeURI(dataD,data.c_str());
		arrays[nameD] = dataD;
		if(src[i] == '/' || !src[i])
		{
			dest.push_back(arrays);
			arrays.clear();
			if(!src[i])
				break;
		}
	}
}

void Time::getDateTime(std::string& dest)
{
	time_t timeData;
	time(&timeData);
	tm t = *localtime(&timeData);
	String s;
	s.printf("%04d-%02d-%02d %02d:%02d:%02d",
		t.tm_year+1900,t.tm_mon+1,t.tm_mday,
		t.tm_hour,t.tm_min,t.tm_sec);
	dest = s;
}
void Time::getDate(std::string& dest)
{
	time_t timeData;
	time(&timeData);
	tm t = *localtime(&timeData);
	String s;
	s.printf("%04d-%02d-%02d",
		t.tm_year+1900,t.tm_mon+1,t.tm_mday);
	dest = s;
}

AccessLog::AccessLog()
{
}

bool AccessLog::init(Stream& s,LPCSTR fileName)
{
	m_db.open(fileName);
	LPCSTR ucode = Method::COOKIE("ucode");
	if(!ucode || ucode[0] == 0)
	{
		String t;
		Time::getDateTime(t);
		MD5::String(t,t);
		s.setCookie("ucode",t);
		m_ucode = t;
	}
	else
		m_ucode = ucode;

	m_userID = -1;
	LPCSTR strID = Method::COOKIE("user_id");
	if(strID)
		m_userID = atoi(strID);

	if(!m_db.isTable("tb_access"))
	{
		LPCSTR sql = 
			"BEGIN;"
			"create table tb_code(id integer primary key,code text,atime date);"
			"create table tb_access(id integer primary key,dtime timestamp,ip1 text,ip2 text,host text,referer text,agent text,ucode int,user_id integer,command text,param text);"
			"COMMIT;";
		SQRes res = m_db.exec(sql);
		return !res->isError();
	}
	return true;
}
bool AccessLog::outLog(LPCSTR cmd,INT param)
{
	String work;
	work.printf("%d",param);
	return outLog(cmd,work);
}
bool AccessLog::outLog(LPCSTR cmd,LPCSTR param)
{

	String date;
	Time::getDateTime(date);

	String forward;
	LPCSTR FORWARDED = getenv("HTTP_FORWARDED");
	LPCSTR XFORWARDED = getenv("HTTP_X_FORWARDED_FOR");

	if(FORWARDED && FORWARDED[0])
		forward = FORWARDED;
	else if(XFORWARDED && XFORWARDED[0])
		forward = XFORWARDED;

	String agent;
	if(Method::isENV("HTTP_USER_AGENT"))
		agent = Method::ENV("HTTP_USER_AGENT");

	String referer;
	if(Method::isENV("HTTP_REFERER"))
		referer = Method::ENV("HTTP_REFERER");

	String remoteAddr;
	if(Method::isENV("REMOTE_ADDR"))
		remoteAddr = Method::ENV("REMOTE_ADDR");

	String remoteHost;
	if(Method::isENV("REMOTE_HOST"))
		remoteHost = Method::ENV("REMOTE_HOST");

	String sql;
	sql.printf(
		"BEGIN TRANSACTION;"
		"DELETE FROM tb_code WHERE atime < datetime('now', '-1 month');"
		"INSERT INTO tb_code(code) SELECT '%s' where (select count(*) from tb_code where code='%s' limit 1)=0;"
		"UPDATE tb_code SET atime=datetime('now') WHERE code='%s';"
		"SELECT id FROM tb_code WHERE code='%s';"
		"COMMIT;",
		m_ucode.c_str(),m_ucode.c_str(),
		m_ucode.c_str(),
		m_ucode.c_str());
	SQRes res = m_db.exec(sql);
	if(res->getRows()==0)
		return false;
	INT ucode = atoi(res->getColumn());

	sql.printf(
		"INSERT INTO tb_access values(NULL,'%s','%s','%s','%s',"
		"'%s','%s',%d,%d,'%s','%s');",
		date.c_str(),SQLSTRING(remoteAddr),SQLSTRING(forward),SQLSTRING(remoteHost),
		SQLSTRING(referer),SQLSTRING(agent),ucode,m_userID,
		SQLSTRING(cmd),SQLSTRING(param));
	res = m_db.exec(sql);
	return !res->isError();
}
SQRes AccessLog::getLog(INT count)
{
	String sql;
	sql.printf("select * from tb_access order by id desc limit %d;",count);
	SQRes res = m_db.exec(sql);
	return res;
}

bool SystemLink::init(SQLite* db)
{
	m_db = db;

	if(!m_db->isTable("tb_link"))
	{
		m_db->exec("CREATE TABLE tb_link(id integer primary key,message_id integer,enable integer,name,url);");
	}
	return true;
}
SQRes SystemLink::getData(INT id)
{
	String sql;
	if(id == -1)
		sql = "select * from tb_link order by id;";
	else
		sql.printf("select * from tb_link where message_id=%d order by name;",id);
	SQRes res = m_db->exec(sql);
	return res;
}
bool SystemLink::delData(INT id)
{
	String sql;
	sql.printf("DELETE FROM tb_link WHERE id=%d",id);
	SQRes res = m_db->exec(sql);
	return !res->isError();
}
bool SystemLink::delFromParent(INT id)
{
	String sql;
	sql.printf("DELETE FROM tb_link WHERE message_id=%d",id);
	SQRes res = m_db->exec(sql);
	return !res->isError();
}
bool SystemLink::setData(INT id,INT mid,bool enable,LPCSTR name,LPCSTR url)
{
	String sql;
	if(id)
	{
		sql.printf("UPDATE tb_link SET enable=%d,name='%s',url='%s' WHERE id=%d",
			enable,SQLSTRING(name),SQLSTRING(url),id);
	}
	else
	{
		sql.printf("INSERT INTO tb_link VALUES(null,%d,%d,'%s','%s');",
			mid,enable,SQLSTRING(name),SQLSTRING(url));
	}
	SQRes res = m_db->exec(sql);
	return !res->isError();

}
void SystemLink::getListID(std::map<String,INT>& dest,INT id)
{
	String sql;
	if(id == -1)
		sql = "select name,id from tb_link order by id;";
	else
		sql.printf("select name,id from tb_link where message_id=%d order by name;",id);
	SQRes res = m_db->exec(sql);
	INT i;
	INT count = res->getRows();
	for(i=0;i<count;i++)
	{
		dest[res->getColumn(i,0)] = atoi(res->getColumn(i,1));
	}
}
bool SystemLink::getURL(String& dest,INT id)
{
	String sql;
	sql.printf("select url from tb_link where id=%d",id);
	SQRes res = m_db->exec(sql);
	if(!res->getRows())
		return false;
	dest = res->getColumn();
	return true;
}

SystemFile::SystemFile()
{
	m_dataDir = "data/";
	m_dbLog = NULL;
}
bool SystemFile::init(SQLite* db,SQLite* dbLog)
{
	m_db = db;
	m_dbLog = dbLog;

	if(!m_db->isTable("tb_file2"))
	{
		m_db->exec("CREATE TABLE tb_file2(file_id integer primary key,file_pid integer,file_kind int,"
			"file_name,file_lname,file_type int,file_size  integer,file_date,file_hash,file_comment);");
		makeDir(0,"root");
	}
	return true;
}
bool SystemFile::getLocalPath(String& dest,INT id)
{
	if(id)
	{
		String sql;
		sql.printf("SELECT file_name,file_lname,file_size,file_type FROM tb_file2 WHERE file_id=%d limit 1;",id);
		SQRes res = m_db->exec(sql);
		if(res->getRows())
		{
			dest.printf("%s%s",m_dataDir.c_str(),res->getColumn(1));
			return true;
		}
	}
	return false;
}
bool SystemFile::tranceFile(Stream& s,INT id)
{
	if(id)
	{
		String sql;
		sql.printf("SELECT file_name,file_lname,file_size,file_type FROM tb_file2 WHERE file_id=%d limit 1;",id);
		SQRes res = m_db->exec(sql);
		if(res->getRows())
		{
			String saveName;
			saveName.printf("%s%s",m_dataDir.c_str(),res->getColumn(1));
			s.setFile(res->getColumn(0),saveName,atoi(res->getColumn(2)),res->getColumn(3));
			return true;
		}
	}
	return false;
}
INT SystemFile::getParentID(INT id)
{
	String sql;
	sql.printf("SELECT file_pid FROM tb_file2 WHERE file_id='%d'",id);
	SQRes res = m_db->exec(sql);
	if(res->getRows() == 0)
		return 0;
	return atoi(res->getColumn());
}
INT SystemFile::getKind(INT id)
{
	String sql;
	sql.printf("SELECT file_kind FROM tb_file2 WHERE file_id='%d'",id);
	SQRes res = m_db->exec(sql);
	if(res->getRows() == 0)
		return -1;
	return atoi(res->getColumn());
}
bool SystemFile::moveFile(INT pid,INT id)
{
	//ファイルタイプがディレクトリか確認
	if(getKind(pid) != 1)
		return false;
	//親IDが配下だったら中断
	INT wid = pid;
	while(wid = getParentID(wid))
	{
		if(wid == id)
			break;
	}
	//ファイルの移動
	if(id>0 && pid>0 && id != pid && wid != id)
	{
		String sql;
		sql.printf("UPDATE tb_file2 SET file_pid=%d WHERE file_id=%d",pid,id);
		m_db->exec(sql);
		return true;
	}
	return false;
}
bool SystemFile::saveFile(LPCSTR fileName,INT pid)
{
	String name = SQLSTRING(fileName);


	String sql;
	SQRes res;
	sql.printf("SELECT file_id,file_lname FROM tb_file2 WHERE file_pid=%d and file_name='%s';", pid, name.c_str());
	res = m_db->exec(sql);
	if (res->getRows())
	{
		LPCSTR lfile = res->getColumn(1);
		if (lfile[0] == 0)
			return false;
		removeFile(atoi(res->getColumn(0)));
	}
	
	LPCSTR data = Method::POST();
	INT length = Method::getPostLength();

	std::string hash;
	MD5::Data(hash, data,length);

	String localName;
	localName.printf("%s.dat", hash.c_str());

	String saveName;
	saveName.printf("%s%s", m_dataDir.c_str(), localName.c_str());

	FILE* file = fopen(saveName,"wb");
	if (file)
	{
		fwrite(data, length, 1, file);
		fclose(file);
	}



	String type;
	String comment;
	String date;

	LPCSTR d = strrchr(SQLSTRING(fileName), '.');
	if (d)
	{
		type = d + 1;
		type.toUpper();
	}

	Time::getDateTime(date);
	sql.printf("INSERT INTO tb_file2 values(null,%d,0,'%s','%s','%s',%d,'%s','%s','');",
		pid, name.c_str(), localName.c_str(), type.c_str(), length,
		date.c_str(), hash.c_str());
	res = m_db->exec(sql);
	return !res->isError();
}
bool SystemFile::saveFile(const MethodFile* file,INT pid)
{
	if(!file || !*file->getName())
		return false;
	String sql;
	SQRes res;
	sql.printf("SELECT file_id,file_lname FROM tb_file2 WHERE file_pid=%d and file_name='%s';",pid,file->getName());
	res = m_db->exec(sql);
	if(res->getRows())
	{
		LPCSTR lfile = res->getColumn(1);
		if(lfile[0] == 0)
			return false;
		removeFile(atoi(res->getColumn(0)));
	}

	std::string hash;
	MD5::File(hash,file->getFile());
	
	String localName;
	localName.printf("%s.dat",hash.c_str());

	String saveName;
	saveName.printf("%s%s",m_dataDir.c_str(),localName.c_str());

	if(!file->save(saveName))
		return false;


	String name;
	String type;
	String comment;
	String date;

	name = SQLSTRING(file->getName());
	LPCSTR d = strrchr(SQLSTRING(file->getName()),'.');
	if(d)
	{
		type = d + 1;
		type.toUpper();
	}
		
	Time::getDateTime(date);
	sql.printf("INSERT INTO tb_file2 values(null,%d,0,'%s','%s','%s',%d,'%s','%s','');",
		pid,name.c_str(),localName.c_str(),type.c_str(),file->getSize(),
		date.c_str(),hash.c_str());
	res = m_db->exec(sql);
	return !res->isError();
}
bool SystemFile::renameFile(INT id,LPCSTR fileName)
{
	String sql;
	SQRes res;
	sql.printf("SELECT 0 FROM tb_file2 WHERE file_name='%s' AND pid=(SELECT file_pid FROM tb_file2 WHERE file_id='%d');",
		SQLSTRING(fileName),id);
	res = m_db->exec(sql);
	if(res->getRows())
		return false;

	sql.printf("UPDATE tb_file2 SET file_name='%s' WHERE file_id=%d;",SQLSTRING(fileName),id);
	res = m_db->exec(sql);
	return !res->isError();
}
bool SystemFile::removeFile(INT id)
{
	String date;
	String sql;
	SQRes res;

	sql.printf("SELECT file_lname FROM tb_file2 WHERE file_id=%d;",id);
	res = m_db->exec(sql);
	if(!res->getRows())
		return false;
	LPCSTR lname = res->getColumn(0);
	if(lname[0])
	{
		String saveName;
		saveName.printf("%s%s",m_dataDir.c_str(),lname);
		remove(saveName);
	}
	sql.printf("SELECT file_id FROM tb_file2 WHERE file_pid=%d",id);
	INT i;
	res = m_db->exec(sql);
	INT count = res->getRows();
	for(i=0;i<count;i++)
	{
		removeFile(atoi(res->getColumn(0)));
		res->next();
	}
	Time::getDateTime(date);
	sql.printf("DELETE FROM tb_file2 WHERE file_id=%d;",id);
	res = m_db->exec(sql);
	return !res->isError();
}

bool SystemFile::makeDir(INT pid,LPCSTR name)
{
	String date;
	String sql;
	SQRes res;

	if(pid)
	{
		sql.printf("SELECT 0 FROM tb_file2 WHERE file_pid=%d and file_name='%s';",pid,name);
		res = m_db->exec(sql);
		if(res->getRows())
			return false;
	}
	Time::getDateTime(date);
	sql.printf("INSERT INTO tb_file2 values(null,%d,1,'%s','','',0,'%s','','');",pid,name,date.c_str());
	res = m_db->exec(sql);
	return !res->isError();
}
bool SystemFile::getFilePath(String& dest,INT id)
{
	String sql;
	sql.printf("SELECT file_pid,file_name FROM tb_file2 WHERE file_id=%d;",id);
	SQRes res = m_db->exec(sql);
	if(res->isError() || res->getRows() == 0)
		return false;

	INT pid = atoi(res->getColumn(0));
	String path = res->getColumn(1);

	while(pid)
	{
		sql.printf("SELECT file_pid,file_name FROM tb_file2 WHERE file_id=%d;",pid);
		SQRes res = m_db->exec(sql);
		if(res->isError() || res->getRows() == 0)
			return false;
		path.printf("%s/%s",res->getColumn(1),path.c_str());
		pid = atoi(res->getColumn(0));
	}
	path.printf("/%s",path.c_str());
	dest = path;
	return true;
}


void SystemFile::outFileList(Stream* s)
{
	INT pid = 0;
	LPCSTR methodPID = Method::GET("pid");
	if(methodPID)
		pid = atoi(methodPID);

	String sql;
	sql.printf("SELECT file_id,file_size,file_date,file_name,file_kind,file_type FROM tb_file2 WHERE file_pid=%d ORDER BY file_kind DESC,upper(file_name);", pid);
	SQRes res = m_db->exec(sql);
	s->setHeader("Content-type", "text/plain; charset=UTF-8");

	JsonHash hash;
	JsonArray* array = new JsonArray();

	while (res->next())
	{
		INT id = atoi(res->getColumn(0));

		JsonHash* file = new JsonHash();
		file->add("id",createJson(id));
		file->add("size", createJson(atoi(res->getColumn(1))));
		file->add("date", createJson(res->getColumn(2)));
		file->add("name", createJson(res->getColumn(3)));
		file->add("kind", createJson(atoi(res->getColumn(4))));
		file->add("type", createJson(res->getColumn(5)));
		array->add(file);
	}
	hash.add("files", array);
	
	String work;
	hash.getString(work);
	s->setHeader("Content-type", "text/plain; charset=UTF-8");
	s->out(work);
	/*
	String sql;
	sql.printf("SELECT file_id,file_size,file_date,file_name,file_kind,file_type FROM tb_file2 WHERE file_pid=%d ORDER BY file_kind DESC,upper(file_name);",pid);
	SQRes res = m_db->exec(sql);
	s->setHeader("Content-type","text/plain; charset=UTF-8");
	s->out("file_id,file_size,file_date,file_name,file_kind,count,file_type\n");


	bool flag = false;
	String ids;
	while(res->next())
	{
		if(flag)
			ids.appendf(",'%s'",res->getColumn());
		else
		{
			ids.appendf("'%s'",res->getColumn());
			flag = true;
		}
	}
	std::map<INT,INT> downloadCount;
	if(m_dbLog)
	{
		String sql;
		sql.printf("SELECT param,count(*) FROM tb_access WHERE command='DOWNLOAD' AND param IN (%s) GROUP BY param;",ids.c_str());
		SQRes res = m_dbLog->exec(sql);
		while(res->next())
		{
			downloadCount[atoi(res->getColumn(0))] = atoi(res->getColumn(1));
		}
	}
	res->setFirst();
	while(res->next())
	{
		INT id = atoi(res->getColumn(0));
		s->printf("%d,%d,%s,%s,%d,%d,%s\n",
			id,
			atoi(res->getColumn(1)),
			SQLSTRING(res->getColumn(2)),
			SQLSTRING(res->getColumn(3)),
			atoi(res->getColumn(4)),
			downloadCount[id],
			SQLSTRING(res->getColumn(5)));
	}*/
}
JsonArray* SystemFile::getDirList(int pid,int level)
{
	String sql;
	sql.printf("SELECT file_id,file_name FROM tb_file2 WHERE file_lname='' AND file_pid=%d ORDER BY file_name;", pid);
	SQRes res = m_db->exec(sql);

	JsonArray* array = new JsonArray();
	while (res->next())
	{
		JsonHash* dir = new JsonHash();
		int id = atoi(res->getColumn(0));
		dir->add("id", createJson(id));
		dir->add("name", createJson(res->getColumn(1)));
		if (level)
		{
			JsonArray* child = getDirList(id, level - 1);
			dir->add("child", child);
		}
		array->add(dir);
	}
	return array;
}

void SystemFile::outDirList(Stream* s)
{
	JsonHash hash;
	String sql;
	sql.printf("SELECT file_id,file_pid,file_name FROM tb_file2 WHERE file_pid=0;");

	int pid = 1;

	JsonArray* array = getDirList(1,-1);
	hash.add("dir", array);

	String work;
	hash.getString(work);
	s->setHeader("Content-type", "text/plain; charset=UTF-8");
	s->out(work);

	/*
	String sql;
	sql.printf("SELECT file_id,file_pid,file_name FROM tb_file2 WHERE file_lname='' ORDER BY file_pid,file_name;");
	SQRes res = m_db->exec(sql);
	s->setHeader("Content-type","text/plain; charset=UTF-8");
	s->out("file_id,file_pid,file_name\n");
	while(res->next())
	{
		s->printf("%d,%d,%s\n",
			atoi(res->getColumn(0)),
			atoi(res->getColumn(1)),
			SQLSTRING(res->getColumn(2)));
	}
	*/
}

SystemUser::SystemUser()
{
	m_db = NULL;
}
void SystemUser::init(SQLite* db)
{
	m_db = db;
	if(!m_db->isTable("tb_user"))
	{
		LPCSTR sql = 
			"CREATE TABLE tb_user(user_id INTEGER PRIMARY KEY,user_enable,user_name,user_password,user_nick);";
		m_db->exec(sql);
	}
}
bool SystemUser::isAdmin()
{
	LPCSTR sql = "SELECT 0 FROM tb_user WHERE user_enable=1;";
	SQRes res = m_db->exec(sql);
	return res->getRows() != 0;
}
INT SystemUser::getUserID(LPCSTR userName)
{
	String sql;
	sql.printf("SELECT user_id FROM tb_user WHERE user_name='%s';",userName);
	SQRes res = m_db->exec(sql);
	if(res->getRows() == 0)
		return 0;
	return atoi(res->getColumn(0));
}
INT SystemUser::getUserID(LPCSTR userName,LPCSTR userPass)
{
	String md5Pass;
	MD5::String(md5Pass,userPass);

	String sql;
	sql.printf("SELECT user_id FROM tb_user WHERE user_name='%s' AND user_password='%s';",userName,md5Pass.c_str());
	SQRes res = m_db->exec(sql);
	if(res->getRows() == 0)
		return 0;
	return atoi(res->getColumn(0));
}
bool SystemUser::getUserNick(String& dest,INT id)
{
	String sql;
	sql.printf("SELECT user_nick FROM tb_user WHERE user_id='%d';",id);
	SQRes res = m_db->exec(sql);
	if(res->getRows() == 0)
		return false;
	dest = res->getColumn(0);
	return true;
}
bool SystemUser::delUser(INT id)
{
	String sql;
	sql.printf("DELETE FROM tb_user where user_id=%d",id);
	SQRes res = m_db->exec(sql);
	return !res->isError();
}
bool SystemUser::setUser(INT id,LPCSTR name,LPCSTR password,LPCSTR nickname)
{
	if(!*name)
		return false;

	String sql;
	String md5Pass;
	MD5::String(md5Pass,password);
	SQRes res;
	if(id == 0)
	{
		if(getUserID(name) != 0)
			return false;
		sql.printf("INSERT INTO tb_user values(null,1,'%s','%s','%s');",
			name,md5Pass.c_str(),nickname);
		res = m_db->exec(sql);
	}
	else
	{
		INT id2 = getUserID(name);
		if(id2 != 0 && id2 != id)
			return false;
		if(password[0])
			sql.printf("UPDATE tb_user set user_name='%s',user_password='%s',user_nick='%s' where user_id=%d;",
			name,md5Pass.c_str(),nickname,id);
		else
			sql.printf("UPDATE tb_user set user_name='%s',user_nick='%s' where user_id=%d;",
				name,nickname,id);
		res = m_db->exec(sql);
	}
	return !res->isError();
}

