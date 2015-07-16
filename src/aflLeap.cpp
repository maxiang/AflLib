#include "aflLeap.h"

JsonHash* JsonBase::getHash(LPCSTR name)
{
	if(getType() != typeid(JsonObject))
		return NULL;
	JsonObject* obj = (JsonObject*)this;
	std::list<JsonHash>::iterator it;
	for(it=obj->getData().begin();it!=obj->getData().end();++it)
	{
		if(strcmp(it->getName(),name) == 0)
			return &*it;
	}
	return NULL;
}

INT JsonBase::getInt(LPCSTR name)
{
	JsonHash* hash = getHash(name);
	if(!hash)
		return 0;
	return ((JsonInt*)hash->getData())->getData();
}
FLOAT JsonBase::getFloat(LPCSTR name)
{
	JsonHash* hash = getHash(name);
	if(!hash)
		return 0;
	return ((JsonFloat*)hash->getData())->getData();
}
LPCSTR JsonBase::getString(LPCSTR name)
{
	JsonHash* hash = getHash(name);
	if(!hash)
		return 0;
	return ((JsonString*)hash->getData())->getData();
}



JsonHash::JsonHash(LPCSTR name,JsonBase* data) : JsonData(SP<JsonBase>(data))
{
	m_name = name;
}
const type_info& JsonHash::getType()
{
	return typeid(*this);
}
	
const type_info& JsonHash::getDataType()
{
	return m_data->getDataType();
}
LPCSTR JsonHash::getName()
{
	return m_name;
}
JsonBase* JsonHash::getData()
{
	return m_data.get();
}



	
void JsonReader::output(JsonBase* sd,int level)
{
	int i;
	for(i=0;i<level;i++)
		printf(" ");
	if(sd->getType() == typeid(JsonHash))
	{
		printf("%s -> ",((JsonHash*)sd)->getName());
		output(((JsonHash*)sd)->getData(),level);
		puts("");
	}
	else if(sd->getType() == typeid(JsonObject))
	{
		JsonObject* object = (JsonObject*)sd;
		printf("{");
		std::list<JsonHash>::iterator it;
		std::list<JsonHash>& list = object->getData();
		for(it = list.begin();it != list.end();++it)
		{			
			output(&*it,level+1);
		}
		for(i=0;i<level;i++)
			printf(" ");
		printf("}\n");
	}else if(sd->getDataType() == typeid(String))
	{
		printf("%s,",((JsonString*)sd)->getData().c_str());
	}	
	else if(sd->getDataType() == typeid(FLOAT))
	{
		printf("%f,",((JsonFloat*)sd)->getData());
	}	
	else if(sd->getDataType() == typeid(INT))
	{
		printf("%d,",((JsonInt*)sd)->getData());
	}	
	else if(sd->getDataType() == typeid(bool))
	{
		printf("%d,",((JsonData<bool>*)sd)->getData());
	}	
	else if(sd->getDataType() == typeid(JsonHash))
	{
		output(&((JsonData<JsonHash>*)sd)->getData(),level+1);
	}	
	else if(sd->getDataType() == typeid(std::list<SP<JsonBase> >))
	{
		std::list<SP<JsonBase> >::iterator it;
		std::list<SP<JsonBase> > list = 
			((JsonData<std::list<SP<JsonBase> > >*)sd)->getData();
		for(i=0;i<level;i++)
			printf(" ");
		printf("[");
		for(it = list.begin();it != list.end();++it)
		{			
			output(it->get(),level);

		}
		for(i=0;i<level;i++)
			printf(" ");
		printf("] ");

	}
}

