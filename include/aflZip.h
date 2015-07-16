//-----------------------------------------------------
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100

#ifndef __INC_AFLZIP
//-----------------------------------------------------

#ifdef _WIN32
	#pragma warning( disable : 4786 )	//STLの警告外し
	#include <windows.h>
#endif

#ifdef _WIN32
#include "../LibOther/Zlib/zlib.h"
#else
#include "Zlib/zlib.h"
#endif
#include "aflStd.h"
namespace AFL{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ZipReader
// 圧縮データ読み込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class ZipReader
{
public:
	ZipReader();
	~ZipReader();
	bool open(LPCSTR fileName);
	bool serSource(LPCVOID buff,size_t size);
	bool close();
	size_t read(LPVOID data,size_t size);

protected:
	z_stream m_zstream;
	Bytef* m_buff;
	FILE* m_file;
	INT m_buffSize;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ZipWriter
// 圧縮データ書き込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class ZipWriter
{
public:
	ZipWriter();
	~ZipWriter();
	bool open(LPCSTR fileName);
	bool open(BinaryStream* bs);
	bool openStd();
	bool close();
	bool write(LPCVOID data,size_t size);
protected:
	BinaryStream* m_bs;
	z_stream m_zstream;
	Bytef* m_buff;
	FILE* m_file;
	INT m_buffSize;
};

}
//-----------------------------------------------------
#define __INC_AFLZIP
#endif
//-----------------------------------------------------
