#ifdef _WIN32
	#include <windows.h>
#endif

#include <time.h>
#include "aflZip.h"

#include "aflXml.h"
#include "aflSock.h"
#include "aflPeercast.h"

using namespace AFL::XML;
using namespace AFL::SOCK;
using namespace AFL::SQLITE;

//----------------------------------------------------
//メモリリークテスト用
#if _MSC_VER && !defined(_WIN32_WCE)
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
		#define CHECK_MEMORY_LEAK
#endif
//----------------------------------------------------
namespace AFL{namespace PEERCAST{

class Time
{
public:
	static void getDateTime(String* dest)
	{
		time_t timeData;
		time(&timeData);
		tm t = *localtime(&timeData);
		String s;
		s.printf("%04d-%02d-%02d %02d:%02d:%02d",
			t.tm_year+1900,t.tm_mon+1,t.tm_mday,
			t.tm_hour,t.tm_min,t.tm_sec);
		*dest = s;
	}
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// STRINGText
// テキスト変換
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

STRINGText::STRINGText(LPCSTR src)
{
	//特殊文字の変換
	INT i;
	for(i=0;src[i];i++)
	{
		if(src[i] == '%')
		{
			i++;
			INT data;
			sscanf(src+i,"%02x",&data);
			m_string += (BYTE)data;
			i++;
		}
		else
			m_string += src[i];
	}
}
STRINGText::operator LPCSTR() const
{
	return m_string.c_str();
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HttpUrl::setUrl(LPCSTR url)
{
	String work;
	LPCSTR buff,buff2;

	//プロトコル部の確認
	buff = strchr(url,':');
	if(buff)
	{
		work.assign(url,buff-url);
		url = buff + 1;
	}
	if(work != "http")
		return false;
	m_proto = "http";

	//絶対指定確認
	if(strncmp(url,"//",2))
		return false;
	url += 2;

	//ホスト部
	work.clear();
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
	m_path = url;
	if(m_path.length() == 0)
		m_path = '/';

	return true;
}
LPCSTR HttpUrl::getProto() const
{
	return m_proto.c_str();
}
INT HttpUrl::getPort() const
{
	return m_port;
}
LPCSTR HttpUrl::getHost() const
{
	return m_host.c_str();
}
LPCSTR HttpUrl::getPath() const
{
	return m_path.c_str();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpData
// httpデータ受信
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HttpData::read(LPCSTR urlAddr)
{
	//URL成分の分解
	HttpUrl url;
	if(!url.setUrl(urlAddr))
		return false;

	//コマンドの生成
	String command;
	command.printf("GET %s HTTP/1.0\n",urlAddr);
	//接続
	AFL::SOCK::Sock socket;
	if(m_proxyName.length())
	{
		//プロキシ接続
		if(!socket.connect(m_proxyName,m_proxyPort,3000))
			return false;
		if(m_proxyPass.length())
		{
			String userData;
			userData.printf("%s:%s",m_proxyID.c_str(),m_proxyPass.c_str());
			String passString;
			AtoB64(passString,userData);

			String authString;
			authString.printf("Proxy-Authorization: Basic %s\n",passString.c_str());
			command += authString;
		}
	}
	else
	{
		if(!socket.connect(url.getHost(),url.getPort(),3000))
			return false;
	}

	String host;
	host.printf("HOST: %s\n",url.getHost());
	command += host;

	command += "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)\n\n";

	socket.send(command.c_str(),(INT)command.length());

	BinaryStream work;
	CHAR buff[4000];
	int size;
	while((size = socket.recv(buff,sizeof(buff),0,3000)) > 0)
	{
		work.write(buff,size);
	}
	if(size == 0)
		return false;


	bool header = true;
	LPCSTR data = (LPCSTR)work.getData();
	LPSTR headerLast = (LPSTR)strstr(data,"\r\n\r\n");
	if(!headerLast)
		return false;
	*headerLast = 0;
	headerLast += 4;
	createHeader(data);
	LPCSTR encoding = getHeader("Content-Encoding");
	if(encoding && strcmp(encoding,"gzip")==0)
	{
		ZipReader reader;
		reader.serSource(headerLast,work.getSize()-(headerLast-data));
		while((size = (INT)reader.read(buff,sizeof(buff))) > 0)
		{
			m_body.append(buff,size);
		}
	}
	else
		m_body = headerLast;
	
	return true;
}
LPCSTR HttpData::getHeader(LPCSTR name) const
{
	std::map<String,String>::const_iterator it = m_headers.find(name);
	if(it != m_headers.end())
		return it->second;
	return NULL;
}
LPCSTR HttpData::getBody() const
{
	return m_body.c_str();
}
void HttpData::setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass)
{
	m_proxyName = name;
	m_proxyPort = port;
	m_proxyID = id;
	m_proxyPass = pass;
}
bool HttpData::createHeader(LPCSTR data)
{
	INT i;
	String headerName;
	String headerData;
	for(i=0;data[i] && data[i]!='\n';i++);
	if(data[i]!='\n')
		return false;
	for(++i;data[i];)
	{
		headerName.clear();
		headerData.clear();
		for(;data[i];i++)
		{
			if(data[i] == ':' || data[i] == '\n'|| data[i] == '\r')
				break;
			headerName += data[i];
		}
		if(data[i] == '\n' || data[i] == '\r')
			return false;
		for(++i;data[i]==' ';i++);
		for(;data[i];i++)
		{
			if(data[i] == '\n' || data[i] == '\r')
				break;
			headerData += data[i];
		}
		m_headers[headerName] = headerData;
		while(data[i] == '\n' || data[i] == '\r')
			++i;
	}
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HtmlSplit
// HTMLデータの分解
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool HtmlSplit::read(LPCSTR url)
{
	HttpData http;
	http.setProxy(m_proxyName,m_proxyPort,m_proxyID,m_proxyPass);
	if(!http.read(url))
		return false;
	splitHTML(http.getBody());
	return true;
}
void HtmlSplit::splitHTML(const char* src)
{
	String data;
	INT mode = 0;
	INT mode2 = 0;

	int i;
	bool line = false;

	for(i=0;src[i];i++)
	{

		INT code;
		code = (UCHAR)src[i];

		switch(mode)
		{
		case 0:
			if(code == '<')
			{
				line = true;
				mode = '<';
			}
			else if(code == '\r' || code == '\n')
			{
				line = true;
			}
			else if(code == '\t' || code == ' ')
			{
				if(data.length())
					data += code;
			}
			else
			{
				data += code;
			}
			break;
		case '<':
			if(mode2 == 0)
			{
				if(code == '>')
				{
					data += code;
					line = true;
					mode = 0;
				}
				else if(code == '\r' || code == '\n')
				{
				}
				else
				{
					if(code == '\'' || code == '"')
						mode2 = code;
					data += code;
				}
			}
			else
			{
				if(code == mode2)
					mode2 = 0;
				data += code;
			}
			break;
		}
		if(line)
		{
			INT length = (INT)data.length();
			if(length && length < 500)
			{
				m_splitData.push_back(data);
			}
			data.clear();
			if(code == '<')
				data += '<';
			line = false;
		}
	}

	m_splitPoint.resize(getCount());

	std::list<String>::iterator itSplit;
	for(i=0,itSplit = m_splitData.begin();itSplit != m_splitData.end();++itSplit,i++)
	{
		m_splitPoint[i] = *itSplit;
	}
}
INT HtmlSplit::getCount() const
{
	return (INT)m_splitData.size();
}
LPCSTR HtmlSplit::getData(INT index)const
{
	if(index < (INT)m_splitPoint.size())
		return m_splitPoint[index].c_str();
	throw(0);
}
void HtmlSplit::setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass)
{
	m_proxyName = name;
	m_proxyPort = port;
	m_proxyID = id;
	m_proxyPass = pass;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ChannelData
// チャンネルデータ記憶用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ChannelData::ChannelData()
{
	ypID = 0;
	channelBitrate = 0;
	hitsListeners = 0;
	hitsFirewalled = 0;
	hitsHosts = 0;
	old = false;
	uptime = 0;
}
ChannelData::ChannelData(LPCSTR id)
{
	channelId = id;
}
bool ChannelData::operator <( const ChannelData& channelData) const
{
	return channelId < channelData.channelId;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeercastChannelDB
// Peercastチャンネルデータ操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PeercastChannelDB::PeercastChannelDB()
{
	m_sqlite.open("PeerView.db");
	if(!m_sqlite.isTable("tb_channel"))
	{
		LPCSTR sql =
			"CREATE TABLE tb_channel(channel_id text,channel_name text,channel_genre text,channel_type text,channel_url text,"
			"channel_info text,channel_bitrate int,track_title text,track_artist text,direct text,"
			"hits_listeners int,hits_hosts int,host_ip text,host_port int,"
			"channel_age int,uptime datetime,yp_id int);"
			"CREATE TABLE tb_yp(yp_id INTEGER PRIMARY KEY,yp_name text,yp_url text,yp_type int);"
			"INSERT INTO tb_yp VALUES(null,'YP','http://peercast.pns.to/?cmd=text',3);";
		m_sqlite.exec(sql);
	}
}
PeercastChannelDB::~PeercastChannelDB()
{
	m_sqlite.exec("vacuum;");
}
void PeercastChannelDB::getYPList(std::set<PeerYPData>& datas)
{
	SQRes res = m_sqlite.exec("SELECT * FROM tb_yp ORDER BY yp_id;");
	while(res->next())
	{
		PeerYPData data;
		data.id = atoi(res->getColumn(0));
		data.name = res->getColumn(1);
		data.url = res->getColumn(2);
		data.type = atoi(res->getColumn(3));
		datas.insert(data);
	}
}
void PeercastChannelDB::getYPList(std::map<INT,PeerYPData>& datas)
{
	SQRes res = m_sqlite.exec("SELECT * FROM tb_yp ORDER BY yp_id;");
	while(res->next())
	{
		PeerYPData data;
		data.id = atoi(res->getColumn(0));
		data.name = res->getColumn(1);
		data.url = res->getColumn(2);
		data.type = atoi(res->getColumn(3));
		data.uptime = getChannelUpdateTime(data.id);
		datas[data.id] = data;
	}
}
void PeercastChannelDB::setYPData(INT id,LPCSTR name,LPCSTR url,INT type)
{
	String sql,sql2;
	if(id > 0)
	{
		if(name)
			sql2.appendf("yp_name='%s'",SQLSTRING(name));
		if(url)
		{
			if(sql2.length())
				sql2 += " AND ";
			sql2.appendf("yp_url='%s'",SQLSTRING(url));
		}
		if(type >= 0)
		{
			if(sql2.length())
				sql2 += " AND ";
			sql2.appendf("yp_type='%d'",type);
		}
		sql.appendf("UPDATE tb_yp SET %s WHERE yp_id='%d';",sql2.c_str(),id);
	}
	else
	{
		sql.printf("INSERT INTO tb_yp VALUES(null,'%s','%s','%d');",
			SQLSTRING(name),SQLSTRING(url),type);
	}
	m_sqlite.exec(sql);
}
void PeercastChannelDB::delYP(INT id) const
{
	String sql;
	sql.printf("DELETE FROM tb_yp WHERE yp_id='%d';DELETE FROM tb_channel WHERE yp_id='%d';",id,id);
	m_sqlite.exec(sql);
}
bool PeercastChannelDB::saveChannel(PeerChannelData* peerChannelData,INT id) const
{
	std::set<ChannelData>& channelData = peerChannelData->m_channel;
	std::set<ChannelData>::const_iterator it;
	
	//DB上だと時間がずれるので個々で生成
	time_t timeData;
	time(&timeData);
	tm t = *localtime(&timeData);
	String s;
	s.printf("%04d-%02d-%02d %02d:%02d:%02d",
		t.tm_year+1900,t.tm_mon+1,t.tm_mday,
		t.tm_hour,t.tm_min,t.tm_sec);


	m_sqlite.exec("begin;");
	m_sqlite.exec("delete from tb_channel where uptime < datetime('now','-1 day');");
	for(it=channelData.begin();it!=channelData.end();++it)
	{
		const ChannelData* cd = &*it;
	
		String sql;
		sql.printf("DELETE FROM tb_channel WHERE channel_id='%s';"
			"INSERT INTO tb_channel "
			"values('%s','%s','%s','%s','%s','%s','%d','%s','%s','%s','%d','%d','%s','%d','%d','%s','%d');",
			
			SQLSTRING(cd->channelId.c_str()),
			
			SQLSTRING(cd->channelId.c_str()),
			SQLSTRING(cd->channelName.c_str()),
			SQLSTRING(cd->channelGenre.c_str()),
			SQLSTRING(cd->channelType.c_str()),
			SQLSTRING(cd->channelUrl.c_str()),
			SQLSTRING(cd->channelInfo.c_str()),
			cd->channelBitrate,
			SQLSTRING(cd->trackTitle.c_str()),
			SQLSTRING(cd->trackArtist.c_str()),
			SQLSTRING(cd->direct.c_str()),
			cd->hitsListeners,
			cd->hitsHosts,
			SQLSTRING(cd->tip.ip.c_str()),
			cd->tip.port,
			cd->channelAge,
			s.c_str(),
			id
			);
		m_sqlite.exec(sql);
	}
	m_sqlite.exec("commit;");
	return true;
}
void TimeToString(String& dest,time_t time)
{
	tm t = *localtime(&time);
	t.tm_mon++;
	t.tm_year += 1900;
	dest.printf("%4d-%02d-%02d %02d:%02d:%02d",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
}
time_t StringToTime(LPCSTR text)
{
	tm t;
	ZeroMemory(&t,sizeof(tm));
	if(sscanf(text,"%4d-%02d-%02d %02d:%02d:%02d",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec)!=6)
		return 0;
	t.tm_mon--;
	t.tm_year -= 1900;
	return mktime(&t);
}
bool PeercastChannelDB::loadChannel(PeerChannelData* peerChannelData,INT id,time_t t)
{
	std::set<ChannelData>& channelData = peerChannelData->m_channel;

	String datetime;
	TimeToString(datetime,t);

	channelData.clear();

	String sql;
	if(id == 0)
		sql.printf("SELECT * FROM tb_channel WHERE uptime > '%s';",datetime.c_str());
	else
		sql.printf("SELECT * FROM tb_channel WHERE yp_id='%d' AND uptime > '%s';",id,datetime.c_str());

	SQRes res = m_sqlite.exec(sql);
	while(res->next())
	{
		ChannelData cd;
		cd.ypID = atoi(res->getColumnFromName("yp_id"));
		cd.channelName = res->getColumnFromName("channel_name");
		cd.channelId = res->getColumnFromName("channel_id");
		cd.channelGenre = res->getColumnFromName("channel_genre");
		cd.channelType = res->getColumnFromName("channel_type");
		cd.channelUrl = res->getColumnFromName("channel_url");
		cd.channelInfo = res->getColumnFromName("channel_info");
		cd.channelBitrate = atoi(res->getColumnFromName("channel_bitrate"));
		cd.trackArtist = res->getColumnFromName("track_artist");
		cd.trackTitle = res->getColumnFromName("track_title");
		cd.direct = res->getColumnFromName("direct");
		cd.hitsHosts = atoi(res->getColumnFromName("hits_hosts"));
		cd.hitsListeners = atoi(res->getColumnFromName("hits_listeners"));
		cd.tip.ip = res->getColumnFromName("host_ip");
		cd.tip.port = atoi(res->getColumnFromName("host_port"));
		cd.channelAge = atoi(res->getColumnFromName("channel_age"));
		cd.uptime = StringToTime(res->getColumnFromName("uptime"));

		channelData.insert(cd);
	}
	return true;
}
time_t PeercastChannelDB::getChannelUpdateTime(INT id)
{
	String sql;
	if(id == 0)
		sql = "SELECT max(uptime) FROM tb_channel;";
	else
		sql.printf("SELECT max(uptime) FROM tb_channel WHERE yp_id='%d';",id);
	SQRes res = m_sqlite.exec(sql);
	LPCSTR data = res->getColumn();
	if(data)
		return StringToTime(data);
	return 0;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerChannelData
// チャンネルデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ChannelData* PeerChannelData::getChannelData(LPCSTR channelID)
{
	std::set<ChannelData>::iterator it;
	it = m_channel.find(channelID);
	if(it == m_channel.end())
		return NULL;
	return (ChannelData*)&*it;
}
INT PeerChannelData::getChannelCount()const
{
	return (INT)m_channel.size();
}
INT PeerChannelData::getChannelHost()const
{
	INT count = 0;
	std::set<ChannelData>::const_iterator it;
	foreach(it,m_channel)
	{
		count += it->hitsHosts;
	}
	return count;
}
INT PeerChannelData::getChannelListener()const
{
	INT count = 0;
	std::set<ChannelData>::const_iterator it;
	foreach(it,m_channel)
	{
		count += it->hitsListeners;
	}
	return count;
}
std::set<ChannelData>& PeerChannelData::getChannelData()
{
	return m_channel;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerRelayData
// リレーデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
RelayData* PeerRelayData::getRelayData(LPCSTR channelID)
{
	std::set<RelayData>::iterator it;
	it = m_relay.find(channelID);
	if(it == m_relay.end())
		return NULL;
	return (RelayData*)&*it;
}
std::set<RelayData>& PeerRelayData::getRelayData()
{
	return m_relay;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerCastData
// 放送設定データ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PeerCastData::PeerCastData()
{
	m_dataName = "NewData";
	m_channelBitrate = 0;
	m_channelType = "WMV";
}
void PeerCastData::setDataName(LPCSTR value)
{
	m_dataName = value;
}
LPCSTR PeerCastData::getDataName()const
{
	return m_dataName;
}
void PeerCastData::setChannelName(LPCSTR value)
{
	m_channelName = value;
}
LPCSTR PeerCastData::getChannelUrl()const
{
	return m_channelUrl;
}
void PeerCastData::setChannelUrl(LPCSTR value)
{
	m_channelUrl = value;
}
LPCSTR PeerCastData::getChannelName()const
{
	return m_channelName;
}
void PeerCastData::setChannelDescription(LPCSTR value)
{
	m_channelDescription = value;
}
LPCSTR PeerCastData::getChannelDescription() const
{
	return m_channelDescription;
}
void PeerCastData::setChannelGenre(LPCSTR value)
{
	m_channelGenre = value;
}
LPCSTR PeerCastData::getChannelGenre()const
{
	return m_channelGenre;
}
void PeerCastData::setChannelContact(LPCSTR value)
{
	m_channelContact = value;
}
LPCSTR PeerCastData::getChannelContact()const
{
	return m_channelContact;
}
void PeerCastData::setChannelBitrate(INT value)
{
	m_channelBitrate = value;
}
INT PeerCastData::getChannelBitrate()const
{
	return m_channelBitrate;
}
void PeerCastData::setChannelType(LPCSTR value)
{
	m_channelType = value;
}
LPCSTR PeerCastData::getChannelType()const
{
	return m_channelType;
}
/*
void PeerCastData::convertUTF8()
{
	SJIStoUTF8(m_dataName,m_dataName);
	SJIStoUTF8(m_channelUrl,m_channelUrl);
	SJIStoUTF8(m_channelName,m_channelName);
	SJIStoUTF8(m_channelDescription,m_channelDescription);
	SJIStoUTF8(m_channelGenre,m_channelGenre);
	SJIStoUTF8(m_channelContact,m_channelContact);
	SJIStoUTF8(m_channelType,m_channelType);
}
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Peercast
// Peercast操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void Peercast::setServerAdr(LPCSTR value)
{
	m_serverAdr = value;
}
void Peercast::setServerPort(INT value)
{
	m_serverPort = value;
}
void Peercast::setServerPass(LPCSTR value)
{
	m_serverPass = value;
}
LPCSTR Peercast::getServerAdr() const
{
	return m_serverAdr;
}
INT Peercast::getServerPort() const
{
	return m_serverPort;
}
LPCSTR Peercast::getServerPass() const
{
	return m_serverPass;
}
void Peercast::setProxy(bool flag)
{
	if(!flag)
	{
		//プロキシの無効化
		m_proxyName = "";
	}
}
void Peercast::setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass)
{
	m_proxyName = name;
	m_proxyPort = port;
	m_proxyID = id;
	m_proxyPass = pass;
}
void convert(String& dest,LPCSTR src)
{
	dest.clear();
	INT i;
	for(i=0;src[i];i++)
	{
		if(src[i] == '%')
		{
			INT code;
			i++;
			sscanf(&src[i],"%02x",&code);
			i++;
			dest += (UCHAR)code;
		}
		else if(src[i] == '+')
			dest += ' ';
		else
			dest += src[i];
	}
}


bool Peercast::createListXML(PeerChannelData* peerChannelData,LPCSTR ypURL)
{
	HttpData httpData;
	httpData.setProxy(m_proxyName,m_proxyPort,m_proxyID,m_proxyPass);
	if(!httpData.read(ypURL))
		return false;

	std::set<ChannelData>& channelDatas = peerChannelData->m_channel;
	//チャンネル情報の初期化
	//channelDatas.clear();

	LPCSTR data = strstr(httpData.getBody(),"<?");
	if(!data)
		return false;
	Xml xml;
	if(!xml.loadMemory(data))
		return false;
	Xml* xmlPeercast = xml.get();
	if(strcmp(xmlPeercast->getName(),"peercast") != 0)
		return false;

	Xml* xmlChannels = NULL;
	while(xmlChannels = xmlPeercast->get(xmlChannels))
	{
		if(strcmp(xmlChannels->getName(),"channels_found") != 0)
			continue;

		Xml* xmlChannel = NULL;
		while(xmlChannel = xmlChannels->get(xmlChannel))
		{
			ChannelData channelData;
			if(strcmp(xmlChannel->getName(),"channel") != 0)
				continue;
			if(xmlChannel->isParam("name"))
				channelData.channelName = xmlChannel->getParam("name");
			if(xmlChannel->isParam("id"))
				channelData.channelId = xmlChannel->getParam("id");
			if(xmlChannel->isParam("bitrate"))
				channelData.channelBitrate = xmlChannel->getParamInt("bitrate");
			if(xmlChannel->isParam("type"))
				channelData.channelType = xmlChannel->getParam("type");
			if(xmlChannel->isParam("genre"))
				channelData.channelGenre = xmlChannel->getParam("genre");
			if(xmlChannel->isParam("desc"))
				channelData.channelInfo = xmlChannel->getParam("desc");
			if(xmlChannel->isParam("url"))
				channelData.channelUrl = xmlChannel->getParam("url");
			if(xmlChannel->isParam("age"))
				channelData.channelAge = atoi(xmlChannel->getParam("age"));
			if(xmlChannel->isParam("listeners"))
				channelData.hitsListeners = xmlChannel->getParamInt("listeners");
			if(xmlChannel->isParam("relays"))
				channelData.hitsHosts = xmlChannel->getParamInt("relays");
			if(xmlChannel->isParam("tip"))
			{
				CHAR host[128];
				INT port;
				sscanf(xmlChannel->getParam("tip"),"%[^:]:%d",host,&port);
				channelData.tip.ip = host;
				channelData.tip.port = port;
			}

			Xml* xmlData = NULL;
			while(xmlData = xmlChannel->get(xmlData))
			{
				if(strcmp(xmlData->getName(),"hits") == 0)
				{
					if(xmlData->isParam("listeners"))
						channelData.hitsListeners = xmlData->getParamInt("listeners");
					if(xmlData->isParam("relays"))
						channelData.hitsHosts = xmlData->getParamInt("relays");

					Xml* xmlHost = NULL;
					while(xmlHost = xmlData->get(xmlHost))
					{
						CHAR host[128];
						INT port;
						sscanf(xmlHost->getParam("ip"),"%[^:]:%d",host,&port);
						channelData.tip.ip = host;
						channelData.tip.port = port;
					}
				}
				else if(strcmp(xmlData->getName(),"track") == 0)
				{
					if(xmlData->isParam("title"))
						channelData.trackTitle = xmlData->getParam("title");
					if(xmlData->isParam("album"))
						channelData.trackAlbum = xmlData->getParam("album");
					if(xmlData->isParam("artist"))
						channelData.trackArtist = xmlData->getParam("artist");
				}
			}
			channelDatas.insert(channelData);

		}
	}
	return true;

}
void split(std::list<String>& dest,LPCSTR src,CHAR c)
{
	while(*src)
	{
		LPCSTR data = strchr(src,c);
		if(data)
		{
			String work;
			work.append(src,data - src);
			dest.push_back(work);
			src = data+1;
		}
		else
		{
			dest.push_back(src);
			break;
		}
	}
}
class HttpText
{
public:

	bool setData(LPCSTR src)
	{
		INT i,j;
		std::list<String> line;
		split(line,src,'\n');

		std::list<String> header;
		std::list<String>::iterator itLine = line.begin();
		if(itLine==line.end())
			return false;
		split(header,*itLine,',');

		//ヘッダーの切り分け
		i=0;
		std::list<String>::iterator itHeader;
		foreach(itHeader,header)
		{
			m_header[STRINGTEXT(*itHeader)] = i;
			i++;
		}
		//データの設定
		m_data.resize(line.size()-1);
		for(i=0,itLine++;itLine!=line.end();itLine++,i++)
		{
			m_data[i].resize(header.size());

			std::list<String> data;
			split(data,*itLine,',');
			std::list<String>::iterator it;
			j=0;
			foreach(it,data)
			{
				m_data[i][j] = STRINGTEXT(*it);
				j++;
			}
		}
		return true;
	}
	LPCSTR getData(INT index,LPCSTR name)
	{
		std::map<String,INT>::iterator itHeader = m_header.find(name);
		if(itHeader == m_header.end())
			return NULL;
		return m_data[index][itHeader->second];
	}
	INT getRows() const
	{
		return (INT)m_data.size();
	}
	INT getCols() const
	{
		return (INT)m_header.size();
	}
protected:
	std::map<String,INT> m_header;
	std::vector<std::vector<String> > m_data;
};
Peercast::Peercast()
{
	m_serverAdr = "localhost";
	m_serverPort = 7144;
}
bool Peercast::createListTEXT(PeerChannelData* peerChannelData,LPCSTR ypURL)
{
	HttpData httpData;
	httpData.setProxy(m_proxyName,m_proxyPort,m_proxyID,m_proxyPass);
	if(!httpData.read(ypURL))
		return false;

	std::set<ChannelData>& channelData = peerChannelData->m_channel;
	//チャンネル情報の初期化
	channelData.clear();

	LPCSTR src = httpData.getBody();
	
	HttpText data;
	data.setData(src);

	INT i;
	INT rows = data.getRows();
	for(i=0;i<rows;i++)
	{
		ChannelData channelData;
		LPCSTR text;
		
		if(text= data.getData(i,"name"))
			channelData.channelName = text;
		if(text= data.getData(i,"info"))
			channelData.channelInfo = text;
		if(text= data.getData(i,"id"))
			channelData.channelId = text;
		if(text= data.getData(i,"bitrate"))
			channelData.channelBitrate = atoi(text);
		if(text= data.getData(i,"type"))
			channelData.channelType = text;
		if(text= data.getData(i,"genre"))
			channelData.channelGenre = text;
		if(text= data.getData(i,"listeners"))
			channelData.hitsListeners = atoi(text);
		if(text= data.getData(i,"relays"))
			channelData.hitsHosts = atoi(text);
		if(text= data.getData(i,"tip"))
		{
			CHAR ip[512];
			INT port;
			if(sscanf(text,"%[^:]:%d",ip,&port) == 2)
			{
				channelData.tip.ip = ip;
				channelData.tip.port = port;
			}
		}
		if(text= data.getData(i,"url"))
			channelData.channelUrl = text;
		if(text= data.getData(i,"title"))
			channelData.trackTitle = text;
		if(text= data.getData(i,"artist"))
			channelData.trackArtist = text;
		channelData.channelAge = 0;
		peerChannelData->getChannelData().insert(channelData);

	}

	return true;
}
bool Peercast::createListVP(PeerChannelData* peerChannelData,LPCSTR ypURL,INT port)
{
	std::set<ChannelData>& channelData = peerChannelData->m_channel;
	String ypUrl;
	if(strchr(ypURL,'?') != NULL)
		ypUrl.printf("%s&host=localhost:%d",ypURL,port);
	else
		ypUrl.printf("%s?host=localhost:%d",ypURL,port);
	HttpData httpData;
	httpData.setProxy(m_proxyName,m_proxyPort,m_proxyID,m_proxyPass);
	if(!httpData.read(ypUrl))
		return false;

	//チャンネル情報の初期化
	channelData.clear();

	std::list<String> dataList;

	INT i = 0;
	String work;
	String work2;
	LPCSTR data = httpData.getBody();
	while(data[i])
	{
		if(data[i] != '<')
			work += data[i];
		else
		{
			i++;
			convert(work2,work.c_str());
			dataList.push_back(work2);
			work.clear();
		}
		i++;
	}
	if(dataList.size()%18 != 0)
		return false;
	std::list<String>::iterator it;
	foreach(it,dataList)
	{
		ChannelData channelData;
		*it++;
		channelData.channelId = *it++;
		
		CHAR ip[512];
		INT port;
		if(sscanf(*it++,"%[^:]:%d",ip,&port) == 2)
		{
			channelData.tip.ip = ip;
			channelData.tip.port = port;
		}
		channelData.channelUrl = *it++;
		channelData.channelGenre = convertString(*it++);
		channelData.channelInfo = convertString(*it++);
		channelData.hitsListeners = (*it++).toInt();
		channelData.hitsHosts = (*it++).toInt();
		channelData.channelBitrate = (*it++).toInt();
		channelData.channelType = convertString(*it++);
		channelData.trackArtist = convertString(*it++);
		channelData.trackAlbum = convertString(*it++);
		channelData.trackTitle = convertString(*it++);
		++it;
		channelData.channelName = convertString(*it++);

		INT hour = 0;
		INT min =  0;
		sscanf(*it++,"%d:%d",&hour,&min);
		channelData.channelAge = hour*60*60+min*60;

		++it;
		peerChannelData->getChannelData().insert(channelData);
	}
	return true;
}

bool Peercast::createList(PeerChannelData* peerChannelData,LPCSTR ypURL,INT type)
{
	if(type == 1)
		return createListVP(peerChannelData,ypURL,getServerPort());
	else if(type == 2)
		return createListXML(peerChannelData,ypURL);
	else if(type == 3)
		return createListTEXT(peerChannelData,ypURL);


	INT i;
	INT rank;
	bool check;
	bool param;
	param = strchr(ypURL,'?') != NULL;


	std::set<ChannelData>& channelData = peerChannelData->m_channel;
	//チャンネル情報の初期化
	//channelData.clear();
	for(rank=1,check=true;check && rank<300;rank+=20)
	{
		HtmlSplit http;
		http.setProxy(m_proxyName,m_proxyPort,m_proxyID,m_proxyPass);
		CHAR ypUrl[512];
		if(param)
			sprintf(ypUrl,"%s&from=%d&host=localhost:%d",ypURL,rank,getServerPort());
		else
			sprintf(ypUrl,"%s?from=%d&host=localhost:%d",ypURL,rank,getServerPort());
		if(!http.read(ypUrl))
			return false;
		check = false;
		INT count = http.getCount();


/*		FILE* file = fopen("debug.txt","wt");
		for(i=0;i<count;i++)
		{
			fprintf(file,"%d %s\n",i,http.getData(i));
		}
		fclose(file);
//		break;
*/
		try
		{
			for(i=0;i<count;i++)
			{
				static LPCSTR header = "<a href=\"";
				static LPCSTR header2 = "<a href=\"";
				static LPCSTR header3 = "<br";
				static LPCSTR header4 = "<i";
				static LPCSTR header5 = "<sma";
				static LPCSTR header6 = "</td";
				static LPCSTR header7 = "<font";
				static LPCSTR header8 = "</i";
				static LPCSTR header9 = "<td align=\"center\">";
				static LPCSTR header10 = "<tr class=";
				
				LPCSTR data;
				//チャンネル情報先頭検索
				while((data = http.getData(i++)) && strncmp(data,header10,strlen(header10))!=0);
				while((data = http.getData(i++)) && strncmp(data,header,strlen(header))!=0);
				
				ChannelData channel;
				channel.channelAge = 0;
				CHAR id[512],ip[512];
				INT port;

				//チャンネルID
				data = strstr(data,"/pls/");
				if(!data)
					continue;

				sscanf(data,"/pls/%[^?\"]",id);
				channel.channelId = id;
				
				channel.tip.ip = "0";
				channel.tip.port = 0;

				INT j;
				for(j=0;data[j] && data[j]!='?';j++);
				if(data[j] == '?')
				{
					while(data[j])
					{
						j++;
						if(data[j] == 't')
						{
							if(sscanf(&data[j],"tip=%[^:]:%d",ip,&port) != 2)
								break;
							channel.tip.ip = ip;
							channel.tip.port = port;
						}
						else
						{
							if(sscanf(&data[j],"ip=%[^:]:%d",ip,&port) != 2)
								break;
							channel.ip.push_back(std::pair<String,INT>(ip,port));
						}
						for(j+=4;data[j] && data[j]!='&';j++);
					}
				}

				//i++;
				data = http.getData(i);
				if(strstr(data,"play1") == NULL)
					channel.old = false;
				else
					channel.old = true;

				i += 5;
				data = http.getData(i);

				//リンク
				bool aFlag = false;
				if(strncmp(data,header2,strlen(header2)) == 0)
				{
					CHAR url[512];
					sscanf(data,"<a href=\"%[^\"]",url);
					channel.channelUrl = url;
					i++;
					aFlag = true;
				}
				data = http.getData(i);
				//チャンネル名
				channel.channelName = convertString(data);

				if(aFlag)
					i++;
				i+=3;

				data = http.getData(i);
				if(strncmp(data,header3,strlen(header3)) == 0)
				{
					i++;
					data = http.getData(i);
					if(*data != '<')
					{
						//追加情報
						CHAR buff[1024];
						if(sscanf(data,"[%[^]]",buff) == 1)
							channel.channelGenre = convertString(buff);
						i++;
					}
				}
				while((data = http.getData(i++)) && strcmp(data,header9)!=0)
				{
					if(data[0] != '<')
					{
						channel.channelInfo = convertString(data);
						break;
					}
				}

				while((data = http.getData(i++)) && strcmp(data,header9)!=0);
				if(!data)
					continue;
				//リスナー/ホスト数
				INT lisners;
				INT hosts;
				data = http.getData(i);
				if(sscanf(data,"%d / %d",&lisners,&hosts) != 2)
				{
					i++;
					data = http.getData(i);
					if(sscanf(data,"%d / %d",&lisners,&hosts) != 2)
						continue;
				}
				channel.hitsListeners = lisners;
				channel.hitsHosts = hosts;
				channel.hitsFirewalled = 0;

				i+=10;
				data = http.getData(i);
				if(!strstr(data,"kb/s"))
					continue;
				//ビットレート
				INT bitrate;
				sscanf(data,"%d",&bitrate);
				channel.channelBitrate = bitrate;

				i+=3;
				data = http.getData(i);
				if(strncmp(data,header2,strlen(header2)) == 0)
				{
					//ダイレクト
					CHAR url[512];
					sscanf(data,"<a href=\"%[^\"]",url);
					channel.direct = url;
					
					i++;
					data = http.getData(i);
				}
				//タイプ
				if(*data == '<')
					continue;
				channel.channelType = data;

				channelData.insert(channel);

				//情報取得フラグ
				check = true;
			}
		}
		catch(INT)
		{
		}
	}
	
	return true;
}
bool Peercast::createStat(PeerRelayData* peerRelayData)
{
	Xml* xml = readXmlData();
	if(!xml)
		return false;

	std::set<RelayData>& relay = peerRelayData->m_relay;
	//リレー情報の初期化
	relay.clear();

	Xml* xmlPeercast = xml->get();
	if(!xmlPeercast)
		return false;
	Xml* xmlWork = NULL;
	while(xmlWork = xmlPeercast->get(xmlWork))
	{
		if(strcmp(xmlWork->getName(),"channels_relayed") == 0)
		{
			RelayData relayData;
			INT relayCount;

			int i;
			Xml* xmlRelay;
			for(i=0,xmlRelay=NULL;xmlRelay=xmlWork->get(xmlRelay);i++);
			relayCount = i;

			for(xmlRelay=NULL;xmlRelay=xmlWork->get(xmlRelay);)
			{
				//チャンネル情報の取得
				relayData.channelId = xmlRelay->getParam("id");
				relayData.channelName = (xmlRelay->getParam("name"));
				relayData.channelGenre = convertString(xmlRelay->getParam("genre"));
				relayData.channelType = convertString(xmlRelay->getParam("type"));
				relayData.channelUrl = convertString(xmlRelay->getParam("url"));
				relayData.channelBitrate = xmlRelay->getParamInt("bitrate");
				relayData.channelComment = convertString(xmlRelay->getParam("comment"));

				Xml* xmlSub = NULL;
				while(xmlSub=xmlRelay->get(xmlSub))
				{
					if(strcmp(xmlSub->getName(),"track") == 0)
					{
						relayData.trackTitle = convertString(xmlSub->getParam("title"));
						relayData.trackArtist = convertString(xmlSub->getParam("artist"));
						relayData.trackAlbum = convertString(xmlSub->getParam("album"));
					}
					else if(strcmp(xmlSub->getName(),"relay") == 0)
					{
						relayData.channelStatus = xmlSub->getParam("status");
						relayData.relayListener = xmlSub->getParamInt("listeners");
						relayData.relayHost = xmlSub->getParamInt("relays");
					}
				}
				relay.insert(relayData);
			}
		}
		else if(strcmp(xmlWork->getName(),"channels_found") == 0)
		{
			INT i;
			Xml* xmlChannel;
			for(i=0,xmlChannel=NULL;xmlChannel=xmlWork->get(xmlChannel);i++)
			{
				if(strcmp(xmlChannel->getName(),"channel") != 0)
					continue;
				/*
				LPCSTR channelID = xmlChannel->getParam("id");
				ChannelData* channelData = getChannelData(channelID);
				if(channelData)
				{
					//チャンネル情報の取得
					channelData->channelName = convertString(xmlChannel->getParam("name"));
					channelData->channelGenre = convertString(xmlChannel->getParam("genre"));
					channelData->channelType = convertString(xmlChannel->getParam("type"));
					channelData->channelId = xmlChannel->getParam("id");
					channelData->channelUrl = convertString(xmlChannel->getParam("url"));
					channelData->channelBitrate = xmlChannel->getParamInt("bitrate");

					Xml* xmlSub;
					for(i=0,xmlSub=NULL;xmlSub=xmlChannel->get(xmlSub);i++)
					{
						if(strcmp(xmlSub->getName(),"track") == 0)
						{
							channelData->trackTitle = convertString(xmlSub->getParam("title"));
							channelData->trackArtist = convertString(xmlSub->getParam("artist"));
						}
						else if(strcmp(xmlSub->getName(),"hits") == 0)
						{
							//channelData->hitsHosts = xmlSub->getParamInt("hosts");
							//channelData->hitsFirewalled = xmlSub->getParamInt("firewalled");
							//channelData->hitsListeners = xmlSub->getParamInt("listeners");

							HostData hostData;
							hostData.channelID = channelID;
							//ホストデータ詳細
							Xml* xmlHost;
							for(xmlHost=NULL;xmlHost=xmlSub->get(xmlHost);)
							{
								if(strcmp(xmlHost->getName(),"host") == 0)
								{
									ChannelHostData channelHostData;
									channelHostData.hostIP = xmlHost->getParam("ip");
									channelHostData.hostHops = xmlHost->getParamInt("hops");
									channelHostData.hostListeners = xmlHost->getParamInt("listeners");
									channelHostData.hostUptime = xmlHost->getParamInt("uptime");
									channelHostData.hostSkips = xmlHost->getParamInt("skips");
									channelHostData.hostPush = xmlHost->getParamInt("push");
									channelHostData.hostBusy = xmlHost->getParamInt("busy");
									channelHostData.hostStable = xmlHost->getParamInt("stable");
									channelHostData.hostUpdate = xmlHost->getParamInt("update");
									hostData.host.push_back(channelHostData);
								}
							}
							m_host.insert(hostData);

						}
					}
				}
				*/
			}
		}
	}

	delete xml;
	return true;
}


bool getString(FILE* file,String& dest)
{
	CHAR buff[512];
	if(!fgets(buff,sizeof(buff),file))
		return false;
	LPSTR r = strchr(buff,'\n');
	if(r)
		*r = 0;
	dest = buff;
	return true;
}
bool getInt(FILE* file,INT& dest)
{
	CHAR buff[512];
	if(!fgets(buff,sizeof(buff),file))
		return false;
	dest = 0;
	return sscanf(buff,"%d",&dest)!=0;
}




void Peercast::AtoB64(LPSTR pDest,LPCSTR pSrc,int iSize)
{
	static const CHAR BASE64CHAR[] = 
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"
	};
	int i,j;
	int	iBit = 2;
	char cWork = 0;
	LPSTR pOut = pDest;
	for(i=0;i<iSize;i++)
	{
		cWork = cWork << ( 6 - (iBit-2) );
		cWork = cWork | (*pSrc >> iBit );
		*pOut++ = BASE64CHAR[ cWork & 0x3f ];
		cWork = *pSrc++;
		iBit += 2;

		if( iBit >= 8 )
		{
			*pOut++ = BASE64CHAR[ cWork & 0x3f ];
			cWork = 0;
			iBit = 2;
		}
	}
	cWork = cWork << ( 6 - (iBit-2) );
	*pOut++ = BASE64CHAR[ cWork & 0x3f ];
	for( j = 0 ; j < (pOut-pDest)%4; j++ )
		*pOut++ = '=';

	*pOut = '\0';
}



String Peercast::convertString(LPCSTR pstrSrc)
{
	if(!pstrSrc)
		return "";
	UCHAR cData;
	String stringWork;
	String stringDest;
	UCHAR const* pString = (UCHAR const*)pstrSrc;
	for(;cData=*pString;pString++)
	{
		if(cData=='&')
		{
			CHAR buff[256];
            if(sscanf((LPCSTR)pString,"&%[^;];",buff))
			{
				if(buff[0] == '#')
					stringWork += (UCHAR)atoi(buff+1);
				else if(strcmp(buff,"amp") == 0)
					stringWork += '&';
				else if(strcmp(buff,"gt") == 0)
					stringWork += '<';
				else if(strcmp(buff,"lt") == 0)
					stringWork += '>';
				else
				{
					stringWork += '&';
					continue;
				}

				pString += strlen(buff) + 1;
			}
			else
				++pString;
			//pString+=5;
		}
		else
			stringWork += cData;
	}
	return stringWork;
}

Xml* Peercast::readXmlData()
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort,3000))
		return NULL;
	String strSendCommand;
	strSendCommand = "GET /admin?cmd=viewxml HTTP/1.0\n";
	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';


	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());

	String work;
	CHAR buff[4000];
	int size;
	while((size = socket.recv(buff,sizeof(buff),0,5000)) > 0)
	{
		work.append(buff,size);
	}

	LPCSTR src = strstr(work.c_str(),"\n\n");
	if(!src || !src[0] || !src[1])
		return NULL;
	Xml* xml = new Xml;
	xml->loadMemory(src+2);

	return xml;
	
}
void Peercast::play(LPCSTR channelID,LPCSTR streamType,LPCSTR topIP,INT topPort)
{
#ifdef _WIN32
	static INT count = 0;
	String fileName;
	CHAR path[MAX_PATH];
	//CHAR cTmpFile[MAX_PATH];
	GetTempPathA(MAX_PATH,path);
	fileName.printf("%s\\PeerView.%05d.tmp.m3u",path,count);
	count++;
			
	FILE* pFile = fopen(fileName,"wt");
	if(pFile)
	{
		if(topIP)
		{
			fprintf(pFile,"http://%s:%d/stream/%s.%s?tip=%s:%d",
				m_serverAdr.c_str(),m_serverPort,channelID,streamType,topIP,topPort);
		}
		else
		{
			fprintf(pFile,"http://%s:%d/stream/%s.%s",m_serverAdr.c_str(),m_serverPort,channelID,streamType);
		}
		fclose(pFile);
		ShellExecuteA(NULL,"open",fileName,NULL,NULL,SW_SHOWNORMAL);
		Sleep(3000);
		remove(fileName);
	}
#endif
}
void Peercast::play(LPCSTR appPath,LPCSTR option,LPCSTR server,INT port,LPCSTR channelID,LPCSTR streamType,LPCSTR topIP,INT topPort)
{
#ifdef _WIN32
	String params;
	if(topIP)
	{
		params.printf("http://%s:%d/stream/%s.%s?tip=%s:%d",
			server,port,channelID,streamType,topIP,topPort);
	}
	else
	{
		params.printf("http://%s:%d/stream/%s.%s",server,port,channelID,streamType);
	}
	/*
	WINDOWS::PathName path(appPath);
	String dir;
	path.getPathDir(dir);
	ShellExecuteA(NULL,"open",appPath,params,dir,SW_SHOWNORMAL);
	*/
	ShellExecuteA(NULL,"open",appPath,params,NULL,SW_SHOWNORMAL);

#endif
}
void Peercast::bump(LPCSTR pID)
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort))
		return;

