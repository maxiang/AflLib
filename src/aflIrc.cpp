#ifdef _WIN32
	#include <windows.h>
	#include <process.h>
#endif
#include <time.h>
#include <stdio.h>
#include "AflIrc.h"
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
using namespace AFL;

namespace AFL{namespace Sock{namespace Irc{

enum
{
	RPL_WELCOME = 1,
	RPL_YOURHOST = 2, 
	RPL_CREATED = 3,
	RPL_MYINFO = 4, 
	RPL_BOUNCE  = 5,
	RPL_USERHOST = 302,
	RPL_AWAY = 301,
	RPL_ISON = 303,
	RPL_UNAWAY = 305,
	RPL_NOWAWAY = 306,
	RPL_WHOISUSER = 311,
	RPL_WHOISSERVER = 312,
	RPL_WHOISOPERATOR = 313,
	RPL_WHOISIDLE = 317,
	RPL_ENDOFWHOIS = 318,
	RPL_WHOISCHANNELS = 319,
	RPL_WHOWASUSER = 314,
	RPL_ENDOFWHOWAS = 369,
	RPL_LISTSTART = 321,
	RPL_LIST = 322,
	RPL_LISTEND = 323,
	RPL_UNIQOPIS = 325,
	RPL_CHANNELMODEIS = 324,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_INVITING = 341,
	RPL_SUMMONING = 342,
	RPL_INVITELIST = 346,
	RPL_ENDOFINVITELIST = 347, 
	RPL_EXCEPTLIST = 348,
	RPL_ENDOFEXCEPTLIST = 349,
	RPL_VERSION = 351,
	RPL_WHOREPLY = 352,
	RPL_ENDOFWHO = 315,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,
	RPL_LINKS = 364,
	RPL_ENDOFLINKS = 345,
	RPL_BANLIST = 367,
	RPL_ENDOFBANLIST = 368,
	RPL_INFO = 371, 
	RPL_ENDOFINFO = 374,
	RPL_MOTDSTART =375 ,
	RPL_MOTD = 372,
	RPL_ENDOFMOTD = 376,
	RPL_YOUREOPER = 381,
	RPL_REHASHING = 382,
	RPL_YOURESERVICE = 383,
	RPL_TIME = 391,
	RPL_USERSSTART = 392,
	RPL_USERS = 393,
	RPL_ENDOFUSERS =394 ,
	RPL_NOUSERS = 395,

	RPL_TRACELINK = 200,
	RPL_TRACECONNECTING = 201,
	RPL_TRACEHANDSHAKE = 202,
	RPL_TRACEUNKNOWN = 203,
	RPL_TRACEOPERATOR = 204,
	RPL_TRACEUSER = 205,
	RPL_TRACESERVER = 206,
	RPL_TRACESERVICE = 207,
	RPL_TRACENEWTYPE = 208,
	RPL_TRACECLASS = 209,
	RPL_TRACERECONNECT = 210,
	RPL_TRACELOG = 261,
	RPL_TRACEEND = 262,
	RPL_STATSLINKINFO = 211,
	RPL_STATSCOMMANDS = 212,
	RPL_ENDOFSTATS = 219,
	RPL_STATSUPTIME= 242,
	RPL_STATSOLINE = 243,
	RPL_UMODEIS = 221,
	RPL_SERVLIST = 234,
	RPL_SERVLISTEND = 235,
	RPL_LUSERCLIENT = 251,
	RPL_LUSEROP = 252,
	RPL_LUSERUNKNOWN = 253,
	RPL_LUSERCHANNELS = 254,
	RPL_LUSERME = 255,
	RPL_ADMINME = 256,
	RPL_ADMINLOC1 = 257,
	RPL_ADMINLOC2 = 258,
	RPL_ADMINEMAIL = 259,
	RPL_TRYAGAIN = 263,

