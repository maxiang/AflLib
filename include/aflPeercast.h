#pragma warning( disable : 4786 )	//STLの警告外し


//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLPEERCAST
//-----------------------------------------------------
#include <string>
#include <set>
#include <list>
#include <vector>
#include "aflStd.h"

#include "aflXml.h"
#include "aflSqlite.h"
//-------------------------------------------------
//namespace AFL::Peercast
namespace AFL{namespace PEERCAST{
using namespace AFL::SQLITE;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// STRINGText
// テキスト変換
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define STRINGTEXT(a) ((LPCSTR)STRINGText(a))
class STRINGText
{
public:
	STRINGText(LPCSTR src);
	operator LPCSTR() const;
protected:
	String m_string;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpUrl
// URL成分解析
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HttpUrl
{
public:
	bool setUrl(LPCSTR url);

	LPCSTR getProto()const;
	INT getPort()const;
	LPCSTR getHost()const;
	LPCSTR getPath()const;
protected:
	String m_proto;
	INT m_port;
	String m_host;
	String m_path;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HttpData
// httpデータ受信
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HttpData
{
public:
	bool read(LPCSTR urlAddr);
	LPCSTR getHeader(LPCSTR name) const;
	LPCSTR getBody()const;
	void setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass);
protected:
	bool createHeader(LPCSTR data);
	std::map<String,String> m_headers;
	String m_body;
	String m_proxyName;
	INT m_proxyPort;
	String m_proxyID;
	String m_proxyPass;

};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HtmlSplit
// HTMLデータの分解
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class HtmlSplit
{
public:
	bool read(LPCSTR url);
	void splitHTML(const char* src);

	INT getCount()const;
	LPCSTR getData(INT index)const;
	void setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass);
protected:
	std::list<String> m_splitData;
	std::vector<String> m_splitPoint;

	String m_proxyName;
	INT m_proxyPort;
	String m_proxyID;
	String m_proxyPass;
};



struct ChannelHostData
{
	String hostIP;
	String hostAgent;
	int hostHops;
	int hostListeners;
	int hostUptime;
	int hostSkips;
	int hostPush;
	int hostBusy;
	int hostStable;
	int hostUpdate;
};
struct HostData
{
	HostData()
	{
	}
	HostData(LPCSTR id)
	{
		channelID = id;
	}
	bool operator <(const HostData& hostData) const
	{
		return channelID < hostData.channelID;
	}
	String channelID;
	std::vector<ChannelHostData> host;
};

struct IPData
{
	String ip;
	INT port;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ChannelData
// チャンネルデータ記憶用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct ChannelData
{
	ChannelData();
	ChannelData(LPCSTR id);
	bool operator <( const ChannelData& channelData) const;
	INT ypID;
	String channelName;
	String channelGenre;
	int channelBitrate;
	String channelType;
	String channelUrl;
	String channelId;
	String channelInfo;
	INT channelAge;
	String trackAlbum;
	String trackTitle;
	String trackArtist;
	String direct;
	int hitsListeners;
	int hitsFirewalled;
	int hitsHosts;
	IPData tip;
	std::list<std::pair<String,INT> > ip;
	bool old;
	time_t uptime;
};
struct RelayData
{
	RelayData()
	{
	}
	RelayData(LPCSTR id)
	{
		channelId = id;
	}
	bool operator <( const RelayData& relayData) const
	{
		return channelId < relayData.channelId;
	}