	String strSendCommand;
	strSendCommand = "GET /admin?cmd=bump&id=";
	strSendCommand += pID;
	strSendCommand += " HTTP/1.0\n";

	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';

	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());
	Sleep(0);
}
void Peercast::stop(LPCSTR pID)
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort))
		return;

	String strSendCommand;
	strSendCommand = "GET /admin?cmd=stop&id=";
	strSendCommand += pID;
	strSendCommand += " HTTP/1.0\n";

	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';

	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());
	Sleep(0);
}
void Peercast::relay(LPCSTR pID)
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort))
		return;

	String strSendCommand;
	strSendCommand = "GET /admin?cmd=keep&id=";
	strSendCommand += pID;
	strSendCommand += " HTTP/1.0\n";

	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';

	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());
	Sleep(0);
}
void Peercast::keep(LPCSTR pID)
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort))
		return;

	String strSendCommand;
	strSendCommand = "GET /admin?cmd=keep&id=";
	strSendCommand += pID;
	strSendCommand += " HTTP/1.0\n";

	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';

	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());
	Sleep(0);
}
void convertURL(String& dest,LPCSTR src)
{
	dest.clear();
	//データをURLに載せられるように変換
	INT i;
	for(i=0;src[i];i++)
	{
		char c = src[i];
		if(c >= 'a' && c<= 'z' || c >= 'A' && c <= 'Z' ||
			c >= '0' && c<= '9' || c >= '_' || c <= '.' )
		{
			dest += c;
		}
		else
		{
			dest.appendf("%%%02X",(UCHAR)c);
		}
	}
}


