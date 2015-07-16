#include <windows.h>

#include "aflD3DUnit.h"

//----------------------------------------------------
//メモリリークテスト用
#if !defined(CHECK_MEMORY_LEAK)
	#if _MSC_VER && !defined(_WIN32_WCE) && _DEBUG
		#include <crtdbg.h>
		inline static void*  operator new(const size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size,_NORMAL_BLOCK,  strFileName, iLine);}
		inline static void operator delete(void* adr, LPCSTR strFileName, INT iLine)
			{_free_dbg(adr,_NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif
#endif


namespace AFL{
bool Unit::m_init = false;

//-----------------------------------------------
// Unit::Unit()
// -----  動作  -----
// コンストラクタ
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
Unit::Unit()
{
	m_unitParent = NULL;
	m_pos  = 0.0f;
	m_scale = 1.0f;
	m_rotation = 0.0f;
	m_center = 0.0f;
	m_relativity = 0.0f;
	m_alpha = 100;

	m_shadow = false;;
	m_zsort = true;
	m_zbuffer = true;
	m_view2D = false;
	m_visible = true;
	m_light = true;
	m_limit = false;
	m_clip = false;
	m_chainW = true;
	m_blendMode = 0;
	m_renderFlag = true;
	m_chainClip = false;
	m_viewClip.top = 0;
	m_viewClip.bottom = 0;
	m_viewClip.left = 0;
	m_viewClip.right = 0;

	//m_elementName = "PLANE";
	//m_vshaderName = "";
	//m_pshaderName;

	if(!m_init)
	{
		m_init = true;
		//頂点データの設定
		const static D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		const static D3D11_INPUT_ELEMENT_DESC layoutVector[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		const static D3D11_INPUT_ELEMENT_DESC layoutVector3D[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		const static D3D11_INPUT_ELEMENT_DESC layoutShadow[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		//頂点データの設定
		const static D3D11_INPUT_ELEMENT_DESC layoutBump[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		const static D3D11_INPUT_ELEMENT_DESC layoutSkin4Shadow[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 1, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 2, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 1, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 2, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 3, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};
		const static D3D11_INPUT_ELEMENT_DESC layoutSkin4[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 1, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 2, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 1, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 2, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 3, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			0
		};

		if(!D3DDevice::getInputElement("SKIN4"))
		{
			D3DDevice::addInputElement("SKIN4",layoutSkin4);
		}
		if(!D3DDevice::getInputElement("PLANE"))
		{
			D3DDevice::addInputElement("PLANE",layout);
		}
		if(!D3DDevice::getInputElement("BUMP"))
		{
			D3DDevice::addInputElement("BUMP",layoutBump);
		}
		if(!D3DDevice::getInputElement("BASE_SHADOW"))
		{
			D3DDevice::addInputElement("BASE_SHADOW",layoutShadow);
		}
		if(!D3DDevice::getInputElement("SKIN4_SHADOW"))
		{
			D3DDevice::addInputElement("SKIN4_SHADOW",layoutSkin4Shadow);
		}
		if(!D3DDevice::getInputElement("VECTOR"))
		{
			D3DDevice::addInputElement("VECTOR",layoutVector);
		}
		if(!D3DDevice::getInputElement("VECTOR3D"))
		{
			D3DDevice::addInputElement("VECTOR3D",layoutVector3D);
		}
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
Unit::~Unit()
{
	std::list<Unit*>::iterator it;
	while(m_unitChilds.size())
	{
		del(m_unitChilds.front());
	}
	if(m_unitParent)
	{
		m_unitParent->del(this);
	}
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
NVector& Unit::getPos()
{
	return m_pos;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
NVector& Unit::getScale()
{
	return m_scale;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
NVector& Unit::getRot()
{
	return m_rotation;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
Frame* Unit::getFrame()
{
	return m_frame.get();
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
Frame* Unit::getFrame(LPCSTR name)
{
	if(m_frame.get())
		return m_frame->getFrame(name);
	return NULL;
}
void Unit::setFrame(Frame* frame)
{
	m_frame = frame;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::copyImage(Unit* unit)
{
	//*(Object*)this = *(Object*)unit;
	m_anime = unit->m_anime;
	m_animationSet = unit->m_animationSet;
	m_frame = unit->m_frame;
	return true;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::openFile(LPCSTR fileName)
{
	return openFile(UCS2(fileName));
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::openFile(LPCWSTR fileName)
{
	FileObject fileObject;
	if(!fileObject.load(fileName))
		return false;
	createObject(&fileObject);
	return true;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::createObject(FileObject* fileObject)
{
	if(!fileObject)
		return false;

	Frame* frameParent = NEW Frame();
	
	std::list<FrameData>::iterator itFrame;
	for(itFrame=fileObject->frame.begin();itFrame!=fileObject->frame.end();++itFrame)
	{
		Frame* frame = readFrame(&*itFrame);
		if(frame)
		{
			frameParent->add(frame);
		}
	}
	m_frame = SP<Frame>(frameParent);


	//アニメーション
	if(fileObject->anime.size())
	{
		m_animationSet = fileObject->anime;
		setAnimation(m_animationSet.begin()->first.c_str());
	}
	return true;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::optimizeMesh(MESHOPTIMIZE& meshOptimize,std::vector<INT>& vertexIndex,std::vector<VERTEXBASE2>& data)
{
	INT i;
	INT dataCount = (INT)vertexIndex.size();
	if(dataCount == 0)
		return false;
	std::map<VERTEXBASE2,WORD>::iterator itMap;
	std::vector<INT>::iterator itIndex;
	INT indexSize = 0;
	INT index;
	meshOptimize.index.resize(dataCount);
	for(itIndex=vertexIndex.begin(),i=0;i<dataCount;i++,++itIndex)
	{
		itMap = meshOptimize.vertex.find(data[*itIndex]);
		if(itMap != meshOptimize.vertex.end())
		{
			index = (*itMap).second;
		}
		else
		{
			index = indexSize;
			meshOptimize.vertex[data[*itIndex]] = index;
			indexSize++;
		}
		meshOptimize.index[i] = index;
	}

	return true;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
Frame* Unit::readFrame(struct FrameData* frameData)
{
	//AFL::Debug::out("X-File FRAME: %s\n",pXFrame->stringName.c_str());

	Frame* frame = NEW Frame;
	frame->setName(frameData->name.c_str());

	std::list<FrameData>::iterator it;
	for(it=frameData->frameChild.begin();it!=frameData->frameChild.end();++it)
	{
		Frame* frameChild = readFrame(&*it);
		frame->add(frameChild);
	}
	frame->setMatrix(&frameData->matrix);

	readMesh(frame,&frameData->mesh);

	return frame;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void createTB(NVector3* u,NVector3* v,MeshData& meshData,DWORD index)
{
	DWORD vi0 = meshData.vertexIndex[index].data[0];
	DWORD vi1 = meshData.vertexIndex[index].data[1];
	DWORD vi2 = meshData.vertexIndex[index].data[2];

	DWORD t0 = meshData.uvIndex[index].data[0];
	DWORD t1 = meshData.uvIndex[index].data[1];
	DWORD t2 = meshData.uvIndex[index].data[2];

	NVector v0[3] =
	{
		meshData.vertexData[vi0].x,meshData.uvData[t0].u,meshData.uvData[t0].v,0.0f,
		meshData.vertexData[vi0].y,meshData.uvData[t0].u,meshData.uvData[t0].v,0.0f,
		meshData.vertexData[vi0].z,meshData.uvData[t0].u,meshData.uvData[t0].v,0.0f,

	};
	NVector v1[3] =
	{
		meshData.vertexData[vi1].x,meshData.uvData[t1].u,meshData.uvData[t1].v,0.0f,
		meshData.vertexData[vi1].y,meshData.uvData[t1].u,meshData.uvData[t1].v,0.0f,
		meshData.vertexData[vi1].z,meshData.uvData[t1].u,meshData.uvData[t1].v,0.0f,
	};
	NVector v2[3] =
	{
		meshData.vertexData[vi2].x,meshData.uvData[t2].u,meshData.uvData[t2].v,0.0f,
		meshData.vertexData[vi2].y,meshData.uvData[t2].u,meshData.uvData[t2].v,0.0f,
		meshData.vertexData[vi2].z,meshData.uvData[t2].u,meshData.uvData[t2].v,0.0f,
	};
	float U[ 3 ], V[ 3 ];
	for ( int i = 0; i < 3; ++i )
	{
		NVector V1 = v1[ i ] - v0[ i ];
		NVector V2 = v2[ i ] - v1[ i ];
		NVector ABC;
		ABC = V1.cross(V2);

		if ( ABC.x != 0.0f )
		{
			U[ i ] = - ABC.y / ABC.x;
			V[ i ] = - ABC.z / ABC.x;
		}
	}
	*u = *(NVector3*)U;
	*v = *(NVector3*)V;
	*u = u->normal();
	*v = v->normal();
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
inline bool isOverride(VERTEXBASE2& v1,VERTEXBASE2& v2)
{
	return v1.isOverride(v2);
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::readMesh(Frame* frame,struct MeshData* meshData)
{
	INT i,j,k;

	//接触判定用
	NVector vect[] =
	{
		{1000000.0f,1000000.0f,1000000.0f,1000000.0f},
		{-1000000.0f,-1000000.0f,-1000000.0f,-1000000.0f}
	};
	INT size = meshData->vertexData.size();
	for(i=0;i<size;i++)
	{
		NVector v;
		v = meshData->vertexData[i];
		vect[0] = vect[0].minimum(v);
		vect[1] = vect[1].maximum(v);
	}
	frame->setBoundingBox(vect);

	//非表示フレーム
	if(frame->getFrameName()[0] == '_')
	{
		return;
	}
	//三角形フェイスの算出

	
	//必用三角形フェイス数
	INT indexCount = (INT)meshData->vertexIndex.size();


	//ボーンデータの展開作業用
	const INT BONEMAX = 4;
	INT boneCount = (INT)meshData->boneData.size();
	//スキンデータを頂点ごとに再配置 <頂点INDEX<BONEINDEX,WEIGHT>>
	std::vector<std::map<INT,FLOAT> > vertexSkin(meshData->vertexData.size());

	for(i=0;i<boneCount;++i)
	{
		std::map<INT,FLOAT>::iterator itWeight;
		for(itWeight=meshData->boneData[i].weight.begin();itWeight!=meshData->boneData[i].weight.end();++itWeight)
		{
			if(itWeight->second)
			{
				DWORD index = itWeight->first;
				vertexSkin[index][i] = itWeight->second;
			}
		}
	}


	//最大マテリアルの抽出
	INT materialMax = 0;
	for(i=0;i<(INT)meshData->materialIndex.size();i++)
		if((INT)meshData->materialIndex[i] > materialMax)
			materialMax = (INT)meshData->materialIndex[i];
	materialMax++;


	//頂点データ展開作業領域の確保
	std::vector<std::vector<INT> > indexVertex(materialMax);
	std::vector<VERTEXBASE2> dataVertex((INT)meshData->vertexIndex.size()*3);
	for(i=0;i<(INT)meshData->vertexIndex.size();i++)
	{
		//マテリアルごとに分類
		DWORD materialIndex = 0;
		if(meshData->materialIndex.size())
		{
			materialIndex = meshData->materialIndex[i];
		}
		VERTEXBASE2 vertex;
		ZeroMemory(&vertex,sizeof(vertex));

		bool bump = false;

		NVector3 u,v;
		if(meshData->vertexData.size() && meshData->uvData.size())
		{
			createTB(&u,&v,*meshData,i);
			bump = true;
		}
		for(j=0;j<3;j++)
		{
			//頂点データの展開
			DWORD dwIndex = meshData->vertexIndex[i].data[j];
			vertex.vectPosition = meshData->vertexData[dwIndex];
			//ボーンの展開
			std::map<INT,FLOAT>::iterator itSkin;
			FLOAT fWeight = 0.0f;
			for(itSkin=vertexSkin[dwIndex].begin(),k=0;itSkin!=vertexSkin[dwIndex].end() && k<BONEMAX;++itSkin,k++)
			{
				vertex.blendIndex[k] = itSkin->first;
				if(k<BONEMAX-1)
					vertex.fBlend[k] = itSkin->second;
				fWeight += itSkin->second;
			}
			if(k && fWeight < 0.999f)
			{
				vertex.fBlend[0] += 1.0f-fWeight;
			}
			if(k<BONEMAX && vertexSkin[dwIndex].size()>4)
			{
				vertex.blendIndex[k] = 255;
			}
			for(;k<BONEMAX;k++)
			{
				if(k<BONEMAX-1)
					vertex.fBlend[k] = 1.0f;
				vertex.blendIndex[k] = 255;
			}

			//法線データの展開
			if(meshData->normalIndex.size())
			{
				DWORD index = meshData->normalIndex[i].data[j];
				vertex.vectNormal = meshData->normalData[index];
			}

			//UV座標の展開
			if(meshData->uvIndex.size())
			{
				DWORD dwIndex = meshData->uvIndex[i].data[j];
				vertex.uv.x = meshData->uvData[dwIndex].u;
				vertex.uv.y = meshData->uvData[dwIndex].v;
			}
			//頂点カラー
			if(meshData->colorData.size())
			{
				DWORD dwIndex = meshData->colorIndex[i].data[j];
				vertex.color = *(NVector*)&meshData->colorData[dwIndex];
			}
			else if(meshData->materialIndex.size())
			{
				vertex.color = *(NVector*)&meshData->materialData[materialIndex].material.Diffuse;
			}
			else
			{
				const NVector c = {1.0f,1.0f,1.0f,1.0f};
				vertex.color = c;
			}
			//バンプマップ
			if(bump)
			{
				vertex.tan = u;
				vertex.bin = v;
			}
			dataVertex[i*3+j] = vertex;
			indexVertex[materialIndex].push_back(i*3+j);
		}
	}
	//メッシュ分解と最適化
	for(k=0;k<materialMax;k++)
	{

		MESHOPTIMIZE meshOptimize;
		if(optimizeMesh(meshOptimize,indexVertex[k],dataVertex))
		{
			//マテリアルの展開
			if(meshData->materialIndex.size())
			{
				meshOptimize.material = meshData->materialData[k].material;
				meshOptimize.textureName = meshData->materialData[k].name;
			}

			meshOptimize.boneMatrixs.resize(boneCount);
			for(i=0;i<boneCount;i++)
			{
				meshOptimize.boneMatrixs[i].name = meshData->boneData[i].name;
				meshOptimize.boneMatrixs[i].matrix = meshData->boneData[i].matrix;
			}
			//最適化済みデータを保存
			readMesh(frame,&meshOptimize);
		}
	
	}
	
	//シャドウボリュームの作成
	if(isShadow())
	{

		//作業用頂点データの展開
		INT count = dataVertex.size();
		
		std::vector<INT> indexShadow;
		//indexShadow.reserve(count*count);
		for(i=0;i<count;++i)
			indexShadow.push_back(i);
		INT faces = count / 3; 
		static const INT idx[] = {0,1,1,2,2,0};
		for(i=0;i<faces-1;i++)
		{
			for(k=0;k<3;k++)
			{
				const int index1 = i*3+idx[k*2];
				const int index2 = i*3+idx[k*2+1];
				for(j=i+1;j<faces;j++)
				{

					INT l;
					for(l=0;l<3;l++)
					{
						const int index3 = j*3+idx[l*2+1];
						const int index4 = j*3+idx[l*2];
						if (dataVertex[index1].isOverride(dataVertex[index3]) && dataVertex[index2].isOverride(dataVertex[index4]))
						{
							indexShadow.push_back(index1);
							indexShadow.push_back(index3);
							indexShadow.push_back(index2);
							
							indexShadow.push_back(index4);
							indexShadow.push_back(index2);		
							indexShadow.push_back(index3);
						}
					}
				}
			}
		}

		MESHOPTIMIZE meshOptimize;
		if(optimizeMesh(meshOptimize,indexShadow,dataVertex))
		{
			meshOptimize.boneMatrixs.resize(boneCount);
			for(i=0;i<boneCount;i++)
			{
				meshOptimize.boneMatrixs[i].name = meshData->boneData[i].name;
				meshOptimize.boneMatrixs[i].matrix = meshData->boneData[i].matrix;
			}
			//最適化済みデータを保存
			readMesh(frame,&meshOptimize,true);
		}
		
	}

}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::readMesh(Frame* frame,MESHOPTIMIZE* xfileMesh,bool shadow)
{
	INT i;
	Mesh* mesh = NEW Mesh();
	INT indexCount = (INT)xfileMesh->index.size();
	INT vertexCount = (INT)xfileMesh->vertex.size();
	//頂点インデックスの設定
	mesh->createIndex(&xfileMesh->index[0],indexCount*sizeof(WORD));
	
	INT boneCount = 0;
	std::map<VERTEXBASE2,WORD>::iterator itVertex;
	for(itVertex=xfileMesh->vertex.begin();itVertex!=xfileMesh->vertex.end();++itVertex)
	{
		for(i=0;i<4;i++)
			if(itVertex->first.blendIndex[i] == 255)
				break;
		if(i>boneCount)
			boneCount = i;
	}

	D3D11_INPUT_ELEMENT_DESC* elementDesc;
	INT elementCount;

	if(!shadow)
	{
		if(boneCount)
		{
			elementDesc = D3DDevice::getInputElement("SKIN4");
			elementCount = D3DDevice::getInputCount("SKIN4");
		}	
		else
		{
			if(xfileMesh->textureName.size() < 2)
			{
				elementDesc = D3DDevice::getInputElement("PLANE");
				elementCount = D3DDevice::getInputCount("PLANE");
			}
			else
			{
				elementDesc = D3DDevice::getInputElement("BUMP");
				elementCount = D3DDevice::getInputCount("BUMP");
			}
		}
	}

	else
	{
		if(boneCount)
		{
			elementDesc = D3DDevice::getInputElement("SKIN4_SHADOW");
			elementCount = D3DDevice::getInputCount("SKIN4_SHADOW");
		}	
		else
		{
			elementDesc = D3DDevice::getInputElement("BASE_SHADOW");
			elementCount = D3DDevice::getInputCount("BASE_SHADOW");
		}
	}

	//頂点配置の調整
	std::map<std::pair<String,UINT>,UINT> map;
	std::map<std::pair<String,UINT>,UINT>::iterator it;
	D3DDevice::getInputMap(map,elementDesc,elementCount);

	struct
	{
		INT Pos;
		INT Nor;
		INT Col;
		INT Tex;
		INT Wei;
		INT Bin;
		INT Tu;
		INT Tv;
	}offset[16];

	for(i=0;i<16;i++)
	{
		offset[i].Pos = -1;
		offset[i].Nor = -1;
		offset[i].Col = -1;
		offset[i].Tex = -1;
		offset[i].Wei = -1;
		offset[i].Bin = -1;
		offset[i].Tu = -1;
		offset[i].Tv = -1;
	}
	for(i=0;i<16;i++)
	{
		it = map.find(std::pair<String,UINT>("POSITION",i));
		if(it != map.end())
			offset[i].Pos = it->second;

		it = map.find(std::pair<String,UINT>("NORMAL",i));
		if(it != map.end())
			offset[i].Nor = it->second;

		it = map.find(std::pair<String,UINT>("COLOR",i));
		if(it != map.end())
			offset[i].Col = it->second;

		it = map.find(std::pair<String,UINT>("TEXCOORD",i));
		if(it != map.end())
			offset[i].Tex = it->second;

		it = map.find(std::pair<String,UINT>("BLENDWEIGHT",i));
		if(it != map.end())
			offset[i].Wei = it->second;

		it = map.find(std::pair<String,UINT>("BLENDINDICES",i));
		if(it != map.end())
			offset[i].Bin = it->second;

		it = map.find(std::pair<String,UINT>("TANGENT",i));
		if(it != map.end())
			offset[i].Tu = it->second;

		it = map.find(std::pair<String,UINT>("BINORMAL",i));
		if(it != map.end())
			offset[i].Tv = it->second;
	}

	if(boneCount)
	{
		if(!shadow)
			mesh->createLayout("SKIN4","SKIN4");
		else
			mesh->createLayout("SKIN4_SHADOW","SKIN4_SHADOW");
	}
	else
	{
		if(!shadow)
		{
			if(xfileMesh->textureName.size() < 2)
			{
				if(isLight())
					mesh->createLayout("BASE","PLANE");
				else
					mesh->createLayout("BASE_NL","PLANE");
			}
			else
				mesh->createLayout("BASE_BUMP","BUMP");
		}
		else
		{
			if(xfileMesh->textureName.size() < 2)
				mesh->createLayout("BASE_SHADOW","BASE_SHADOW");
			else
				mesh->createLayout("BASE_SHADOW","BASE_SHADOW");
		}
	}
	size_t size = mesh->getStrideSize()*vertexCount;
	LPBYTE data = NEW BYTE[size];
	LPBYTE back = data;

	itVertex=xfileMesh->vertex.begin();
	NVector max;
	max = itVertex->first.vectPosition;
	NVector min;
	min = itVertex->first.vectPosition;
	INT j;
	for(j=0;j<vertexCount;j++)
	{
		const VERTEXBASE2& vertex = itVertex->first;
		const WORD vertexIndex = itVertex->second;
		data = back + mesh->getStrideSize()*vertexIndex;
		for(i=0;i<16;i++)
		{
			if(offset[i].Pos == -1)
				break;
			*(NVector3*)(data+offset[i].Pos) = vertex.vectPosition;
			min = vertex.vectPosition.minimum(min);
			max = vertex.vectPosition.maximum(max);
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Nor == -1)
				break;
			*(NVector3*)(data+offset[i].Nor) = vertex.vectNormal;
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Col == -1)
				break;
			*(NVector*)(data+offset[i].Col) = vertex.color;
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Tex == -1)
				break;
			*(NVector2*)(data+offset[i].Tex) = *(NVector2*)&vertex.uv;
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Wei == -1)
				break;
			*(FLOAT*)(data+offset[i].Wei) = vertex.fBlend[i];
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Bin == -1)
				break;
			*(FLOAT*)(data+offset[i].Bin) = (FLOAT)vertex.blendIndex[i];
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Tu == -1)
				break;
			*(NVector3*)(data+offset[i].Tu) = vertex.tan;
		}
		for(i=0;i<16;i++)
		{
			if(offset[i].Tv == -1)
				break;
			*(NVector3*)(data+offset[i].Tv) = vertex.bin;
		}
		++itVertex;
	
	}
	mesh->setVertexRange(&min,&max);
	mesh->createVertex(back,size);
	delete[] back;
		
	mesh->setBoneMatrix(xfileMesh->boneMatrixs);
	Material material  = xfileMesh->material;


	std::list<std::string>::iterator itTex;
	for(itTex=xfileMesh->textureName.begin();itTex!=xfileMesh->textureName.end();++itTex)
	{
		if(*itTex != "")	
		{
			Texture* texture = NEW Texture();
			if(itTex->c_str()[0] && texture->open(itTex->c_str()))
			{
				mesh->addTexture(texture);
			}
			else
			{
				delete texture;
			}
		}
	}
	//テクスチャの有無でシェーダの切り替え
	if(shadow)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		mesh->setRasterizer(&rasterizerDesc);

		//シャドーボリューム用ステンシル設定
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.StencilEnable = true;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		mesh->setDepthStencil(&desc);

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = false;
		mesh->setBlend(&blendDesc);

		mesh->setPShader("SHADOW");
	}
	else if(mesh->getTextureCount()==1)
	{
		mesh->setPShader("TEXTURE");
	}
	else if(mesh->getTextureCount()==2)
	{
		mesh->setPShader("BASE_BUMP");
	}
	else
	{
		mesh->setPShader("VCOLOR");
	}


	mesh->setMaterial(material);
	if(shadow)
		frame->addShadow(mesh);
	else
		frame->add(mesh);
	delete mesh;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
NMatrix Unit::getMatrix()
{
	return getMatrix(getPosX(),getPosY(),getPosZ());
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
NMatrix Unit::getMatrix(FLOAT x,FLOAT y,FLOAT z)
{
	NMatrix matrix;
	//回転行列作成
	if(getRotationX() || getRotationY() || getRotationZ())
	{
		FLOAT RX = NPI / 180.0f * getRotationX();
		FLOAT RY = NPI / 180.0f * getRotationY();
		FLOAT RZ = NPI / 180.0f * getRotationZ();
		//相対移動行列作成
		if(getRelativityX() || getRelativityY() || getRelativityZ() || getCenterX()|| getCenterY()|| getCenterZ())
		{
			matrix.setTranslation(
				getRelativityX()-getCenterX(),
				getRelativityY()-getCenterY(),
				getRelativityZ()-getCenterZ());
			matrix *= NMatrix().setRotationX(RX);		
		}
		else
			matrix.setRotationX(RX);	
		matrix *= NMatrix().setRotationY(RY);		
		matrix *= NMatrix().setRotationZ(RZ);		
	}
	else//移動行列作成
		matrix.setTranslation(getRelativityX()-getCenterX(),getRelativityY()-getCenterY(),getRelativityZ()-getCenterZ());

	//スケーリング
	if(getScaleX()!=1 || getScaleY()!=1 || getScaleZ()!=1)
	{
		matrix = NMatrix().setScaling(getScaleX(),getScaleY(),getScaleZ()) * matrix;
	}

	//位置を戻す
	matrix._41 += getCenterX() + x;
	matrix._42 += getCenterY() + y;
	matrix._43 += getCenterZ() + z;
	return matrix;
}

//-----------------------------------------------
// bool Unit::isViewClip() const
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isViewClip() const
{
	return m_clip;
}
//-----------------------------------------------
// void Unit::setViewClip(bool flag)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setViewClip(bool flag)
{
	m_clip = flag;
}
//-----------------------------------------------
// void Unit::setPosX(FLOAT x)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPosX(FLOAT x)
{
	setPos(x,getPosY(),getPosZ(),getPosW());
}
//-----------------------------------------------
// void Unit::setPosY(FLOAT y)
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPosY(FLOAT y)
{
	setPos(getPosX(),y,getPosZ(),getPosW());
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPosZ(FLOAT z)
{
	setPos(getPosX(),getPosY(),z,getPosW());
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPosW(FLOAT w)
{
	setPos(getPosX(),getPosY(),getPosZ(),w);
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y)
{
	setPos(x,y,getPosZ(),getPosW());
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y,FLOAT z)
{
	setPos(x,y,z,getPosW());
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y,FLOAT z,FLOAT w)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
	m_pos.w = w;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getPosX() const
{
	return m_pos.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getPosY() const
{
	return m_pos.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getPosZ() const
{
	return m_pos.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getPosW() const
{
	return m_pos.w;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getAbsX()
{
	FLOAT x = getPosX();
	if(m_unitParent)
		x += m_unitParent->getAbsX();
	return x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getAbsY()
{
	FLOAT y = getPosY();
	if(m_unitParent)
		y += m_unitParent->getAbsY();
	return y;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getAbsZ()
{
	FLOAT z = getPosZ();
	if(m_unitParent)
		z += m_unitParent->getAbsZ();
	return z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getAbsW()
{
	FLOAT w = getPosW();
	if(m_unitParent)
		w += m_unitParent->getAbsW();
	return w;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setScale(FLOAT fScale)
{
	m_scale = fScale;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setScaleX(FLOAT fScale)
{
	m_scale.x = fScale;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setScaleY(FLOAT fScale)
{
	m_scale.y = fScale;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setScaleZ(FLOAT fScale)
{
	m_scale.z = fScale;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleX()
{
	return m_scale.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleY()
{
	return m_scale.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleZ()
{
	return m_scale.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterX() const
{
	return m_center.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterY() const
{
	return m_center.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterZ() const
{
	return m_center.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setCenter(FLOAT x,FLOAT y,FLOAT z)
{
	m_center.x = x;
	m_center.y = y;
	m_center.z = z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setChainW(bool flag)
{
	m_chainW = flag;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isChainW() const
{
	return m_chainW;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setChainClip(bool flag)
{
	m_chainClip = flag;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isChainClip() const
{
	return m_chainClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setAlpha(DWORD alpha)
{
	m_alpha = alpha;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
DWORD Unit::getAlpha() const
{
	return m_alpha;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRotationX(FLOAT fAngle)
{
	m_rotation.x = fAngle;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRotationY(FLOAT fAngle)
{
	m_rotation.y = fAngle;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRotationZ(FLOAT fAngle)
{
	m_rotation.z = fAngle;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationX()
{
	return m_rotation.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationY()
{
	return m_rotation.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationZ()
{
	return m_rotation.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRelativityX(FLOAT fPoint)
{
	m_relativity.x=fPoint;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRelativityY(FLOAT fPoint)
{
	m_relativity.y=fPoint;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setRelativityZ(FLOAT fPoint)
{
	m_relativity.z=fPoint;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityX()
{
	return m_relativity.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityY()
{
	return m_relativity.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityZ()
{
	return m_relativity.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipX(FLOAT fClip)
{
	m_clipPoint.x=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipY(FLOAT fClip)
{
	m_clipPoint.y=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipZ(FLOAT fClip)
{
	m_clipPoint.z=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipX()
{
	return m_clipPoint.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipY()
{
	return m_clipPoint.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipZ()
{
	return m_clipPoint.z;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipWidth(FLOAT fClip)
{
	m_clipSize.x=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipHeight(FLOAT fClip)
{
	m_clipSize.y=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setClipDepth(FLOAT fClip)
{
	m_clipSize.z=fClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipWidth()
{
	return m_clipSize.x;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipHeight()
{
	return m_clipSize.y;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
FLOAT Unit::getClipDepth()
{
	return m_clipSize.z;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isShadow() const
{
	return m_shadow;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setShadow(bool bShadow)
{
	m_shadow=bShadow;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isLight() const
{
	return m_light;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setLight(bool bLight)
{
	m_light=bLight;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isView2D() const
{
	return m_view2D;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setView2D(bool bView)
{
	m_view2D=bView;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isVisible() const
{
	return m_visible;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setVisible(bool bVisible)
{
	if(m_visible != bVisible)
	{
		m_visible=bVisible;
		resetRenderFlag();
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isZBuffer() const
{
	return m_zbuffer;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setZBuffer(bool bZBuffer)
{
	m_zbuffer=bZBuffer;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getBlendMode() const
{
	return m_blendMode;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setBlendMode(INT iMode)
{
	m_blendMode=iMode;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setLimit(bool bFlag)
{
	m_limit=bFlag;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isLimit() const
{
	return m_limit;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setZSort(bool bFlag)
{
	m_zsort=bFlag;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isZSort() const
{
	return m_zsort;
}

//-----------------------------------------------
// void Unit::add(Unit* unit)
// -----  動作  -----
// 子ユニットを追加する
// -----  引数  -----
// unit  子ユニット
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Unit::add(Unit* unit)
{
	unit->m_unitParent = this;
	m_unitChilds.push_back(unit);
}
//-----------------------------------------------
// void Unit::del(Unit* unit)
// -----  動作  -----
// 子ユニットを削除する
// -----  引数  -----
// unit  子ユニット
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void Unit::del(Unit* unit)
{
	unit->m_unitParent = NULL;
	m_unitChilds.remove(unit);
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getAllFrameCount()
{
	INT count = getFrameCount();
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		count += (*it)->getAllFrameCount();
	}
	return count;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getAllUnitCount()
{
	INT count = 1;
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		count += (*it)->getAllUnitCount();
	}
	return count;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getAllMeshCount()
{
	INT count = getMeshCount();
	if(isShadow())
		count += count;
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		count += (*it)->getAllMeshCount();
	}
	return count;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getFrameCount()
{
	if(m_frame.get())
		return m_frame->getAllFrameCount()+1;
	else 
		return 0;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
INT Unit::getMeshCount()
{
	if(m_frame.get())
		return m_frame->getAllMeshCount();
	else
		return 0;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::createBoundingBox()
{
	Frame* frame = m_frame.get();
	if(frame)
	{
		frame->createBoundingBox();
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::getBoundingBox(NVector* vect)
{
	Frame* frame = m_frame.get();
	if(frame)
	{
		NVector vectSrc[2] = {1000000,1000000,1000000,0,-1000000,-1000000,-1000000,0};

		NMatrix matrix = getMatrix(getPosX(),getPosY(),getPosZ());
		NMatrix matrixTop;
		matrixTop.setIdentity();
		frame->getBoundingBox(this,&matrixTop,&matrix,vectSrc);
		vect[0] = vectSrc[0];
		vect[1] = vectSrc[1];
	}
}
void Unit::getBoundingBox(Frame* frame,NVector* vect)
{
	NVector vect2[] =
	{
		{1000000.0f,1000000.0f,1000000.0f,1000000.0f},
		{-1000000.0f,-1000000.0f,-1000000.0f,-1000000.0f}
	};
	if(frame)
	{
		frame->getBoundingBox(vect2,this);
		vect[0] = vect2[0];
		vect[1] = vect2[1];
	}
}
void Unit::setShader(bool flag)
{
	//m_frame->setShader(flag);
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::addAnimationTime(DWORD count)
{
	std::list<Animation>::iterator it;
	for(it=m_anime.begin();it!=m_anime.end();++it)
	{
		if(it==m_anime.begin())
			it->setTimeCount(it->getTimeCount()+count);
		else
			it->setTimeChangeWork(it->getTimeChangeWork()+count);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setAnimation(LPCSTR name,DWORD count,bool loop)
{
	std::map<std::string,ANIMATIONSET>::iterator it;
	it = m_animationSet.find(name);
	if(it != m_animationSet.end())
	{
		Animation anime;
		anime.setTimeChange(count);
		anime.setTimeChangeWork(1);
		anime.setLoop(loop);
		m_anime.push_back(anime);
		m_anime.back().setAnimation(name,it->second.keys);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
LPCSTR Unit::getAnimationName()
{
	std::list<Animation>::reverse_iterator it;
	it = m_anime.rbegin();
	if(it == m_anime.rend())
		return NULL;
	return it->getName();

}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::isAnimation() const
{
	if(m_anime.size() > 1)
		return true;
	std::list<Animation>::const_iterator it;
	for(it=m_anime.begin();it!=m_anime.end();++it)
	{
		if(it->isAnimation())
			return true;
	}
	return false;	
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::onAction(LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onAction(value);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::onStart(LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onStart(value);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::onIdel(LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onIdel(value);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::onEnd(LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onEnd(value);
	}
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z)
{
	return true;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::onRenderMesh(Mesh* mesh,LPVOID value)
{
	return true;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::onRenderEnd()
{
	m_renderFlag = false;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
RECT* Unit::getViewClip(RECT* rect)
{
	if(rect)
		*rect = m_viewClip;
	return &m_viewClip;
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
void Unit::setViewClip(const RECT* rect)
{
	m_viewClip = *rect;
/*	if(m_viewClip.top < 0)
		m_viewClip.top = 0;
	if(m_viewClip.left < 0)
		m_viewClip.left = 0;
	if(m_viewClip.top >  m_viewClip.bottom)
		m_viewClip.bottom = m_viewClip.top;
	if(m_viewClip.left >  m_viewClip.right)
		m_viewClip.right = m_viewClip.left;
*/
}
//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
std::list<Unit*>* Unit::getChilds()
{
	return &m_unitChilds;
}

//-----------------------------------------------
// 
// -----  動作  -----
// 
// -----  引数  -----
// 
// ----- 戻り値 -----
// 
//-----------------------------------------------
bool Unit::getAnimationMatrix(LPCSTR frameName,NMatrix& matrix)
{
	INT flag = 0;
	NVector position,scale;
	NVector rotation;
	
	NVector position2,scale2;
	NVector rotation2;

	position.setZero();
	scale.setZero();
	rotation.setZero();

	ZeroMemory(&matrix,sizeof(NMatrix));
	std::list<Animation>::iterator it;
	std::list<Animation>::iterator it2;
	
	INT i,j,k;
	std::vector<FLOAT> weightList(m_anime.size());
	std::vector<std::list<Animation>::iterator> weightIt(m_anime.size());
	for(i=0,it=m_anime.begin();it!=m_anime.end();++it,++i)
	{
		FLOAT weight = 1.0f;
		if(it->getTimeChange())
		{
			weight = (FLOAT)it->getTimeChangeWork() / it->getTimeChange();
			if(weight > 1.0f)
				weight = 1.0f;
		}
		weightList[i] = weight;
		weightIt[i] = it;
	}
	//ブレンドウエイトの算出
	for(j=1;j<i;j++)
	{
		for(k=0;k<j;k++)
		{
			weightList[k] *= 1.0f - weightList[j]; 
		}
	}

	
	for(i=0,it=m_anime.begin();it!=m_anime.end();++it,++i)
	{
		NMatrix matirixAnime;
		Animation* anime = &*it;
		FLOAT fWeight = weightList[i];
		if(fWeight)
		{
			INT ret = anime->getAnimationKey(frameName,&matirixAnime,&position2,&scale2,&rotation2);
			if(ret == 1)
			{
				flag |= 1;
				if(fWeight == 1.0f)
				{
					matrix += matirixAnime;
					break;
				}
				else if(fWeight != 0)
					matrix += matirixAnime * fWeight;
			}
			else if(ret == 2)
			{
				flag |= 2;
				if(fWeight == 1.0f)
				{
					position = position2;
					scale = scale2;
					rotation = rotation2;
					break;
				}
				else if(fWeight != 0)
				{
					position += position2 * fWeight;
					scale += scale2 * fWeight;
					if(i==0)
						rotation = rotation2;
					else
						rotation = rotation.slerpQuaternion(rotation2,fWeight);
					//rotation += rotation2*fWeight;
				}
			}
		}
	}
	//不要データの削除
	for(i=0;i<(INT)weightList.size();i++)
	{
		if(weightList[i] == 0)
		{
			m_anime.erase(weightIt[i]);
		}
	}
	if(flag & 2)
	{
		NMatrix matrixKey =
		{
			scale.x,0,0,0,
			0,scale.y,0,0,
			0,0,scale.x,0,
			position.x,position.y,position.z,1.0f
		};
		rotation.w *= -1;
		NMatrix rot;
		rot.setRotationQuaternion(rotation);
		matrix += rot * matrixKey;
	}
	return flag != 0;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSprite
// 2D用スプライトユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitSprite::UnitSprite()
{
	setView2D(true);
	setZBuffer(false);
	setLight(false);
	m_pointStat = POINT_LEFT|POINT_TOP;
	m_centerX = 0.0f;
	m_centerY = 0.0f;
	m_partWidth = 0;
	m_partHeight = 0;
	m_partIndex = 0;

	m_vsName = "TEXTURE2";
	m_psName = "TEXTURE";


}
Texture* UnitSprite::getTexture() const
{
	if(m_frame == NULL)
		return NULL;
	Mesh* mesh = m_frame->getMesh();
	if(mesh &&  mesh->getMaterial())
		return mesh->getTexture();
	else
		return NULL;
}
HDC UnitSprite::getDC() const
{
	Texture* texture = getTexture();
	return texture->getDC();
}
void UnitSprite::releaseDC(HDC hdc) const
{
	Texture* texture = getTexture();
	texture->releaseDC();
}
INT UnitSprite::getImageWidth() const
{
	if(!getTexture())
		return 0;
	return getTexture()->getImageWidth();
}
INT UnitSprite::getImageHeight() const
{
	if(!getTexture())
		return 0;
	return getTexture()->getImageHeight();
}

bool UnitSprite::clear(DWORD color)
{
	if(!getTexture())
		return false;
	getTexture()->clear(color);
	return true;
}
void UnitSprite::setPartSize(INT width,INT height)
{
	m_partWidth = width;
	m_partHeight = height;
}
INT UnitSprite::getPartWidth()const
{
	return m_partWidth;
}
INT UnitSprite::getPartHeight()const
{
	return m_partHeight;
}
INT UnitSprite::getPartIndex()const
{
	return m_partIndex;
}
void UnitSprite::setPartIndex(INT index)
{
	m_partIndex = index;
	resetRenderFlag();

}



bool UnitSprite::createImage(INT width,INT height,D3D11_USAGE usage)
{
	//テクスチャの作成
	Texture* texture = NEW Texture();
	if(!texture->create(width,height,usage))
	{
		delete texture;
		return false;
	}
	//頂点の作成
	_createFromTexture(SP<Texture>(texture));
	return true;
}

bool UnitSprite::openImage(LPCSTR fileName,D3D11_USAGE usage)
{
	return openImage(UCS2(fileName),usage);
}
bool UnitSprite::openImage(LPCWSTR fileName,D3D11_USAGE usage)
{
	//テクスチャの作成
	Texture* texture = NEW Texture();
	if(!texture->open(fileName,usage))
	{
		delete texture;
		return false;
	}
	//頂点の作成
	_createFromTexture(texture);
	return true;

}
bool UnitSprite::_createFromTexture(Texture* texture)
{
	return _createFromTexture(SP<Texture>(texture));
}
bool UnitSprite::_createFromTexture(SP<Texture>& texture)
{
	GETVERTEX vertex;
	if(!_getPointVertex(&vertex,texture.get()))
		return false;

	FLOAT vertexSprite[] =
	{
		 vertex.fX1,vertex.fY1, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX,               vertex.fTY,
		 vertex.fX2,vertex.fY1, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX+vertex.fTWidth,vertex.fTY,
		 vertex.fX1,vertex.fY2, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX,               vertex.fTY+vertex.fTHeight,
		 vertex.fX2,vertex.fY2, 0.0f, 0,0,-1, 1.0f,1.0f,1.0f,1.0f, vertex.fTX+vertex.fTWidth,vertex.fTY+vertex.fTHeight
	};


	WORD wIndex[]={0,1,2,3,2,1};

	//メッシュの作成
	Mesh mesh;
	//mesh.createLayout("BILLBOARD");
	mesh.createLayout(m_vsName);
	mesh.setPShader(m_psName);
	mesh.setTexture(texture);
	mesh.createVertex(vertexSprite,sizeof(vertexSprite));
	mesh.createIndex(wIndex,sizeof(wIndex));

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	mesh.setRasterizer(&rasterizerDesc);


	//フレームの作成
	m_frame = SP<Frame>(NEW Frame());
	m_frame->add(&mesh);

	RECT clipRect = {0,0,texture->getImageWidth(),texture->getImageHeight()};
	setViewClip(&clipRect);
	resetRenderFlag();
	return true;
}
bool UnitSprite::onRenderMesh(Mesh* mesh,LPVOID value)
{
	if(getTexture())
	{
		if(getPartWidth() && getPartHeight())
		{
			FLOAT width = (FLOAT)getPartWidth()/(FLOAT)getImageWidth();
			FLOAT height = (FLOAT)getPartHeight()/(FLOAT)getImageHeight();
			FLOAT sx = width*(getPartIndex()%(INT)(1.0f/width));
			FLOAT sy = height*(getPartIndex()/(INT)(1.0f/width));
			const NVector v = 
			{
				width,height,sx,sy
			};
			if(m_frame == NULL)
				return false;
			mesh->updateVS("ConstantBuffer3",&v);

		}
		else
		{
			const NVector v = 
			{
				1.0f,1.0f,0.0f,0.0f
			};
			if(m_frame == NULL)
				return false;
			mesh->updateVS("ConstantBuffer3",&v);
		}
	}
	return true;

}


bool UnitSprite::_getPointVertex(PGETVERTEX vertexSprite,Texture* textureSrc)
{
	if(!textureSrc)
		return false;

	//サイズの保存
	INT width = textureSrc->getImageWidth();
	INT height = textureSrc->getImageHeight();
	INT textureWidth = textureSrc->getTextureWidth();
	INT textureHeight = textureSrc->getTextureHeight();

	INT widthCount;
	if(m_partWidth)
		widthCount = width / m_partWidth;
	else
		widthCount = 1;



	INT pointStat = m_pointStat;
	FLOAT centerX = m_centerX;
	FLOAT centerY = m_centerY;

	if(m_pointStat & POINT_LEFT)
	{
		vertexSprite->fX1 = centerX;
		vertexSprite->fX2 = centerX + width;
	}
	else if(m_pointStat == POINT_RIGHT)
	{
		vertexSprite->fX1 = centerX + width;
		vertexSprite->fX2 = centerX + width*2;
	}
	else //Center
	{
		vertexSprite->fX1 = centerX - width/2;
		vertexSprite->fX2 = centerX + width/2;
	}
	if(m_pointStat & POINT_TOP)
	{
		vertexSprite->fY1 = centerY;
		vertexSprite->fY2 = centerY + height;
	}
	else if(m_pointStat & POINT_BOTTOM)
	{
		vertexSprite->fY1 = centerY;
		vertexSprite->fY2 = centerY + height;
	}
	else //Center
	{
		vertexSprite->fY1 = centerY - height/2;
		vertexSprite->fY2 = centerY + height/2;
	}
	vertexSprite->fTX = 0.0f;
	vertexSprite->fTY = 0.0f;
	vertexSprite->fTWidth = (FLOAT)width/textureWidth;
	vertexSprite->fTHeight = (FLOAT)height/textureHeight;

	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitText
// TEXTスプライトユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitText::UnitText()
{
	m_color = 0xffffffff;
	m_bcolor = 0x80000000;
	m_font.setSize(16);
}
void UnitText::setText(LPCSTR text)
{
	setText(UCS2(text));
}
void UnitText::setText(LPCWSTR text)
{
	if(m_drawString != text)
	{
		resetRenderFlag();
		m_drawString = text;
	}
}
bool UnitText::onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag())
	{
		SIZE s;
		m_font.getFontSize(&s,m_drawString);
		if(getImageWidth() < s.cx+2 || getImageHeight() < s.cy+2)
		{
			createImage(s.cx + 2, s.cy + 2, D3D11_USAGE_DYNAMIC);
		}
		
		if(getTexture())
		{
			getTexture()->clear(0);
			getTexture()->drawOutlineText(0,0,m_drawString.c_str(),m_font,m_color,m_bcolor);
		}
	}
	return true;
}
void UnitText::setTextColor(DWORD color)
{
	resetRenderFlag();
	m_color = color;
}
void UnitText::setBackColor(DWORD color)
{
	resetRenderFlag();
	m_bcolor = color;
}
void UnitText::setFontSize(INT size)
{
	m_font.setSize(size);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitFPS
// DirectX - FPS表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitFPS::UnitFPS()
{
	//setTextureFilter(D3DTEXF_LINEAR);
	setZBuffer(false);
	//setPosW(2000);
	m_count = 0;
	m_time = 0;
	m_fps = 0;
	setTextColor(0xffffffff);
	setBackColor(0x80000000);
	m_font.setBold(1000);
}

bool UnitFPS::onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	INT timeCount;
	m_count++;
	timeCount = GetTickCount() - m_time;
	//setScaleX(1.0f-abs(timeCount-500)/800.0f);
	//setScaleY(1.0f-abs(timeCount-500)/800.0f);
	//setCenter(getImageWidth()/2.0f,getImageHeight()/2.0f,0);
	if(timeCount >= 1000)
	{
		INT fps;
		CHAR buff[100];
		fps = m_count*1000 / timeCount;
		if(m_fps != fps)
		{
			m_fps = fps;
			sprintf(buff,"%3d FPS",fps);
			setText(buff);
		}
		m_count = 0;
		m_time = GetTickCount();
	}
	return UnitText::onRender(world,x,y,z);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVector
// 図形用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitVector::UnitVector()
{
	setView2D(true);
	setZSort(false);
	m_flag3d = true;

	m_elementName = "VECTOR3D";
	m_vsName = "VECTOR";
	m_psName = "VCOLOR";
}
void UnitVector::create(VectorObject* vo)
{
	resetRenderFlag();
	m_vectorObject = *vo;
}
bool UnitVector::onRender(LPVOID world, FLOAT& x, FLOAT& y, FLOAT& z)
{
	if (!isRenderFlag())
		return Unit::onRender(world, x, y, z);
	INT vcount = m_vectorObject.getVertexCount();
	VERTEXVECTOR* vv = m_vectorObject.getVertexData();

	WORD* index = NEW WORD[vcount];


	INT i;
	if (m_flag3d)
	{
		VERTEXVECTOR3D* v = NEW VERTEXVECTOR3D[vcount];

		std::map<VERTEXVECTOR3D, WORD>::iterator it;
		std::map<VERTEXVECTOR3D, WORD> opMap;
		for (i = 0; i<vcount; i++)
		{
			//法線生成
			v[i] = vv[i];
			if (i % 3 == 2)
			{
				NVector vec1, vec2;
				vec1 = v[i - 1].vectPosition - v[i - 2].vectPosition;
				vec2 = v[i - 0].vectPosition - v[i - 1].vectPosition;
				vec1 = vec1.cross(vec2).normal();

				v[i - 2].vectNormal = vec1;
				v[i - 1].vectNormal = vec1;
				v[i - 0].vectNormal = vec1;
			}
		}
		for (i = 0; i<vcount; i++)
		{
			it = opMap.find(v[i]);
			if (it == opMap.end())
			{
				index[i] = (INT)opMap.size();
				opMap[v[i]] = index[i];
			}
			else
			{
				index[i] = it->second;
			}
		}

		INT vcount2 = (INT)opMap.size();
		VERTEXVECTOR3D* vv2 = NEW VERTEXVECTOR3D[vcount2];
		for (it = opMap.begin(); it != opMap.end(); ++it)
		{
			vv2[it->second] = it->first;
		}

		//メッシュの作成
		Mesh mesh;
		mesh.createLayout(m_vsName, m_elementName);
		mesh.setPShader(m_psName);
		mesh.createVertex(vv2, vcount2*sizeof(VERTEXVECTOR3D));
		mesh.createIndex(index, vcount*sizeof(WORD));

		COLOR4 c(0.5f, 0.5f, 0.5f, 1.0f);
		COLOR4 c2(0.2f, 0.2f, 0.2f, 1.0f);
		mesh.getMaterial()->Diffuse = c;
		mesh.getMaterial()->Ambient = c;
		mesh.getMaterial()->Emissive = c2;

		//フレームの作成
		m_frame = SP<Frame>(NEW Frame());
		m_frame->add(&mesh);
		delete[] vv2;
		delete[] v;
	}
	else
	{
		//頂点データから最適化インデックスの作成

		std::map<VERTEXVECTOR, WORD>::iterator it;
		std::map<VERTEXVECTOR, WORD> opMap;
		for (i = 0; i<vcount; i++)
		{
			it = opMap.find(vv[i]);
			if (it == opMap.end())
			{
				index[i] = (INT)opMap.size();
				opMap[vv[i]] = index[i];
			}
			else
			{
				index[i] = it->second;
			}
		}
		INT vcount2 = (INT)opMap.size();
		VERTEXVECTOR* vv2 = NEW VERTEXVECTOR[vcount2];
		for (it = opMap.begin(); it != opMap.end(); ++it)
		{
			vv2[it->second] = it->first;
		}

		//メッシュの作成
		Mesh mesh;
		mesh.createLayout(m_vsName, m_elementName);
		mesh.setPShader(m_psName);
		mesh.createVertex(vv2, vcount2*sizeof(VERTEXVECTOR3D));
		mesh.createIndex(index, vcount*sizeof(WORD));
		//フレームの作成
		m_frame = SP<Frame>(NEW Frame());
		m_frame->add(&mesh);
		delete[] vv2;
	}



	delete[] vv;

	delete[] index;
	return Unit::onRender(world, x, y, z);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitMap
// DirectX - チップ型2Dマップ用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitMap::UnitMap()
{
	const static D3D11_INPUT_ELEMENT_DESC layoutTexture[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		0
	};

	if(!D3DDevice::getInputElement("TEXTURE"))
	{
		D3DDevice::addInputElement("TEXTURE",layoutTexture);
	}


	setView2D(true);
	setZBuffer(false);
	setLight(false);
	//分割サイズ
	m_partWidth = 32;
	m_partHeight = 32;
	m_viewWidth = 128;
	m_viewHeight = 128;
	m_drawX = 0;
	m_drawY = 0;
	//メッシュの作成
	Mesh mesh;
	mesh.createLayout("TEXTURE","TEXTURE");
	mesh.setPShader("TEXTURE");
	//フレームの作成
	m_frame = SP<Frame>(new Frame());
	m_frame->add(&mesh);
	m_spriteMesh = m_frame->getMesh();

	m_map = NULL;
}
UnitMap::~UnitMap()
{
	delete[] m_map;
}
void UnitMap::setDrawPoint(INT x,INT y)
{
	if(m_drawX != x || m_drawY != y)
	{
		m_drawX = x;
		m_drawY = y;
		resetRenderFlag();
	}
}
void UnitMap::setViewSize(INT x,INT y)
{
	m_viewWidth = x;
	m_viewHeight = y;
}
INT UnitMap::getDrawPointX() const
{
	return m_drawX;
}
INT UnitMap::getDrawPointY() const
{
	return m_drawY;
}

void UnitMap::setMapSize(INT width,INT height)
{
	INT size = width*height;
	m_map = new BYTE[size];
	m_mapWidth = width;
	m_mapHeight = height;
	ZeroMemory(m_map,size);
	resetRenderFlag();
}
void UnitMap::setMapIndex(INT x,INT y,INT index)
{
	if(x>=0 && x<m_mapWidth && y>=0 && y<m_mapHeight)
		m_map[y*m_mapWidth+x] = index;
	resetRenderFlag();
}
INT UnitMap::getMapIndex(INT x,INT y)
{
	if(x>=0 && x<m_mapWidth && y>=0 && y<m_mapHeight)
		return (INT)m_map[y*m_mapWidth+x];
	return 0;
}

bool UnitMap::openImage(LPCSTR fileName)
{
	return openImage(UCS2(fileName));
}
bool UnitMap::openImage(LPCWSTR fileName)
{
	//テクスチャの作成
	SP<Texture> texture = new Texture();
	if(!texture->open(fileName))
		return false;
	Material material;
 	//メッシュの作成
	m_spriteMesh->addTexture(texture);
	m_spriteMesh->setMaterial(material);


	resetRenderFlag();
	return true;
}
void UnitMap::setVertex(MAPVERTEX* vertex,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2,FLOAT tx1,FLOAT ty1,FLOAT tx2,FLOAT ty2)
{
	vertex[0].pos.x = x1;
	vertex[0].pos.y = y1;
	vertex[0].tex.x = tx1;
	vertex[0].tex.y = ty1;

	vertex[1].pos.x = x2;
	vertex[1].pos.y = y1;
	vertex[1].tex.x = tx2;
	vertex[1].tex.y = ty1;

	vertex[2].pos.x = x1;
	vertex[2].pos.y = y2;
	vertex[2].tex.x = tx1;
	vertex[2].tex.y = ty2;

	vertex[3].pos.x = x2;
	vertex[3].pos.y = y2;
	vertex[3].tex.x = tx2;
	vertex[3].tex.y = ty2;
}
Texture* UnitMap::getTexture() const
{
	if(m_spriteMesh && m_spriteMesh->getMaterial())
		return m_spriteMesh->getTexture();
	else
		return NULL;
}
bool UnitMap::setVertex()
{
	Texture* texture = getTexture();
	if(!texture)
		return false;
	INT width = texture->getImageWidth();
	INT height = texture->getImageHeight();
	INT textureWidth = texture->getTextureWidth();
	INT textureHeight = texture->getTextureHeight();
	FLOAT txWidth = (FLOAT)width/textureWidth/(width/m_partWidth);
	FLOAT txHeight = (FLOAT)height/textureHeight/(height/m_partHeight);


	INT countX = m_viewWidth/m_partWidth+1;
	INT countY = m_viewHeight/m_partHeight+1;
	INT vertexCount = countX*countY*4;
	MAPVERTEX* vertexSprite = new MAPVERTEX[vertexCount];
	INT i,j;
	INT index = 0;

	INT pdx = (INT)m_drawX % m_partWidth;
	INT pdy = (INT)m_drawY % m_partHeight;
	INT indexX = (INT)m_drawX / m_partWidth;
	INT indexY = (INT)m_drawY / m_partHeight;
	if(pdx < 0)
	{
		pdx = m_partWidth + pdx;
		--indexX;
	}
	if(pdy < 0)
	{
		pdy = m_partHeight + pdy;
		--indexY;
	}
	for(j=0;j<countY;j++)
	{
		for(i=0;i<countX;i++)
		{
			FLOAT x1 = (FLOAT)i*m_partWidth-pdx;
			FLOAT y1 = (FLOAT)j*m_partHeight-pdy;
			FLOAT x2 = x1 + m_partWidth;
			FLOAT y2 = y1 + m_partHeight;
			INT mapIndex = getMapIndex(i+indexX,j+indexY);
			FLOAT ix = (mapIndex % m_partWidth)*txWidth;
			FLOAT iy = (mapIndex / m_partHeight)*txHeight;
			setVertex(&vertexSprite[index],x1,y1,x2,y2,ix,iy,ix+txWidth-0.005f,iy+txHeight-0.005f);
			index += 4;
		}
	}
	m_spriteMesh->createVertex(vertexSprite,vertexCount*sizeof(MAPVERTEX));
	delete[] vertexSprite;


	INT indexCount = countX*countY;
	LPWORD indexData = new WORD[indexCount*6];
	for(i=0;i<indexCount;i++)
	{
		indexData[i*6+0] = i*4+0;
		indexData[i*6+1] = i*4+1;
		indexData[i*6+2] = i*4+2;
		indexData[i*6+3] = i*4+3;
		indexData[i*6+4] = i*4+2;
		indexData[i*6+5] = i*4+1;
	}

	m_spriteMesh->createIndex(indexData,indexCount*6*sizeof(WORD));
	delete[] indexData;
	return true;
}
bool UnitMap::onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag())
	{
		setVertex();
	}
	return true;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GDIPlus
// GDIPlus初期化用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
GDIPlus::GDIPlus()
{
	m_token=0;
	Gdiplus::GdiplusStartupInput si;
	Gdiplus::GdiplusStartup(&m_token, &si, NULL);
}
GDIPlus::~GDIPlus()
{
	Gdiplus::GdiplusShutdown(m_token);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitGdip
// GDIPlus表示用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
GDIPlus UnitGdip::m_gdiPlus;

UnitGdip::UnitGdip()
{
	m_bitmap = NULL;
	m_graphics = NULL;
}

UnitGdip::~UnitGdip()
{
	releaseGraphics();
}
bool UnitGdip::onRender(LPVOID value,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(isRenderFlag())
	{
		if(m_bitmap)
		{
			if(m_bitmap->GetWidth() != getImageWidth() || m_bitmap->GetHeight() != getImageHeight())
			{
				createImage(m_bitmap->GetWidth(),m_bitmap->GetHeight());
			}
			Texture* texture = getTexture();
			if(texture)
				D3DDevice::getContext()->UpdateSubresource(*texture,0,NULL,m_data.get(),m_bitmap->GetWidth()*4,0);
		}

	}
	return UnitSprite::onRender(value,x,y,z);
}

void UnitGdip::setSize(INT width,INT height)
{
	releaseGraphics();

	m_data = new BYTE[width*4*height];
	ZeroMemory(m_data.get(),width*4*height);
	m_bitmap = new Gdiplus::Bitmap(width,height,width*4,PixelFormat32bppARGB,m_data.get());
	m_graphics = new Gdiplus::Graphics(m_bitmap);
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	m_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
}

Gdiplus::Graphics* UnitGdip::getGraphics()
{
	resetRenderFlag();
	return m_graphics;
}
void UnitGdip::releaseGraphics()
{
	if(m_graphics)
	{
		m_graphics->Flush(Gdiplus::FlushIntentionSync);
		delete m_graphics;
		m_graphics = NULL;
	}
	if(m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = NULL;
	}
}
void UnitGdip::clear(DWORD color)
{
	
	Gdiplus::Graphics* g = getGraphics();
	if(g)
		g->Clear(Gdiplus::Color(color));
		
	//getTexture()->clear(color);
}
void UnitGdip::drawLine(INT x,INT y,INT x2,INT y2,DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if(g)
		g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(color)),x, y, x2,y2);
}
void UnitGdip::drawBoxLine(INT x,INT y,INT width,INT height,DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if(g)
	{
		g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(color)),x, y, x+width,y);
		g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(color)),x, y, x,y+height);
		g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(color)),x, y+height, x+width,y+height);
		g->DrawLine(&Gdiplus::Pen(Gdiplus::Color(color)),x+width, y, x+width,y+height);
	}
}
void UnitGdip::drawBox(INT x,INT y,INT width,INT height,DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if(g)
		g->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(color)),x, y, width, height);
}


void UnitGdip::drawString(LPCWSTR text,HFONT font,DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if(!g)
		return;
	
	HDC hDC = CreateCompatibleDC(NULL);
	
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	g->DrawString(text, (INT)wcslen(text), &Gdiplus::Font(hDC, font),
		Gdiplus::PointF(0,0),Gdiplus::StringFormat::GenericTypographic(),&Gdiplus::SolidBrush(Gdiplus::Color(color)));
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	DeleteDC(hDC);
}
void UnitGdip::drawString(INT x,INT y,LPCWSTR text,HFONT font,DWORD color)
{
	Gdiplus::Graphics* g = getGraphics();
	if(!g)
		return;
	
	HDC hDC = CreateCompatibleDC(NULL);
	
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	g->DrawString(text, (INT)wcslen(text), &Gdiplus::Font(hDC, font),
		Gdiplus::PointF((FLOAT)x, (FLOAT)y), Gdiplus::StringFormat::GenericTypographic(), &Gdiplus::SolidBrush(Gdiplus::Color(color)));
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);

	DeleteDC(hDC);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitEffect
// エフェクト用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitEffect::UnitEffect()
{
	const static D3D11_INPUT_ELEMENT_DESC layoutSkin4[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ROTATION", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		0
	};

	if (!D3DDevice::getInputElement("EFFECT"))
	{
		D3DDevice::addInputElement("EFFECT", layoutSkin4);
	}
	setPosW(1.0f);
}
bool UnitEffect::openImage(LPCSTR fileName)
{
	return openImage(UCS2(fileName));
}

bool UnitEffect::openImage(LPCWSTR fileName)
{
	//テクスチャの作成
	Texture* texture = new Texture();
	if (!texture->open(fileName))
	{
		delete texture;
		return false;
	}

	//メッシュの作成
	Mesh mesh;
	mesh.createLayout("EFFECT", "EFFECT");
	mesh.setPShader("TEXTURE");
	mesh.setTexture(texture);


	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mesh.setDepthStencil(&desc);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	mesh.setBlend(&blendDesc);

	addMesh(&mesh);

	return true;
}
void UnitEffect::add(FLOAT x, FLOAT y, FLOAT z, FLOAT size, DWORD color, INT count,FLOAT rot)
{
	EffectData data;
	data.pos.x = x;
	data.pos.y = y;
	data.pos.z = z;
	data.size = size;
	data.rot = rot/180.0f*3.141592f;
	data.color.x = (color & 0xff) / 255.0f;
	data.color.y = ((color >> 8) & 0xff) / 255.0f;
	data.color.z = ((color >> 16) & 0xff) / 255.0f;
	data.color.w = (color >> 24) / 255.0f;
	data.count = count;
	data.countNow = 0;
	m_effectData.push_back(data);
	resetRenderFlag();
}
void UnitEffect::clear()
{
	m_effectData.clear();
	resetRenderFlag();
}

bool UnitEffect::onRenderMesh(Mesh* mesh, LPVOID value)
{
	if (isRenderFlag())
	{
		INT vsize = sizeof(FLOAT)* 11 * 4 * m_effectData.size();
		PFLOAT vertex = new FLOAT[vsize];
		INT i = 0;
		for (auto it = m_effectData.begin(); it != m_effectData.end(); ++it, i++)
		{
			float alpha = (it->count - it->countNow) / (FLOAT)it->count*0.5f+0.5f;
			FLOAT v[] =
			{
				it->pos.x, it->pos.y, it->pos.z, it->size, it->rot, it->color.x, it->color.y, it->color.z, it->color.w*alpha, 0, 0,
				it->pos.x, it->pos.y, it->pos.z, it->size, it->rot, it->color.x, it->color.y, it->color.z, it->color.w*alpha, 1, 0,
				it->pos.x, it->pos.y, it->pos.z, it->size, it->rot, it->color.x, it->color.y, it->color.z, it->color.w*alpha, 0, 1,
				it->pos.x, it->pos.y, it->pos.z, it->size, it->rot, it->color.x, it->color.y, it->color.z, it->color.w*alpha, 1, 1
			};
			CopyMemory(&vertex[i * 44], v, sizeof(FLOAT)* 11 * 4);
		}
		Mesh* mesh = getMesh();
		if (mesh)
		{
			mesh->createVertex(vertex, vsize);
			mesh->createIndexAuto(m_effectData.size());
		}
		delete[] vertex;
	}

	return true;
}
void UnitEffect::onAction(LPVOID value)
{
	for (auto it = m_effectData.begin(); it != m_effectData.end();)
	{
		if (it->count != -1)
		{
			auto it2 = it;
			++it;
			if (it2->count == it2->countNow)
			{
				m_effectData.erase(it2);
				resetRenderFlag();
			}
			else
				it2->countNow++;
		}
		else
			++it;
	}
}
}