#include <stdlib.h>

#include "aflWibs.h"
#include "aflAjax.h"

WibsBase::WibsBase()
{
	m_dataDir = "wibs_data/";
}
bool WibsBase::init()
{
	if(!m_sqlite.isTable("tb_link"))
		m_sqlite.exec("CREATE TABLE tb_link(id integer primary key,message_id int,enable int,name,url);");
	if(!m_sqlite.isTable("tb_mfile"))
		m_sqlite.exec("CREATE TABLE tb_mfile(id integer primary key,enable integer,parent_id integer,name,data,option1,option2);");

	return true;
}
bool WibsBase::outRes(LPCSTR sql)
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	SQRes res = m_sqlite.exec(sql);
	if(res->isError())
		return false;
	outResHeader(res);
	outResData(res);
	return true;
}
void WibsBase::outRes(SQRes& res)
{
	outResHeader(res);
	outResData(res);
}
void WibsBase::outResHeader(SQRes& res)
{
	String s;
	INT c;
	INT cols = res->getFeilds();
	String header;
	for(c=0;c<cols;c++)
	{
		if(c)
			s += ',';
		s += TEXTSTRING(res->getFeildName(c));
	}
	s += '\n';
	m_stream->out(s);
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
}
void WibsBase::outResData(SQRes& res)
{
	String s;
	INT c,r;
	INT cols = res->getFeilds();
	INT rows = res->getRows();

	
	for(r=0;r<rows;r++)
	{
		String work;
		for(c=0;c<cols;c++)
		{
			if(c)
				work += ',';
			work += TEXTSTRING(res->getColumn(r,c));
		}
		work += '\n';
		s += work;
	}
	m_stream->out(s);
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
}

bool WibsBase::command()
{
	String cmd;
	if(Method::isGET("cmd"))
		cmd = Method::GET("cmd");

	if(cmd == "log_list")
		outLogList();
	else if(cmd == "user_login")
		outLogin();
	else if(cmd == "user_list")
		outUserList();
	else if(cmd == "user_write")
		writeUser();
	else if(cmd == "user_delete")
		deleteUser();
	else if(cmd == "file_link_delete")
		deleteFileLink();
	else if(cmd == "file_rename")
		renameFile();
	else if(cmd == "file_link_write")
		writeFileLink();
	else if(cmd == "file_link")
		outFileLink();
	else if(cmd == "file_list")
	{
		SystemFile file;
		file.init(&m_sqlite,m_accessLog.getDB());
		file.outFileList(m_stream);
	}
	else if(cmd == "file_dir_list")
	{
		SystemFile file;
		file.init(&m_sqlite);
		file.outDirList(m_stream);
	}
	else if(cmd == "file_make_dir")
		makeDir();
	else if(cmd == "file_remove")
		removeFile();
	else if(cmd == "file_move")
		moveFile();
	else if(cmd == "file_download")
		outFile();
	else if(cmd == "file_upload")
		outUpload();
	else if(cmd == "message_link")
		outMessageLink();
	else if(cmd == "message_link_delete")
		deleteMessageLink();
	else if(cmd == "message_link_write")
		writeMessageLink();
	else if(cmd == "jump")
		jump();
	else return false;
	return true;
}
void WibsBase::outLogin()
{
	SystemUser user;
	user.init(&m_sqlite);
	LPCSTR methodID = Method::GET("id");
	LPCSTR methodPass = Method::GET("pass");
	INT id = 0;
	if(methodID && methodPass)
	{
		if(*methodID && strcmp(methodID,"guest") != 0)
		{
			id = user.getUserID(methodID,methodPass);
			if(id)
				m_accessLog.outLog("LOGIN",methodID);
			else
				m_accessLog.outLog("LOGIN ERROR",methodID);
		}
	}
	
	
	LPCSTR url = Method::URL();
	String cookieUser,cookiePass;
	cookieUser.printf("%suser_name",url);
	cookiePass.printf("%suser_pass",url);

	m_stream->out("id,name,nick\n");
	if(id)
	{
		String nick;
		user.getUserNick(nick,id);
		m_stream->printf("%d,%s,%s\n",id,TEXTSTRING(methodID),TEXTSTRING(nick));
		m_stream->setCookie(cookieUser,methodID);
		m_stream->setCookie(cookiePass,methodPass);
	}
	else
	{
		SystemUser user;
		user.init(&m_sqlite);
		if(!user.isAdmin())
		{
			m_stream->printf("0,Admin,Admin\n");
			m_stream->setCookie(cookieUser,"Admin");
			m_stream->setCookie(cookiePass,"");
		}
		else
		{
			m_stream->out("0,guest,GUEST\n");
			m_stream->setCookie(cookieUser,"Guest");
			m_stream->setCookie(cookiePass,"");
		}
	}
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
}
bool WibsBase::isEdit()
{
	m_edit = -1;
	if(m_edit == -1)
	{
		LPCSTR methodMode = "EDIT";//Method::COOKIE("mode");
		if(!methodMode || !isUser())
			m_edit = 0;
		else if(strcmp(methodMode,"EDIT") == 0)
			m_edit = 1;
		else
			m_edit = 0;
	}
	return m_edit == 1;
	
}
INT WibsBase::getUserID()
{
	SystemUser user;
	user.init(&m_sqlite);
	LPCSTR url = Method::URL();
	String work;
	work.printf("%suser_name",url);
	LPCSTR methodID = Method::COOKIE(work);
	work.printf("%suser_pass",url);
	LPCSTR methodPass = Method::COOKIE(work);
	INT id = 0;
	if(methodID && methodPass)
		return user.getUserID(methodID,methodPass);
	return 0;
}
bool WibsBase::isUser()
{
	SystemUser user;
	user.init(&m_sqlite);
	if(!user.isAdmin())
		return true;
	return getUserID()>0;
}
void WibsBase::outLogList()
{
	if(!isUser())
		return;
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	SQRes res = m_accessLog.getLog(100);
	outRes(res);
		
}
void WibsBase::deleteUser()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isUser())
		return;
	LPCSTR methodID = Method::GET("id");
	if(!methodID)
	{
		m_stream->out("0");
		return;
	}
	INT id = atoi(methodID);
	SystemUser user;
	user.init(&m_sqlite);
	user.delUser(id);
	m_stream->out("1");
}
void WibsBase::writeUser()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isUser())
		return;

	LPCSTR data = Method::POST("data");
	if(!data)
		return;
	
	SystemUser user;
	user.init(&m_sqlite);

	std::list<std::map<String,String> > datas;
	decodeArrays(datas,data);
	std::list<std::map<String,String> >::iterator itList;
	foreach(itList,datas)
	{
		INT id = atoi((*itList)["user_id"]);
		bool enable = atoi((*itList)["user_enable"])==1;
		LPCSTR name = (*itList)["user_name"];
		LPCSTR user_pass = (*itList)["user_pass"];
		LPCSTR user_nick = (*itList)["user_nick"];

		if(strcmp(user_pass,"--------") == 0)
			user_pass = "";
		user.setUser(id,name,user_pass,user_nick);
	}
}
void WibsBase::outUserList()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isUser())
		return;
	LPCSTR sql = "SELECT user_id,user_enable,user_name,user_nick FROM tb_user";
	SQRes res = m_sqlite.exec(sql);
	outRes(res);
}