	ERR_NOSUCHNICK = 401, 
	ERR_NOSUCHSERVER = 402,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_TOOMANYCHANNELS = 405,
	ERR_WASNOSUCHNICK = 406,
	ERR_TOOMANYTARGETS = 407,
	ERR_NOSUCHSERVICE = 408,
	ERR_NOORIGIN = 409,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NOTOPLEVEL = 413,
	ERR_WILDTOPLEVEL = 414,
	ERR_BADMASK = 415,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NOMOTD = 422,
	ERR_NOADMININFO = 423,
	ERR_FILEERROR = 424,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	ERR_NICKCOLLISION = 436,
	ERR_UNAVAILRESOURCE = 437,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	ERR_NOLOGIN = 444,
	ERR_SUMMONDISABLED = 445,
	ERR_USERSDISABLED = 446,
	ERR_NOTREGISTERED = 451,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTRED = 462,
	ERR_NOPERMFORHOST = 463,
	ERR_PASSWDMISMATCH = 464,
	ERR_YOUREBANNEDCREEP = 465,
	ERR_YOUWILLBEBANNED = 466,
	ERR_KEYSE = 467,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	ERR_BANNEDFROMCHAN = 474,
	ERR_BADCHANNELKEY = 475,
	ERR_BADCHANMASK =476 ,
	ERR_NOCHANMODES = 477,
	ERR_BANLISTFULL = 478,
	ERR_NOPRIVILEGES = 481,
	ERR_CHANOPRIVSNEEDED = 482,
	ERR_CANTKILLSERVER = 483,
	ERR_RESTRICTED = 484,
	ERR_UNIQOPPRIVSNEEDED = 485,
	ERR_NOOPERHOST = 491,
	ERR_UMODEUNKNOWNFLAG = 501,
	ERR_USERSDONTMATCH = 502
};


AflIRC::AflIRC()
{
	m_pcallMessage = NULL;

	m_strLoginName = "IRC_USER";
	m_strName = "AflIRC";

	m_bActive = false;
}
void  AflIRC::setNickName(LPCSTR pString)
{
	if(isConnect())
		sendNick(pString);
	if(!m_bActive)
		m_strNickName = pString;
}
bool AflIRC::connect(LPCSTR pHostName,INT iPort,LPCSTR pPassword)
{
	m_strHostName = pHostName;
	if(pPassword)
		m_strServerPass = pPassword;
	else
		m_strServerPass = "";
	m_iPort = iPort;
	AflSocket::connect(pHostName,iPort);
	return true;
}
bool AflIRC::quit(LPCSTR pMessage)
{
	std::string strMessage = "QUIT ";
	if(pMessage)
		strMessage += pMessage;
	return sendCommand(strMessage.c_str());
}

bool AflIRC::sendPrivmsg(LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"PRIVMSG %s :%s",pUser,pMessage);
	return sendCommand(cBuff);
}
bool AflIRC::sendNotice(LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"Notice %s :%s",pUser,pMessage);
	return sendCommand(cBuff);
}
bool AflIRC::sendPrivmsgCTCP(LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"PRIVMSG %s :\1%s\1",pUser,pMessage);
	return sendCommand(cBuff);
}
bool AflIRC::sendNoticeCTCP(LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"Notice %s :\1%s\1",pUser,pMessage);
	return sendCommand(cBuff);
}
bool AflIRC::sendNick(LPCSTR pUser)
{
	CHAR cBuff[1024];
	sprintf(cBuff,"NICK %s",pUser);
	return sendCommand(cBuff);
}
bool AflIRC::sendChannelTopic(LPCSTR pChannel,LPCSTR pMessage)
{
	CHAR cBuff[512];
	if(pMessage)
		sprintf(cBuff,"TOPIC %s :%s",pChannel,pMessage);
	else
		sprintf(cBuff,"TOPIC %s",pChannel);
	return sendCommand(cBuff);
}

