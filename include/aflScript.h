#pragma once
#define SQUNICODE
#include <list>
#include "aflStd.h"
#include "../LibProjects/squirrel/include/squirrel.h"


namespace AFL{
struct DataObject
{
	DataObject();
	~DataObject();
	enum DATA_TYPE
	{
		TYPE_NULL,
		TYPE_UINT,
		TYPE_SHORT,
		TYPE_USHORT,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_DOUBLE,
		TYPE_CHAR,
		TYPE_UCHAR,
		TYPE_LONG,
		TYPE_ULONG,
		TYPE_ADR,
		TYPE_DATA
	};
	union
	{
		int i;
		short s;
		float f;
		double d;
		char c;
		void* p; 
	};
	DATA_TYPE type;
	size_t size;
	void (*_removeAdr)(LPVOID);
	LPVOID (*_newAdr)();
	void (*_copyAdr)(LPVOID,LPVOID);
	template<class T> static void _remove(LPVOID p)
	{
		delete (T*)p;
	}
	template<class T> static void _copy(LPVOID dest,LPVOID src)
	{
		*(T*)dest = *(T*)src; 
	}
	template<class T> static LPVOID _new()
	{
		return new T();
	}
	template<class T> DataObject& operator=(T value)
	{
		remove();
		T* data = new T;
		*data = value;
		p = data;
		size = sizeof(T);
		_removeAdr = _remove<T>;
		_newAdr = _new<T>;
		_copyAdr = _copy<T>;
		type = TYPE_DATA;
		return *this;
	}

	template<class T> T get()
	{
		switch(type)
		{
		case TYPE_DATA:
			return *(T*)p;
		default:
			throw "NULL";
		}
	}
	#define GET(T) \
	template<> T DataObject::get()\
	{\
		switch(type)\
		{\
		case TYPE_INT:\
		case TYPE_UINT:\
			return (T)i;\
		case TYPE_SHORT:\
		case TYPE_USHORT:\
			return (T)s;\
		case TYPE_FLOAT:\
			return (T)f;\
		case TYPE_DOUBLE:\
			return (T)d;\
		case TYPE_CHAR:\
		case TYPE_UCHAR:\
			return (T)c;\
		case TYPE_DATA:\
			return *(T*)p;\
		default:\
			throw "NULL";\
		}\
	}
	GET(INT)
	GET(UINT)
	GET(CHAR)
	GET(UCHAR)
	GET(SHORT)
	GET(USHORT)
	GET(LONG)
	GET(ULONG)
	GET(FLOAT)
	GET(DOUBLE)

	DataObject& operator=(DataObject& value);
	DataObject& operator=(INT value);
	DataObject& operator=(SHORT value);
	DataObject& operator=(USHORT value);
	DataObject& operator=(LONG value);
	DataObject& operator=(FLOAT value);
	DataObject& operator=(DOUBLE value);
	DataObject& operator=(CHAR value);
	DataObject& operator=(ULONG value);
	DataObject& operator=(UINT value);
	DataObject& operator=(UCHAR value);
	DataObject& operator=(LPVOID value);
	void remove();
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Script
// Squirrel用スクリプトクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct SqComplieError
{
	LPCWSTR desc;
	LPCWSTR source;
	INT line;
	INT column;
};

class Script
{
	static void _printfunc(HSQUIRRELVM v, const SQChar *s, ...);
	static void _compileError(HSQUIRRELVM v,const SQChar* desc,const SQChar *source,SQInteger line,SQInteger column);
	void _print(LPCWSTR string);
	void _error(SqComplieError* s);
	static void _push(HSQUIRRELVM v,LPCWSTR value);
	static void _push(HSQUIRRELVM v,INT value);
	static void _push(HSQUIRRELVM v,LPVOID value);
	static void _push(HSQUIRRELVM v,bool value);
public:
	template<class T> bool addClass(const SQChar *name)
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_newclass(m_vm, SQFalse);
		//コンストラクタの作成
		sq_pushstring(m_vm, L"constructor", -1);
		sq_newclosure(m_vm, _constructor<T>, 0);
		sq_newslot(m_vm, -3,false);

