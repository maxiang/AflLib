#ifndef __ANDROID__
	#include <windows.h>
#else
	#include "AndroidApp.h"
	#include <android/log.h>
#endif
#include "afl3DBase.h"

//----------------------------------------------------
//メモリリークテスト用
#if _MSC_VER && !defined(_WIN32_WCE) && _DEBUG
	//メモリリークテスト
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
#endif

namespace AFL{


//-----------------------------------------------------
//	初期化
//-----------------------------------------------------
Rect3DF::Rect3DF()
{
	left=right=top=bottom=high=low=0;
}
//-----------------------------------------------------
//	初期化
//-----------------------------------------------------
Rect3DF::Rect3DF(FLOAT nLeft,FLOAT nTop,FLOAT nHigh,FLOAT nRight,FLOAT nBottom,FLOAT nLow)
{
	setRect(nLeft,nTop,nHigh,nRight,nBottom,nLow);
}
Rect3DF::Rect3DF(FLOAT nLeft,FLOAT nTop,FLOAT nRight,FLOAT nBottom)
{
	setRect(nLeft,nTop,nRight,nBottom);
}
//-----------------------------------------------------
//	座標設定
//-----------------------------------------------------
void Rect3DF::setRect(FLOAT nLeft,FLOAT nTop,FLOAT nHigh,FLOAT nRight,FLOAT nBottom,FLOAT nLow)
{
	left = nLeft; right=nRight;
	top = nTop; bottom = nBottom;
	high = nHigh; low = nLow;
}
void Rect3DF::setRect(FLOAT nLeft,FLOAT nTop,FLOAT nRight,FLOAT nBottom)
{
	left = nLeft; right=nRight;
	top = nTop; bottom = nBottom;
	high = 0; low = 0;
}
//-----------------------------------------------------
//	座標移動
//-----------------------------------------------------
void Rect3DF::offsetRect(FLOAT nX,FLOAT nY,FLOAT nZ)
{
	left += nX; right += nX;
	top += nY; bottom += nY;
	high += nZ; low += nZ;
}
//-----------------------------------------------------
//	全て0かどうかの判定
//-----------------------------------------------------
bool Rect3DF::isRectNull() const
{
	if(left || right || top || bottom || high || low)
		return false;
	else
		return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MemoryFile
// ファイルデータメモリアクセス用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
MemoryFile::MemoryFile()
{
	m_data = NULL;
	m_size = 0;
	m_seek = 0;
	m_flag = false;
}
MemoryFile::~MemoryFile()
{
	release();
}
void MemoryFile::setData(LPVOID data,INT size)
{
	m_flag = false;
	m_data =	(LPSTR)data;
	m_size = size;
	m_seek = 0;
}
bool MemoryFile::open(LPCSTR fileName)
{
	FILE* pFile = fopen(fileName,"rb");
	if(!pFile)
		return NULL;

	int length;
	fseek(pFile,0,SEEK_END);
	length = ftell(pFile);
	fseek(pFile,0,SEEK_SET);

	char* data = new char[length];
	if(!data)
		return false;
	fread(data,length,1,pFile);
	fclose(pFile);
	m_size = length;
	m_data = data;
	m_flag = true;
	return true;
}
#ifndef __ANDROID__
bool MemoryFile::open(LPCWSTR fileName)
{
	FILE* pFile = _wfopen(fileName,L"rb");
	if(!pFile)
		return NULL;

	int length;
	fseek(pFile,0,SEEK_END);
	length = ftell(pFile);
	fseek(pFile,0,SEEK_SET);

	char* data = new char[length];
	if(!data)
		return false;
	fread(data,length,1,pFile);
	fclose(pFile);
	m_size = length;
	m_data = data;
	m_flag = true;
	return true;
}
#endif
void MemoryFile::release()
{
	if(m_flag && m_data)
	{
		delete[] m_data;
	}
	m_data = 0;
	m_size = 0;
}
INT MemoryFile::getAllSize() const
{
	return m_size;
}
INT MemoryFile::getSize() const
{
	return m_size - m_seek;
}
LPCSTR MemoryFile::getData()
{
	return m_data + m_seek;
}
bool MemoryFile::seek(INT seek)
{
	if(m_seek + seek <= m_size)
	{
		m_seek += seek;
		return true;
	}
	return false;
}
bool MemoryFile::read(LPVOID dest,INT size)
{
	if(getSize() < size)
		return false;
	CopyMemory(dest,getData(),size);
	seek(size);
	return true;
}



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// FileObject
// ファイル入出力用モデル情報コンテナ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void saveKind(FILE* file,LPCSTR name)
{
	//データ種別
	INT size = (INT)strlen(name) + 1;
	fwrite(&size,sizeof(INT),1,file);
	fwrite(name,size,1,file);
}
INT dataStart(FILE* file,LPCSTR name)
{
	//データ名
	saveKind(file,name);
	//現在位置の記録
	INT seek = (INT)ftell(file);
	//サイズ用エリアの確保
	fseek(file,sizeof(INT),SEEK_CUR);
	return seek;
}
void dataEnd(FILE* file,INT seek)
{
	//サイズの書き込み
	INT seek2 = (INT)ftell(file);
	INT size = seek2-seek-sizeof(INT);

	fseek(file,seek,SEEK_SET);
	fwrite(&size,sizeof(INT),1,file);
	fseek(file,seek2,SEEK_SET);
	fflush(file);
}
void blockEnd(FILE* file)
{
	//範囲終了
	INT size = 0;
	fwrite(&size,sizeof(INT),1,file);
}

template<class T> void saveData(FILE* file,LPCSTR name,const std::vector<T>& data)
{
	if(data.size())
	{
		INT seek = dataStart(file,name);
		typedef typename std::vector<T>::const_iterator IT;
		IT it;
		for(it=data.begin();it!=data.end();++it)
		{
			fwrite(&*it,sizeof(T),1,file);
		}
		dataEnd(file,seek);
	}
}

void saveMesh(FILE* file,MeshData& mesh)
{
	//データが無ければ戻る
	if(!mesh.vertexIndex.size())
		return;
	//サイズ用空間の確保
	INT seek;
	seek = dataStart(file,"+MESH");
	//付属データ終了
	dataEnd(file,seek);

	//頂点インデックスの保存
	saveData(file,"*VINDEX",mesh.vertexIndex);
	//頂点データの保存
	saveData(file,"*VERTEX",mesh.vertexData);
	//法線インデックスの保存
	saveData(file,"*NINDEX",mesh.normalIndex);
	//法線データの保存
	saveData(file,"*NORMAL",mesh.normalData);
	//UVインデックスの保存
	saveData(file,"*UINDEX",mesh.uvIndex);
	//UVデータの保存
	saveData(file,"*UV",mesh.uvData);
	//カラーインデックスの保存
	saveData(file,"*CINDEX",mesh.colorIndex);
	//カラーデータの保存
	saveData(file,"*COLOR",mesh.colorData);
	//マテリアルインデックスの保存
	saveData(file,"*MINDEX",mesh.materialIndex);

	{
		INT seek = dataStart(file,"*MATERIAL");
		//マテリアル個数
		INT size = (INT)mesh.materialData.size();
		fwrite(&size,sizeof(INT),1,file);
		std::vector<MaterialData>::iterator it;
		for(it=mesh.materialData.begin();it!=mesh.materialData.end();++it)
		{
			//マテリアルデータ
			fwrite(&it->material,sizeof(Material),1,file);
			//テクスチャ個数
			size = (INT)it->name.size();
			fwrite(&size,sizeof(INT),1,file);
			//テクスチャファイル名
			std::list<String>::iterator itTex;
			for(itTex=it->name.begin();itTex!=it->name.end();++itTex)
				fwrite(itTex->c_str(),itTex->length()+1,1,file);
		}
		dataEnd(file,seek);
	}
	{
		//ボーン数
		INT size = (INT)mesh.boneData.size();
		if(size)
		{
			INT seek = dataStart(file,"*BONE");


			fwrite(&size,sizeof(INT),1,file);

			std::vector<BoneData>::iterator it;
			for(it=mesh.boneData.begin();it!=mesh.boneData.end();++it)
			{
				fwrite(&it->matrix,sizeof(NMatrix),1,file);
				INT weightCount = (INT)it->weight.size();
				fwrite(&weightCount,sizeof(INT),1,file);
				std::map<INT,FLOAT>::iterator itMap;
				for(itMap=it->weight.begin();itMap!=it->weight.end();++itMap)
				{
					fwrite(&itMap->first,sizeof(INT),1,file);
					fwrite(&itMap->second,sizeof(FLOAT),1,file);
				}
				fwrite(it->name.c_str(),it->name.length()+1,1,file);
			}
			dataEnd(file,seek);
		}
	}


	std::vector<BoneData> boneData;

	//範囲終了
	blockEnd(file);
}



void saveFrame(FILE* file,FrameData& frame)
{
	//サイズ用空間の確保
	INT seek;
	seek = dataStart(file,"+FRAME");

	//行列
	fwrite(&frame.matrix,sizeof(NMatrix),1,file);
	//フレーム名
	fwrite(frame.name.c_str(),frame.name.length()+1,1,file);

	//付属データ終了
	dataEnd(file,seek);

	//メッシュの保存
	saveMesh(file,frame.mesh);

	//チャイルドフレームの保存
	std::list<FrameData>::iterator it;
	for(it=frame.frameChild.begin();it!=frame.frameChild.end();++it)
	{
		saveFrame(file,*it);
	}

	//範囲終了
	blockEnd(file);

}

template<class T> void saveData(FILE* file,LPCSTR name,const std::map<INT,T>& data)
{
	typedef typename std::map<INT,T>::const_iterator IT;
	if(data.size())
	{
		INT seek = dataStart(file,name);
		IT it;
		for(it=data.begin();it!=data.end();++it)
		{
			fwrite(&it->first,sizeof(INT),1,file);
			fwrite(&it->second,sizeof(T),1,file);
		}
		dataEnd(file,seek);
	}
}

void saveAnimeKey(FILE* file,LPCSTR name,AnimationKey& key)
{
	//サイズ用空間の確保
	INT seek;
	seek = dataStart(file,"+ANIMEKEY");

	//フレーム名
	fwrite(name,strlen(name)+1,1,file);
	//付属データ終了
	dataEnd(file,seek);

	//頂点インデックスの保存
	saveData(file,"*POSITION",key.position);
	//頂点データの保存
	saveData(file,"*SCALE",key.scale);
	//頂点データの保存
	saveData(file,"*ROTATION",key.rotation);
	//法線インデックスの保存
	saveData(file,"*MATRIX",key.matrix);

	//範囲終了
	blockEnd(file);
}
void saveAnimeSet(FILE* file,LPCSTR name,ANIMATIONSET& animetionSet)
{
	//サイズ用空間の確保
	INT seek;
	seek = dataStart(file,"+ANIMESET");

	//アニメーションセット名
	fwrite(name,strlen(name)+1,1,file);

	//付属データ終了
	dataEnd(file,seek);

	//キーの出力
	std::map<String,AnimationKey>::iterator it;
	for(it=animetionSet.keys.begin();it!=animetionSet.keys.end();++it)
	{
		saveAnimeKey(file,it->first.c_str(),it->second);
	}


	//範囲終了
	blockEnd(file);
}
#ifndef __ANDROID__
bool FileObject::save(LPCWSTR fileName)
{
	FILE* file = _wfopen(fileName,L"wb");

	LPCSTR header = "MDO";
	fwrite(header,4,1,file);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		saveFrame(file,*itFrame);
	}

	std::map<String,ANIMATIONSET>::iterator itAnime;
	for(itAnime=anime.begin();itAnime!=anime.end();++itAnime)
	{
		saveAnimeSet(file,itAnime->first.c_str(),itAnime->second);
	}
	fclose(file);
	return true;
}
#endif
bool FileObject::save(LPCSTR fileName)
{
	FILE* file = fopen(fileName,"wb");

	LPCSTR header = "MDO";
	fwrite(header,4,1,file);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		saveFrame(file,*itFrame);
	}