bool AflIRC::sendChannelMode(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMode)
{
	CHAR cBuff[512];
	sprintf(cBuff,"MODE %s %s %s",pChannel,pMode,pUser);
	return sendCommand(cBuff);
}
bool AflIRC::sendChannelKick(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[512];
	if(!pMessage)
		pMessage = "";
	sprintf(cBuff,"KICK %s %s :",pChannel,pUser,pMessage);
	return sendCommand(cBuff);
}
bool AflIRC::sendPing(LPCSTR pUser)
{
	CHAR cBuff[512];
	sprintf(cBuff,"PING %u",(DWORD)clock());
	return sendPrivmsgCTCP(pUser,cBuff);
}
bool AflIRC::sendTime(LPCSTR pUser)
{
	return sendPrivmsgCTCP(pUser,"TIME");
}
bool AflIRC::sendVersion(LPCSTR pUser)
{
	return sendPrivmsgCTCP(pUser,"VERSION");
}
bool AflIRC::sendClientInfo(LPCSTR pUser)
{
	return sendPrivmsgCTCP(pUser,"CLIENTINFO");
}
bool AflIRC::sendUserInfo(LPCSTR pUser)
{
	return sendPrivmsgCTCP(pUser,"USERINFO");
}
bool AflIRC::sendWhois(LPCSTR pUser)
{
	char cBuff[1024];
	sprintf(cBuff,"WHOIS %s",pUser);
	return sendCommand(cBuff);
}
bool AflIRC::sendCommand(LPCSTR pString)
{
	CHAR cBuff[1024];
	if(strlen(pString) >= 512)
		return false;

	SJIStoJIS(cBuff,pString);
	send(cBuff,strlen(cBuff));
	send("\r\n",2);
	return true;
}

void AflIRC::onSocketConnect(bool bConnect)
{
	onIRCConnect(bConnect);
}

void AflIRC::onSocketClose()
{
	m_bActive = false;
	onIRCClose();
}
void AflIRC::onSocketRecv(INT iSize,LPVOID pvAddr)
{
//	fwrite(psockData->pbyData,psockData->iDataSize,1,stdout);

	//データを[\r]で分解
	INT i,j;
	INT iLength = iSize;
	LPCSTR pString = (LPCSTR)pvAddr;
	 
	for(i=0;i<iLength;i++)
	{
		for(j=0;i+j<iLength && pString[i+j] != '\r';j++);

		m_strRecvBuffer.append(pString+i,j);
		if(pString[i+j] == '\r')
		{
			onIRCRecv(m_strRecvBuffer.c_str());
			m_strRecvBuffer.erase();
			i++;
		}
		i += j;
	}
}

void AflIRC::onIRCConnect(bool bConnect)
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CONNECT);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,bConnect);

	onIRCMessage(&ircMessage);

	//-------------

	if(bConnect)
	{
		CHAR cBuff[512];

		//サーバパスワードの送信
		if(m_strServerPass.c_str()[0])
		{
			sprintf(cBuff,"PASS %s",m_strServerPass.c_str());
			sendCommand(cBuff);
		}
		//ログイン情報の送信
		sprintf(cBuff,"USER %s 0 * :%s",m_strLoginName.c_str(),m_strName.c_str());
		sendCommand(cBuff);
		//ニックネームの送信
		sprintf(cBuff,"NICK %s",m_strNickName.c_str());
		sendCommand(cBuff);
	}
}

void AflIRC::onIRCClose()
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CLOSE);
	ircMessage.setParam(0,(DWORD)this);
	onIRCMessage(&ircMessage);
}

void AflIRC::onIRCErrorNick()
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_ERROR_NICK);
	ircMessage.setParam(0,(DWORD)this);
	onIRCMessage(&ircMessage);
}
void AflIRC::onIRCNoticeCTCP(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
	CHAR cBuff[1024];
	strcpy(cBuff,pMessage+1);
	cBuff[strlen(cBuff) - 1] = 0;

	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_NOTICE_CTCP);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pUser);
	ircMessage.setParam(3,(DWORD)cBuff);
	onIRCMessage(&ircMessage);
}
void AflIRC::onIRCNick(LPCSTR pUser,LPCSTR pNicName)
{
	if(strcmp(m_strNickName.c_str(),pUser)==0)
	{
		m_strNickName = pNicName;
	}
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_NICK);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pUser);
	ircMessage.setParam(2,(DWORD)pNicName);
	onIRCMessage(&ircMessage);

	//-------------
}
void AflIRC::onIRCMotd(LPCSTR pString,bool bNext)
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_MOTD);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pString);
	ircMessage.setParam(2,(DWORD)bNext);
	onIRCMessage(&ircMessage);
	//-------------
}

