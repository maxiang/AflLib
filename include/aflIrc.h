#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLIRC

#include "AflSock.h"
#include <list>


namespace AFL{namespace Sock{namespace Irc{

enum 
{
	AFLIRC_CONNECT,
	AFLIRC_CLOSE,
	AFLIRC_WHOIS,
	AFLIRC_MOTD,
	AFLIRC_MOTD_END,
	AFLIRC_CHANNEL_JOIN,
	AFLIRC_CHANNEL_PART,
	AFLIRC_CHANNEL_USER_JOIN,
	AFLIRC_CHANNEL_USER_PART,
	AFLIRC_CHANNEL_USER_KICK,
	AFLIRC_CHANNEL_MODE,
	AFLIRC_CHANNEL_TOPIC,
	AFLIRC_USER_QUIT,
	AFLIRC_CHANNEL_USER,
	AFLIRC_PRIVMSG,
	AFLIRC_NOTICE,
	AFLIRC_PRIVMSG_CTCP,
	AFLIRC_NOTICE_CTCP,
	AFLIRC_NICK,
	AFLIRC_ERROR_NICK
};

struct AflIrcWhois
{
	std::string m_strUserName;
	std::string m_strLoginName;
	std::string m_strRealName;
	std::string m_strServerName;
	std::string m_strServerInfo;
	std::string m_strAddress;
	int m_iIdel;
	std::list<std::string> m_listChannel;
};
struct AflIrcChannelUser
{
	std::string m_strUserName;
	bool m_boolOperator;
};
class AflIrcChannel
{
public:
	AflIrcChannel(LPCSTR pstrChannelName)
	{
		m_dwMode = 0;
		m_strChannelName = pstrChannelName;
	}
	LPCSTR getChannelName()const{return m_strChannelName.c_str();}
	void addUser(LPCSTR pstrUser)
	{
		AflIrcChannelUser* pUser = getChannelUser(pstrUser);
		if(!pUser)
		{
			m_listChannelUser.push_back(AflIrcChannelUser());
			pUser = &m_listChannelUser.back();
		}
		if(pstrUser[0] == '@')
		{
			pUser->m_strUserName = pstrUser + 1;
			pUser->m_boolOperator = true;
		}
		else
		{
			pUser->m_strUserName = pstrUser;
			pUser->m_boolOperator = false;
		}
	}
	void delUser(LPCSTR pstrUser)
	{
		LPCSTR pString;
		if(pstrUser[0] == '@')
			pString  = pstrUser+1;
		else
			pString  = pstrUser;
		std::list<AflIrcChannelUser>::iterator it;
		for(it=m_listChannelUser.begin();it!=m_listChannelUser.end();it++)
		{
			if(strcmp((*it).m_strUserName.c_str(),pString)==0)
			{
				m_listChannelUser.erase(it);
				break;
			}
		}
	}
	AflIrcChannelUser* getChannelUser(LPCSTR pstrUser)
	{
		LPCSTR pString;
		if(pstrUser[0] == '@')
			pString  = pstrUser+1;
		else
			pString  = pstrUser;
		std::list<AflIrcChannelUser>::iterator it;
		for(it=m_listChannelUser.begin();it!=m_listChannelUser.end();it++)
		{
			if(strcmp((*it).m_strUserName.c_str(),pString)==0)
				return &(*it);
		}
		return NULL;
	}
	bool isChannelOperator(LPCSTR pstrUser)
	{
		AflIrcChannelUser* pUser = getChannelUser(pstrUser);
		if(pUser)
		{
			return pUser->m_boolOperator;
		}
		return false;
	}
	std::list<AflIrcChannelUser>* getChannelUser()
	{
		return &m_listChannelUser;
	}
	void setMode(CHAR cMode,bool bFlag)
	{
		int iIndex = getModeIndex(cMode);
		if(iIndex < 0)
			return;
		if(bFlag)
		{
			m_dwMode |= (1<<iIndex);
		}
		else
		{
			m_dwMode |= (1<<iIndex);
			m_dwMode ^= (1<<iIndex);
		}

	}
	bool isMode(CHAR cMode)
	{
		int iIndex = getModeIndex(cMode);
		if(iIndex < 0)
			return false;
		return (m_dwMode & (1<<iIndex))!=0;
	}
	LPCSTR getTopic()const{return m_strTopic.c_str();}
	void setTopic(LPCSTR pTopic){m_strTopic = pTopic;}
protected:
	int getModeIndex(CHAR cMode)
	{
		static LPCSTR strMode = "aimnpsrtkl";
		int i;
		for(i=0;strMode[i];i++)
		{
			if(strMode[i] == cMode)
				return i;
		}
		return -1;
	}
	std::string m_strChannelName;
	std::string m_strTopic;
	std::list<AflIrcChannelUser> m_listChannelUser;
	DWORD m_dwMode;
};
class AflIRCMessage
{
public:
	AflIRCMessage()
	{
	}
	AflIRCMessage(DWORD dwMessage)
	{
		m_dwMessage = dwMessage;
	}
	DWORD getMessage()const{return m_dwMessage;}
	DWORD getParam(INT iIndex)const{return m_dwParams[iIndex];}
	void setParam(INT iIndex,DWORD dwData){m_dwParams[iIndex] = dwData;}
protected:
	DWORD m_dwMessage;
	DWORD m_dwParams[16];
};
class AflIRC : public AFL::Sock::AflSocket
{
public:
	AflIRC();
	virtual ~AflIRC(){}
	bool isActive()const{return m_bActive;}
	bool connect(LPCSTR pHostName,INT iPort=6667,LPCSTR pPassword=NULL);
	bool quit(LPCSTR pMessage=NULL);
	bool sendCommand(LPCSTR pString);
	bool sendPrivmsg(LPCSTR pUser,LPCSTR pMessage);
	bool sendNotice(LPCSTR pUser,LPCSTR pMessage);
	bool sendPrivmsgCTCP(LPCSTR pUser,LPCSTR pMessage);
	bool sendNoticeCTCP(LPCSTR pUser,LPCSTR pMessage);
	bool sendPing(LPCSTR pUser);
	bool sendTime(LPCSTR pUser);
	bool sendVersion(LPCSTR pUser);
	bool sendClientInfo(LPCSTR pUser);
	bool sendUserInfo(LPCSTR pUser);
	bool sendWhois(LPCSTR pUser);
	bool sendChannelTopic(LPCSTR pChannel,LPCSTR pMessage=NULL);
	bool sendChannelMode(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMode);
	bool sendChannelKick(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage=NULL);
	bool sendNick(LPCSTR pUser);