	std::map<String,ANIMATIONSET>::iterator itAnime;
	for(itAnime=anime.begin();itAnime!=anime.end();++itAnime)
	{
		saveAnimeSet(file,itAnime->first.c_str(),itAnime->second);
	}
	fclose(file);
	return true;
}
bool readKind(MemoryFile& mf,String& s)
{
	if(mf.getSize() < sizeof(INT))
		return false;

	INT size = *(INT*)mf.getData();
	mf.seek(sizeof(INT));
	if(size)
	{
		s = (LPCSTR) mf.getData();
		mf.seek(size);
	}
	else
	{
		s = "";
	}
	return true;
}
bool readSize(MemoryFile& mf,INT& size)
{
	if(mf.getSize()<4)
		return false;
	size = *(INT*)mf.getData();
	mf.seek(sizeof(INT));
	return true;
}

bool readSkip(MemoryFile& mf,LPCSTR name)
{
	//付属データのスキップ
	INT size;
	if(!readSize(mf,size))
		return false;
	mf.seek(size);

	if(name[0] == '+')
	{
		String kind;
		while(mf.getSize())
		{
			if(!readKind(mf,kind))
				return false;
			if(kind == "")
				break;
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return true;
}
template<class T> bool readData(MemoryFile& mf,std::vector<T>& data)
{
	INT size;
	if(!readSize(mf,size))
		return false;
	if(size)
	{
		data.resize(size/sizeof(T));
		memcpy(&data[0],mf.getData(),size);
		mf.seek(size);
	}
	return true;
}

bool readMesh(MemoryFile& mf,MeshData& mesh)
{
	INT size;
	if(!readSize(mf,size))
		return false;

	if(!mf.seek(size))
		return false;

	while(mf.getSize())
	{
		String kind;
		readKind(mf,kind);
		if(kind == "*VINDEX")
		{
			if(!readData(mf,mesh.vertexIndex))
				return false;
		}
		else if(kind == "*VERTEX")
		{
			if(!readData(mf,mesh.vertexData))
				return false;
		}
		else if(kind == "*NINDEX")
		{
			if(!readData(mf,mesh.normalIndex))
				return false;
		}
		else if(kind == "*NORMAL")
		{
			if(!readData(mf,mesh.normalData))
				return false;
		}
		else if(kind == "*CINDEX")
		{
			if(!readData(mf,mesh.colorIndex))
				return false;
		}
		else if(kind == "*COLOR")
		{
			if(!readData(mf,mesh.colorData))
				return false;
		}
		else if(kind == "*UINDEX")
		{
			if(!readData(mf,mesh.uvIndex))
				return false;
		}
		else if(kind == "*UV")
		{
			if(!readData(mf,mesh.uvData))
				return false;
		}
		else if(kind == "*MINDEX")
		{
			if(!readData(mf,mesh.materialIndex))
				return false;
		}
		else if(kind == "*MATERIAL")
		{
			INT size;
			if(!readSize(mf,size))
				return false;

			LPCSTR data = mf.getData();
			INT count = *(INT*)data;
			data += sizeof(INT);

			mesh.materialData.resize(count);
			INT i,j;

			for(i=0;i<count;i++)
			{
				memcpy(&mesh.materialData[i].material,data,sizeof(Material));
				data += sizeof(Material);
				INT texCount = *(INT*)data;
				data += sizeof(INT);
				for(j=0;j<texCount;j++)
				{
					mesh.materialData[i].name.push_back(data);
					data += strlen(data)+1;
				}
			}

			mf.seek(size);

		}
		else if(kind == "*BONE")
		{
			INT size;
			if(!readSize(mf,size))
				return false;

			LPCSTR data = mf.getData();
			INT count = *(INT*)data;
			data += sizeof(INT);

			mesh.boneData.resize(count);
			INT i,j;
			for(i=0;i<count;i++)
			{
				//ボーンへの相対行列
				memcpy(&mesh.boneData[i].matrix,data,sizeof(NMatrix));
				data += sizeof(NMatrix);
				//ウエイト数
				INT weightCount = *(INT*)data;
				data += sizeof(INT);

				for(j=0;j<weightCount;j++)
				{
					INT index = *(const INT*)data;
					data += sizeof(INT);
					//何故かFLOAT*のキャストで代入するとJNIでエラー
					FLOAT f;
					memcpy(&f,data,sizeof(FLOAT));
					mesh.boneData[i].weight[index] = f;
					data += sizeof(FLOAT);
				}
				mesh.boneData[i].name = data;
				data += strlen(data)+1;
			}

			mf.seek(size);
		}
		else if(kind == "")
		{
			return true;
		}
		else
		{
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return false;
}

bool readFrame(MemoryFile& mf,FrameData& frame)
{
	INT size;
	if(!readSize(mf,size))
		return false;

	LPCSTR data = mf.getData();

	if(!mf.seek(size))
		return false;

	memcpy(&frame.matrix,data,sizeof(frame.matrix));
	frame.name = data+sizeof(frame.matrix);

	while(mf.getSize())
	{
		String kind;
		readKind(mf,kind);
		if(kind == "+FRAME")
		{
			frame.frameChild.resize(frame.frameChild.size()+1);
			readFrame(mf,frame.frameChild.back());
		}else if(kind == "")
		{
			return true;
		}
		else if(kind == "+MESH")
		{
			if(!readMesh(mf,frame.mesh))
				return false;
		}
		else
		{
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return false;
}
template<class T> bool readData(MemoryFile& mf,std::map<INT,T>& data)
{
	INT size;
	if(!readSize(mf,size))
		return false;
	INT count = size/(sizeof(INT)+sizeof(T));
	INT i;
	for(i=0;i<count;i++)
	{
		INT index = *(INT*)mf.getData();
		if(!mf.seek(sizeof(INT)))
			return false;
		T f;
		memcpy(&f,mf.getData(),sizeof(T));
		data[index] = f;
		if(!mf.seek(sizeof(T)))
			return false;
	}
	return true;
}

bool readAnimeKey(MemoryFile& mf,std::map<String,AnimationKey>& animeKey)
{
	INT size;
	if(!readSize(mf,size))
		return false;

	//フレーム名
	LPCSTR name = mf.getData();

	if(!mf.seek(size))
		return false;

	AnimationKey key;

	while(mf.getSize())
	{
		String kind;
		if(!readKind(mf,kind))
			return false;

		if(kind == "*POSITION")
		{
			if(!readData(mf,key.position))
				return false;
		}
		else if(kind == "*SCALE")
		{
			if(!readData(mf,key.scale))
				return false;
		}
		else if(kind == "*ROTATION")
		{
			if(!readData(mf,key.rotation))
				return false;
		}
		else if(kind == "*MATRIX")
		{
			if(!readData(mf,key.matrix))
				return false;
		}
		else if(kind == "")
		{
			animeKey[name] = key;
			return true;
		}
		else
		{
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return false;

}

bool readAnimeSet(MemoryFile& mf,std::map<String,ANIMATIONSET>& anime)
{
	INT size;
	if(!readSize(mf,size))
		return false;

	LPCSTR name = mf.getData();

	if(!mf.seek(size))
		return false;

	ANIMATIONSET animetionSet;
	while(mf.getSize())
	{
		String kind;
		if(!readKind(mf,kind))
			return false;

		if(kind == "+ANIMEKEY")
		{

			if(!readAnimeKey(mf,animetionSet.keys))
				return false;
		}
		else if(kind == "")
		{
			anime[name] = animetionSet;
			return true;
		}
		else
		{
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return false;
}
bool FileObject::loadMemory(MemoryFile& mf)
{
	if(mf.getSize()>=4 && strncmp(mf.getData(),"MDO",4) == 0)
	{
		mf.seek(4);
		return loadMOD(mf);
	}
	if(mf.getSize()>=4 && strncmp(mf.getData(),"PMX ",4) == 0)
	{
		mf.seek(4);
		return loadPMX(mf);
	}
	return false;
}
template<class T1> bool getPMXData(T1& value,MemoryFile& mf,INT size)
{
	if(mf.getSize() >= size)
	{
		if(size == 1)
			value = (T1)*(BYTE*)mf.getData();
		else if(size == 2)
			value = (T1)*(WORD*)mf.getData();
		else if(size == 4)
			value = (T1)*(INT*)mf.getData();
		mf.seek(size);
		return true;
	}
	return false;
}

template<class T2,class T1> bool getPMXData(T1& value,MemoryFile& mf)
{
	if(mf.getSize() >= sizeof(T2))
	{
		value = (T1)*(T2*)mf.getData();
		mf.seek(sizeof(T2));
		return true;
	}
	return false;
}



bool getPMXString(WString& dest,MemoryFile& mf,INT encode)
{
	INT size;
	if(!getPMXData<INT>(size,mf))
		return false;
	bool flag = true;

	if(encode == 0)
	{
		dest.append((LPCWSTR)mf.getData(),size/2);
		mf.seek(size);
		return true;
	}else if(encode == 1)
	{
		UTF8toUCS2(dest,mf.getData(),size);
		mf.seek(size);
		return true;
	}
	return false;

}
struct PMX_HEADER
{
	BYTE encode;		//エンコード方式  | 0:UTF16 1:UTF8
	BYTE uvCount;		//追加UV数 	| 0～4 詳細は頂点参照
	BYTE vIndexSize;	//頂点Indexサイズ | 1,2,4 のいずれか
	BYTE tIndexSize;	//テクスチャIndexサイズ | 1,2,4 のいずれか
	BYTE mIndexSize;	//材質Indexサイズ | 1,2,4 のいずれか
	BYTE bIndexSize;	//ボーンIndexサイズ | 1,2,4 のいずれか
	BYTE mfIndexSize;	//モーフIndexサイズ | 1,2,4 のいずれか
	BYTE rIndexSize;	//剛体Indexサイズ | 1,2,4 のいずれか
};
struct PMX_VERTEX
{
	NVector3 vertex;
	NVector3 normal;
	TEXTUREUV uv;
	NVector uv2;
	FLOAT edge;
	INT blendIndex[4];
	FLOAT blendWeight[3];
};

struct PMX_MATERIAL : public MaterialData
{
	INT faces;
};

bool FileObject::loadPMX(MemoryFile& mf)
{
	INT i;
	//バージョン情報の取得
	float version;
	if(!getPMXData<FLOAT>(version,mf))
		return false;
	//基本ヘッダの読み出し
	INT headerSize;
	if(!getPMXData<BYTE>(headerSize,mf))
		return false;
	if(mf.getSize() < sizeof(PMX_HEADER))
		return false;
	PMX_HEADER pmxHeader;
	CopyMemory(&pmxHeader,mf.getData(),sizeof(PMX_HEADER));
	mf.seek(headerSize);
	//モデル名等
	if(!getPMXString(m_textModel1,mf,pmxHeader.encode))
		return false;
	if(!getPMXString(m_textModel2,mf,pmxHeader.encode))
		return false;
	if(!getPMXString(m_textComment,mf,pmxHeader.encode))
		return false;
	if(!getPMXString(m_textComment2,mf,pmxHeader.encode))
		return false;
	//頂点情報
	INT vertexCount;
	if(!getPMXData<INT>(vertexCount,mf))
		return false;
	PMX_VERTEX* vertex = new PMX_VERTEX[vertexCount];
	ZeroMemory(vertex,sizeof(PMX_VERTEX)*vertexCount);
	for(i=0;i<vertexCount;i++)
	{
		vertex[i].vertex = *(NVector3*)mf.getData();
		mf.seek(sizeof(NVector3));
		vertex[i].normal = *(NVector3*)mf.getData();
		mf.seek(sizeof(NVector3));
		vertex[i].uv = *(TEXTUREUV*)mf.getData();
		mf.seek(sizeof(NVector2));
		CopyMemory(vertex[i].uv2,mf.getData(),sizeof(FLOAT)*pmxHeader.uvCount);
		mf.seek(sizeof(FLOAT)*pmxHeader.uvCount);

		INT weightParam = *(BYTE*)mf.getData();
		mf.seek(sizeof(BYTE));

		INT bSize = 0;
		switch(weightParam)
		{
		case 0:
			getPMXData(vertex[i].blendIndex[0],mf,pmxHeader.bIndexSize);
			break;
		case 1:
			getPMXData(vertex[i].blendIndex[0],mf,pmxHeader.bIndexSize);
			getPMXData(vertex[i].blendIndex[1],mf,pmxHeader.bIndexSize);
			getPMXData<FLOAT>(vertex[i].blendWeight[0],mf);
			break;
		case 2:
			getPMXData(vertex[i].blendIndex[0],mf,pmxHeader.bIndexSize);
			getPMXData(vertex[i].blendIndex[1],mf,pmxHeader.bIndexSize);
			getPMXData(vertex[i].blendIndex[2],mf,pmxHeader.bIndexSize);
			getPMXData(vertex[i].blendIndex[3],mf,pmxHeader.bIndexSize);
			getPMXData<FLOAT>(vertex[i].blendWeight[0],mf);
			getPMXData<FLOAT>(vertex[i].blendWeight[1],mf);
			getPMXData<FLOAT>(vertex[i].blendWeight[2],mf);
			//getPMXData<FLOAT>(vertex[i].blendWeight[2],mf);
			break;
		case 3:
			//未対応
			mf.seek(pmxHeader.bIndexSize*2+4+12*3);
			break;
		}
		getPMXData<FLOAT>(vertex[i].edge,mf);
	}
	//ファイス
	INT indexCount;
	if(!getPMXData<INT>(indexCount,mf))
		return false;
	INT* vertexIndex = new INT[indexCount];
	for(i=0;i<indexCount;i++)
	{
		getPMXData(vertexIndex[i],mf,pmxHeader.vIndexSize);
	}

	//テクスチャファイル
	INT textureCount;
	if(!getPMXData<INT>(textureCount,mf))
		return false;
	WString* textureFile = new WString[i];
	for(i=0;i<textureCount;i++)
	{
		if(!getPMXString(textureFile[i],mf,pmxHeader.encode))
			return false;
	}


	//マテリアル
	INT matCount;
	if(!getPMXData<INT>(matCount,mf))
		return false;
	PMX_MATERIAL* mat = new PMX_MATERIAL[matCount];
	for(i=0;i<matCount;i++)
	{
		//マテリアル名
		WString name;
		if(!getPMXString(name,mf,pmxHeader.encode))
			return false;
		if(!getPMXString(name,mf,pmxHeader.encode))
			return false;

		COLOR4 Diffuse;
		COLOR3 Specular;
		FLOAT SpecularFac;
		COLOR3 Ambient;
		BYTE flag;
		NVector Ege;
		FLOAT EgeSize;
		mf.read(&Diffuse,sizeof(Diffuse));
		mf.read(&Specular,sizeof(Specular));
		mf.read(&SpecularFac,sizeof(SpecularFac));
		mf.read(&Ambient,sizeof(Ambient));
		mf.read(&flag,sizeof(flag));
		mf.read(Ege,sizeof(Ege));
		mf.read(&EgeSize,sizeof(EgeSize));


		INT texIndex1,texIndex2;
		INT texMode;
		INT toon;
		INT toonIndex = 0;
		getPMXData(texIndex1,mf,pmxHeader.tIndexSize);
		getPMXData(texIndex2,mf,pmxHeader.tIndexSize);
		getPMXData<BYTE>(texMode,mf);
		getPMXData<BYTE>(toon,mf);
		if(toon == 0)
		{
			getPMXData(toonIndex,mf,pmxHeader.tIndexSize);
		}
		else
		{
			getPMXData<BYTE>(toonIndex,mf);
		}
		if(!getPMXString(name,mf,pmxHeader.encode))
			return false;
		INT faces;
		getPMXData<INT>(faces,mf);

		mat[i].material.Diffuse = Diffuse;
		mat[i].material.Specular = Specular;
		mat[i].material.Power = SpecularFac;
		mat[i].material.Ambient = Ambient;
		if(texIndex1 >= 0 && texIndex1 < textureCount)
			mat[i].name.push_back(UTF8(textureFile[texIndex1]));
		if(texIndex2 >= 0 && texIndex1 < textureCount)
			mat[i].name.push_back(UTF8(textureFile[texIndex2]));
		mat[i].faces = faces;
	}

	//メッシュの作成
	MeshData mesh;
	INT faceCount = indexCount / 3;
	mesh.vertexData.resize(vertexCount);
	mesh.normalData.resize(vertexCount);
	mesh.uvData.resize(vertexCount);
	mesh.vertexIndex.resize(faceCount);
	mesh.normalIndex.resize(faceCount);
	mesh.uvIndex.resize(faceCount);
	mesh.materialData.resize(matCount);
	mesh.materialIndex.resize(faceCount);

	//頂点データの設定
	for(i=0;i<vertexCount;i++)
	{
		mesh.vertexData[i] = vertex[i].vertex;
		mesh.normalData[i] = vertex[i].normal;
		mesh.uvData[i] = vertex[i].uv;
	}
	//マテリアルデータの設定
	INT matIndex = 0;
	for(i=0;i<matCount;i++)
	{
		mesh.materialData[i] = mat[i];
		INT j;
		for(j = 0;j<mat[i].faces/3;j++)
			mesh.materialIndex[matIndex+j] = i;
		matIndex += j;
	}
	//頂点インデックスの設定
	for(i=0;i<faceCount;i++)
	{
		mesh.vertexIndex[i].type = 3;
		mesh.vertexIndex[i].data[0] = vertexIndex[i*3+0];
		mesh.vertexIndex[i].data[1] = vertexIndex[i*3+1];
		mesh.vertexIndex[i].data[2] = vertexIndex[i*3+2];

		mesh.normalIndex[i].type = 3;
		mesh.normalIndex[i].data[0] = vertexIndex[i*3+0];
		mesh.normalIndex[i].data[1] = vertexIndex[i*3+1];
		mesh.normalIndex[i].data[2] = vertexIndex[i*3+2];

		mesh.uvIndex[i].type = 3;
		mesh.uvIndex[i].data[0] = vertexIndex[i*3+0];
		mesh.uvIndex[i].data[1] = vertexIndex[i*3+1];
		mesh.uvIndex[i].data[2] = vertexIndex[i*3+2];
	}
	//フレームの作成
	FrameData frame;
	frame.matrix.setIdentity();
	frame.mesh = mesh;
	this->frame.push_back(frame);

	delete[] vertex;
	delete[] vertexIndex;
	return true;
}
bool FileObject::loadMOD(MemoryFile& mf)
{
	while(mf.getSize())
	{
		String kind;
		if(!readKind(mf,kind))
			return false;
		if(kind == "+FRAME")
		{
			frame.resize(frame.size()+1);
			if(!readFrame(mf,frame.back()))
				return false;
		}
		else if(kind == "+ANIMESET")
		{
			if(!readAnimeSet(mf,anime))
				return false;
		}
		else
		{
			if(!readSkip(mf,kind))
				return false;
		}
	}
	return true;
}
bool FileObject::load(LPCSTR fileName)
{
	MemoryFile mf;

#ifndef __ANDROID__
	mf.open(fileName);
	return loadMemory(mf);
#else
	INT length = AndroidApp::getFileSize(fileName);
	if(!length)
		return false;
	char* fileData = AndroidApp::readFile(fileName);
	mf.setData(fileData,length);
	bool ret = loadMemory(mf);
	delete[] fileData;
	return ret;
#endif
}
bool FileObject::load(LPCWSTR fileName)
{
	MemoryFile mf;

#ifndef __ANDROID__
	mf.open(fileName);
	return loadMemory(mf);
#else
	String s;
	s = UTF8(fileName);
	INT length = AndroidApp::getFileSize(s);
	if(!length)
		return false;
	char* fileData = AndroidApp::readFile(s);
	mf.setData(fileData,length);
	bool ret = loadMemory(mf);
	delete[] fileData;
	return ret;
#endif
}

void FileObject::clearNormal()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->clearNormal();
	}
}
void FileObject::clearPath()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->clearPath();
	}
}
void FileObject::clearAnime()
{
	anime.clear();
}
void FileObject::clearVColor()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->clearVColor();
	}
}
void FileObject::clearSkin()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->clearSkin();
	}
}
void FileObject::clearUV()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->clearUV();
	}
}