		//ゲッターの作成
		sq_pushstring(m_vm,L"_get", -1);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_prop_get<T>, 1);
		sq_createslot(m_vm, -3);
/*		//セッターの作成
		sq_pushstring(m_vm,L"_set", -1);
		push_method(m_vm,prop);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_prop_get, 2);
		sq_createslot(m_vm, -3);
*/
		sq_newslot(m_vm, -3,false);
		sq_pop(m_vm, 1);
		return true;
	}
	template<class T> bool addClass2(const SQChar *name)
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_newclass(m_vm, SQFalse);
		sq_pushstring(m_vm, L"constructor", -1);
		sq_newclosure(m_vm, _constructor2<T>, 0);
		sq_newslot(m_vm, -3,false);
		sq_newslot(m_vm, -3,false);
		sq_pop(m_vm, 1);
		return true;
	}

	std::list<ClassProc> m_stopFunc;
	void addStopFunc(ClassProc& proc)
	{
		m_stopFunc.push_back(proc);
	}
	void delStopFunc(ClassProc& proc)
	{
		m_stopFunc.remove(proc);
	}
	bool stop()
	{
		m_suspend = true;
		std::list<ClassProc>::iterator it;
		foreach(it,m_stopFunc)
		{
			it->call(this);
		}
		while(sq_getvmstate(m_vm) == SQ_VMSTATE_RUNNING)
			Sleep(1);
		return true;
		
	}
	template<class T> static SQInteger _class_prop_get(HSQUIRRELVM v)
	{
		T* inst;
		Script* sc;
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);
		//sq_getuserdata(v,-1,(SQUserPointer*)&prop,0);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);

		const SQChar *pname;
		sq_getstring(v,-3,&pname);

		return 1;
	}
	template<class R,class T> void _getProp(T* inst)
	{
		R T::*prop;
		sq_getuserdata(v,-1,(SQUserPointer*)&prop,0);
		_push(v,prop);
	}
	template<class R,class T> bool addProp(const SQChar *cname,const SQChar *name,R T::*prop)
	{
		//m_classProp<R,T>[cname][name] = prop;
		return true;
	}
	static SQInteger _func(HSQUIRRELVM v)
	{
		Script* sc;
		void (*method)(Script*);
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		(**method)(sc);
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 0;
	}
	bool addFunc(const SQChar *name, void (__cdecl *method)(Script*))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return true;
	}
	template<class R> static SQInteger _func2(HSQUIRRELVM v)
	{
		Script* sc;
		void (*method)(Script*);
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		_push((**method)(sc));
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 1;
	}
	template<class R> bool addFunc(const SQChar *name, R (__cdecl *method)(Script*))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func2, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return true;
	}

	template<class T> static SQInteger _class_func01(HSQUIRRELVM v)
	{
		T* inst;
		Script* sc;
		void (T::**method)(Script*);
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);

		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		(inst->**method)(sc);
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 0;
	}
	template<class T> bool addFunc(const SQChar *cname,const SQChar *name, void (T::*method)(Script*))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		push_method(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_func01<T>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	template<class R,class T> static SQInteger _class_func02(HSQUIRRELVM v)
	{
		T* inst;
		Script* sc;
		R (T::**method)(Script*);
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);

		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		(inst->**method)(sc);
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 0;
	}
	template<class R,class T> bool addFunc(const SQChar *cname,const SQChar *name, R (T::*method)(Script*))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		push_method(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_func02<R,T>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}

	Script();
	Script(Script* s);
	~Script();
	void setCompileError(ClassProc& classProc);
	void setRuntimeError(ClassProc& classProc);
	void setPrint(ClassProc& classProc);
	bool exec(LPCWSTR cmd);
	HSQUIRRELVM getVM() const;
	bool isConstable(LPCWSTR name);
	bool isVariable(LPCWSTR name);
	template<class T> T getConstable(LPCWSTR name,T value)
	{
		sq_pushconsttable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_get(m_vm,-2);
		sq_getinteger(m_vm,-1,value);
		sq_pop(m_vm,2);

	}
	template<class T> void addConstable(LPCWSTR name,T value)
	{
		sq_pushconsttable(m_vm);
		sq_pushstring(m_vm, name, -1);
		_push(m_vm,value);
		sq_createslot(m_vm,-3);
		sq_pop(m_vm,1);
	}
	template<class T> void addConstable(LPCWSTR cname,LPCWSTR name,T value)
	{
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		_push(m_vm,value);
		sq_createslot(m_vm,-3);
		sq_pop(m_vm,1);
	}
	template<class R> R getData(LPCWSTR name)
	{
		//sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_get(m_vm, -2);
		R data = get<R>(m_vm,1);
		sq_pop(m_vm,1);
		return data;
	}
	template<class T> void addVariable(LPCWSTR name,T value)
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		_push(m_vm,value);
		sq_createslot(m_vm,-3);
		sq_pop(m_vm,1);
	}
	template<class T> void addVariable(LPCWSTR cname,LPCWSTR name,T value)
	{
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		_push(m_vm,value);
		sq_createslot(m_vm,-3);
		sq_pop(m_vm,1);
	}


	template<class T> void push_method(HSQUIRRELVM v,T& method)
	{
		LPVOID data = (LPVOID)sq_newuserdata(v, sizeof(method));
		CopyMemory(data,&method,sizeof(method));
	}
	bool addFunc(const SQChar *name, void (__cdecl *method)(void))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func1C, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	template<class R> bool addFunc(const SQChar *name, R (__cdecl *method)(void))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func2C<R>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	bool addFunc(const SQChar *name, void (__stdcall *method)(void))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func1S, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	template<class R> bool addFunc(const SQChar *name, R (__stdcall *method)(void))
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, name, -1);
		sq_pushuserpointer(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _func2S<R>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	template<class T> bool addFunc(const SQChar *cname,const SQChar *name, void (T::*method)())
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		push_method(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_func1<T>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	template<class R,class T> bool addFunc(const SQChar *cname,const SQChar *name, R (T::*method)())
	{
		sq_pushroottable(m_vm);
		sq_pushstring(m_vm, cname, -1);
		sq_get(m_vm,-2);
		sq_pushstring(m_vm, name, -1);
		push_method(m_vm,method);
		sq_pushuserpointer(m_vm,this);
		sq_newclosure(m_vm, _class_func2<R,T>, 2);
		sq_createslot(m_vm, -3);
		sq_pop(m_vm, 1);
		return false;
	}
	#define ADDFUNC(N,...) \
public:\
	template<class T,__VA_ARGS__> bool addFunc(const SQChar *cname,const SQChar *name, void (T::*method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, cname, -1);\
		sq_get(m_vm,-2);\
		sq_pushstring(m_vm, name, -1);\
		push_method(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _class_func1<T,__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
	template<class R,class T,__VA_ARGS__> bool addFunc(const SQChar *cname,const SQChar *name, R (T::*method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, cname, -1);\
		sq_get(m_vm,-2);\
		sq_pushstring(m_vm, name, -1);\
		push_method(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _class_func2<R,T,__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
	template<__VA_ARGS__> bool addFunc(const SQChar *name, void (__cdecl *method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, name, -1);\
		sq_pushuserpointer(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _func1C<__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
	template<__VA_ARGS__> bool addFunc(const SQChar *name, void (__stdcall *method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, name, -1);\
		sq_pushuserpointer(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _func1S<__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
	template<typename R,__VA_ARGS__> bool addFunc(const SQChar *name, R (__cdecl *method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, name, -1);\
		sq_pushuserpointer(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _func2C<R,__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
	template<typename R,__VA_ARGS__> bool addFunc(const SQChar *name, R (__stdcall *method)(__VA_ARGS__))\
	{\
		sq_pushroottable(m_vm);\
		sq_pushstring(m_vm, name, -1);\
		sq_pushuserpointer(m_vm,method);\
		sq_pushuserpointer(m_vm,this);\
		sq_newclosure(m_vm, _func2S<R,__VA_ARGS__>, 2);\
		sq_createslot(m_vm, -3);\
		sq_pop(m_vm, 1);\
		return false;\
	}\
protected:\
	template<class T,__VA_ARGS__> static SQInteger _class_func1(HSQUIRRELVM v)\
	{\
		T* inst;\
		Script* sc;\
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);\
		void (T::**method)(__VA_ARGS__);\
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		(inst->**method)(PARAM##N(v));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 0;\
	}\
	template<class R,class T,__VA_ARGS__> static SQInteger _class_func2(HSQUIRRELVM v)\
	{\
		T* inst;\
		Script* sc;\
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);\
		R (T::**method)(__VA_ARGS__);\
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		_push(v,(inst->**method)(PARAM##N(v)));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 1;\
	}\
	template<__VA_ARGS__> static SQInteger _func1C(HSQUIRRELVM v)\
	{\
		Script* sc;\
		void (__cdecl *method)(__VA_ARGS__);\
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		(*method)(PARAM##N(v));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 0;\
	}\
	template<__VA_ARGS__> static SQInteger _func1S(HSQUIRRELVM v)\
	{\
		Script* sc;\
		void (__stdcall *method)(__VA_ARGS__);\
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		(*method)(PARAM##N(v));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 0;\
	}\
	template<typename R,__VA_ARGS__> static SQInteger _func2C(HSQUIRRELVM v)\
	{\
		Script* sc;\
		R (*method)(__VA_ARGS__);\
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		_push(v,(*method)(PARAM##N(v)));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 1;\
	}\
	template<typename R,__VA_ARGS__> static SQInteger _func2S(HSQUIRRELVM v)\
	{\
		Script* sc;\
		R (*method)(__VA_ARGS__);\
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);\
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);\
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		_push(v,(*method)(PARAM##N(v)));\
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 1;\
	}

	#define PARAM1(v) get<P1>(v,1)
	#define PARAM2(v) get<P1>(v,1),get<P2>(v,2)
	#define PARAM3(v) get<P1>(v,1),get<P2>(v,2),get<P3>(v,3)
	#define PARAM4(v) get<P1>(v,1),get<P2>(v,2),get<P3>(v,3),get<P4>(v,4)
	#define PARAM5(v) get<P1>(v,1),get<P2>(v,2),get<P3>(v,3),get<P4>(v,4),get<P5>(v,5)
	ADDFUNC(1,typename P1)
	ADDFUNC(2,typename P1,typename P2)
	ADDFUNC(3,typename P1,typename P2,typename P3)
	ADDFUNC(4,typename P1,typename P2,typename P3,typename P4)
	ADDFUNC(5,typename P1,typename P2,typename P3,typename P4,typename P5)

protected:
	static SQInteger _func1C(HSQUIRRELVM v)
	{
		Script* sc;
		void (__cdecl *method)();
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);\
		sq_settop(v,top-2);\
		(*method)();
		sq_settop(v,top);\
		if(sc->isSuspend())\
			return sq_suspendvm(v);\
		return 0;
	}
	template<class R> static SQInteger _func2C(HSQUIRRELVM v)
	{
		Script* sc;
		R (__cdecl *method)();
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		_push(v,(*method)());
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 1;
	}
	static SQInteger _func1S(HSQUIRRELVM v)
	{
		Script* sc;
		void (__stdcall *method)();
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		(*method)();
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 0;
	}
	template<class R> static SQInteger _func2S(HSQUIRRELVM v)
	{
		Script* sc;
		R (__stdcall *method)();
		sq_getuserpointer(v,-1,(SQUserPointer*)&method);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		_push(v,(*method)());
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 1;
	}
	template<typename R> static R get(HSQUIRRELVM v,INT index)
	{
		SQInteger i;
		sq_getinteger(v, -index, &i);
		return (R)i;
	}
	template<> static LPCWSTR get<LPCWSTR>(HSQUIRRELVM v,INT index)
	{
		LPCWSTR data;
		sq_getstring(v, -index,&data);
		return data;
	}
	template<> static bool get<bool>(HSQUIRRELVM v,INT index)
	{
		SQBool data;
		sq_getbool(v, -index,&data);
		return data!=0;
	}
	template<> static FLOAT get<FLOAT>(HSQUIRRELVM v,INT index)
	{
		SQFloat data;
		sq_getfloat(v, -index,&data);
		return data;
	}

	template<> static LPVOID get<LPVOID>(HSQUIRRELVM v,INT index)
	{
		SQUserPointer data = NULL;
		if(sq_gettype(v, -index) == OT_INSTANCE)
			sq_getinstanceup(v, -index,&data,NULL);
		else
			sq_getuserpointer(v, -index,&data);
		return data;
	}

	template<class T> static SQInteger _class_func1(HSQUIRRELVM v)
	{
		Script* sc;
		T* inst;
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);
		void (T::**method)();
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		(inst->**method)();
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 0;
	}
	template<class R,class T> static SQInteger _class_func2(HSQUIRRELVM v)
	{
		Script* sc;
		T* inst;
		sq_getinstanceup(v,1,(SQUserPointer*)&inst,0);
		R (T::**method)();
		sq_getuserdata(v,-1,(SQUserPointer*)&method,0);
		sq_getuserpointer(v,-2,(SQUserPointer*)&sc);
		INT top = sq_gettop(v);
		sq_settop(v,top-2);
		_push(v,(inst->**method)());
		sq_settop(v,top);
		if(sc->isSuspend())
			return sq_suspendvm(v);
		return 1;
	}
	template<class T> static SQInteger _constructor(HSQUIRRELVM v)
	{
		T *obj = new T(); 
		sq_setinstanceup(v, 1, obj);
		sq_setreleasehook(v, 1, _releasehook<T>);
		return 1;
	}
	template<class T> static SQInteger _constructor2(HSQUIRRELVM v)
	{
		T *obj = new T(v); 
		sq_setinstanceup(v, 1, obj);
		sq_setreleasehook(v, 1, _releasehook<T>);
		return 1;
	}
	template<class T> static SQInteger _releasehook(SQUserPointer p, SQInteger size)
	{
		delete (T*)p;
		return 1;
	}


	HSQUIRRELVM m_vm;
	ClassProc m_printFunc;
	ClassProc m_errorRuntime;
	ClassProc m_errorCompile;
	bool m_suspend;
	bool isSuspend() const
	{
		return m_suspend;
	}
};

}