void WibsBase::outFile()
{
	LPCSTR methodID = Method::GET("id");
	INT id = 0;
	if(methodID)
		id = atoi(methodID);
	if(id)
	{
		SystemFile file;
		file.setDataDir(m_dataDir);
		file.init(&m_sqlite);
		if(file.tranceFile(*m_stream,id))
		{
			m_accessLog.outLog("DOWNLOAD",id);
		}
		else
		{
			m_accessLog.outLog("DOWNLOAD ERROR",id);
		}

	}
}
void WibsBase::deleteFileLink()
{
	if(!isEdit())
		return;
	LPCSTR methodID = Method::GET("id");
	if(!methodID)
	{
		m_stream->out("0");
		return;
	}
	INT id = atoi(methodID);
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	String sql;
	sql.printf("DELETE FROM tb_mfile WHERE id=%d;",id);
	m_sqlite.exec(sql);
	m_stream->printf("%d",1);

}
void WibsBase::outFileLink()
{
	if(!isEdit())
		return;
	INT id = 0;
	LPCSTR methodID = Method::GET("pid");
	if(methodID)
		id = atoi(methodID);

	String sql;
	if(id == 0)
		sql = "SELECT tb_mfile.*,tb_file2.file_name FROM tb_mfile JOIN tb_file2 on cast(data as int)=tb_file2.file_id;";
	else
		sql.printf("SELECT tb_mfile.*,tb_file2.file_name FROM tb_mfile JOIN tb_file2 on tb_mfile.parent_id=%d and cast(data as int)=tb_file2.file_id;",
			id);
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	SQRes res = m_sqlite.exec(sql);
	outRes(res);
}
void WibsBase::writeFileLink()
{
	if(!isEdit())
		return;
	LPCSTR data = Method::POST("data");
	if(!data)
		return;
	std::list<std::map<String,String> > datas;
	decodeArrays(datas,data);
	std::list<std::map<String,String> >::iterator itList;
	foreach(itList,datas)
	{
		INT id = atoi((*itList)["id"]);
		INT mid = atoi((*itList)["parent_id"]);
		bool enable = atoi((*itList)["enable"])==1;
		LPCSTR name = (*itList)["name"];
		INT data = atoi((*itList)["data"]);
		LPCSTR option1 = (*itList)["option1"];
		LPCSTR option2 = (*itList)["option2"];

		String sql;
		if(id == 0)
		{
			sql.printf("INSERT INTO tb_mfile VALUES(NULL,%d,%d,'%s',%d,'%s','%s');)",
				enable,mid,SQLSTRING(name),data,SQLSTRING(option1),SQLSTRING(option2));
		}
		else
		{
			sql.printf("UPDATE tb_mfile SET enable=%d,name='%s',data=%d,option1='%s',option2='%s' WHERE id=%d",
				enable,SQLSTRING(name),data,SQLSTRING(option1),SQLSTRING(option2),id);
		}
		m_sqlite.exec(sql);
	}
}