	String channelName;
	String channelGenre;
	int channelBitrate;
	String channelType;
	String channelUrl;
	String channelId;
	String channelComment;
	String trackTitle;
	String trackArtist;
	String trackAlbum;
	String channelStatus;
	int relayListener;
	int relayHost;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerChannelData
// チャンネルデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PeerChannelData 
{
	friend class Peercast;
	friend class PeercastChannelDB;
public:
	ChannelData* getChannelData(LPCSTR channelID);
	INT getChannelCount()const;
	INT getChannelHost()const;
	INT getChannelListener()const;
	std::set<ChannelData>& getChannelData();
protected:
	std::set<ChannelData> m_channel;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerRelayData
// リレーデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PeerRelayData
{
	friend class Peercast;
public:
	RelayData* getRelayData(LPCSTR channelID);
	std::set<RelayData>& getRelayData();
protected:
	std::set<RelayData> m_relay;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeerCastData
// 放送設定データ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PeerCastData
{
public:
	PeerCastData();
	void setDataName(LPCSTR value);
	LPCSTR getDataName()const;
	void setChannelName(LPCSTR value);
	LPCSTR getChannelUrl()const;
	void setChannelUrl(LPCSTR value);
	LPCSTR getChannelName()const;
	void setChannelDescription(LPCSTR value);
	LPCSTR getChannelDescription() const;
	void setChannelGenre(LPCSTR value);
	LPCSTR getChannelGenre()const;
	void setChannelContact(LPCSTR value);
	LPCSTR getChannelContact()const;
	void setChannelBitrate(INT value);
	INT getChannelBitrate()const;
	void setChannelType(LPCSTR value);
	LPCSTR getChannelType()const;
	//void convertUTF8();
protected:
	String m_dataName;
	String m_channelUrl;
	String m_channelName;
	String m_channelDescription;
	String m_channelGenre;
	String m_channelContact;
	INT m_channelBitrate;
	String m_channelType;
};
struct PeerYPData
{
	bool operator <(const PeerYPData& data) const
	{
		return id < data.id;
	}
	INT id;
	String name;
	String url;
	INT type;
	time_t uptime;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PeercastChannelDB
// Peercastチャンネルデータ操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class PeercastChannelDB
{
public:
	PeercastChannelDB();
	~PeercastChannelDB();
	time_t getChannelUpdateTime(INT id);
	bool loadChannel(PeerChannelData* peerChannelData,INT id,time_t t=0);
	bool saveChannel(PeerChannelData* peerChannelData,INT id)const;
	void getYPList(std::set<PeerYPData>& datas);
	void getYPList(std::map<INT,PeerYPData>& datas);
	void setYPData(INT id,LPCSTR name,LPCSTR url=NULL,INT type=-1);
	void delYP(INT id) const;
protected:
	SQLITE::SQLite m_sqlite;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Peercast
// Peercast操作用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Peercast
{
public:
	Peercast();
	bool createList(PeerChannelData* channel,LPCSTR ypURL,INT type=0);
	bool createStat(PeerRelayData* peerRelayData);

	void play(LPCSTR appPath,LPCSTR option,LPCSTR server,INT port,LPCSTR channelID,LPCSTR streamType,LPCSTR topIP,INT topPort);
	void play(LPCSTR channelID,LPCSTR streamType,LPCSTR topIP=NULL,INT topPort=0);
	void bump(LPCSTR pID); 
	void stop(LPCSTR pID); 
	void relay(LPCSTR pID); 
	void keep(LPCSTR pID);
	void cast(PeerCastData* castData);

	void setProxy(bool flag);
	void setProxy(LPCSTR name,INT port,LPCSTR id,LPCSTR pass);
	void setServerAdr(LPCSTR value);
	void setServerPort(INT value);
	void setServerPass(LPCSTR value);
	LPCSTR getServerAdr() const;
	INT getServerPort() const;
	LPCSTR getServerPass() const;
protected:
	bool createListXML(PeerChannelData* peerChannelData,LPCSTR ypURL);
	bool createListVP(PeerChannelData* channel,LPCSTR ypURL,INT port);
	bool createListTEXT(PeerChannelData* channel,LPCSTR ypURL);

	static void AtoB64(LPSTR pDest,LPCSTR pSrc,int iSize);
	static String convertString(LPCSTR pstrSrc);
	AFL::XML::Xml* readXmlData();

	String m_proxyName;
	INT m_proxyPort;
	String m_proxyID;
	String m_proxyPass;
	String m_serverAdr;
	INT m_serverPort;
	String m_serverPass;

};
//namespace
};};

//-----------------------------------------------------
#define __INC_AFLPEERCAST
#endif
//-----------------------------------------------------