	virtual void onSocketConnect(bool bConnect);
	virtual void onSocketClose();
	virtual void onSocketRecv(INT iSize,LPVOID pvAddr);

	virtual void onIRCConnect(bool bConnect);
	virtual void onIRCClose();
	virtual void onIRCErrorNick();
	virtual void onIRCMotd(LPCSTR pString,bool bNext);
	virtual void onIRCChannelJoin(LPCSTR pChannel);
	virtual void onIRCChannelPart(LPCSTR pChannel);
	virtual void onIRCChannelUserJoin(LPCSTR pChannel,LPCSTR pUser);
	virtual void onIRCChannelUserPart(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCUserQuit(LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCChannelUserKick(LPCSTR pChannel,LPCSTR pFromUser,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCChannelMode(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMode,LPCSTR pFromUser);
	virtual void onIRCChannelTopic(LPCSTR pChannel,LPCSTR pMessage);
	virtual void onIRCChannelUser(LPCSTR pChannel,LPCSTR pUser);
	virtual void onIRCChannelUserEnd(LPCSTR pChannel);
	virtual void onIRCPrivmsg(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCNotice(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCPrivmsgCTCP(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCNoticeCTCP(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage);
	virtual void onIRCNick(LPCSTR pUser,LPCSTR pNicName);
	virtual void onIRCPing(LPCSTR pCommand,LPCSTR pParams);
	virtual void onIRCMessage(LPCSTR pCommand,LPCSTR pParams);
	virtual void onIRCCommandMessage(LPCSTR pCommand,LPCSTR pServer,LPCSTR* ppOption,LPCSTR pParams);
	virtual void onIRCServerMessage(INT iIndex,LPCSTR pServer,LPCSTR* ppOption,LPCSTR pParams);
	virtual void onIRCCommand(LPCSTR pPrefix,LPCSTR pCommand,LPCSTR pParams);
	virtual void onIRCRecv(LPCSTR pString);
	virtual void onIRCWhois(AflIrcWhois* pWhois);

	virtual void onIRCMessage(AflIRCMessage* pMessage);

	
	LPCSTR JIStoSJIS(PSTR pDest,PCSTR pSrc);
	LPCSTR SJIStoJIS(PSTR pDest,PCSTR pSrc);
	void StoJ(PSTR ch1, PSTR ch2);

	void setCallMessage(AFL::AflClassCallBack* pCallback){m_pcallMessage = pCallback;}

	void setNickName(LPCSTR pString);

	LPCSTR getNickName()const{return m_strNickName.c_str();}
	void joinChannel(LPCSTR pChannelName,LPCSTR pChannelPassword=NULL);
	void partChannel(LPCSTR pChannelName,LPCSTR pMessage=NULL);
	void kickUser(LPCSTR pChannelName,LPCSTR pUserName,LPCSTR pMessage=NULL);
	void modeUser(LPCSTR pChannelName,LPCSTR pMode,LPCSTR pUserName);

	std::list<AflIrcChannel>* getChannelList()
	{
		return &m_listChannel;
	}
	AflIrcChannel* getChannel(LPCSTR pstrChannel)
	{
		std::list<AflIrcChannel>::iterator it;
		for(it=m_listChannel.begin();it!=m_listChannel.end();it++)
		{
			if(strcmp((*it).getChannelName(),pstrChannel)==0)
			{
				return &(*it);
			}
		}
		return NULL;
	}
	void delChannel(LPCSTR pstrChannel)
	{
		std::list<AflIrcChannel>::iterator it;
		for(it=m_listChannel.begin();it!=m_listChannel.end();it++)
		{
			if(strcmp((*it).getChannelName(),pstrChannel)==0)
			{
				m_listChannel.erase(it);
				break;
			}
		}
	}
protected:
	bool isChannel(LPCSTR pstrChannel)
	{
		std::list<AflIrcChannel>::iterator it;
		for(it=m_listChannel.begin();it!=m_listChannel.end();it++)
		{
			if(strcmp((*it).getChannelName(),pstrChannel)==0)
			{
				return true;
			}
		}
		return false;
	}
	AflIrcChannel* addChannel(LPCSTR pstrChannel)
	{
		m_listChannel.push_back(AflIrcChannel(pstrChannel));
		return &m_listChannel.back();
	}


	std::string m_strServerPass;
	std::string m_strLoginName;
	std::string m_strName;
	std::string m_strUserInfo;
	std::string m_strVersion;
	std::string m_strClientInfo;
	std::string m_strHostName;
	std::string m_strNickName;
	INT m_iPort;
	bool m_bActive;


	std::string m_strServerInfo;
	std::string m_strRecvBuffer;

	AFL::AflClassCallBack* m_pcallMessage;

	std::list<AflIrcChannel> m_listChannel;
	AflIrcWhois m_ircWhois;

};

};};};

#define __INC_AFLIRC
#endif