void FileObject::changeCoordinate()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		changeCoordinate(&*itFrame);
	}
	std::map<String,ANIMATIONSET>::iterator itAnime;
	for(itAnime=anime.begin();itAnime!=anime.end();++itAnime)
	{
		changeCoordinate(&itAnime->second);
	}

}
void FileObject::changeCoordinate(ANIMATIONSET* animeSet)
{
	std::map<String,AnimationKey>::iterator itKey;
	for(itKey=animeSet->keys.begin();itKey!=animeSet->keys.end();++itKey)
	{
		std::map<INT,NMatrix>::iterator itMatrix;
		for(itMatrix=(*itKey).second.matrix.begin();itMatrix!=(*itKey).second.matrix.end();++itMatrix)
			itMatrix->second.changeMatrixCoordinate();

		std::map<INT,NVector>::iterator itPos;
		for(itPos=(*itKey).second.position.begin();itPos!=(*itKey).second.position.end();++itPos)
			itPos->second.z = -itPos->second.z;

		std::map<INT,NVector>::iterator itRot;
		for(itRot=(*itKey).second.rotation.begin();itRot!=(*itKey).second.rotation.end();++itRot)
		{
			itRot->second.z = -itRot->second.z;
			itRot->second.w = -itRot->second.w;
		}
	}
}