void AflIRC::onIRCChannelUserPart(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
	AflIrcChannel* pircChannel = getChannel(pChannel);
	if(pircChannel)
		pircChannel->delUser(pUser);
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_USER_PART);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pUser);
	ircMessage.setParam(3,(DWORD)pMessage);
	onIRCMessage(&ircMessage);
	//-------------

	if(pUser == m_strNickName)
	{
		onIRCChannelPart(pChannel);
	}
}
void AflIRC::onIRCChannelUserKick(LPCSTR pChannel,LPCSTR pUserFrom,LPCSTR pUser,LPCSTR pMessage)
{
	AflIrcChannel* pircChannel = getChannel(pChannel);
	if(pircChannel)
		pircChannel->delUser(pUser);
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_USER_KICK);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pUserFrom);
	ircMessage.setParam(3,(DWORD)pUser);
	ircMessage.setParam(4,(DWORD)pMessage);
	onIRCMessage(&ircMessage);
	//-------------
	if(pUser == m_strNickName)
	{
		onIRCChannelPart(pChannel);
	}
}
void AflIRC::onIRCUserQuit(LPCSTR pUser,LPCSTR pMessage)
{
	std::list<AflIrcChannel>::iterator itChannel;
	for(itChannel=m_listChannel.begin();itChannel!=m_listChannel.end();itChannel++)
	{
		std::list<AflIrcChannelUser>* plistChannelUser = (*itChannel).getChannelUser();
		std::list<AflIrcChannelUser>::iterator itChannelUser;
		for(itChannelUser=plistChannelUser->begin();itChannelUser!=plistChannelUser->end();itChannelUser++)
		{
			if(strcmp((*itChannelUser).m_strUserName.c_str(),pUser)==0)
			{
				onIRCChannelUserPart((*itChannel).getChannelName(),pUser,pMessage);
				break;
			}
		}
	}
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_USER_QUIT);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pUser);
	ircMessage.setParam(2,(DWORD)pMessage);
	onIRCMessage(&ircMessage);
	//-------------
}
void AflIRC::onIRCChannelTopic(LPCSTR pChannel,LPCSTR pMessage)
{
	AflIrcChannel* pircChannel = getChannel(pChannel);
	if(pircChannel)
		pircChannel->setTopic(pMessage);

	AflIRCMessage ircMessage(AFLIRC_CHANNEL_TOPIC);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pMessage);
	onIRCMessage(&ircMessage);
}

void AflIRC::onIRCChannelMode(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMode,LPCSTR pFromUser)
{
	//メッセージのルーティング
	int i;
	if(pMode)
	{
		CHAR cMode = pMode[0];
		CHAR cUserSend[512];
		CHAR cModeSend[3];
		LPCSTR pUserString = pUser;
		cModeSend[0] = cMode;
		cModeSend[3] = 0;	//送信モード用null文字
		AflIrcChannelUser* pChannelUser;

		AflIrcChannel* pircChannel = getChannel(pChannel);
		if(!pircChannel)
			pircChannel = addChannel(pChannel);
		for(i=0;pMode[i+1];i++)
		{

			//モード設定
			cModeSend[1] = pMode[i+1];

			switch(cModeSend[1])
			{
			case 'o':
				sscanf(pUserString,"%s",cUserSend);			//ユーザデータの抽出
				pUserString = strchr(pUserString,' ')+1;	//次のユーザ名の抽出

				pChannelUser = pircChannel->getChannelUser(cUserSend);
				if(cMode == '+')
					pChannelUser->m_boolOperator = true;
				else if(cMode == '-')
					pChannelUser->m_boolOperator = false;
				break;
			default:
				if(cMode == '+')
					pircChannel->setMode(cModeSend[1],true);
				else
					pircChannel->setMode(cModeSend[1],false);
				continue;
			}	
			AflIRCMessage ircMessage(AFLIRC_CHANNEL_MODE);
			ircMessage.setParam(0,(DWORD)this);
			ircMessage.setParam(1,(DWORD)pChannel);
			ircMessage.setParam(2,(DWORD)cUserSend);
			ircMessage.setParam(3,(DWORD)cModeSend);
			ircMessage.setParam(4,(DWORD)pFromUser);
			onIRCMessage(&ircMessage);
		}
	}
	//-------------
}
void AflIRC::onIRCChannelUserJoin(LPCSTR pChannel,LPCSTR pUser)
{
	AflIrcChannel* pircChannel = getChannel(pChannel);
	if(!pircChannel)
		pircChannel = addChannel(pChannel);
	pircChannel->addUser(pUser);
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_USER_JOIN);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pUser);
	onIRCMessage(&ircMessage);
	//-------------
}
void AflIRC::onIRCChannelUser(LPCSTR pChannel,LPCSTR pUser)
{
	AflIrcChannel* pircChannel = getChannel(pChannel);
	if(!pircChannel)
		pircChannel = addChannel(pChannel);
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_USER);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
		
	INT i,j;
	CHAR cBuff[1024];
	for(j=0,i=0;;i++,j++)
	{
		if(pUser[i] == ' ' || !pUser[i])
		{
			if(j)
			{
				cBuff[j] = 0;
				pircChannel->addUser(cBuff);
				
				ircMessage.setParam(2,(DWORD)cBuff);
				onIRCMessage(&ircMessage);
			}
			j=-1;
			if(!pUser[i])
				break;
		}
		else
			cBuff[j] = pUser[i];
	}		


}
void AflIRC::onIRCChannelJoin(LPCSTR pChannel)
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_JOIN);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	onIRCMessage(&ircMessage);
}
void AflIRC::onIRCChannelPart(LPCSTR pChannel)
{
	delChannel(pChannel);
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_CHANNEL_PART);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	onIRCMessage(&ircMessage);
}

