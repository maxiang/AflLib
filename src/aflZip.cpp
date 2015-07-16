#ifdef _WIN32
	#include <windows.h>
#endif
#include <sys/stat.h> 
#include <stdio.h>
#include <stdarg.h>
#include "aflZip.h"


//----------------------------------------------------
//メモリリークテスト用
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
		#define NEW new
		#define CHECK_MEMORY_LEAK
#endif
//----------------------------------------------------


namespace AFL{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ZipReader
// 圧縮データ読み込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ZipReader::ZipReader()
{
	ZeroMemory(&m_zstream,sizeof(z_stream));
	m_buff = NULL;
	m_buffSize = 2048;
	m_file = NULL;
}
ZipReader::~ZipReader()
{
	close();
}
bool ZipReader::open(LPCSTR fileName)
{
	close();

	m_file = fopen(fileName,"rb");
	if(!m_file)
		return false;

	if(inflateInit2(&m_zstream,-MAX_WBITS) != Z_OK)
		return false;

	m_buff = new Bytef[m_buffSize];
	m_zstream.next_in = (Bytef*)m_buff;
	m_zstream.avail_in = 0;
	return true;
}
bool ZipReader::serSource(LPCVOID buff,size_t size)
{
	close();
	if(inflateInit2(&m_zstream,-MAX_WBITS) != Z_OK)
		return false;
	m_zstream.next_in = (Bytef*)buff+10;
	m_zstream.avail_in = size-10;
	return true;

}
bool ZipReader::close()
{
	if(m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}
	if(m_buff)
	{
		delete[] m_buff;
		m_buff = NULL;
	}	
	bool flag = inflateEnd(&m_zstream) == Z_OK;
	ZeroMemory(&m_zstream,sizeof(z_stream));
	return flag;

}

size_t ZipReader::read(LPVOID data,size_t size)
{
	m_zstream.next_out = (Bytef*)data;
	m_zstream.avail_out = (INT)size;
	if(m_file)
	{
		if(!m_buff)
			return 0;
		
		while(m_zstream.avail_out)
		{
			size_t rsize = fread(m_buff,1,m_buffSize - m_zstream.avail_in,m_file);
			if(rsize == 0)
				break;
			m_zstream.avail_in += rsize;
			INT status = inflate(&m_zstream, Z_NO_FLUSH);
			if(status < 0)
				return 0;

			if(m_zstream.avail_in == 0)
			{
				m_zstream.next_in = (Bytef*)m_buff;
			}
		}
		inflate(&m_zstream, Z_PARTIAL_FLUSH);
	}
	else
	{
		INT status = inflate(&m_zstream, Z_NO_FLUSH);
		if(status < 0)
			return 0;
		inflate(&m_zstream, Z_PARTIAL_FLUSH);
	}
	return size - m_zstream.avail_out;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ZipWriter
// 圧縮データ書き込み用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
ZipWriter::ZipWriter()
{
	ZeroMemory(&m_zstream,sizeof(z_stream));
	m_buff = NULL;
	m_buffSize = 2048;
	m_file = NULL;
	m_bs = NULL;
}
ZipWriter::~ZipWriter()
{
	close();
}
bool ZipWriter::open(LPCSTR fileName)
{
	close();

	m_file = fopen(fileName,"wb");
	if(!m_file)
		return false;


	if (deflateInit2(&m_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
		return false;

	m_buff = new Bytef[m_buffSize];
	m_zstream.next_out = (Bytef*)m_buff;
	m_zstream.avail_out = m_buffSize;
	return true;
}
bool ZipWriter::open(BinaryStream* bs)
{
	close();

	m_bs = bs;

	if (deflateInit2(&m_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
		return false;

	m_buff = new Bytef[m_buffSize];
	m_zstream.next_out = (Bytef*)m_buff;
	m_zstream.avail_out = m_buffSize;
	return true;
}
bool ZipWriter::openStd()
{
	close();

	m_file = stdout;

	if (deflateInit2(&m_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
            MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
		return false;

	m_buff = new Bytef[m_buffSize];
	m_zstream.next_out = (Bytef*)m_buff;
	m_zstream.avail_out = m_buffSize;
	return true;
}
bool ZipWriter::close()
{
	if(m_file)
	{
		INT flag;
		do
		{
			flag = deflate(&m_zstream, Z_FINISH);
			fwrite(m_buff,1,m_buffSize - m_zstream.avail_out,m_file);
			m_zstream.next_out = (Bytef*)m_buff;
			m_zstream.avail_out = m_buffSize;
		}while(flag == Z_OK);


		if(	m_file != stdout)
			fclose(m_file);
		m_file = NULL;
	}
	else if(m_bs)
	{
		INT flag;
		do
		{
			flag = deflate(&m_zstream, Z_FINISH);
			m_bs->write(m_buff,m_buffSize - m_zstream.avail_out);
			m_zstream.next_out = (Bytef*)m_buff;
			m_zstream.avail_out = m_buffSize;
		}while(flag == Z_OK);
		m_bs = NULL;
	}

	if(m_buff)
	{
		delete[] m_buff;
		m_buff = NULL;
	}		
	bool flag = deflateEnd(&m_zstream) == Z_OK;
	ZeroMemory(&m_zstream,sizeof(z_stream));
	return flag;

}
bool ZipWriter::write(LPCVOID data,size_t size)
{
	if(!m_buff)
		return false;
	
	m_zstream.next_in = (Bytef*)data;
	m_zstream.avail_in = (INT)size;

	while(m_zstream.avail_in)
	{
        INT status = deflate(&m_zstream, Z_NO_FLUSH);
		if(status < 0)
			return false;

		if(m_bs)
			m_bs->write(m_buff,m_buffSize - m_zstream.avail_out);
		else
			fwrite(m_buff,1,m_buffSize - m_zstream.avail_out,m_file);
		m_zstream.next_out = (Bytef*)m_buff;
		m_zstream.avail_out = m_buffSize;
	}
	return true;
}


}