void FileObject::changeCoordinate(FrameData* frameData)
{
	frameData->matrix.changeMatrixCoordinate();
	changeCoordinate(&frameData->mesh);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		changeCoordinate(&*itFrame);
	}
}
void FileObject::changeCoordinate(MeshData* meshData)
{
	INT i;
	INT dataCount = (INT)meshData->vertexData.size();
	for(i=0;i<dataCount;i++)
	{
		meshData->vertexData[i].z = -meshData->vertexData[i].z;
	}
	dataCount = (INT)meshData->normalData.size();
	for(i=0;i<dataCount;i++)
	{
		meshData->normalData[i].z = -meshData->normalData[i].z;
	}
	dataCount = (INT)meshData->boneData.size();
	for(i=0;i<dataCount;i++)
	{
		meshData->boneData[i].matrix.changeMatrixCoordinate();
	}

	dataCount = (INT)meshData->vertexIndex.size();
	for(i=0;i<dataCount;i++)
	{
		DWORD d;
		d = meshData->vertexIndex[i].data[1];
		meshData->vertexIndex[i].data[1] = meshData->vertexIndex[i].data[2];
		meshData->vertexIndex[i].data[2] = d;
	}
	dataCount = (INT)meshData->uvIndex.size();
	for(i=0;i<dataCount;i++)
	{
		DWORD d;
		d = meshData->uvIndex[i].data[1];
		meshData->uvIndex[i].data[1] = meshData->uvIndex[i].data[2];
		meshData->uvIndex[i].data[2] = d;
	}
	dataCount = (INT)meshData->colorIndex.size();
	for(i=0;i<dataCount;i++)
	{
		DWORD d;
		d = meshData->colorIndex[i].data[1];
		meshData->colorIndex[i].data[1] = meshData->colorIndex[i].data[2];
		meshData->colorIndex[i].data[2] = d;
	}
	dataCount = (INT)meshData->normalIndex.size();
	for(i=0;i<dataCount;i++)
	{
		DWORD d;
		d = meshData->normalIndex[i].data[1];
		meshData->normalIndex[i].data[1] = meshData->normalIndex[i].data[2];
		meshData->normalIndex[i].data[2] = d;
	}


}



void FileObject::getBoneNameList(std::set<String>& listBone)
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		getBoneNameList(listBone,&*itFrame);
	}
}
void FileObject::getBoneNameList(std::set<String>& listBone,FrameData* frameData)
{
	INT i;
	for(i=0;i<(INT)frameData->mesh.boneData.size();i++)
		listBone.insert(frameData->mesh.boneData[i].name);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		getBoneNameList(listBone,&*itFrame);
	}
}
void FileObject::triConvert()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		triConvert(&*itFrame);
	}
}
void FileObject::triConvert(FrameData* frameData)
{
	triConvert(&frameData->mesh);
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		triConvert(&*itFrame);
	}
}
void FileObject::triConvert(MeshData* meshData)
{
	INT i;
	INT indexCount = (INT)meshData->vertexIndex.size();
	INT typeCount4 = 0;
	for(i=0;i<indexCount;i++)
		if(meshData->vertexIndex[i].type == 4)
			typeCount4++;
	if(!typeCount4)
		return;
	//変換後のサイズ
	INT newCount = indexCount+typeCount4;

	//マテリアル
	if(meshData->materialIndex.size())
	{
		std::vector<DWORD> materialIndex;
		materialIndex.reserve(newCount);
		for(i=0;i<indexCount;i++)
		{
			if(meshData->vertexIndex[i].type == 4)
				materialIndex.push_back(meshData->materialIndex[i]);
			materialIndex.push_back(meshData->materialIndex[i]);
		}
		meshData->materialIndex = materialIndex;
	}
	//法線
	if(meshData->normalIndex.size())
	{
		std::vector<INDEX4> normalIndex;
		normalIndex.reserve(newCount);
		for(i=0;i<indexCount;i++)
		{
			if(meshData->normalIndex[i].type == 4)
			{
				meshData->normalIndex[i].type = 3;
				INDEX4 index4 = meshData->normalIndex[i];
				index4.data[0] = index4.data[3];
				normalIndex.push_back(index4);
			}
			normalIndex.push_back(meshData->normalIndex[i]);
		}
		meshData->normalIndex = normalIndex;
	}
	//UV
	if(meshData->uvIndex.size())
	{
		std::vector<INDEX4> uvIndex;
		uvIndex.reserve(newCount);
		for(i=0;i<indexCount;i++)
		{
			if(meshData->uvIndex[i].type == 4)
			{
				meshData->uvIndex[i].type = 3;
				uvIndex.push_back(meshData->uvIndex[i]);
			}
			uvIndex.push_back(meshData->uvIndex[i]);
		}
		meshData->uvIndex = uvIndex;
	}

	//頂点
	std::vector<INDEX4> vertexIndex;
	vertexIndex.reserve(newCount);
	for(i=0;i<indexCount;i++)
	{
		if(meshData->vertexIndex[i].type == 4)
		{
			meshData->vertexIndex[i].type = 3;
			INDEX4 index4;// = meshData->vertexIndex[i];
			index4.type = 3;
			index4.data[0] = meshData->vertexIndex[i].data[2];
			index4.data[1] = meshData->vertexIndex[i].data[3];
			index4.data[2] = meshData->vertexIndex[i].data[0];
			vertexIndex.push_back(index4);
		}
		vertexIndex.push_back(meshData->vertexIndex[i]);
	}
	meshData->vertexIndex = vertexIndex;
}