JsonBase*JsonReader:: parse(LPCSTR data)
{
	LPCSTR work = data;
	return getData(work);
}
bool JsonReader::getNext(LPCSTR& src,String& dest)
{
	bool flag = true;
	String s;
	char data;
	dest.clear();
	while(flag && (data = *(src++)) != 0)
	{
		switch(data)
		{
		case '"':
			dest = "\"";
			while((data =  *(src++)) != 0 && data != '"')
			{
				dest += data;
			}
			return true;
		case '[':
		case ']':
		case '{':
		case '}':
		case ':':
		case ',':
			dest += data;
			return true;
		case ' ':
			break;
		default:
			flag = false;
			break;
		}
	}
	do
	{
		if(data == '+' || data == '-' || data == '.' || data >= '0' && data <= '9' || data >= 'a' && data <= 'z')
			dest += data;
		else
		{
			src--;
			break;
		}
	}
	while((data = *(src++)) != 0);

	if(dest.length())
		return true;
	return false;
}
JsonArray* JsonReader::getArray(LPCSTR& src)
{
	std::list<SP<JsonBase> > list;
	String data;
	while(getNext(src,data))
	{		
		if(data.at(0) == '[')
			list.push_back(getArray(src));
		else if(data.at(0) == '{')
			list.push_back(getBlock(src));
		else if(data.at(0) == ']')
			break;
		else if(data.at(0) == ',')
			continue;
		else
		{
			//puts(data);
			list.push_back(getValue(data));
		}
	}
	return new JsonArray(list);
}
JsonObject* JsonReader::getBlock(LPCSTR& src)
{
	std::list<JsonHash> list;
	JsonHash* child;
	while((child = (JsonHash*)getData(src))!=NULL)
	{
		//puts(child->getName());
		list.push_back(*child);
		delete child;
	}
	return new JsonObject(list);
}
JsonBase* JsonReader::getData(LPCSTR& src)
{
	String name;
	String data;
	if(!getNext(src,data))
		return NULL;
	if(data.at(0) == ',')
		if(!getNext(src,data))
			return NULL;
	if(data.at(0) == '"')
	{
		//データが文字列
		name = data.c_str()+1;
		if(getNext(src,data))
		{
			//ラベルチェック
			if(data.at(0) != ':')
				return NULL;
		}
		getNext(src,data);
	}
	if(data.at(0) == '{')
	{
		//ブロック終端まで読み込む
		if(name.length())
			return new JsonHash(name,getBlock(src));
		else
			return getBlock(src);

	}else if(data.at(0) == '}')
		return NULL;
	else if(data.at(0) == '[')
	{
		//配列終端まで読み込む
		if(name.length())
			return new JsonHash(name,getArray(src));
		else
			return getArray(src);
	}
	else
	{
		if(name.length())
			return new JsonHash(name,getValue(data.c_str()));
		else
			return getValue(data.c_str());
	}
	return NULL;
}
JsonBase* JsonReader::getValue(LPCSTR value)
{
	if(value[0] == '"')
		return new JsonString(String(value+1));
	else if(strcmp(value,"true")==0)
		return new JsonBool(true);
	else if(strcmp(value,"false")==0)
		return new JsonBool(false);
	else if(strchr(value,'.'))
		return new JsonFloat((FLOAT)atof(value));
	return new JsonInt(atoi(value));
}


LeapObject::LeapObject(INT id,INT pid,NVector3 pos,NVector3 dir)
{
	m_id = id;
	m_pid = pid;
	m_position = pos;
	m_direction = dir;
}
INT LeapObject::getID()
{
	return m_id;
}
INT LeapObject::getPID()
{
	return m_pid;
}
NVector3& LeapObject::getPos()
{
	return m_position;
}



Leap::~Leap()
{
	close();
}
bool Leap::connect()
{
	if(!m_sock.connect("127.0.0.1",6437))
		return false;
		
	m_sock.send(
		"GET /v3.json HTTP/1.1\r\n"
		"Upgrade: websocket\r\n"
		"\r\n"
		"{\"enableGestures\":false}\r\n");
	m_threadEnable = true;
	m_thread.startThread(CLASSPROC(this,Leap,onRead));
	return true;
}
bool Leap::close()
{
	m_threadEnable = false;
	m_sock.close();
	while(m_thread.isActiveThread())
		Sleep(1);
	return true;
}
NVector3 Leap::getVector()
{
	m_critical.lock();
	NVector3 vect = {0,0,10000};
	std::list<LeapObject>::iterator it;
	if(m_listFinger.size())
	{
		for(it=m_listFinger.begin();it!=m_listFinger.end();++it)
		{
			if(it->getPos().z < vect.z)
				vect = it->getPos();
		}
	}
	else
	{
		for(it=m_listHand.begin();it!=m_listHand.end();++it)
		{
			if(it->getPos().z < vect.z)
				vect = it->getPos();
		}
	}
	m_critical.unlock();
	return vect;
}

