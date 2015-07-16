#include "aflStd.h"
#include "aflMath.h"
#include "aflSock.h"

#include <list>

using namespace AFL;
/*
//class JsonHash;
class JsonBase
{
public:
	virtual const type_info& getType() = 0;
	virtual const type_info& getDataType() = 0;
	virtual ~JsonBase(){}
	JsonHash* getHash(LPCSTR name);
	INT getInt(LPCSTR name);
	FLOAT getFloat(LPCSTR name);
	LPCSTR getString(LPCSTR name);
};
template<class T> class JsonData : public JsonBase
{
public:
	JsonData(T data)
	{
		m_data = data;
	}
	virtual const type_info& getType()
	{
		return typeid(*this);
	}
	virtual const type_info& getDataType()
	{
		return typeid(T);
	}
	T& getData()
	{
		return m_data;
	}
protected:
	T m_data;
};
typedef std::list<SP<JsonBase> > JsonArrayList;
typedef JsonData<JsonArrayList> JsonArray;
typedef JsonData<FLOAT> JsonFloat;
typedef JsonData<INT> JsonInt;
typedef JsonData<String> JsonString;
typedef JsonData<bool> JsonBool;
/*
class JsonHash : public JsonData<SP<JsonBase> >
{
public:
	JsonHash(LPCSTR name,JsonBase* data);
	virtual const type_info& getType();
	virtual const type_info& getDataType();
	LPCSTR getName();
	JsonBase* getData();
	JsonHash* getData(LPCSTR name);
protected:
	String m_name;
};
typedef JsonData<std::list<JsonHash> > JsonObject;
*/
class JsonReader
{
public:
	
	static void output(JsonBase* sd,int level);
	static JsonBase* parse(LPCSTR data);
	static bool getNext(LPCSTR& src,String& dest);
	static JsonArray* getArray(LPCSTR& src);
	static JsonObject* getBlock(LPCSTR& src);
	static JsonBase* getData(LPCSTR& src);
	static JsonBase* getValue(LPCSTR value);
};

class LeapObject
{
public:
	LeapObject(INT id,INT pid,NVector3 pos,NVector3 dir);
	INT getID();
	INT getPID();
	NVector3& getPos();
protected:
	INT m_id;
	INT m_pid;
	NVector3 m_position;
	NVector3 m_direction;
};


class Leap
{
	bool m_threadEnable;
public:
	
	~Leap();
	bool connect();
	bool close();
	NVector3 getVector();
protected:
	DWORD onRead(LPVOID v);
	bool getVector(JsonHash* hash,NVector3& vect);
	void onData(JsonBase* json);

	AFL::SOCK::Sock m_sock;
	Thread m_thread;
	std::vector<CHAR> m_data;
	std::list<LeapObject> m_listHand;
	std::list<LeapObject> m_listFinger;
	AFL::Critical m_critical;
};