void FileObject::optimize()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		optimize(&*itFrame);
	}
	std::map<String,ANIMATIONSET>::iterator itAnime;
	for(itAnime=anime.begin();itAnime!=anime.end();++itAnime)
	{
		std::map<String,AnimationKey>::iterator itAnimeKey;
		for(itAnimeKey = itAnime->second.keys.begin();itAnimeKey != itAnime->second.keys.end();++itAnimeKey)
		{
			itAnimeKey->second.optimize();
		}
	}

}
void FileObject::optimize(FrameData* frameData)
{
	optimize(&frameData->mesh);
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		optimize(&*itFrame);
	}

}
void FileObject::optimize(MeshData* meshData)
{
	INT i,j;
	INT dataCount;

	//UV座標の重複削除
	dataCount = (INT)meshData->uvData.size();
	if(dataCount)
	{
		INT index;
		std::vector<INT> convertIndex(dataCount);
		std::map<TEXTUREUV,INT>::iterator itMap;
		std::map<TEXTUREUV,INT> dataMap;
		for(i=0;i<dataCount;i++)
		{
			itMap = dataMap.find(meshData->uvData[i]);
			if(itMap != dataMap.end())
			{
				index = (*itMap).second;
			}
			else
			{
				index = (INT)dataMap.size();
				dataMap[meshData->uvData[i]] = index;
			}
			convertIndex[i] = index;
		}

		meshData->uvData.resize(dataMap.size());
		for(itMap=dataMap.begin();itMap!=dataMap.end();++itMap)
		{
			meshData->uvData[(*itMap).second] = (*itMap).first;
		}
		INT indexCount = (INT)meshData->uvIndex.size();
		for(i=0;i<indexCount;i++)
		{
			INT type = meshData->uvIndex[i].type;
			for(j=0;j<type;j++)
				meshData->uvIndex[i].data[j] = convertIndex[meshData->uvIndex[i].data[j]];
		}
	}
	//COLORの重複削除
	dataCount = (INT)meshData->colorData.size();
	if(dataCount)
	{
		INT index;
		std::vector<INT> convertIndex(dataCount);
		std::map<COLOR4,INT>::iterator itMap;
		std::map<COLOR4,INT> dataMap;
		for(i=0;i<dataCount;i++)
		{
			itMap = dataMap.find(*(COLOR4*)&meshData->colorData[i]);
			if(itMap != dataMap.end())
			{
				index = (*itMap).second;
			}
			else
			{
				index = (INT)dataMap.size();
				dataMap[*(COLOR4*)&meshData->colorData[i]] = index;
			}
			convertIndex[i] = index;
		}

		meshData->colorData.resize(dataMap.size());
		for(itMap=dataMap.begin();itMap!=dataMap.end();++itMap)
		{
			meshData->colorData[(*itMap).second] = (*itMap).first;
		}
		INT indexCount = (INT)meshData->colorIndex.size();
		for(i=0;i<indexCount;i++)
		{
			INT type = meshData->colorIndex[i].type;
			for(j=0;j<type;j++)
				meshData->colorIndex[i].data[j] = convertIndex[meshData->colorIndex[i].data[j]];
		}
	}
	//頂点
	dataCount = (INT)meshData->vertexData.size();
	if(dataCount)
	{
		INT index;
		std::vector<INT> convertIndex(dataCount);
		std::map<NVector3,INT>::iterator itMap;
		std::map<NVector3,INT> dataMap;
		for(i=0;i<dataCount;i++)
		{
			NVector3* vertex = (NVector3*)&meshData->vertexData[i];
			itMap = dataMap.find(*vertex);
			if(itMap != dataMap.end())
			{
				index = (*itMap).second;
			}
			else
			{
				index = (INT)dataMap.size();
				dataMap[*vertex] = index;
			}
			convertIndex[i] = index;
		}
		INT newCount = (INT)dataMap.size();
		meshData->vertexData.resize(newCount);
		for(itMap=dataMap.begin();itMap!=dataMap.end();++itMap)
		{
			meshData->vertexData[(*itMap).second] = (*itMap).first;
		}
		INT indexCount = (INT)meshData->vertexIndex.size();
		for(i=0;i<indexCount;i++)
		{
			INT type = meshData->vertexIndex[i].type;
			for(j=0;j<type;j++)
				meshData->vertexIndex[i].data[j] = convertIndex[meshData->vertexIndex[i].data[j]];
		}
		INT boneCount = (INT)meshData->boneData.size();
		if(boneCount)
		{
			std::vector<INT> boneIndex(newCount);
			std::vector<INT> boneWeight(newCount);
			for(i=0;i<boneCount;i++)
			{
				std::map<INT,FLOAT> boneMap;
				std::map<INT,FLOAT>::iterator itWeight;
				for(itWeight=meshData->boneData[i].weight.begin();itWeight!=meshData->boneData[i].weight.end();++itWeight)
				{
					INT newIndex = convertIndex[itWeight->first];
					boneMap[newIndex] = itWeight->second;
				}
				meshData->boneData[i].weight = boneMap;
			}
		}
	}
	//法線
	dataCount = (INT)meshData->normalData.size();
	if(dataCount)
	{
		INT index;
		std::vector<INT> convertIndex(dataCount);
		std::map<NVector3,INT>::iterator itMap;
		std::map<NVector3,INT> dataMap;
		for(i=0;i<dataCount;i++)
		{
			itMap = dataMap.find(*(NVector3*)&meshData->normalData[i]);
			if(itMap != dataMap.end())
			{
				index = (*itMap).second;
			}
			else
			{
				index = (INT)dataMap.size();
				dataMap[*(NVector3*)&meshData->normalData[i]] = index;
			}
			convertIndex[i] = index;
		}

		meshData->normalData.resize(dataMap.size());
		for(itMap=dataMap.begin();itMap!=dataMap.end();++itMap)
		{
			meshData->normalData[(*itMap).second] = (*itMap).first;
		}
		INT indexCount = (INT)meshData->normalIndex.size();
		for(i=0;i<indexCount;i++)
		{
			INT type = meshData->normalIndex[i].type;
			for(j=0;j<type;j++)
				meshData->normalIndex[i].data[j] = convertIndex[meshData->normalIndex[i].data[j]];
		}
	}
}

void FileObject::setNormal(NVector vector)
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		setNormal(&*itFrame,vector);
	}
}
void FileObject::setNormal(FrameData* frameData,NVector vector)
{
	INT i;
	NVector vect = vector;
	for(i=0;i<(INT)frameData->mesh.normalData.size();i++)
	{
		NVector normal;
		normal = frameData->mesh.normalData[i];
		normal *= vect;
		frameData->mesh.normalData[i],normal = normal.normal();
	}
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		setNormal(&*itFrame,vector);
	}
}

void FileObject::setCull()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		setCull(&*itFrame);
	}
}
void FileObject::setCull(FrameData* frameData)
{
	INT i;
	for(i=0;i<(INT)frameData->mesh.vertexIndex.size();i++)
	{
		DWORD data = frameData->mesh.vertexIndex[i].data[0];
		frameData->mesh.vertexIndex[i].data[0] = frameData->mesh.vertexIndex[i].data[2];
		frameData->mesh.vertexIndex[i].data[2] = data;
	}
	for(i=0;i<(INT)frameData->mesh.normalIndex.size();i++)
	{
		DWORD data = frameData->mesh.normalIndex[i].data[0];
		frameData->mesh.normalIndex[i].data[0] = frameData->mesh.normalIndex[i].data[2];
		frameData->mesh.normalIndex[i].data[2] = data;
	}

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		setCull(&*itFrame);
	}

}
void FileObject::setRotation(FLOAT x,FLOAT y,FLOAT z)
{
	NMatrix matrix;
	NVector rotation = {y*NPI/180.0f,x*NPI/180.0f,z*NPI/180,0.0f};
	rotation = rotation.quotanion();
	rotation.w = -rotation.w;
	matrix.setRotationQuaternion(rotation);
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->matrix *= matrix;
		//itFrame->matrix.setRotationQuaternion(rotation);
		std::map<String,ANIMATIONSET>::iterator itAnimeSet;
		for(itAnimeSet=anime.begin();itAnimeSet!=anime.end();++itAnimeSet)
		{
			std::map<String,AnimationKey>::iterator itAnime =
				itAnimeSet->second.keys.find(itFrame->name);
			if(itAnime!=itAnimeSet->second.keys.end())
			{
				std::map<INT,NVector>::iterator itRot;
				for(itRot=itAnime->second.rotation.begin();itRot!=itAnime->second.rotation.end();++itRot)
				{
					itRot->second *= rotation;
				}
			}
		}
	}

}