void AflIRC::onIRCChannelUserEnd(LPCSTR pChannel)
{
	onIRCChannelJoin(pChannel);	
	//-------------
}
void AflIRC::onIRCPrivmsg(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_PRIVMSG);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pChannel);
	ircMessage.setParam(2,(DWORD)pUser);
	ircMessage.setParam(3,(DWORD)pMessage);
	onIRCMessage(&ircMessage);
	//-------------
}
void AflIRC::onIRCWhois(AflIrcWhois* pWhois)
{
	//メッセージのルーティング
	AflIRCMessage ircMessage(AFLIRC_WHOIS);
	ircMessage.setParam(0,(DWORD)this);
	ircMessage.setParam(1,(DWORD)pWhois);
	onIRCMessage(&ircMessage);
	//-------------
}

void AflIRC::onIRCNotice(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
}
void AflIRC::onIRCPrivmsgCTCP(LPCSTR pChannel,LPCSTR pUser,LPCSTR pMessage)
{
	enum{IRC_CTCP_PING,IRC_CTCP_VERSION,IRC_CTCP_CLIENTINFO,IRC_CTCP_USERINFO,IRC_CTCP_TIME,IRC_DCC};
	static PCSTR pCName[] =
	{
		"PING",
		"VERSION",
		"CLIENTINFO",
		"USERINFO",
		"TIME",
		"DCC",
		NULL
	};
	CHAR cBuff[1024];
	CHAR cBuff2[1024];
	INT i;
	for(i=0;pCName[i] && strstr(pMessage+1,pCName[i]) != pMessage+1;i++);
	switch(i)
	{
	case IRC_CTCP_PING:
		sscanf(pMessage+1,"%*s %s",cBuff2);
		sprintf(cBuff,"\1PING %s\1",cBuff2);
		sendNotice(pUser,cBuff);
		break;
	case IRC_CTCP_VERSION:
		sprintf(cBuff,"\1VERSION %s\1",m_strVersion.c_str());
		sendNotice(pUser,cBuff);
		break;
	case IRC_CTCP_CLIENTINFO:
		sprintf(cBuff,"\1CLIENTINFO %s\1",m_strClientInfo.c_str());
		sendNotice(pUser,cBuff);
		break;
	case IRC_CTCP_USERINFO:
		sprintf(cBuff,"\1USERINFO %s\1",m_strUserInfo.c_str());
		sendNotice(pUser,cBuff);
		break;
	case IRC_CTCP_TIME:
		{
			time_t timeNow;
			time(&timeNow);
			ctime(&timeNow); 
			sprintf(cBuff,"\1TIME %s",ctime(&timeNow));
			cBuff[strlen(cBuff) - 1] = -1;
			sendNotice(pUser,cBuff);
		}
		break;
	case IRC_DCC:
		break;
	
	}
}