void Peercast::cast(PeerCastData* castData)
{
	Sock socket;
	if(!socket.connect(m_serverAdr,m_serverPort))
		return;

	String url,name,desc,genre,contact,type;
	convertURL(url,castData->getChannelUrl());
	convertURL(name,castData->getChannelName());
	convertURL(desc,castData->getChannelDescription());
	convertURL(genre,castData->getChannelGenre());
	convertURL(contact,castData->getChannelContact());
	convertURL(type,castData->getChannelType());

	//コマンドの作成
	String strSendCommand;
	strSendCommand.printf("GET /admin?cmd=fetch&url=%s&name=%s&"
		"desc=%s&genre=%s&contact=%s&bitrate=%d&type=%s&stream=CreateRelay HTTP/1.0\n",
		url.c_str(),name.c_str(),desc.c_str(),genre.c_str(),contact.c_str(),
		castData->getChannelBitrate(),type.c_str());

	//認証ヘッダの追加
	if(m_serverPass.length())
	{
		CHAR cBuff[1024];
		String strPassword = ":";
		strPassword += m_serverPass; 
		AtoB64(cBuff,strPassword.c_str(),(int)strPassword.length());
		strSendCommand += "Authorization: Basic ";
		strSendCommand += cBuff;
		strSendCommand += "\n\n";
	}
	else
		strSendCommand += '\n';

	socket.send(strSendCommand.c_str(),(int)strSendCommand.length());
	Sleep(1);	
}

//namespace
};};