DWORD Leap::onRead(LPVOID v)
{
	INT i;
	CHAR buff[65536];
	INT size;
	INT count = 0;
	INT flag = false;
	while((size = m_sock.recv(buff,65536,0,500))>=0 && m_threadEnable)
	{
		if(size==0)
			continue;
		for(i=0;i<size;i++)
		{
			//ヘッダーのスキップ
			if(count < 2)
			{
				if(!flag)
				{
					if(buff[i] == '\r')
						flag = true;
					else
						count = 0;
				}
				else
				{
					flag = false;
					if(buff[i] == '\n')
					{
						count++;
						if(count == 2)
						{
							fwrite(buff,i,1,stdout);

						}
					}
					else
						count = 0;
				}
			}
			else
			{
				//JSONデータの読み出し
				if(buff[i])
					m_data.push_back(buff[i]);
				else
				{
					//JSONデータの解析
					JsonBase* jd = JsonReader::parse(&m_data[0]);
					m_data.clear();

					if(jd)
					{
						onData(jd);
					}
				}
			}
		}
	}
	return 0;
}
bool Leap::getVector(JsonHash* hash,NVector3& vect)
{
	if(!hash)
		return false;
	JsonArray* v = (JsonArray*)hash->getData();
	JsonArrayList::iterator it = v->getData().begin();
	NVector3 value = 
	{
		((JsonFloat*)it++->get())->getData(),
		((JsonFloat*)it++->get())->getData(),
		((JsonFloat*)it->get())->getData()
	};
	vect = value;
	return true;

}
void Leap::onData(JsonBase* json)
{
	//JsonReader::output(json,-1);


	JsonHash* data = json->getHash("currentFrameRate");
	if(data)
	{
		//printf("FrameRate: %f\n",((JsonFloat*)data->getData())->getData());
	}
	//data = (JsonHash*)json->getData("pointables");
	JsonHash* hands = json->getHash("hands");

	std::list<LeapObject> listHand;
	if(hands)
	{
		JsonArray* handArray = (JsonArray*)hands->getData();
		if(handArray)
		{
			JsonArrayList::iterator it;
			for(it=handArray->getData().begin();it!=handArray->getData().end();++it)
			{
				INT id = (*it)->getInt("id");
				NVector3 pos,dir;
				getVector((*it)->getHash("palmPosition"),pos);
				getVector((*it)->getHash("direction"),dir);

				listHand.push_back(LeapObject(id,0,pos,dir));
			}
		}
	}
	JsonHash* fingers = json->getHash("pointables");
	std::list<LeapObject> listFinger;
	if(fingers)
	{
		JsonArray* array = (JsonArray*)fingers->getData();
		if(array)
		{
			JsonArrayList::iterator it;
			for(it=array->getData().begin();it!=array->getData().end();++it)
			{
				INT id = (*it)->getInt("id");
				INT pid = (*it)->getInt("handId");
				NVector3 pos,dir;
				getVector((*it)->getHash("tipPosition"),pos);
				getVector((*it)->getHash("direction"),dir);

				listFinger.push_back(LeapObject(id,pid,pos,dir));
			}
		}
	}

	delete json;
	/*
	std::list<LeapObject>::iterator it;
	for(it=listHand.begin();it!=listHand.end();++it)
	{
		printf("手: %d %10.3f,%10.3f,%10.3f\n",it->getID(),it->getPos().x,it->getPos().y,it->getPos().z);
	}
	for(it=listFinger.begin();it!=listFinger.end();++it)
	{
		printf("指: %d %d %10.3f,%10.3f,%10.3f\n",it->getID(),it->getPID(),it->getPos().x,it->getPos().y,it->getPos().z);
	}
	*/
	m_critical.lock();
	m_listHand = listHand;
	m_listFinger = listFinger;
	m_critical.unlock();
}