void AflIRC::onIRCPing(LPCSTR pCommand,LPCSTR pParams)
{
	std::string strPing;
	strPing = "PONG ";
	strPing += pCommand;
	sendCommand(strPing.c_str());
}
void AflIRC::onIRCMessage(LPCSTR pCommand,LPCSTR pParams)
{
	CHAR cServer[512];
	CHAR cCommand[512];
	CHAR cOption[10][512] = {{0},{0},{0},{0},{0}};
	LPCSTR ppOption[] =
	{
		cOption[0],cOption[1],cOption[2],cOption[3],cOption[4],
		cOption[5],cOption[6],cOption[7],cOption[8],cOption[9]
	};

	sscanf(pCommand,"%s %s %s %s %s %s %s",cServer,cCommand,cOption[0],cOption[1],cOption[2],cOption[3],cOption[4]);
	if(isdigit(cCommand[0]))
		onIRCServerMessage(atoi(cCommand),cServer,ppOption,pParams);
	else
		onIRCCommandMessage(cCommand,cServer,ppOption,pParams);
}
void AflIRC::onIRCCommandMessage(LPCSTR pCommand,LPCSTR pServer,LPCSTR* ppOption,LPCSTR pParams)
{
	enum
	{
		IRC_JOIN,IRC_PART,IRC_KICK,IRC_QUIT,IRC_MODE,IRC_TOPIC,IRC_PRIVMSG,IRC_NOTICE,IRC_NICK
	};
	static LPCSTR ppCommand[] = {"JOIN","PART","KICK","QUIT","MODE","TOPIC","PRIVMSG","NOTICE","NICK",NULL};
	INT i;
	
	std::string strUserName;
	strUserName.reserve(1024);
	for(i=0;pServer[i] && pServer[i] != '!';i++)
		strUserName += pServer[i];
	
	for(i=0;ppCommand[i] && strcmp(ppCommand[i],pCommand)!=0;i++);
	
	switch(i)
	{
	case IRC_JOIN:
		onIRCChannelUserJoin(pParams,strUserName.c_str());
		break;
	case IRC_PART:
		onIRCChannelUserPart(ppOption[0],strUserName.c_str(),pParams);
		break;
	case IRC_KICK:
		onIRCChannelUserKick(ppOption[0],strUserName.c_str(),ppOption[1],pParams);
		break;
	case IRC_QUIT:
		onIRCUserQuit(strUserName.c_str(),pParams);
		break;
	case IRC_TOPIC:
		onIRCChannelTopic(ppOption[0],pParams);
		break;
	case IRC_MODE:
		onIRCChannelMode(ppOption[0],ppOption[2],ppOption[1],strUserName.c_str());
		break;
	case IRC_PRIVMSG:
		if(pParams[0] != 1)
			onIRCPrivmsg(ppOption[0],strUserName.c_str(),pParams);
		else
			onIRCPrivmsgCTCP(ppOption[0],strUserName.c_str(),pParams);
		break;
	case IRC_NOTICE:
		if(pParams[0] != 1)
			onIRCNotice(ppOption[0],strUserName.c_str(),pParams);
		else
			onIRCNoticeCTCP(ppOption[0],strUserName.c_str(),pParams);
		break;
	case IRC_NICK:
			onIRCNick(strUserName.c_str(),pParams);
		break;
	default:
		break;
	}

}
void AflIRC::onIRCServerMessage(INT iIndex,LPCSTR pServer,LPCSTR* ppOption,LPCSTR pParams)
{
	switch(iIndex)
	{
	case RPL_WELCOME:
		m_strServerInfo = pParams;
		break;
	case RPL_YOURHOST:
	case RPL_CREATED:
	case RPL_MYINFO: 
	case RPL_LUSERCLIENT:
	case RPL_LUSEROP:
	case RPL_LUSERUNKNOWN:
	case RPL_LUSERCHANNELS:
	case RPL_LUSERME:
		m_strServerInfo += pParams;
		m_strServerInfo += '\n';
		break;
	case RPL_MOTDSTART:
	case  RPL_MOTD:
		onIRCMotd(pParams + 2,true);
		break;
	case ERR_NOMOTD:
		onIRCMotd(pParams,false);
		break;
	case RPL_ENDOFMOTD:
		onIRCMotd(pParams,false);
		break;
	case RPL_TOPIC:
		onIRCChannelTopic(ppOption[1],pParams);
		break;
	case RPL_NOTOPIC:
		onIRCChannelTopic(ppOption[1],"");
		break;
	case ERR_NICKNAMEINUSE:
	case ERR_NONICKNAMEGIVEN:
		onIRCErrorNick();
		break;
	case RPL_NAMREPLY:
		onIRCChannelUser(ppOption[2],pParams);
		break;
	case RPL_ENDOFNAMES: 
		onIRCChannelUserEnd(ppOption[1]);
		break;
	case RPL_WHOISUSER: 
		//ユーザ情報の初期化
		m_ircWhois.m_listChannel.clear();
		m_ircWhois.m_strUserName = ppOption[1];
		m_ircWhois.m_strLoginName = ppOption[2];
		m_ircWhois.m_strAddress = ppOption[3];
		m_ircWhois.m_strRealName = pParams;
		break;
	case RPL_WHOISCHANNELS:
		m_ircWhois.m_listChannel.push_back(pParams);
		break;
	case RPL_WHOISSERVER: 
		m_ircWhois.m_strServerName = ppOption[2];
		m_ircWhois.m_strServerInfo = pParams;
		break;
//	case RPL_WHOISOPERATOR: 
	case RPL_WHOISIDLE: 
		m_ircWhois.m_iIdel = atoi(ppOption[2]);
		break;
	case RPL_ENDOFWHOIS: 
		onIRCWhois(&m_ircWhois);
		break;
	default:
		break;
	}
}