void WibsBase::deleteMessageLink()
{
	if(!isUser())
		return;
	LPCSTR methodID = Method::GET("id");
	if(!methodID)
	{
		m_stream->out("0");
		return;
	}
	INT id = atoi(methodID);
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	SystemLink link;
	link.init(&m_sqlite);
	m_stream->printf("%d",link.delData(id));

}
void WibsBase::writeMessageLink()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(isEdit())
	{
		LPCSTR methodMID = Method::GET("mid");
		LPCSTR data = Method::POST("data");
		if(methodMID && data)
		{
			INT mid = atoi(methodMID);
			SystemLink link;
			link.init(&m_sqlite);

			std::list<std::map<String,String> > datas;
			decodeArrays(datas,data);
			std::list<std::map<String,String> >::iterator itList;
			foreach(itList,datas)
			{
				INT id = atoi((*itList)["id"]);
				LPCSTR url = (*itList)["url"];
				LPCSTR name = (*itList)["name"];
				bool enable = atoi((*itList)["enable"])==1;
				link.setData(id,mid,enable,name,url);
			}
			m_stream->out("1");
			return;
		}
	}
	m_stream->out("0");
}
void WibsBase::outMessageLink()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isEdit())
		return;
	LPCSTR methodMID = Method::GET("mid");
	if(!methodMID)
	{
		return;
	}
	INT mid = atoi(methodMID);
	SystemLink link;
	link.init(&m_sqlite);
	SQRes res = link.getData(mid);
	
	outRes(res);
}
void WibsBase::outUpload()
{
	//m_stream->setHeader("Content-type","text/html; charset=UTF-8");
	if(!isUser())
		return;
	LPCSTR methodPID = Method::GET("pid");
	if(!methodPID)
	{
		m_stream->out("0");
		return;
	}
	INT pid = atoi(methodPID);
	if(pid == 0)
	{
		m_stream->out("0");
		return;
	}

	SystemFile file;
	file.setDataDir(m_dataDir);
	file.init(&m_sqlite);

	LPCSTR fileName = Method::GET("filename");
	if(file.saveFile(fileName, pid))
		m_stream->out("1");
	else
		m_stream->out("0");
}
void WibsBase::renameFile()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isUser())
		return;
	LPCSTR methodID = Method::GET("id");
	LPCSTR methodName = Method::GET("file_name");
	if(!methodID && methodName)
	{
		m_stream->out("0");
		return;
	}
	INT id = abs(atoi(methodID));
	SystemFile file;
	file.init(&m_sqlite);
	m_stream->printf("1",file.renameFile(id,methodName));		
}
void WibsBase::removeFile()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	if(!isUser())
		return;
	LPCSTR methodID = Method::GET("id");
	if(!methodID)
	{
		m_stream->out("0");
		return;
	}
	INT id = atoi(methodID);
	SystemFile file;
	file.init(&m_sqlite);
	m_stream->printf("1",file.removeFile(id));
}
void WibsBase::makeDir()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");

	LPCSTR methodPID = Method::GET("pid");
	LPCSTR methodName = Method::GET("file_name");
	if(!methodPID || !methodName)
	{
		m_stream->out("0");
		return;
	}
	INT pid = atoi(methodPID);
	if(pid == 0)
	{
		m_stream->out("0");
		return;
	}
	SystemFile file;
	file.init(&m_sqlite);
	m_stream->printf("%d",file.makeDir(pid,methodName));

}
void WibsBase::moveFile()
{
	m_stream->setHeader("Content-type","text/plain; charset=UTF-8");
	INT ret = 0;
	if(isUser())
	{
		LPCSTR methodID = Method::GET("id");
		LPCSTR methodPID = Method::GET("pid");
		if(methodID && methodPID)
		{
			INT id = atoi(methodID);
			INT pid = atoi(methodPID);

			SystemFile file;
			file.init(&m_sqlite);
			ret = file.moveFile(pid,id);
		}
	}
	m_stream->printf("%d",ret);
}
bool WibsBase::jump()
{
	LPCSTR idString;
	idString = Method::GET("id");
	INT id = 0;
	if(idString)
		id = atoi(idString);

	SystemLink link;
	
	String url;
	link.init(&m_sqlite);
	link.getURL(url,id);

	m_stream->setHeader("Location",url);
	return true;
}

