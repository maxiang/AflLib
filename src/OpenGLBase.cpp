#ifndef __ARM_NEON__
	#include <windows.h>
#endif

#include "afl3DBase.h"

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
// FileObject
// ファイル入出力用モデル情報コンテナ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void FileObject::changeCoordinate()
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		changeCoordinate(&*itFrame);
	}
	std::map<std::string,ANIMATIONSET>::iterator itAnime;
	for(itAnime=anime.begin();itAnime!=anime.end();++itAnime)
	{
		changeCoordinate(&itAnime->second);
	}

}
void FileObject::changeCoordinate(ANIMATIONSET* animeSet)
{
	std::map<std::string,AnimationKey>::iterator itKey;
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



void FileObject::getBoneNameList(std::set<std::string>& listBone)
{
	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		getBoneNameList(listBone,&*itFrame);
	}
}
void FileObject::getBoneNameList(std::set<std::string>& listBone,FrameData* frameData)
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
	NVector rotation = {y*NPI/180,x*NPI/180,z*NPI/180,0.0f};
	rotation = rotation.quotanion();
	rotation.z = -rotation.z;
	matrix.rotationRollPitchYaw(y*NPI/180,x*NPI/180,z*NPI/180);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->matrix *= matrix;

		std::map<std::string,ANIMATIONSET>::iterator itAnimeSet;
		for(itAnimeSet=anime.begin();itAnimeSet!=anime.end();++itAnimeSet)
		{
			std::map<std::string,AnimationKey>::iterator itAnime = 
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
	matrix.scaling(x,y,z);

	std::list<FrameData>::iterator itFrame;
	for(itFrame=frame.begin();itFrame!=frame.end();++itFrame)
	{
		itFrame->matrix *= matrix;

		std::map<std::string,ANIMATIONSET>::iterator itAnimeSet;
		for(itAnimeSet=anime.begin();itAnimeSet!=anime.end();++itAnimeSet)
		{
			std::map<std::string,AnimationKey>::iterator itAnime = 
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
		pMatrix->rotationQuaternion(q);
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

}