void AflIRC::onIRCCommand(LPCSTR pPrefix,LPCSTR pCommand,LPCSTR pParams)
{
	enum
	{
		IRC_SERVER_MESSAGE,IRC_PING
	};
	static LPCSTR ppPrefix[] = {"","PING ",NULL};
	INT i;
	for(i=0;ppPrefix[i] && strcmp(pPrefix,ppPrefix[i]);i++);
	switch(i)
	{
	case IRC_SERVER_MESSAGE:
		onIRCMessage(pCommand,pParams);
		break;
	case IRC_PING:
		onIRCPing(pCommand,pParams);
		break;
	default:
		break;
	}
}
void AflIRC::onIRCRecv(LPCSTR pString)
{
	INT i;
	LPCSTR pWork;
	CHAR cBuff[2048];

	std::string strPrefix;
	std::string strCommand;
	std::string strParams;

	JIStoSJIS(cBuff,pString);

	pWork = cBuff;
	for(i=0;pWork[i] != ':';i++);
	strPrefix.assign(pWork,i);
	pWork += i+1;

	for(i=0;pWork[i] && pWork[i] != ':';i++);
	strCommand.assign(pWork,i);
	if(pWork[i])
		pWork += i+1;
	
	strParams.assign(pWork);

	onIRCCommand(strPrefix.c_str(),strCommand.c_str(),strParams.c_str());
}