void FileObject::setScale(FLOAT x,FLOAT y,FLOAT z)
{
	NMatrix matrix;
	matrix.setScaling(x,y,z);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->matrix *= matrix;

		std::map<String,ANIMATIONSET>::iterator itAnimeSet;
		for(itAnimeSet=anime.begin();itAnimeSet!=anime.end();++itAnimeSet)
		{
			std::map<String,AnimationKey>::iterator itAnime =
				itAnimeSet->second.keys.find(itFrame->name);
			if(itAnime!=itAnimeSet->second.keys.end())
			{
				std::map<INT,NVector>::iterator itSca;
				for(itSca=itAnime->second.scale.begin();itSca!=itAnime->second.scale.end();++itSca)
				{
					itSca->second.x *= x;
					itSca->second.y *= y;
					itSca->second.z *= z;
				}
			}
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AnimationKey
// アニメーションキー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
DWORD AnimationKey::getAllCount() const
{
	INT dwMax = 0;;
	if(!matrix.empty())
	{
		dwMax = matrix.rbegin()->first;
		return dwMax;
	}
	if(!position.empty())
	{
		dwMax = position.rbegin()->first;
	}
	if(!scale.empty())
	{
		INT count = scale.rbegin()->first;
		if(count > dwMax)
			dwMax = count;
	}
	if(!rotation.empty())
	{
		INT count = rotation.rbegin()->first;
		if(count > dwMax)
			dwMax = count;
	}
	return dwMax;
}
bool AnimationKey::getAnimationKey(DWORD dwTime,NMatrix* destMatrix,
		NVector* destPosition,NVector* destScale,NVector* destRotation,bool loop)
{
	if(!matrix.empty())
	{
		getMatrix(dwTime,destMatrix,loop);
		return false;
	}
	getRotation(dwTime,destRotation,loop);
	getScale(dwTime,destScale,loop);
	getPosition(dwTime,destPosition,loop);
	return true;
}
bool AnimationKey::getAnimationKey2(DWORD dwTime,NMatrix* destMatrix,
		NVector* destPosition,NVector* destScale,NVector* destRotation,bool loop)
{
	if(!matrix.empty())
	{
		getMatrix2(dwTime,destMatrix,loop);
		return false;
	}
	getPosition2(dwTime,destPosition,loop);
	getRotation2(dwTime,destRotation,loop);
	getScale2(dwTime,destScale,loop);
	return true;
}


bool AnimationKey::getAnimationMatrix(DWORD dwTime,NMatrix* pMatrix,bool loop)
{
	if(!matrix.empty())
	{
		getMatrix(dwTime,pMatrix,loop);
		return true;
	}

	if(!rotation.empty())
	{
		NVector q;
		getRotation(dwTime,&q,loop);
		pMatrix->setRotationQuaternion(q);
	}

	NVector vectPosition;
	NVector vectScale;
	getScale(dwTime,&vectScale,loop);
	getPosition(dwTime,&vectPosition,loop);

	NMatrix matrix =
	{
		vectScale.x,0,0,0,
		0,vectScale.y,0,0,
		0,0,vectScale.z,0,
		vectPosition.x,vectPosition.y,vectPosition.z,1
	};
	if(!rotation.empty())
		*pMatrix *= matrix;
	else
		*pMatrix = matrix;
	return true;
}
void AnimationKey::getMatrix(DWORD dwTime,NMatrix* pMatrix,bool loop)
{
	DWORD dwLastTime = matrix.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;

	std::map<INT,NMatrix>::iterator it1,it2;
	it1 = it2 = matrix.upper_bound(dwNowTime);
	it1--;
	if(it2 != matrix.end())
	{
		if(it1 == matrix.end())
			*pMatrix = it2->second;
		else if(it1->first == dwNowTime)
			*pMatrix = it1->second;
		else
		{
			DWORD dwTime1 = it1->first;
			DWORD dwTime2 = it2->first;
			FLOAT fVect = (FLOAT)((INT)dwTime1 - (INT)dwNowTime) / (FLOAT)((INT)dwTime1-(INT)dwTime2);

			INT i;
			for(i=0;i<4;i++)
			{
				pMatrix->r[i] = it1->second.r[i].lerp(it2->second.r[i],fVect);
			}
		}
	}
	else
	{
		*pMatrix = it1->second;
	}
}
void AnimationKey::getMatrix2(DWORD dwTime,NMatrix* pMatrix,bool loop)
{
	DWORD dwLastTime = matrix.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;

	std::map<INT,NMatrix>::iterator it1,it2;
	it1 = it2 = matrix.upper_bound(dwNowTime);
	it1--;
	if(it2 != matrix.end())
	{
		if(it1 == matrix.end())
			*pMatrix = it2->second;
		else
			*pMatrix = it1->second;
	}
	else
	{
		*pMatrix = it1->second;
	}
}
void AnimationKey::getPosition(DWORD dwTime,NVector* pVector,bool loop)
{
	if(position.empty())
	{
		pVector->setZero();
		return;
	}
	DWORD dwLastTime = position.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;
	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = position.upper_bound(dwNowTime);
	it1--;
	if(it2 != position.end())
	{
		if(it1 == position.end())
			*pVector = it2->second;
		else if(it1->first == dwNowTime)
			*pVector = it1->second;
		else
		{
			DWORD dwTime1 = it1->first;
			DWORD dwTime2 = it2->first;
			FLOAT fVect = (FLOAT)((INT)dwTime1 - (INT)dwNowTime) / (FLOAT)((INT)dwTime1-(INT)dwTime2);

			*pVector = it1->second.lerp(it2->second,fVect);
		}
	}
	else
	{
		*pVector = it1->second;
	}
}
void AnimationKey::getPosition2(DWORD dwTime,NVector* pVector,bool loop)
{
	if(position.empty())
	{
		pVector->setZero();
		return;
	}
	DWORD dwLastTime = position.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;

	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = position.upper_bound(dwNowTime);
	it1--;
	if(it2 != position.end())
	{
		if(it1 == position.end())
			*pVector = it2->second;
		else
			*pVector = it1->second;
	}
	else
	{
		*pVector = it1->second;
	}
}

void AnimationKey::getScale(DWORD dwTime,NVector* pVector,bool loop)
{
	if(scale.empty())
	{
		pVector->setZero();
		return;
	}
	DWORD dwLastTime = scale.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;

	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = scale.upper_bound(dwNowTime);
	it1--;
	if(it2 != scale.end())
	{
		if(it1 == scale.end())
			*pVector = it2->second;
		else if(it1->first == dwNowTime)
			*pVector = it1->second;
		else
		{
			DWORD dwTime1 = it1->first;
			DWORD dwTime2 = it2->first;
			FLOAT fVect = (FLOAT)((INT)dwTime1 - (INT)dwNowTime) / (FLOAT)((INT)dwTime1-(INT)dwTime2);

			*pVector = it1->second.lerp(it2->second,fVect);
		}
	}
	else
	{
		*pVector = it1->second;
	}
}
void AnimationKey::getScale2(DWORD dwTime,NVector* pVector,bool loop)
{
	if(scale.empty())
	{
		pVector->setZero();
		return;
	}
	DWORD dwLastTime = scale.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;

	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = scale.upper_bound(dwNowTime);
	it1--;
	if(it2 != scale.end())
	{
		if(it1 == scale.end())
			*pVector = it2->second;
		else
			*pVector = it1->second;
	}
	else
	{
		*pVector = it1->second;
	}
}

void AnimationKey::getRotation(DWORD dwTime,NVector* pQuatenion,bool loop)
{
	if(rotation.empty())
	{
		const static NVector value = {0.0f,0.0f,0.0f,1.0f};
		*pQuatenion = value;
		return;
	}
	DWORD dwLastTime = rotation.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;


	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = rotation.upper_bound(dwNowTime);
	it1--;
	if(it2 != rotation.end())
	{
		if(it1 == rotation.end())
			*pQuatenion = it2->second;
		else if(it1->first == dwNowTime)
			*pQuatenion = it1->second;
		else
		{
			DWORD dwTime1 = it1->first;
			DWORD dwTime2 = it2->first;
			FLOAT fVect = (FLOAT)((INT)dwTime1 - (INT)dwNowTime) / (FLOAT)((INT)dwTime1-(INT)dwTime2);

			*pQuatenion = it1->second.slerpQuaternion(it2->second,fVect);
		}
	}
	else
	{
		*pQuatenion = it1->second;
	}
}
void AnimationKey::getRotation2(DWORD dwTime,NVector* pQuatenion,bool loop)
{
	if(rotation.empty())
	{
		const static NVector value = {0.0f,0.0f,0.0f,1.0f};
		*pQuatenion = value;
		return;
	}
	DWORD dwLastTime = rotation.rbegin()->first + 1;
	DWORD dwNowTime;
	if(loop)
		dwNowTime = dwTime % dwLastTime;
	else if(dwTime >= dwLastTime)
		dwNowTime = dwLastTime - 1;
	else
		dwNowTime = dwTime;


	std::map<INT,NVector>::iterator it1,it2;
	it1 = it2 = rotation.upper_bound(dwNowTime);
	it1--;
	if(it2 != rotation.end())
	{
		if(it1 == rotation.end())
			*pQuatenion = it2->second;
		else
			*pQuatenion = it1->second;
	}
	else
	{
		*pQuatenion = it1->second;
	}
}
void distinct(std::map<INT,NVector>& vector)
{
	std::map<INT,NVector>::iterator it;
	for(it=vector.begin();it!=vector.end();++it)
	{
		std::map<INT,NVector>::iterator itStart = it;
		std::map<INT,NVector>::iterator it2 = it;
		++itStart;
		for(it2=itStart;it2!=vector.end();it2++)
		{
			if(!((it2->second - it->second).abs() < 0.000001f))
				break;
		}
		if(it2!=itStart)
		{
			--it2;
			vector.erase(itStart,it2);
		}
	}
}
void removeComplement2(std::map<INT,NVector>& q)
{
	std::map<INT,NVector>::iterator it;
	std::map<INT,NVector>::iterator it2;
	std::map<INT,NVector>::iterator it3;
	for(it=q.begin();it!=q.end();it++)
	{
		++(it2=it);
		if(it2==q.end())
			break;
		++(it3=it2);
		if(it2==q.end())
			break;
		NVector qn;
		qn = it->second.slerpQuaternion(it3->second,((FLOAT)it2->first - it->first) / ((FLOAT)it3->first - it->first));
		if((qn - it2->second).abs() < 0.005f)
			q.erase(it2);
	}
}

void removeComplement(std::map<INT,NVector>& q)
{
	std::map<INT,NVector>::iterator it;
	std::map<INT,NVector>::iterator it2;
	std::map<INT,NVector>::iterator it3;
	for(it=q.begin();it!=q.end();it++)
	{
		++(it2=it);
		if(it2==q.end())
			break;
		++(it3=it2);
		if(it2==q.end())
			break;
		NVector qn;
		qn = it->second.lerp(it3->second,((FLOAT)it2->first - it->first) / ((FLOAT)it3->first - it->first));
		if((qn - it2->second).abs() < 0.005f)
			q.erase(it2);
	}


/*
		while(1)
		{
			std::map<INT,NVector>::iterator it2 = it;
			++it2;
			if(it2 == q.end())
				break;
			std::map<INT,NVector>::iterator it3 = it2;
			++it3;
			if(it3 == q.end())
				break;

			NVector qn;
			//qn = it->second.slerpQuaternion(it3->second,((FLOAT)it2->first - it->first) / ((FLOAT)it3->first - it->first));
			qn = it->second.lerp(it3->second,((FLOAT)it2->first - it->first) / ((FLOAT)it3->first - it->first));
			if((qn - it2->second).abs() < 0.005f)
			{
				q.erase(it2);
			}
			else
				break;
		}
*/

}

void AnimationKey::optimize()
{
	//行列の重複要素を削除
	if(matrix.size() > 1)
	{
		std::map<INT,NMatrix>::iterator itMatrix = matrix.begin();
		NMatrix mat = itMatrix->second;
		++itMatrix;
		while(itMatrix != matrix.end())
		{
			if(mat == itMatrix->second)
			{
				std::map<INT,NMatrix>::iterator itMatrix2 = itMatrix;
				++itMatrix;
				matrix.erase(itMatrix2);
			}
			else
			{
				mat = itMatrix->second;
				++itMatrix;
			}
		}
	}
	//行列の要素を分解
	if(matrix.size())
	{
		std::map<INT,NMatrix>::iterator itMatrix = matrix.begin();
		for(itMatrix = matrix.begin();itMatrix != matrix.end();++itMatrix)
		{
			NVector pos,rot,sca;
			itMatrix->second.decompMatrix(pos,sca,rot);

			INT t = itMatrix->first;
			position[t] = pos;
			scale[t] = sca;
			rotation[t] = rot;
		}
		matrix.clear();
	}
	//重複キーの削除
	distinct(position);
	distinct(scale);
	distinct(rotation);
	removeComplement(position);
	removeComplement(scale);
//	removeComplement2(rotation);

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Animation
// コンストラクタ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Animation::Animation()
{
	m_animationTime = 0;
	m_animationTimeStart = 0;
	m_animationTimeLength = 0;
	m_animationTimeChange = 0;
	m_animationTimeChangeWork = 0;
	m_loop = true;
}


//-----------------------------------------------
// ---  動作  ---
// アニメーションのキーを取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
//
//-----------------------------------------------
INT Animation::getAnimationKey(LPCSTR pFrameName,NMatrix* matrix,NVector* position,NVector* scale,NVector* rotation)
{
	std::map<String,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return 0;
	DWORD dwAnimationCount = m_animationTime;
	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationKey(dwAnimationCount,matrix,position,scale,rotation,m_loop) + 1;
}
INT Animation::getAnimationKey2(LPCSTR pFrameName,NMatrix* matrix,NVector* position,NVector* scale,NVector* rotation)
{
	std::map<String,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return 0;
	DWORD dwAnimationCount = m_animationTime;

	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationKey2(dwAnimationCount,matrix,position,scale,rotation,m_loop) + 1;
}
//-----------------------------------------------
// ---  動作  ---
// アニメーションの行列を取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
//
//-----------------------------------------------
bool Animation::getAnimationMatrix(LPCSTR pFrameName,NMatrix* pMatrix)
{
	std::map<String,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return false;

	DWORD dwAnimationCount = m_animationTime;
	if(m_animationTimeLength)
	{
		if(m_loop)
		{
			dwAnimationCount = dwAnimationCount % m_animationTimeLength;
			dwAnimationCount += m_animationTimeStart;
		}
		else
		{
			if(dwAnimationCount >= m_animationTimeLength)
				dwAnimationCount = m_animationTimeLength-1;
		}
	}
	return (*it).second.getAnimationMatrix(dwAnimationCount,pMatrix,m_loop);
}

//-----------------------------------------------
// ---  動作  ---
// フレームのキーデータを取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
//
//-----------------------------------------------
AnimationKey* Animation::getAnimationKey(LPCSTR pFrameName)
{
	std::map<String,AnimationKey>::iterator it;
	it = m_animationKey.find(pFrameName);
	if(it == m_animationKey.end())
		return NULL;
	return &(*it).second;

}

//-----------------------------------------------
// ---  動作  ---
// フレームのキーの数を取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
//
//-----------------------------------------------
INT Animation::getAnimationCount() const
{
	return (INT)m_animationKey.size();
}

//-----------------------------------------------
// ---  動作  ---
// アニメーションリストからまとめて転送
// ---  引数  ---
// 無し
// --- 戻り値 ---
//
//-----------------------------------------------
void Animation::setAnimation(String name,std::map<String,AnimationKey>& animationKey)
{
	m_name = name;
	m_animationKey = animationKey;
	m_animationTimeLast = 0;
	std::map<String,AnimationKey>::const_iterator it;
	for(it=m_animationKey.begin();it!=m_animationKey.end();++it)
	{
		DWORD last = it->second.getAllCount();
		if(last > m_animationTimeLast)
			m_animationTimeLast = last;
	}
}
void Animation::setTimeCount(DWORD dwCount)
{
	m_animationTime = dwCount;
}
void Animation::setTimeStart(DWORD dwCount)
{
	m_animationTimeStart = dwCount;
}
void Animation::setTimeLength(DWORD dwCount)
{
	m_animationTimeLength = dwCount;
}
DWORD Animation::getTimeCount() const
{
	return m_animationTime;
}
DWORD Animation::getTimeStart() const
{
	return m_animationTimeStart;
}
DWORD Animation::getTimeLength() const
{
	return m_animationTimeLength;
}
void Animation::setTimeChange(DWORD count)
{
	m_animationTimeChange = count;
}
DWORD Animation::getTimeChange() const
{
	return m_animationTimeChange;
}
void Animation::setTimeChangeWork(DWORD count)
{
	m_animationTimeChangeWork = count;
}
DWORD Animation::getTimeChangeWork() const
{
	return m_animationTimeChangeWork;
}
LPCSTR Animation::getName() const
{
	return m_name.c_str();
}
void Animation::setLoop(bool loop)
{
	m_loop = loop;
}
bool Animation::isAnimation() const
{
	if(m_loop)
		return true;
	if(m_animationTime >=  (DWORD)getLastTime())
		return false;
	return true;
}
DWORD Animation::getLastTime() const
{
	return m_animationTimeLast;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Camera3D
// 3Dカメラ系
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Camera3D::Camera3D()
{
	m_depth = 0.0f;
	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
	m_rotX = 0.0f;
	m_rotY = 0.0f;
	m_rotZ = 0.0f;
	m_deviceWidth = 0;
	m_deviceHeight = 0;

}
void Camera3D::resetPoint()
{
	//setAngle(Device::getTargetWidth(),Device::getTargetHeight());
	m_y = m_depth;

}

void Camera3D::getVector(NVector* pVect,FLOAT fX,FLOAT fY)
{
	NMatrix mat;
	mat = m_matView;
	mat.setInverse();
	NVector vect = {fX,fY,getBaseDepth(),0};
	vect.transformNormal(mat);
	*pVect = vect.normal();
}


bool Camera3D::setAngle(UINT uDeviceWidth,UINT uDeviceHeight)
{
	NMatrix ext = NMatrix().setTranslation(-m_x,-m_y,-m_z);
	ext *= NMatrix().setRotationZ(NPI * -m_rotZ / 180.0f);
	ext *= NMatrix().setRotationY(NPI * -m_rotY / 180.0f);
	ext *= NMatrix().setRotationX(NPI * -m_rotX / 180.0f);
	if(m_deviceWidth != uDeviceWidth || m_deviceHeight != uDeviceHeight)
	{
		//視野角度
		FLOAT fViewAngle = 45;
		//デバイスサイズの半分を各頂点に
		FLOAT fWidth = (FLOAT)uDeviceWidth/2;
		FLOAT fHeight = (FLOAT)uDeviceHeight/2;
		//アスペクト比(高さを1としたときの幅)
		FLOAT fAspect = (FLOAT)uDeviceWidth/uDeviceHeight;
		//視野をZ=0でデバイスの幅と高さに合わせる
		FLOAT fFovy = fViewAngle*NPI/180.0f;
		//奥行き
		FLOAT fDepth = (FLOAT)fHeight/(FLOAT)tan(fFovy/2.0f);


		m_matViewBase.setLookAtLH(
			NVector::set( fWidth,fHeight,fDepth,0),
			NVector::set( fWidth,fHeight, 0.0f,0),
			NVector::set( 0.0f, -1.0f, 0.0f,0 ) );

		if (isLeftHand())
		{
			//ビューの設定
			m_matBase3D.setLookAtLH(
				NVector::set(0.0f, 0.0f, 0, 0),
				NVector::set(0.0f, -fDepth, 0, 0),
				NVector::set(0.0f, 0.0f, 1.0f, 0));
			//奥行きに対する比率を調整
			m_matProjection.setPerspectiveFovLH(fFovy, fAspect, 50.0f, 20000.0f);
		}
		else
		{
			//ビューの設定
			m_matBase3D.setLookAtRH(
				NVector::set(0.0f, 0.0f, 0, 0),
				NVector::set(0.0f, fDepth, 0, 0),
				NVector::set(0.0f, 0.0f, 1.0f, 0));
			//奥行きに対する比率を調整
			m_matProjection.setPerspectiveFovRH(fFovy, fAspect, 50.0f, 20000.0f);
		}

		m_depth = fDepth;
		m_deviceWidth = uDeviceWidth;
		m_deviceHeight = uDeviceHeight;
	}
	getExt() = ext;
	m_matView = ext * m_matBase3D;
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VectorObject
// DirectX - 図形用頂点データ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
VectorObject::VectorObject()
{
}
void VectorObject::drawBox(FLOAT x, FLOAT y, FLOAT width, FLOAT height, DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_BOX;
	vd.x = x;
	vd.y = y;
	vd.z = 0.0f;
	vd.width = width;
	vd.height = height;
	vd.depth = 0.0f;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}
void VectorObject::drawLine(FLOAT x, FLOAT y, FLOAT x2, FLOAT y2, DWORD color, FLOAT bold)
{
	VectorData vd;
	vd.cmd = VECTOR_LINE;
	if (((x - x2) - (y - y2)) < 0)
	{
		vd.x = x;
		vd.y = y;
		vd.z = 0.0f;
		vd.x2 = x2;
		vd.y2 = y2;
		vd.z2 = 0.0f;
	}
	else
	{
		vd.x = x2;
		vd.y = y2;
		vd.z = 0.0f;
		vd.x2 = x;
		vd.y2 = y;
		vd.z2 = 0.0f;
	}
	vd.color1 = color;
	vd.bold = bold;
	m_vectorData.push_back(vd);
}

void VectorObject::drawTriangle(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x1;
	vd.y = y1;
	vd.z = 0.0f;
	vd.x2 = x2;
	vd.y2 = y2;
	vd.z2 = 0.0f;
	vd.x3 = x3;
	vd.y3 = y3;
	vd.z3 = 0.0f;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}
void VectorObject::drawLineBox(FLOAT x, FLOAT y, FLOAT width, FLOAT height, DWORD color, FLOAT bold)
{
	drawLine(x, y, x + width - bold, y, color, bold);
	drawLine(x, y + bold, x, y + height - bold, color, bold);
	drawLine(x + width - bold, y + bold, x + width - bold, y + height - bold, color, bold);
	drawLine(x + bold, y + height - bold, x + width - bold, y + height - bold, color, bold);
}
INT VectorObject::getVertexCount() const
{
	INT count = 0;
	std::list<VectorData>::const_iterator it;
	for (it = m_vectorData.begin(); it != m_vectorData.end(); ++it)
	{
		switch (it->cmd)
		{
		case VECTOR_TRIANGLE:
			count += 3;
			break;
		case VECTOR_LINE:
			if (it->x != it->x2 && it->y != it->y2)
			{
				count += 12;
				break;
			}
		case VECTOR_QUADRANGLE:
		case VECTOR_BOX:
			count += 6;
			break;
		}
	}
	return count;
}
VERTEXVECTOR* VectorObject::getVertexData()
{
	INT index = 0;
	INT vcount = getVertexCount();
	VERTEXVECTOR* vv = NEW VERTEXVECTOR[vcount];


	std::list<VectorData>::const_iterator it;
	for (it = m_vectorData.begin(); it != m_vectorData.end(); ++it)
	{
		switch (it->cmd)
		{
		case VECTOR_TRIANGLE:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x2;
			vv[index].vectPosition.y = it->y2;
			vv[index].vectPosition.z = it->z2;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x3;
			vv[index].vectPosition.y = it->y3;
			vv[index].vectPosition.z = it->z3;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			break;
		case VECTOR_BOX:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;

			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x + it->width;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y + it->height;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			break;
		case VECTOR_QUADRANGLE:
			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x2;
			vv[index].vectPosition.y = it->y2;
			vv[index].vectPosition.z = it->z2;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x4;
			vv[index].vectPosition.y = it->y4;
			vv[index].vectPosition.z = it->z4;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;

			vv[index].vectPosition.x = it->x;
			vv[index].vectPosition.y = it->y;
			vv[index].vectPosition.z = it->z;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x4;
			vv[index].vectPosition.y = it->y4;
			vv[index].vectPosition.z = it->z4;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			vv[index].vectPosition.x = it->x3;
			vv[index].vectPosition.y = it->y3;
			vv[index].vectPosition.z = it->z3;
			vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
			vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
			vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
			vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
			index++;
			break;
		case VECTOR_LINE:
		{
			FLOAT x1 = it->x;
			FLOAT y1 = it->y;
			FLOAT z1 = it->z;
			FLOAT x2 = it->x2;
			FLOAT y2 = it->y2;
			FLOAT z2 = it->z2;
			FLOAT width = x2 - x1;
			FLOAT height = y2 - y1;
			FLOAT bold = it->bold;

			if (width == 0 || height == 0)
			{
				if (x1 > x2)
					x1 += bold;
				else
					x2 += bold;
				if (y1 > y2)
					y1 += bold;
				else
					y2 += bold;

				vv[index].vectPosition.x = x1;
				vv[index].vectPosition.y = y1;
				vv[index].vectPosition.z = z2;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;
				vv[index].vectPosition.x = x1;
				vv[index].vectPosition.y = y2;
				vv[index].vectPosition.z = z2;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;
				vv[index].vectPosition.x = x2;
				vv[index].vectPosition.y = y1;
				vv[index].vectPosition.z = z1;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;

				vv[index].vectPosition.x = x1;
				vv[index].vectPosition.y = y2;
				vv[index].vectPosition.z = z2;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;
				vv[index].vectPosition.x = x2;
				vv[index].vectPosition.y = y1;
				vv[index].vectPosition.z = z1;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;
				vv[index].vectPosition.x = x2;
				vv[index].vectPosition.y = y2;
				vv[index].vectPosition.z = z1;
				vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
				vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
				vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
				vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
				index++;
			}
			else
			{
				if (x1 > x2)
				{
					x1 = it->x2;
					y1 = it->y2;
					x2 = it->x;
					y2 = it->y;
				}
				if (y1 > y2)
				{
					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x1 + bold;
					vv[index].vectPosition.y = y1 + bold;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x1 + bold;
					vv[index].vectPosition.y = y1 + bold;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1 + bold;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
				}
				else
				{
					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x1 + bold;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2 + bold;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;

					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x2;
					vv[index].vectPosition.y = y2 + bold;
					vv[index].vectPosition.z = z2;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
					vv[index].vectPosition.x = x1;
					vv[index].vectPosition.y = y1 + bold;
					vv[index].vectPosition.z = z1;
					vv[index].color.x = ((LPBYTE)&it->color1)[2] / 255.0f;
					vv[index].color.y = ((LPBYTE)&it->color1)[1] / 255.0f;
					vv[index].color.z = ((LPBYTE)&it->color1)[0] / 255.0f;
					vv[index].color.w = ((LPBYTE)&it->color1)[3] / 255.0f;
					index++;
				}
			}

		}
			break;
		}
	}
	return vv;
}
void VectorObject::add(VectorObject* vo)
{
	m_vectorData.insert(m_vectorData.end(), vo->m_vectorData.begin(), vo->m_vectorData.end());
}
void VectorObject3D::drawTriangle(FLOAT x1, FLOAT y1, FLOAT z1, FLOAT x2, FLOAT y2, FLOAT z2, FLOAT x3, FLOAT y3, FLOAT z3, DWORD color)
{
	VectorData vd;
	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x1;
	vd.y = y1;
	vd.z = z1;
	vd.x2 = x2;
	vd.y2 = y2;
	vd.z2 = z2;
	vd.x3 = x3;
	vd.y3 = y3;
	vd.z3 = z3;
	vd.color1 = color;
	m_vectorData.push_back(vd);
}

void VectorObject3D::drawQuadrangle(FLOAT x, FLOAT y, FLOAT z, FLOAT x2, FLOAT y2, FLOAT z2, DWORD color, FLOAT bold)
{
	//VectorData vd;
	//vd.cmd = VECTOR_BOX;

	NVector v;
	v.x = x - x2;
	v.y = y - y2;
	v.z = z - z2;
	v = v.normal3();

	FLOAT b;
	b = bold*0.5f;
	v *= b;

	VectorData vd;


	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x;
	vd.y = y;
	vd.z = z;
	vd.x2 = x - v.y;
	vd.y2 = y + v.x;
	vd.z2 = z;
	vd.x3 = x - v.z;
	vd.y3 = y;
	vd.z3 = z + v.x;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_TRIANGLE;
	vd.x = x;
	vd.y = y;
	vd.z = z;
	vd.x2 = x - v.y;
	vd.y2 = y + v.x;
	vd.z2 = z;
	vd.x3 = x;
	vd.y3 = y - v.z;
	vd.z3 = z + v.y;
	vd.color1 = color;
	m_vectorData.push_back(vd);
	/*
	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x-v.y;
	vd.y = y+v.x;
	vd.z = z;
	vd.x2 = x+v.y;
	vd.y2 = y-v.x;
	vd.z2 = z;
	vd.x3 = x2-v.y;
	vd.y3 = y2+v.x;
	vd.z3 = z2;
	vd.x4 = x2+v.y;
	vd.y4 = y2-v.x;
	vd.z4 = z2;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x-v.z;
	vd.y = y;
	vd.z = z+v.x;
	vd.x2 = x+v.z;
	vd.y2 = y;
	vd.z2 = z-v.x;
	vd.x3 = x2-v.z;
	vd.y3 = y2;
	vd.z3 = z2+v.x;
	vd.x4 = x2+v.z;
	vd.y4 = y2;
	vd.z4 = z2-v.x;
	vd.color1 = color;
	m_vectorData.push_back(vd);

	vd.cmd = VECTOR_QUADRANGLE;
	vd.x = x;
	vd.y = y-v.z;
	vd.z = z+v.y;
	vd.x2 = x;
	vd.y2 = y+v.z;
	vd.z2 = z-v.y;
	vd.x3 = x2;
	vd.y3 = y2-v.z;
	vd.z3 = z2+v.y;
	vd.x4 = x2;
	vd.y4 = y2+v.z;
	vd.z4 = z2-v.y;
	vd.color1 = color;
	m_vectorData.push_back(vd);
	*/
}


void VectorObject3D::drawLine(FLOAT x, FLOAT y, FLOAT z, FLOAT x2, FLOAT y2, FLOAT z2, DWORD color, FLOAT bold)
{
	VectorData vd;
	vd.cmd = VECTOR_LINE;
	if (((x - x2) - (y - y2)) < 0)
	{
		vd.x = x;
		vd.y = y;
		vd.z = z;
		vd.x2 = x2;
		vd.y2 = y2;
		vd.z2 = z2;
	}
	else
	{
		vd.x = x2;
		vd.y = y2;
		vd.z = z2;
		vd.x2 = x;
		vd.y2 = y;
		vd.z2 = z;
	}
	vd.color1 = color;
	vd.bold = bold;
	m_vectorData.push_back(vd);
}

void VectorObject3D::drawLineBox(FLOAT x, FLOAT y, FLOAT z, FLOAT width, FLOAT height, FLOAT depth, DWORD color, FLOAT bold)
{
	drawLine(x, y, z, x + width - bold, y, z, color, bold);
	drawLine(x, y + bold, z, x, y + height - bold, z, color, bold);
	drawLine(x + width - bold, y + bold, z, x + width - bold, y + height - bold, z, color, bold);
	drawLine(x + bold, y + height - bold, z, x + width - bold, y + height - bold, z, color, bold);

	drawLine(x, y, z + depth, x + width - bold, y, z + depth, color, bold);
	drawLine(x, y + bold, z + depth, x, y + height - bold, z + depth, color, bold);
	drawLine(x + width - bold, y + bold, z + depth, x + width - bold, y + height - bold, z + depth, color, bold);
	drawLine(x + bold, y + height - bold, z + depth, x + width - bold, y + height - bold, z + depth, color, bold);

	drawLine(x, y, z, x, y, z + depth, color, bold);
	drawLine(x, y + height - bold, z, x, y + height - bold, z + depth, color, bold);
	drawLine(x + width - bold, y, z, x + width - bold, y + height - bold, z + depth, color, bold);
	//drawLine(x+bold,y+height-bold,z+depth,x+width-bold,y+height-bold,z+depth,color,bold);
}

}
