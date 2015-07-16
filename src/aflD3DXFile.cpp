#include <windows.h>
#include <rmxftmpl.h> //RMテンプレート
#include <rmxfguid.h>
#include "aflStd.h"
#include "aflD3DXFile.h"

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"libcp.lib\"")

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

#pragma warning(disable : 4996)

namespace AFL{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFileBinaryReader
// X-File用バイナリデータ読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
XFileBinaryReader::XFileBinaryReader(LPCVOID data)
{
	m_dataAddress = (const BYTE*)data;
}
void XFileBinaryReader::set(LPCVOID data)
{
	m_dataAddress = (const BYTE*)data;
}
INT XFileBinaryReader::getInt()
{
	INT data = *(const INT*)m_dataAddress;
	m_dataAddress += sizeof(INT);
	return data;
}
DWORD XFileBinaryReader::getDWord()
{
	DWORD data = *(const DWORD*)m_dataAddress;
	m_dataAddress += sizeof(DWORD);
	return data;
	
}
FLOAT XFileBinaryReader::getFloat()
{
	FLOAT data = *(const FLOAT*)m_dataAddress;
	m_dataAddress += sizeof(FLOAT);
	return data;
}
DWORD XFileBinaryReader::getLast() const
{
	return *(const DWORD*)(m_dataAddress-sizeof(DWORD));
}
LPCVOID XFileBinaryReader::getAddress(INT dataSize)
{
	INT size = dataSize*getLast();
	LPCVOID address = m_dataAddress;
	m_dataAddress += size;
	return address;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFileLibraly
// X-File用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


XFileLibraly XFileInterface::m_xfilLibraly;

//-----------------------------------------------
//XFileLibraly::XFileLibraly()
// ---  動作  ---
// DLLを動的に呼び出す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
#define EXTEND_TEMPLATES \
        "xof 0303txt 0032\
		template BumpFilename {\
		 <E00F3D6B-705A-4B32-8CB6-623313225FA1>\
		 STRING filename;\
		}"
XFileLibraly::XFileLibraly()
{
	//ポインタ初期化
	m_xfile = NULL;

	if(D3DXFileCreate(&m_xfile) == DXFILE_OK)
	{
		//RMテンプレートの登録
		m_xfile->RegisterTemplates( (LPVOID)D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES );	
		m_xfile->RegisterTemplates( (LPVOID)XSKINEXP_TEMPLATES, (INT)strlen(XSKINEXP_TEMPLATES) );	
		m_xfile->RegisterTemplates( (LPVOID)EXTEND_TEMPLATES, (INT)strlen(EXTEND_TEMPLATES) );	
	}
}

//-----------------------------------------------
//XFileLibraly::~XFileLibraly()
// ---  動作  ---
// d3dxof.dllを解放
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
XFileLibraly::~XFileLibraly()
{
	//D3D解放
	if(m_xfile)
		m_xfile->Release();

}

//-----------------------------------------------
//ID3DXFile* XFileLibraly::getInterface() const
// ---  動作  ---
// IXFileの取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// ID3DXFileのポインタ
//-----------------------------------------------
ID3DXFile* XFileLibraly::getInterface() const
{
	return m_xfile;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFile
// X-File用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
XFile::XFile()
{
	m_setNormal = true;
	m_setUV = true;
	m_setTextureFullPath = true;
	m_setVColor = true;
	m_setSkin = true;
	m_setAnime = true;
}
FileObject* XFile::load(LPCWSTR fileName)
{
	FILE* pFile = _wfopen(fileName,L"rb");
	if(!pFile)
		return NULL;

	int length;
	fseek(pFile,0,SEEK_END);
	length = ftell(pFile);
	fseek(pFile,0,SEEK_SET);

	LPBYTE xfileData = NEW BYTE[length];
	fread(xfileData,length,1,pFile);
	fclose(pFile);

	FileObject* worldObject = loadMemory(xfileData,length);
	delete[] xfileData;

	if(!worldObject)
		return NULL;

	clearData(worldObject);
	return worldObject;
}
FileObject* XFile::load(LPCSTR fileName)
{
	return load(UCS2(fileName));
}


FileObject* XFile::loadMemory(LPBYTE fileData,INT length)
{
	//X-Fileインタフェイスの呼び出し
	ID3DXFile* xfileInterface = XFileInterface::getInterface();
	if(!xfileInterface)
		return NULL;

	//データ列挙用インタフェイス
	ID3DXFileEnumObject* xfileEnum = NULL;
	//メモリー情報の設定
	D3DXF_FILELOADMEMORY xfileMemory = {(LPVOID)fileData,length};

	//Xファイルのオープン
	HRESULT hr = xfileInterface->CreateEnumObject(&xfileMemory,D3DXF_FILELOAD_FROMMEMORY ,&xfileEnum);
	if(hr != D3D_OK)	//失敗
		return NULL;

	//データ保存用ワールドオブジェクト
	FileObject* worldObject = NEW FileObject;

	//列挙データ保存用
	ID3DXFileData* xfileData;
	//データ取得ループi
	INT i;
	for(i=0;xfileEnum->GetChild(i,&xfileData) == DXFILE_OK;i++)
	{
		GUID xfileGUID;
		//GUIDから情報の識別
		if(xfileData->GetType(&xfileGUID) == DXFILE_OK)
		{
			if(xfileGUID == TID_D3DRMFrame)
			{
				//フレーム情報の読み出し
				FrameData frameData;
				if(readFrame(xfileData,&frameData))
					worldObject->frame.push_back(frameData);
			}
			else if(xfileGUID == TID_D3DRMAnimationSet)
			{
				readAnimation(xfileData,worldObject);
			}
			else if(xfileGUID == TID_D3DRMMesh)
			{
				FrameData frameData;
				frameData.name = "MeshData";
				D3DXMatrixIdentity((D3DXMATRIX*)&frameData.matrix);
				readMesh(xfileData,&frameData.mesh);
				worldObject->frame.push_back(frameData);
			}
		}
		xfileData->Release();
	}
	xfileEnum->Release();
	return worldObject;
}
bool XFile::readFrame(ID3DXFileData* xfileData,FrameData* frameData)
{
	//フレーム名の読み出し
	CHAR buff[1024];
	SIZE_T dwSize = 1024;
	xfileData->GetName(buff,&dwSize);
	frameData->name = buff;

	//フレームデータ列挙用
	ID3DXFileData* xfileObject;
	//フレームデータ読み出しループ
	INT i;
	for(i=0;xfileData->GetChild(i,&xfileObject) == DXFILE_OK;i++)
	{
		//オブジェクト内のデータ読み出し用
		//データ取得
		GUID xfileGUID;
		//データの種類識別
		if(xfileObject->GetType(&xfileGUID) == DXFILE_OK)
		{
			if(xfileGUID == TID_D3DRMFrameTransformMatrix)
			{
				//ローカル行列
				SIZE_T dwSize;
				NMatrix* pmat;
				xfileObject->Lock(&dwSize,(LPCVOID*)&pmat);
				frameData->matrix = *pmat;
				xfileObject->Unlock();
			}
			else if(xfileGUID == TID_D3DRMFrame)
			{
				//子フレーム
				FrameData frameChild;
				if(readFrame(xfileObject,&frameChild))
					frameData->frameChild.push_back(frameChild);
			}
			else if(xfileGUID == TID_D3DRMMesh)
			{
				//メッシュの読み出し
				readMesh(xfileObject,&frameData->mesh);
			}

		}
		xfileObject->Release();

	}
	return true;
}

bool XFile::readMesh(ID3DXFileData* xfileData,MeshData* meshData)
{
	LPBYTE data;
	SIZE_T size;
	xfileData->Lock(&size,(LPCVOID*)&data);

	INT i;

	XFileBinaryReader reader(data);
	//頂点サイズ
	DWORD vertexSize = reader.getDWord();
	//頂点データ
	NVector3* vertexData = (NVector3*)reader.getAddress(sizeof(FLOAT)*3);
	//インデックスサイズ
	DWORD indexSize = reader.getDWord();
	//インデックスデータ
	LPDWORD indexData = (LPDWORD)reader.getAddress();


	//頂点データの保存
	meshData->vertexData.reserve(vertexSize);
	for(i=0;i<(INT)vertexSize;i++)
		meshData->vertexData.push_back(vertexData[i]);
	//インデックスデータの保存
	meshData->vertexIndex.resize(indexSize);
	for(i=0;i<(INT)indexSize;i++)
	{
		INDEX4* index4 = &meshData->vertexIndex[i];
		index4->type = *indexData;
		index4->data[0] = indexData[1]; 
		index4->data[1] = indexData[2]; 
		index4->data[2] = indexData[3];
		if(*indexData == 3)
			indexData += 4;
		else if(*indexData == 4)
		{
			index4->data[3] = indexData[4];
			indexData += 5;
		}
		else break; //想定外
	}
	xfileData->Unlock();

	//サブデータの読み出し
	ID3DXFileData* xfileObject;
	for(i=0;xfileData->GetChild(i,&xfileObject) == DXFILE_OK;i++)
	{
		GUID xfileGUID;
		if(xfileObject->GetType(&xfileGUID) == DXFILE_OK)
		{
			if(xfileGUID == TID_D3DRMMeshNormals)
			{
				//法線
				xfileObject->Lock(&size,(LPCVOID*)&data);

				reader.set(data);
				//法線サイズ
				DWORD normalSize = reader.getDWord();
				//法線データ
				NVector3* normalData = (NVector3*)reader.getAddress(sizeof(NVector3));
				//法線インデックスサイズ
				DWORD indexSize = reader.getDWord();;
				//法線インデックスデータ
				LPDWORD indexData = (LPDWORD)reader.getAddress();;

				INT i;
				//法線データの保存
				meshData->normalData.reserve(normalSize);
				for(i=0;i<(INT)normalSize;i++)
					meshData->normalData.push_back(normalData[i]);
				//法線インデックスデータの保存
				meshData->normalIndex.resize(indexSize);
				for(i=0;i<(INT)indexSize;i++)
				{
					INDEX4* index4 = &meshData->normalIndex[i];
					index4->type = *indexData;
					index4->data[0] = indexData[1]; 
					index4->data[1] = indexData[2]; 
					index4->data[2] = indexData[3];
					if(*indexData == 3)
						indexData += 4;
					else if(*indexData == 4)
					{
						index4->data[3] = indexData[4];
						indexData += 5;
					}
					else break; //想定外
				}
				xfileObject->Unlock();
			}
			else if(xfileGUID == TID_D3DRMMeshMaterialList)
			{
				//マテリアル
				xfileObject->Lock(&size,(LPCVOID*)&data);
				reader.set(data);
				//マテリアル数
				DWORD materialSize = reader.getDWord();
				//マテリアルインデックス数
				DWORD materialIndexSize = reader.getDWord();
				//マテリアルインデックスデータ
				LPDWORD materialIndex = (LPDWORD)reader.getAddress();

				//マテリアルフェイス対応インデックスの読み出し
				meshData->materialIndex.resize(materialIndexSize);
				CopyMemory(&meshData->materialIndex[0],materialIndex,
					sizeof(DWORD)*materialIndexSize);

				//マテリアルデータ領域の確保と読み出し
				meshData->materialData.reserve(materialSize);
				readMaterial(xfileObject,meshData);

				xfileObject->Unlock();
			}
			else if(xfileGUID == TID_D3DRMMeshTextureCoords)
			{
				//テクスチャUV座標
				xfileObject->Lock(&size,(LPCVOID*)&data);
				DWORD textureSize = *(LPDWORD)data;
				data += sizeof(DWORD);
				TEXTUREUV* uvData = (TEXTUREUV*)data;
				meshData->uvData.resize(textureSize);
				CopyMemory(&meshData->uvData[0],uvData,
					sizeof(TEXTUREUV)*textureSize);
				meshData->uvIndex = meshData->vertexIndex;
				xfileObject->Unlock();					
			}
			else if(xfileGUID == TID_D3DRMMeshVertexColors)
			{
				//頂点カラー
				xfileObject->Lock(&size,(LPCVOID*)&data);
				reader.set(data);

				DWORD colorSize = reader.getDWord();
				meshData->colorData.resize(colorSize);
				INT i;
				for(i=0;i<(INT)colorSize;i++)
				{
					reader.getDWord();
					meshData->colorData[i].r = reader.getFloat();
					meshData->colorData[i].g = reader.getFloat();
					meshData->colorData[i].b = reader.getFloat();
					meshData->colorData[i].a = reader.getFloat();
				}
				meshData->colorIndex = meshData->vertexIndex;
				xfileObject->Unlock();
			}
			else if(xfileGUID == DXFILEOBJ_SkinWeights)
			{
				//ボーン
				xfileObject->Lock(&size,(LPCVOID*)&data);

				BoneData boneData;
				boneData.name = (LPCSTR)data;
				data += boneData.name.length()+1;//sizeof(LPCSTR*);
				DWORD boneCount = *(LPDWORD)data;
				data += sizeof(DWORD);
				LPDWORD boneIndex = (LPDWORD)data;
				data += sizeof(DWORD) * boneCount;
				PFLOAT boneWeights = (PFLOAT)data;
				data += sizeof(FLOAT) * boneCount;
				boneData.matrix = *(NMatrix*)data;
				INT i;
				for(i=0;i<(INT)boneCount;i++)
					boneData.weight[boneIndex[i]] = boneWeights[i];
				meshData->boneData.push_back(boneData);
				xfileObject->Unlock();
			}
		}
		xfileObject->Release();
	}
	return true;
}
bool XFile::readMaterial(ID3DXFileData* xfileData,MeshData* meshData)
{
	INT i;
	LPBYTE data;
	SIZE_T size;
	ID3DXFileData* xfileObject;

	for(i=0;xfileData->GetChild(i,&xfileObject) == DXFILE_OK;i++)
	{
		GUID xfileGUID;
		if(xfileObject->GetType(&xfileGUID) == DXFILE_OK)
		{
			if(xfileGUID == TID_D3DRMMaterial)
			{
				MaterialData materialData;
				xfileObject->Lock(&size,(LPCVOID*)&data);
				PFLOAT pfMaterial = (PFLOAT)data;

				Material material = 
				{
					COLOR4(pfMaterial[0],pfMaterial[1],pfMaterial[2],pfMaterial[3]),
					COLOR4(pfMaterial[0]*0.3f,pfMaterial[1]*0.3f,pfMaterial[2]*0.3f,1.0f),
					COLOR4(pfMaterial[5],pfMaterial[6],pfMaterial[7],0.0f),
					COLOR4(pfMaterial[8],pfMaterial[9],pfMaterial[10],0.0f),
					pfMaterial[4]

				};
				materialData.material = material;
				xfileObject->Unlock();

				ID3DXFileData* xfileName;
				INT j;
				for(j=0;xfileObject->GetChild(j,&xfileName) == DXFILE_OK;j++)
				{
					GUID pGuid;
					if(xfileName->GetType(&pGuid) == DXFILE_OK)
					{
						xfileName->Lock(&size,(LPCVOID*)&data);
						materialData.name.push_back((LPCSTR)data);
						xfileName->Unlock();
					}
					xfileName->Release();
				}
				meshData->materialData.push_back(materialData);
			}
		}
		xfileObject->Release();
	}
	return true;
}
bool XFile::readAnimation(ID3DXFileData* xfileData,FileObject* worldObject)
{
	//アニメーションセット名を読み出し
	CHAR buff[256]; 
	SIZE_T size = 256;
	xfileData->GetName(buff,&size);
	ANIMATIONSET* animationSet = &worldObject->anime[buff];

	ID3DXFileData* xfileObject;

	INT i;
	for(i=0;xfileData->GetChild(i,&xfileObject) == DXFILE_OK;i++)
	{
		String animeFrame;
		AnimationKey animationKey;
		GUID xfileGUID;

		if(xfileObject->GetType(&xfileGUID) == DXFILE_OK)
		{
			//アニメーションデータ
			if(xfileGUID == TID_D3DRMAnimation)
			{
				ID3DXFileData* xfileAnimation;
				INT j;
				for(j=0;xfileObject->GetChild(j,&xfileAnimation) == DXFILE_OK;j++)
				{
					GUID xfileGUID;
					if(xfileAnimation->GetType(&xfileGUID) == DXFILE_OK)
					{
						//アニメーションキーの読み出し
						if(xfileGUID == TID_D3DRMAnimationKey)
						{
							LPBYTE pData;
							SIZE_T dwSize;
							xfileAnimation->Lock(&dwSize,(LPCVOID*)&pData);
							DWORD dwType = *(LPDWORD)pData;
							pData += sizeof(DWORD);
							DWORD dwAnimation = *(LPDWORD)pData;
							pData += sizeof(DWORD);

							INT i;
							for(i=0;i<(INT)dwAnimation;i++)
							{
								//キーフレーム
								DWORD dwTime = *(LPDWORD)pData;
								pData += sizeof(DWORD);
								//データサイズ
								DWORD dwSize = *(LPDWORD)pData;
								pData += sizeof(DWORD);
						
								if(dwType == 4)
								{
									//行列
									animationKey.matrix[dwTime] = *(NMatrix*)pData;
								}
								else if(dwType == 0)
								{
									//回転
									PFLOAT pRot = (PFLOAT)pData;
									NVector q;
									q.x = pRot[1];
									q.y = pRot[2];
									q.z = pRot[3];
									q.w = pRot[0];
									animationKey.rotation[dwTime] = q;
								}
								else if(dwType == 1)
								{
									//スケール
									animationKey.scale[dwTime] = *(NVector*)pData;
								}
								else if(dwType == 2)
								{
									//ポジション
									animationKey.position[dwTime] = *(NVector*)pData;

								}
								pData += sizeof(FLOAT)*dwSize; 
							}
							xfileAnimation->Unlock();
						}
						else if(xfileAnimation->IsReference())
						{
							SIZE_T dwSize=256;
							CHAR cBuff[256]; 
							xfileAnimation->GetName(cBuff,&dwSize);
							animeFrame = cBuff;
						}
					}
					xfileAnimation->Release();
				}
			}
			xfileObject->Release();
		}
		animationSet->keys[animeFrame] = animationKey;
	}
	return true;
}

struct VERTEXINDEX
{
	bool operator < ( CONST VERTEXINDEX& index) const
	{
		if(vertex < index.vertex)
			return true;
		if(vertex == index.vertex)
		{
			if(normal < index.normal)
				return true;
			if(normal == index.normal)
			{
				if(uv < index.uv)
					return true;
				if(uv == index.uv && color < index.color)
					return true;
			}
		}
		return false;
	}
	DWORD vertex;
	DWORD normal;
	DWORD uv;
	DWORD color;
};
bool XFile::convertD3DX(MeshData* meshData,bool flag3DX)
{
	INT i,j;
	//フラグに応じて不要データの削除
	if(!m_setNormal)
	{
		meshData->normalData.clear();
		meshData->normalIndex.clear();
	}
	if(!m_setSkin)
		meshData->boneData.clear();
	if(!m_setUV)
	{
		meshData->uvData.clear();
		meshData->uvIndex.clear();
	}
	if(!m_setVColor)
	{
		meshData->colorData.clear();
		meshData->colorIndex.clear();
	}
	if(!m_setTextureFullPath)
	{
		INT materialCount = (INT)meshData->materialData.size();
		for(i=0;i<materialCount;i++)
		{
			std::list<String>::iterator it;
			for(it=meshData->materialData[i].name.begin();it!=meshData->materialData[i].name.end();++it)
			{
				INT c = (INT)it->rfind('\\');
				if(c != -1)
				{
					*it = it->substr(c+1).c_str();
				}
			}
		}
	}
	if(!flag3DX)
		return true;

	std::map<VERTEXINDEX,INT> indexMap;
	INT vertexCount = (INT)meshData->vertexIndex.size();
	INT normalCount = (INT)meshData->normalIndex.size();
	INT uvCount = (INT)meshData->uvIndex.size();
	INT colorCount = (INT)meshData->colorIndex.size();
	INT boneCount = (INT)meshData->boneData.size();
	if(!vertexCount)
		return false;
	if(boneCount == 0)
		normalCount = 0;
	if(colorCount == 0 && uvCount == 0 && normalCount == 0)
		return true;
	for(i=0;i<vertexCount;i++)
	{
		INT indexType = meshData->vertexIndex[i].type;
		for(j=0;j<indexType;j++)
		{
			VERTEXINDEX indexData;
			indexData.vertex = meshData->vertexIndex[i].data[j];
			indexData.normal = normalCount ? meshData->normalIndex[i].data[j] : 0;
			indexData.uv = uvCount ? meshData->uvIndex[i].data[j] : 0;
			indexData.color = colorCount ? meshData->colorIndex[i].data[j] : 0;
			
			INT index;
			std::map<VERTEXINDEX,INT>::iterator itIndex = indexMap.find(indexData);
			if(itIndex == indexMap.end())
			{
				index = (INT)indexMap.size(); 
				indexMap[indexData] = index;
			}
			else
			{
				index = (*itIndex).second;
			}
			meshData->vertexIndex[i].data[j] = index;
			if(uvCount)
				meshData->uvIndex[i].data[j] = index;
			if(colorCount)
				meshData->colorIndex[i].data[j] = index;
		}
		std::vector<BoneData>::iterator itBone;
	}
	NVector3* vertexData =  &meshData->vertexData[0];
	TEXTUREUV* uvData = uvCount ? &meshData->uvData[0] : NULL;
	COLOR4* colorData = colorCount ? &meshData->colorData[0] : NULL;
	BoneData* boneData = boneCount ? &meshData->boneData[0] : NULL;

	INT mapCount = (INT)indexMap.size();
	std::vector<NVector3> vertex(mapCount);
	std::vector<TEXTUREUV> uv(mapCount);
	std::vector<COLOR4> color(mapCount);
	std::vector<BoneData> bone(boneCount);

	std::multimap<INT,INT> indexConvertMap;
	std::map<VERTEXINDEX,INT>::iterator itIndex;
	for(itIndex=indexMap.begin();itIndex!=indexMap.end();++itIndex)
	{
		INT index = (*itIndex).second;
		const VERTEXINDEX* indexData = &(*itIndex).first;
		vertex[index] = vertexData[indexData->vertex];
		if(uvCount)
			uv[index] = uvData[indexData->uv];
		if(colorCount)
			color[index] = colorData[indexData->color];
		indexConvertMap.insert(std::pair<INT,INT>(indexData->vertex,index));
	}
	//ボーン再配置
	for(i=0;i<boneCount;i++)
	{
		std::map<INT,FLOAT> boneOptMap;
		std::map<INT,FLOAT>::iterator itBoneData;
		for(itBoneData=boneData[i].weight.begin();itBoneData!=boneData[i].weight.end();++itBoneData)
		{
			std::multimap<INT,INT>::iterator itConvert;
			itConvert = indexConvertMap.find(itBoneData->first);
			for(;itConvert != indexConvertMap.end() && (*itConvert).first == itBoneData->first;++itConvert)
			{
				boneOptMap[(*itConvert).second] = itBoneData->second;
			}
		}
		boneData[i].weight = boneOptMap;
	}


	meshData->vertexData = vertex;
	if(uvCount)
		meshData->uvData = uv;
	if(colorCount)
		meshData->colorData = color;
	return true;
}
bool XFile::convertD3DX(FrameData* frameData,bool flag3DX)
{
	convertD3DX(&frameData->mesh,flag3DX);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		convertD3DX(&*itFrame,flag3DX);
	}	
	return true;
}
FileObject* XFile::convertD3DX(FileObject* worldObject,bool flag3DX)
{
	//出力用にD3DX仕様へ変換
	FileObject* fileObject = NEW FileObject;
	*fileObject = *worldObject;
	//アニメーション削除
	if(!m_setAnime)
		fileObject->anime.clear();

	fileObject->optimize();

	std::list<FrameData>::iterator itFrame;
	for(itFrame=fileObject->frame.begin();itFrame!=fileObject->frame.end();++itFrame)
	{
		convertD3DX(&*itFrame,flag3DX);
	}	
	return fileObject;
}
bool XFile::clearData(MeshData* meshData)
{
	//フラグに応じて不要データの削除
	if(!m_setNormal)
	{
		meshData->normalData.clear();
		meshData->normalIndex.clear();
	}
	if(!m_setSkin)
		meshData->boneData.clear();
	if(!m_setUV)
	{
		meshData->uvData.clear();
		meshData->uvIndex.clear();
	}
	if(!m_setVColor)
	{
		meshData->colorData.clear();
		meshData->colorIndex.clear();
	}
	return true;
}
bool XFile::clearData(FrameData* frameData)
{
	clearData(&frameData->mesh);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		clearData(&*itFrame);
	}	
	return true;
}
void XFile::clearData(FileObject* fileObject)
{
	//アニメーション削除
	if(!m_setAnime)
		fileObject->anime.clear();
	
	std::list<FrameData>::iterator itFrame;
	for(itFrame=fileObject->frame.begin();itFrame!=fileObject->frame.end();++itFrame)
	{
		clearData(&*itFrame);
	}	
}
bool XFile::save(LPCSTR fileName,FileObject* worldObject,DXFILEFORMAT format,bool flag3DX)
{
	return save(UCS2(fileName),worldObject,format,flag3DX);
}

bool XFile::save(LPCWSTR fileName,FileObject* worldObject,DXFILEFORMAT format,bool flag3DX)
{
	HRESULT hr;
	//X-Fileインタフェイスの呼び出し
	ID3DXFile* xfileInterface = XFileInterface::getInterface();
	if(!xfileInterface)
		return NULL;


	LPCSTR templateData =
		"xof 0303txt 0032\
		template AnimTicksPerSecond \
		{ \
			<9E415A43-7BA6-4a73-8743-B73D47E88476> \
			DWORD AnimTicksPerSecond; \
		} \
		template XSkinMeshHeader {\
		 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>\
		 WORD nMaxSkinWeightsPerVertex;\
		 WORD nMaxSkinWeightsPerFace;\
		 WORD nBones;\
		}\
		template SkinWeights {\
		 <6f0d123b-bad2-4167-a0d0-80224f25fabb>\
		 STRING transformNodeName;\
		 DWORD nWeights;\
		 array DWORD vertexIndices[nWeights];\
		 array FLOAT weights[nWeights];\
		 Matrix4x4 matrixOffset;\
		}";

	hr = xfileInterface->RegisterTemplates((LPVOID)templateData,(DWORD)strlen(templateData));
	if(hr != DXFILE_OK)	//失敗
		return false;
	ID3DXFileSaveObject* xfileSave;
	hr = xfileInterface->CreateSaveObject(fileName,D3DXF_FILESAVE_TOWFILE,format,&xfileSave);
	if(hr != DXFILE_OK)	//失敗
		return false;

	FileObject* fileObject = convertD3DX(worldObject,flag3DX);


	if(m_setAnime)
	{
		static DWORD tickTime = 4800;
		ID3DXFileSaveData* xfileData;
		xfileSave->AddDataObject(DXFILEOBJ_AnimTicksPerSecond ,NULL,NULL,sizeof(DWORD),&tickTime,&xfileData);
		xfileData->Release();
	}

	//フレームデータの保存
	std::list<FrameData>::iterator itFrame;
	for(itFrame=fileObject->frame.begin();itFrame!=fileObject->frame.end();++itFrame)
	{
		ID3DXFileSaveData* xfileData = writeFrame(xfileSave,NULL,&*itFrame);
		if(xfileData)
			xfileData->Release();
	}
	//アニメーションセットの保存
	std::map<String,ANIMATIONSET>::iterator itAnime;
	for(itAnime=fileObject->anime.begin();itAnime!=fileObject->anime.end();++itAnime)
	{
		ID3DXFileSaveData* xfileData = writeAnimation(xfileSave,itAnime->first.c_str(),&itAnime->second);
		if(xfileData)
			xfileData->Release();
	}
	xfileSave->Save();
	xfileSave->Release();

	delete fileObject;
	return true;
}

ID3DXFileSaveData* XFile::writeAnimation(ID3DXFileSaveObject* xfileSave,LPCSTR name,ANIMATIONSET* anime)
{
	HRESULT hr;
	ID3DXFileSaveData* xfileAnimeSet;
	hr = xfileSave->AddDataObject(TID_D3DRMAnimationSet,name,NULL,0,NULL,&xfileAnimeSet);
	if(hr != DXFILE_OK)
		return NULL;
	std::map<String,AnimationKey>::iterator itKey;
	for(itKey=anime->keys.begin();itKey!=anime->keys.end();++itKey)
	{
		//アニメーショングループの登録
		ID3DXFileSaveData* xfileAnime;
		xfileAnimeSet->AddDataObject(TID_D3DRMAnimation,NULL,NULL,0,NULL,&xfileAnime);
		xfileAnime->AddDataReference(itKey->first.c_str(),NULL);

		//行列キー
		if(itKey->second.matrix.size())
		{
			INT objectSize = (sizeof(DWORD)*2 + sizeof(NMatrix))*(INT)itKey->second.matrix.size();
			INT dataSize = sizeof(DWORD)*2 + objectSize;
			LPBYTE dataObject = NEW BYTE[dataSize];
			LPBYTE data = dataObject;
			*(LPDWORD)data = 4;
			data += sizeof(DWORD);
			*(LPDWORD)data = (DWORD)itKey->second.matrix.size();
			data += sizeof(DWORD);

			std::map<INT,NMatrix>::iterator it;
			for(it=itKey->second.matrix.begin();it!=itKey->second.matrix.end();++it)
			{
				*(LPDWORD)data = it->first;
				data += sizeof(DWORD);
				*(LPDWORD)data = 16;
				data += sizeof(DWORD);
				CopyMemory(data,&it->second,sizeof(NMatrix));
				data += sizeof(NMatrix);
			}
			ID3DXFileSaveData* xfileData;
			xfileAnime->AddDataObject(TID_D3DRMAnimationKey,NULL,NULL,dataSize,dataObject,&xfileData);
			xfileData->Release();
			delete[] dataObject;
		}
		//回転キー
		if(itKey->second.rotation.size())
		{
			INT objectSize = (sizeof(DWORD)*2+sizeof(FLOAT)*4) * (INT)itKey->second.rotation.size();
			INT dataSize = sizeof(DWORD)*2 + objectSize;
			LPBYTE dataObject = NEW BYTE[dataSize];
			LPBYTE data = dataObject;
			//データタイプ
			*(LPDWORD)data = 0;
			data += sizeof(DWORD);
			*(LPDWORD)data = (DWORD)itKey->second.rotation.size();
			data += sizeof(DWORD);

			std::map<INT,NVector>::iterator it;
			for(it=itKey->second.rotation.begin();it!=itKey->second.rotation.end();++it)
			{
				*(LPDWORD)data = it->first;
				data += sizeof(DWORD);
				*(LPDWORD)data = 4;
				data += sizeof(DWORD);
				*(PFLOAT)data = it->second.w;
				data += sizeof(FLOAT);
				*(PFLOAT)data = it->second.x;
				data += sizeof(FLOAT);
				*(PFLOAT)data = it->second.y;
				data += sizeof(FLOAT);
				*(PFLOAT)data = it->second.z;
				data += sizeof(FLOAT);
			}
			ID3DXFileSaveData* xfileData;
			xfileAnime->AddDataObject(TID_D3DRMAnimationKey,NULL,NULL,dataSize,dataObject,&xfileData);
			xfileData->Release();
			delete[] dataObject;
		}

		//スケールキー
		if(itKey->second.scale.size())
		{
			INT objectSize = (sizeof(DWORD)*2+sizeof(FLOAT)*3) * (INT)itKey->second.scale.size();
			INT dataSize = sizeof(DWORD)*2 + objectSize;
			LPBYTE dataObject = NEW BYTE[dataSize];
			LPBYTE data = dataObject;
			//データタイプ
			*(LPDWORD)data = 1;
			data += sizeof(DWORD);
			*(LPDWORD)data = (DWORD)itKey->second.scale.size();
			data += sizeof(DWORD);

			std::map<INT,NVector>::iterator it;
			for(it=itKey->second.scale.begin();it!=itKey->second.scale.end();++it)
			{
				*(LPDWORD)data = it->first;
				data += sizeof(DWORD);
				*(LPDWORD)data = 3;
				data += sizeof(DWORD);
				*(NVector3*)data = *(NVector3*)&it->second;
				data += sizeof(NVector3);
			}
			ID3DXFileSaveData* xfileData;
			xfileAnime->AddDataObject(TID_D3DRMAnimationKey,NULL,NULL,dataSize,dataObject,&xfileData);
			xfileData->Release();
			delete[] dataObject;
		}

		//ポジションキー
		if(itKey->second.position.size())
		{
			INT objectSize = (sizeof(DWORD)*2+sizeof(FLOAT)*3) * (INT)itKey->second.position.size();
			INT dataSize = sizeof(DWORD)*2 + objectSize;
			LPBYTE dataObject = NEW BYTE[dataSize];
			LPBYTE data = dataObject;
			//データタイプ
			*(LPDWORD)data = 2;
			data += sizeof(DWORD);
			*(LPDWORD)data = (DWORD)itKey->second.position.size();
			data += sizeof(DWORD);

			std::map<INT,NVector>::iterator it;
			for(it=itKey->second.position.begin();it!=itKey->second.position.end();++it)
			{
				*(LPDWORD)data = it->first;
				data += sizeof(DWORD);
				*(LPDWORD)data = 3;
				data += sizeof(DWORD);
				*(NVector3*)data = *(NVector3*)&it->second;
				data += sizeof(NVector3);
			}
			ID3DXFileSaveData* xfileData;
			xfileAnime->AddDataObject(TID_D3DRMAnimationKey,NULL,NULL,dataSize,dataObject,&xfileData);
			xfileData->Release();
			delete[] dataObject;
		}

		xfileAnime->Release();
	}
	return xfileAnimeSet;
}

ID3DXFileSaveData* XFile::writeMesh(ID3DXFileSaveData* xfileSave,MeshData* meshData)
{
	HRESULT hr;
	ID3DXFileSaveData* xfileMesh;
	INT vertexSize = (INT)meshData->vertexData.size() * sizeof(meshData->vertexData[0]);
	INT indexSize = getIndexSize(meshData->vertexIndex);
	if(!vertexSize || !indexSize)
		return NULL;

	INT dataSize = vertexSize+indexSize+sizeof(DWORD)*2;
	LPBYTE dataObject = NEW BYTE[dataSize];
	LPBYTE data = dataObject;
	//頂点データの保存
	*(LPDWORD)data = (DWORD)meshData->vertexData.size();
	data += sizeof(DWORD);
	CopyMemory(data,&meshData->vertexData[0],vertexSize);
	data += vertexSize;
	//インデックスデータの保存
	*(LPDWORD)data = (DWORD)meshData->vertexIndex.size();
	data += sizeof(DWORD);
	setIndex(data,meshData->vertexIndex);

	hr = xfileSave->AddDataObject(TID_D3DRMMesh,NULL,NULL,dataSize,dataObject,&xfileMesh);
	if(hr != DXFILE_OK)	//失敗
		return NULL;
	delete[] dataObject;

	//法線
	if(meshData->normalData.size())
	{
		INT normalSize = (INT)meshData->normalData.size() * sizeof(meshData->normalData[0]);
		INT indexSize = getIndexSize(meshData->normalIndex);
		INT dataSize = normalSize+indexSize+sizeof(DWORD)*2;
		LPBYTE dataObject = NEW BYTE[dataSize];
		LPBYTE data = dataObject;
		//法線データの保存
		*(LPDWORD)data = (DWORD)meshData->normalData.size();
		data += sizeof(DWORD);
		CopyMemory(data,&meshData->normalData[0],normalSize);
		data += normalSize;
		//法線インデックスデータの保存
		*(LPDWORD)data = (DWORD)meshData->normalIndex.size();
		data += sizeof(DWORD);
		setIndex(data,meshData->normalIndex);
		//法線オブジェクトの作成
		ID3DXFileSaveData* xfileData;
		hr = xfileMesh->AddDataObject(TID_D3DRMMeshNormals,NULL,NULL,dataSize,dataObject,&xfileData);
		xfileData->Release();
		delete[] dataObject;
	}
	//UV座標
	if(meshData->uvData.size())
	{
		INT uvSize = (INT)meshData->uvData.size() * sizeof(meshData->uvData[0]);
		INT dataSize = uvSize + sizeof(DWORD);
		LPBYTE dataObject = NEW BYTE[dataSize];
		LPBYTE data = dataObject;
		*(LPDWORD)data = (DWORD)meshData->uvData.size();
		data += sizeof(DWORD);
		CopyMemory(data,&meshData->uvData[0],uvSize);

		ID3DXFileSaveData* xfileData;
		hr = xfileMesh->AddDataObject(TID_D3DRMMeshTextureCoords,NULL,NULL,dataSize,dataObject,&xfileData);
		xfileData->Release();
		delete[] dataObject;
	}
	//COLOR
	if(meshData->colorData.size())
	{
		INT i;
		INT colorSize = (INT)meshData->colorData.size() * (sizeof(meshData->colorData[0])+sizeof(DWORD));
		INT dataSize = colorSize + sizeof(DWORD);
		LPBYTE dataObject = NEW BYTE[dataSize];
		LPBYTE data = dataObject;
		*(LPDWORD)data = (DWORD)meshData->colorData.size();
		data += sizeof(DWORD);
		for(i=0;i<(INT)meshData->colorData.size();i++)
		{
			*(LPDWORD)data = (DWORD)i;
			data += sizeof(DWORD);
			*(COLOR4*)data = meshData->colorData[i];
			data += sizeof(COLOR4);

		}
		ID3DXFileSaveData* xfileData;
		xfileMesh->AddDataObject(TID_D3DRMMeshVertexColors,NULL,NULL,dataSize,dataObject,&xfileData);
		xfileData->Release();
		delete[] dataObject;
	}
	//マテリアルリスト
	if(meshData->materialIndex.size())
	{
		ID3DXFileSaveData* xfileMaterialList;
		INT materialIndexSize = (INT)meshData->materialIndex.size() * sizeof(meshData->materialIndex[0]); 
		INT dataSize = materialIndexSize + sizeof(DWORD)*2;
		LPBYTE dataObject = NEW BYTE[dataSize];
		LPBYTE data = dataObject;
		//マテリアルデータの個数
		*(LPDWORD)data = (DWORD)meshData->materialData.size();
		data += sizeof(DWORD);
		//マテリアルリストの個数
		*(LPDWORD)data = (DWORD)meshData->materialIndex.size();
		data += sizeof(DWORD);
		//マテリアルリストデータ
		CopyMemory(data,&meshData->materialIndex[0],materialIndexSize);
		data += materialIndexSize;

		hr = xfileMesh->AddDataObject(TID_D3DRMMeshMaterialList,NULL,NULL,dataSize,dataObject,&xfileMaterialList);
		delete[] dataObject;

		//マテリアルデータ
		std::vector<MaterialData>::iterator itMaterial;
		for(itMaterial=meshData->materialData.begin();itMaterial!=meshData->materialData.end();++itMaterial)
		{
			ID3DXFileSaveData* xfileMaterial;
			Material* material = &(*itMaterial).material;
			INT dataSize = sizeof(FLOAT) * 11;
			FLOAT data[11] = 
			{
				material->Diffuse.r,material->Diffuse.g,material->Diffuse.b,material->Diffuse.a,
				material->Power,
				material->Specular.r,material->Specular.g,material->Specular.b,
				material->Emissive.r,material->Emissive.g,material->Emissive.b

			};
			hr = xfileMaterialList->AddDataObject(TID_D3DRMMaterial,NULL,NULL,dataSize,data,&xfileMaterial);
			//テクスチャネーム
			std::list<String>::iterator it;
			for(it=(*itMaterial).name.begin();it!=(*itMaterial).name.end();++it)
			{
				if(*it != "")
				{
					LPCSTR data = it->c_str();
					//\を\\に変換
					INT i,j;
					INT sCount = 0;
					for(i=0;data[i];i++)
						if(data[i] == '\\')
							sCount++;
					String convertString;
					convertString.reserve(it->length()+sCount);
					for(j=0,i=0;data[i];i++)
					{
						convertString += data[i];
						if(data[i] == '\\')
							convertString += data[i];
					}
					*it = convertString.c_str();
					ID3DXFileSaveData* xfileData;
					hr = xfileMaterial->AddDataObject(TID_D3DRMTextureFilename,NULL,NULL,
						(DWORD)convertString.length()+1,(LPVOID)it->c_str(),&xfileData);
					xfileData->Release();
				}
			}
			xfileMaterial->Release();
			
		}
		xfileMaterialList->Release();

	}
	//ボーン
	if(meshData->boneData.size())
	{
		//ヘッダの出力
		ID3DXFileSaveData* xfileMeshHeader;
		WORD data[3] = {2,0};
		data[2] = (WORD)meshData->boneData.size();
		xfileMesh->AddDataObject(DXFILEOBJ_XSkinMeshHeader,NULL,NULL,sizeof(data),data,&xfileMeshHeader);
		xfileMeshHeader->Release();

		//ボーンウエイトの出力
		std::vector<BoneData>::iterator itBone;
		for(itBone=meshData->boneData.begin();itBone!=meshData->boneData.end();++itBone)
		{
			std::map<INT,FLOAT>::iterator itWeight;
			INT boneNameSize = (INT)(*itBone).name.length()+1;
			INT boneIndexSize = (INT)itBone->weight.size() * sizeof(INT);
			INT boneWeightSize = (INT)itBone->weight.size() * sizeof(FLOAT);
			INT dataSize = boneNameSize+boneIndexSize+boneWeightSize+sizeof(DWORD)+sizeof(NMatrix);
			LPBYTE dataObject = NEW BYTE[dataSize];
			LPBYTE data = dataObject;
			//対応フレームネーム
			CopyMemory(data,(*itBone).name.c_str(),boneNameSize);
			data += boneNameSize;
			//ボーンリストサイズ
			*(LPDWORD)data = (DWORD)(*itBone).weight.size();
			data += sizeof(DWORD);
			//ボーンリストデータ
			for(itWeight=itBone->weight.begin();itWeight!=itBone->weight.end();++itWeight)
			{
				*(INT*)data = itWeight->first;
				data += sizeof(INT);
			}
			//ボーンウエイトデータ
			for(itWeight=itBone->weight.begin();itWeight!=itBone->weight.end();++itWeight)
			{
				*(FLOAT*)data = itWeight->second;
				data += sizeof(FLOAT);
			}
			//相対行列
			*(NMatrix*)data = (*itBone).matrix;
			//ボーンオブジェクトの作成
			ID3DXFileSaveData* xfileData;
			hr = xfileMesh->AddDataObject(DXFILEOBJ_SkinWeights,NULL,NULL,dataSize,dataObject,&xfileData);
			xfileData->Release();

			delete[] dataObject;
		}
	}

	return xfileMesh;

}

ID3DXFileSaveData* XFile::writeFrame(ID3DXFileSaveObject* xfileSave,ID3DXFileSaveData* xfileSaveData,FrameData* frameData)
{
	ID3DXFileSaveData* xfileFrame;
	HRESULT hr;
	if(xfileSaveData)
	{
		hr = xfileSaveData->AddDataObject(TID_D3DRMFrame,frameData->name.c_str(),NULL,0,NULL,&xfileFrame);
		if(hr != DXFILE_OK)	//失敗
			return NULL;
	}
	else
	{
		hr = xfileSave->AddDataObject(TID_D3DRMFrame,frameData->name.c_str(),NULL,0,NULL,&xfileFrame);
		if(hr != DXFILE_OK)	//失敗
			return NULL;
	}
	//ローカル行列
	ID3DXFileSaveData* xfileMatrix;
	hr = xfileFrame->AddDataObject(TID_D3DRMFrameTransformMatrix,
		NULL,NULL,sizeof(NMatrix),&frameData->matrix,&xfileMatrix);
	xfileMatrix->Release();

	//メッシュ
	ID3DXFileSaveData* xfileMesh;
	xfileMesh = writeMesh(xfileFrame,&frameData->mesh);
	if(xfileMesh)
		xfileMesh->Release();

	//子フレーム
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frameData->frameChild.begin();itFrame!=frameData->frameChild.end();++itFrame)
	{
		ID3DXFileSaveData* xfileFrameChild;
		xfileFrameChild = writeFrame(xfileSave,xfileFrame,&*itFrame);
		if(xfileFrameChild)
			xfileFrameChild->Release();
	}
	return xfileFrame;
}

INT XFile::getIndexSize(std::vector<INDEX4>& index)
{
	INT i;
	INT size = 0;
	INT count = (INT)index.size();
	for(i=0;i<count;i++)
	{
		if(index[i].type == 4)
			size += 5;
		else
			size += 4;
	}
	return size *= sizeof(DWORD);
}
void XFile::setIndex(LPVOID dest,std::vector<INDEX4>& index)
{
	INT i;
	INT count = (INT)index.size();
	LPDWORD data = (LPDWORD)dest;
	for(i=0;i<count;i++)
	{
		data[0] = index[i].type;
		data[1] = index[i].data[0];
		data[2] = index[i].data[1];
		data[3] = index[i].data[2];
		if(index[i].type == 4)
		{
			data[4] = index[i].data[3];
			data += 5;
		}
		else
			data += 4;
	}
}


}