LPCSTR AflIRC::JIStoSJIS(PSTR pDest,PCSTR pSrc)
{
	int i,j;
	BOOL bJis = FALSE;
	for(j=i=0;pSrc[i];)
	{
		if(pSrc[i] == '\x1b')
		{
			if(pSrc[i+1] == '$')
				bJis = TRUE;
			else if(pSrc[i+1] == '(')
				bJis = FALSE;
			i += 3;
			continue;
		}
		if(bJis)
		{
			if (pSrc[i] % 2)
			{
				pDest[j] = ((pSrc[i] + 1) / 2) + 0x70;
				pDest[j+1] = pSrc[i+1] + 0x1f;
			}
			else
			{
				pDest[j] = (pSrc[i] / 2) + 0x70;
				pDest[j+1] = pSrc[i+1] + 0x7d;
			}
		    if((UCHAR)pDest[j] >= 0xa0)
				pDest[j] = pDest[j] + 0x40;
			if((UCHAR)pDest[j+1] >= 0x7f)
				pDest[j+1] = pDest[j+1] + 1;
			j += 2;
			i += 2;
		}
		else
		{
			pDest[j] = pSrc[i];
			j++;
			i++;
		}
	}
	pDest[j] = '\0';
	return pDest;
}
LPCSTR AflIRC::SJIStoJIS(PSTR pDest,PCSTR pSrc)
{
	BOOL bFlag = FALSE;
	int nDestP = 0;
	int nSrcP = 0;
	while(pSrc[nSrcP])
	{
		if(isSJIS(pSrc[nSrcP]))
		{
			if(!bFlag)
			{
				pDest[nDestP] = 0x1b;
				pDest[nDestP+1] = 0x24;
				pDest[nDestP+2] = 0x42;
				nDestP += 3;
				bFlag = TRUE;
			}
			UCHAR Dt1,Dt2;
			Dt1 = (UCHAR)pSrc[nSrcP];
			Dt2 = (UCHAR)pSrc[nSrcP+1];
			nSrcP+=2;
			StoJ((PSTR)&Dt1,(PSTR)&Dt2);
			pDest[nDestP] = Dt1;
			pDest[nDestP+1] = Dt2;
			nDestP+=2;
		}
		else
		{
			if(bFlag)
			{
				pDest[nDestP] = 0x1b;
				pDest[nDestP+1] = 0x28;
				pDest[nDestP+2] = 0x42;
				nDestP += 3;
				bFlag = FALSE;
			}
			pDest[nDestP] = pSrc[nSrcP];
			nDestP++;
			nSrcP++;
		}
	}
	if(bFlag)
	{
		pDest[nDestP] = 0x1b;
		pDest[nDestP+1] = 0x28;
		pDest[nDestP+2] = 0x4a;
		nDestP += 3;
	}
	pDest[nDestP] = 0;
	return pDest;
}
void AflIRC::StoJ(PSTR ch1, PSTR ch2)
{
	int high=(UCHAR)*ch1;
	int low=(UCHAR)*ch2;

   if(high <= 0x9f)
	   high -= 0x71;
   else
	   high -= 0xb1;
   high=high*2+1;
   if(low > 0x7f)
	   --low;
   if(low >= 0x9e)
   {
       low -= 0x7d;
       ++high;
   }
   else
       low -= 0x1f;
   *ch1 = (unsigned char)high;
   *ch2 = (unsigned char)low;
}
void AflIRC::joinChannel(LPCSTR pChannelName,LPCSTR pChannelPassword)
{
	std::string strCommand = "JOIN ";
	strCommand += pChannelName;
	if(pChannelPassword && *pChannelPassword)
	{
		strCommand += ' ';
		strCommand += pChannelPassword;
	}
	sendCommand(strCommand.c_str());
}

void AflIRC::partChannel(LPCSTR pChannelName,LPCSTR pMessage)
{
	std::string strCommand = "PART ";
	strCommand += pChannelName;
	strCommand += ' ';
	if(pMessage)
	{
		strCommand += pMessage;
	}
	sendCommand(strCommand.c_str());
}
void AflIRC::kickUser(LPCSTR pChannelName,LPCSTR pUserName,LPCSTR pMessage)
{
	std::string strCommand = "KICK ";
	strCommand += pChannelName;
	strCommand += ' ';
	strCommand += pUserName;
	strCommand += ' ';
	if(pMessage)
	{
		strCommand += pMessage;
	}
	sendCommand(strCommand.c_str());
}
void AflIRC::modeUser(LPCSTR pChannelName,LPCSTR pMode,LPCSTR pUserName)
{
	std::string strCommand = "MODE ";
	strCommand += pChannelName;
	strCommand += ' ';
	strCommand += pMode;
	strCommand += ' ';
	strCommand += pUserName;

	sendCommand(strCommand.c_str());
}
void AflIRC::onIRCMessage(AflIRCMessage* pMessage)
{
	if(m_pcallMessage)
		m_pcallMessage->callProcess(pMessage);
}


};};};
