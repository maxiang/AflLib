#include <windows.h>
#include "aflScript.h"
#include "sqstdblob.h"
#include "sqstdsystem.h"
#include "sqstdio.h"
#include "sqstdmath.h"	
#include "sqstdstring.h"
#include "sqstdaux.h"


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
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif

namespace AFL
{
DataObject::DataObject()
{
	type = TYPE_NULL;
}
DataObject::~DataObject()
{
	remove();
}




DataObject& DataObject::operator=(DataObject& value)
{
	remove();
	p = value._newAdr();
	value._copyAdr(p,value.p);
	type = value.type;
	_removeAdr = value._removeAdr;
	_newAdr = value._newAdr;
	_copyAdr = value._copyAdr;
	return *this;
}
DataObject& DataObject::operator=(INT value)
{
	remove();
	i = value;
	type = TYPE_INT;
	return *this;
}
DataObject& DataObject::operator=(SHORT value)
{
	remove();
	s = value;
	type = TYPE_SHORT;
	return *this;
}
DataObject& DataObject::operator=(USHORT value)
{
	remove();
	s = value;
	type = TYPE_USHORT;
	return *this;
}
DataObject& DataObject::operator=(LONG value)
{
	remove();
	i = value;
	type = TYPE_LONG;
	return *this;
}
DataObject& DataObject::operator=(FLOAT value)
{
	remove();
	f = value;
	type = TYPE_FLOAT;
	return *this;
}
DataObject& DataObject::operator=(DOUBLE value)
{
	remove();
	d = value;
	type = TYPE_DOUBLE;
	return *this;
}
DataObject& DataObject::operator=(CHAR value)
{
	remove();
	c = value;
	type = TYPE_CHAR;
	return *this;
}
DataObject& DataObject::operator=(ULONG value)
{
	remove();
	i = value;
	type = TYPE_ULONG;
	return *this;
}
DataObject& DataObject::operator=(UINT value)
{
	remove();
	i = value;
	type = TYPE_UINT;
	return *this;
}
DataObject& DataObject::operator=(UCHAR value)
{
	remove();
	c = value;
	type = TYPE_UCHAR;
	return *this;
}
DataObject& DataObject::operator=(LPVOID value)
{
	remove();
	p = value;
	type = TYPE_ADR;
	return *this;
}
void DataObject::remove()
{
	if(type == TYPE_DATA)
	{
		type = TYPE_NULL;
		_removeAdr(p);
	}
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Script
// Squirrel用スクリプトクラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
//
// ---  動作  ---
// 
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
void Script::_printfunc(HSQUIRRELVM v, const SQChar *s, ...) 
{
	WString work;
	va_list arglist; 
	va_start(arglist, s); 
	work.vprintf(s,arglist);
	va_end(arglist); 

	Script* script = (Script*)sq_getforeignptr(v);
	if(script)
		script->_print(work);
} 
void Script::_compileError(HSQUIRRELVM v,const SQChar* desc,const SQChar *source,SQInteger line,SQInteger column)
{
	SqComplieError s = {desc,source,(INT)line,(INT)column};
	Script* script = (Script*)sq_getforeignptr(v);
	if(script)
		script->_error(&s);
}
void Script::_print(LPCWSTR string)
{
	m_printFunc.call((LPVOID)string);
}
void Script::_error(SqComplieError* s)
{
	m_errorCompile.call((LPVOID)s);
}

Script::Script()
{
	m_suspend = false;
	HSQUIRRELVM v = sq_open(1024);
	m_vm = v;
	sq_enabledebuginfo(v,true);
	sqstd_register_bloblib(v);
	sqstd_register_iolib(v);
	sqstd_register_systemlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);

	sq_setprintfunc(v, _printfunc);
	sq_setcompilererrorhandler(v, _compileError);
	sq_setforeignptr(v,this);
}
Script::Script(Script* s)
{
	HSQUIRRELVM v = sq_newthread(s->getVM(),1024);
	sq_setforeignptr(v,s);
	m_vm = v;
}

Script::~Script()
{
	stop();
	sq_close(m_vm); 
}

bool Script::isConstable(LPCWSTR name)
{
	bool flag = true;
	sq_pushconsttable(m_vm);
	sq_pushstring(m_vm, name, -1);
	if(SQ_SUCCEEDED(sq_get(m_vm,-2)))
	{
		flag = true;
		sq_pop(m_vm,2);
	}
	else
		sq_pop(m_vm,1);
	return flag;
}
bool Script::isVariable(LPCWSTR name)
{
	bool flag = true;
	sq_pushroottable(m_vm);
	sq_pushstring(m_vm, name, -1);
	sq_get(m_vm,-2);
	if(sq_gettype(m_vm,-1) == OT_NULL)
		flag = false;
	sq_pop(m_vm,2);
	return flag;
}
void Script::setCompileError(ClassProc& classProc)
{
	m_errorCompile = classProc;
}
void Script::setRuntimeError(ClassProc& classProc)
{
	m_errorRuntime = classProc;
}
void Script::setPrint(ClassProc& classProc)
{
	m_printFunc = classProc;
}
bool Script::exec(LPCWSTR cmd)
{
	if(SQ_SUCCEEDED(sq_compilebuffer(m_vm, cmd, (int)wcslen(cmd)*sizeof(SQChar), L"Compile", true)))
	{
		SQObjectType t = sq_gettype(m_vm,0);
		sq_pushroottable(m_vm);
		bool flag = SQ_SUCCEEDED(sq_call(m_vm, 1, true,true));
		sq_pop(m_vm,1); 
		if(flag)
			return true;
	}
	const SQChar *err;
	sq_getlasterror(m_vm);
	if(SQ_SUCCEEDED(sq_getstring(m_vm,-1,&err)))
	{
		m_errorRuntime.call((LPVOID)err);
		sqstd_printcallstack(m_vm);
	}
	return false;

}
HSQUIRRELVM Script::getVM() const
{
	return m_vm;
}
void Script::_push(HSQUIRRELVM v,LPCWSTR value)
{
	sq_pushstring(v,value,-1);
}
void Script::_push(HSQUIRRELVM v,INT value)
{
	sq_pushinteger(v,value);
}
void Script::_push(HSQUIRRELVM v,LPVOID value)
{
	sq_pushuserpointer(v,value);
}
void Script::_push(HSQUIRRELVM v,bool value)
{
	sq_pushbool(v,value);
}
}
