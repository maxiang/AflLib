#pragma once

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

#include "aflMath.h"

namespace AFL{


struct Rect2DF 
{
	FLOAT left,right;
	FLOAT top,bottom;
};
struct Rect3DF
{
	Rect3DF();
	Rect3DF(FLOAT Left,FLOAT Top,FLOAT High,FLOAT Right,FLOAT Bottom,FLOAT Low);
	Rect3DF(FLOAT Left,FLOAT Top,FLOAT Right,FLOAT Bottom);
	void setRect(FLOAT Left,FLOAT Top,FLOAT Right,FLOAT Bottom);
	void setRect(FLOAT Left,FLOAT Top,FLOAT High,FLOAT Right,FLOAT Bottom,FLOAT Low);
	void offsetRect(FLOAT X,FLOAT Y,FLOAT Z=0);
	bool isRectNull() const;
	FLOAT getWidth(){return right - left;}
	FLOAT getHeight(){return bottom - top;}

	FLOAT left,right;
	FLOAT top,bottom;
	FLOAT high,low;
};
struct Size2DF
{
	FLOAT cx,cy;
};
void decompMatrix(const NMatrix& m,NVector& p,NVector& s,NVector& q);

struct COLOR3
{
	union
	{
		struct
		{
			FLOAT r,g,b;
		};
	};
	COLOR3(){}
	COLOR3(FLOAT r,FLOAT g,FLOAT b)
	{
		COLOR3::r = r;
		COLOR3::g = g;
		COLOR3::b = b;
	}
	bool operator < ( const COLOR3& color) const
	{
		if(r < color.r) return true;
		if(r == color.r)
		{
			if(g < color.g)
				return true;
			if(g == color.g)
			{
				if(b < color.b)
					return true;
			}
		}
		return false;
	}
};

struct COLOR4
{
	FLOAT r,g,b,a;

	COLOR4(){}
	COLOR4(FLOAT r,FLOAT g,FLOAT b,FLOAT a)
	{
		COLOR4::r = r;
		COLOR4::g = g;
		COLOR4::b = b;
		COLOR4::a = a;
	}
	COLOR4(NVector s)
	{
		*(NVector*)this = s;
	}

	COLOR4& operator=(const NVector& s)
	{
		*(NVector*)this = s;
		return *this;
	}
	COLOR4& operator=(const COLOR3& c)
	{
		COLOR4::r = c.r;
		COLOR4::g = c.g;
		COLOR4::b = c.b;
		COLOR4::a = 1.0f;
		return *this;
	}
	operator NVector() const
	{
		return *(NVector*)this;
	}
	bool operator < ( const COLOR4& color) const
	{
		if(r < color.r) return true;
		if(r == color.r)
		{
			if(g < color.g)
				return true;
			if(g == color.g)
			{
				if(b < color.b)
					return true;
				if(b == color.b && a < color.a)
					return true;
			}
		}
		return false;
	}
};
struct Material
{
	COLOR4 Diffuse;
	COLOR4 Ambient;
	COLOR4 Specular;
	COLOR4 Emissive;
	FLOAT Power;

};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VERTEXBASE
// 基本頂点情報格納用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct VERTEXBASE
{
	//重複チェック用
	bool operator==(const VERTEXBASE& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXBASE)) == 0;
	}
	bool operator<(const VERTEXBASE& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXBASE)) < 0;
	}
	bool isOverride(const VERTEXBASE& v) const
	{
		NVector3 p = vectPosition - v.vectPosition;
				
		if(p.abs()  < 0.0001f)
			return vectNormal != v.vectNormal;
		else 
			return false;
	}

	NVector3 vectPosition;		// オブジェクト座標
	FLOAT fBlend[3];			//頂点ブレンドウエイト
	union
	{
		BYTE byBlendIndex[4];	//頂点ブレンドインデックス
		DWORD dwBlendIndex;
	};
	NVector3 vectNormal;		// オブジェクト法線
	COLOR4 color;				// 色情報
	FLOAT fTu,fTv;				// テクスチャ座標
	NVector3 tan;				//TANGENT
	NVector3 bin;				//BINORMAL
};
struct VERTEXBASE2
{
	//重複チェック用
	bool operator==(const VERTEXBASE2& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXBASE2)) == 0;
	}
	bool operator<(const VERTEXBASE2& vt) const
	{
		return memcmp(this,&vt,sizeof(VERTEXBASE2)) < 0;
	}
	bool isOverride(const VERTEXBASE2& v) const
	{
		if (vectNormal == v.vectNormal)
			return false;
		NVector p = vectPosition - v.vectPosition;
		return p.abs() < 0.0001f;
	}

	NVector vectPosition;		// オブジェクト座標
	FLOAT fBlend[3];			//頂点ブレンドウエイト
	INT blendIndex[4];
	NVector vectNormal;			// オブジェクト法線
	NVector color;				// 色情報
	NVector2 uv;				// テクスチャ座標
	NVector tan;				//TANGENT
	NVector bin;				//BINORMAL
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AnimationKey
// アニメーションキー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct AnimationKey
{ 
	std::map<INT,NVector> position;
	std::map<INT,NVector> scale;
	std::map<INT,NVector> rotation;
	std::map<INT,NMatrix> matrix;

	DWORD getAllCount() const;
	bool getAnimationKey(DWORD dwTime,NMatrix* matrix,NVector* position,NVector* scale,NVector* rotation,bool loop);
	bool getAnimationKey2(DWORD dwTime,NMatrix* matrix,NVector* position,NVector* scale,NVector* rotation,bool loop);
	bool getAnimationMatrix(DWORD dwTime,NMatrix* pMatrix,bool loop);

	void getMatrix(DWORD dwTime,NMatrix* pMatrix,bool loop);
	void getPosition(DWORD dwTime,NVector* pVector,bool loop);
	void getScale(DWORD dwTime,NVector* pVector,bool loop);
	void getRotation(DWORD dwTime,NVector* pQuatenion,bool loop);
	void getMatrix2(DWORD dwTime,NMatrix* pMatrix,bool loop);
	void getPosition2(DWORD dwTime,NVector* pVector,bool loop);
	void getScale2(DWORD dwTime,NVector* pVector,bool loop);
	void getRotation2(DWORD dwTime,NVector* pQuatenion,bool loop);

	void optimize();

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Animation
// アニメーションデータ管理
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Animation
{
public:
	Animation();
	bool getAnimationMatrix(LPCSTR pFrameName,NMatrix* pNMatrix);
	INT getAnimationKey(LPCSTR pFrameName,NMatrix*  NMatrix,NVector* position,NVector* scale,NVector* rotation);
	INT getAnimationKey2(LPCSTR pFrameName,NMatrix* NMatrix,NVector* position,NVector* scale,NVector* rotation);
	AnimationKey* getAnimationKey(LPCSTR pFrameName);
	INT getAnimationCount() const;
	void setAnimation(String name,std::map<String,AnimationKey>& animationKey);
	void setTimeCount(DWORD dwCount);
	void setTimeStart(DWORD dwCount);
	void setTimeLength(DWORD dwCount);
	DWORD getTimeCount()const;
	DWORD getTimeStart()const;
	DWORD getTimeLength()const;
	void setTimeChange(DWORD count);
	DWORD getTimeChange()const;
	void setTimeChangeWork(DWORD count);
	DWORD getTimeChangeWork()const;
	LPCSTR getName()const;
	void setLoop(bool loop);
	bool isAnimation() const;
	DWORD getLastTime() const;

protected:
	String m_name;
	std::map<String,AnimationKey> m_animationKey;
	DWORD m_animationTime;
	DWORD m_animationTimeStart;
	DWORD m_animationTimeLength;
	DWORD m_animationTimeChange;
	DWORD m_animationTimeChangeWork;
	DWORD m_animationTimeLast;
	bool m_loop;
};


struct INDEX4
{
	DWORD type;
	DWORD data[4];
};
struct TEXTUREUV
{
	bool operator < ( const TEXTUREUV& a) const
	{
		if(u < a.u) return true;
		else if(u == a.u && v < a.v) return true;
		return false;
	}

	FLOAT u,v;
};

struct BONEMATRIX
{
	String name;
	NMatrix matrix;
};
struct BoneData
{
	NMatrix matrix;
	String name;
	std::map<INT,FLOAT> weight;
};
struct MaterialData
{
	Material material;
	std::list<String> name;
};


struct MeshData
{
	std::vector<INDEX4> vertexIndex;
	std::vector<NVector3> vertexData;
	std::vector<INDEX4> normalIndex;
	std::vector<NVector3> normalData;
	std::vector<INDEX4> uvIndex;
	std::vector<TEXTUREUV> uvData;
	std::vector<INDEX4> colorIndex;
	std::vector<COLOR4> colorData;
	std::vector<BoneData> boneData;
	std::vector<MaterialData> materialData;
	std::vector<DWORD> materialIndex;

	void clearPath()
	{
		std::vector<MaterialData>::iterator itMat;
		for(itMat=materialData.begin();itMat!=materialData.end();++itMat)
		{
			std::list<String>::iterator it;
			for(it=itMat->name.begin();it!=itMat->name.end();++it)
			{
				LPCSTR p1 = strrchr(it->c_str(),'\\');
				LPCSTR p2 = strrchr(it->c_str(),'/');
				if(p2 > p1)
					p1 = p2;
				if(p1)
					*it = p1+1;
			}
		}
	}
};
struct FrameData
{
	String name;
	NMatrix matrix;
	MeshData mesh;
	std::list<FrameData> frameChild;
	void clearNormal()
	{
		mesh.normalData.clear();
		mesh.normalIndex.clear();
		std::list<FrameData>::iterator it;
		for(it=frameChild.begin();it!=frameChild.end();++it)
			it->clearNormal();
	}
	void clearPath()
	{
		mesh.clearPath();
		std::list<FrameData>::iterator it;
		for(it=frameChild.begin();it!=frameChild.end();++it)
			it->clearPath();
	}
	void clearVColor()
	{
		mesh.colorData.clear();
		mesh.colorIndex.clear();
		std::list<FrameData>::iterator it;
		for(it=frameChild.begin();it!=frameChild.end();++it)
			it->clearVColor();
	}
	void clearSkin()
	{
		mesh.boneData.clear();
		std::list<FrameData>::iterator it;
		for(it=frameChild.begin();it!=frameChild.end();++it)
			it->clearSkin();
	}
	void clearUV()
	{
		mesh.uvData.clear();
		mesh.uvIndex.clear();
		std::list<FrameData>::iterator it;
		for(it=frameChild.begin();it!=frameChild.end();++it)
			it->clearUV();
	}
};
struct ANIMATIONSET
{
	std::map<String,AnimationKey> keys;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MemoryFile
// ファイルデータメモリアクセス用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class MemoryFile
{
public:
	MemoryFile();
	~MemoryFile();
	void setData(LPVOID data,INT size);
	bool open(LPCSTR fileName);
#ifndef __ANDROID__
	bool open(LPCWSTR fileName);
#endif
	void release();
	INT getAllSize() const;
	INT getSize() const;
	LPCSTR getData();
	bool seek(INT seek);
	bool read(LPVOID dest,INT size);
protected:
	LPSTR m_data;
	INT m_size;
	INT m_seek;
	bool m_flag;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// FileObject
// ファイル入出力用モデル情報コンテナ
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct FileObject
{
	void optimize();
	void optimize(MeshData* meshData);
	void optimize(FrameData* frameData);
	void setRotation(FLOAT x,FLOAT y,FLOAT z);
	void setScale(FLOAT x,FLOAT y,FLOAT z);

	void changeCoordinate();
	void changeCoordinate(FrameData* frameData);
	void changeCoordinate(MeshData* meshData);
	void changeCoordinate(ANIMATIONSET* animeSet);
	void clearNormal();
	void clearPath();
	void clearAnime();
	void clearVColor();
	void clearSkin();
	void clearUV();

	void triConvert();
	void triConvert(FrameData* frameData);
	void triConvert(MeshData* meshData);

	void setNormal(NVector vector);
	void setNormal(FrameData* frameData,NVector vector);
	void setCull();
	void setCull(FrameData* frameData);
	
	void getBoneNameList(std::set<String>& listBone);
	void getBoneNameList(std::set<String>& listBone,FrameData* frameData);

	std::list<FrameData> frame;
	std::map<String,ANIMATIONSET> anime;

	bool save(LPCSTR fileName);
	bool save(LPCWSTR fileName);
	bool load(LPCSTR fileName);
	bool load(LPCWSTR fileName);
	bool loadMemory(MemoryFile& mf);
	bool loadMOD(MemoryFile& mf);
	bool loadPMX(MemoryFile& mf);

	WString m_textModel1;
	WString m_textModel2;
	WString m_textComment;
	WString m_textComment2;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MeshOptimize
// レンダリング用メッシュ展開用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct MeshOptimize
{
	MeshOptimize()
	{
		indexData = NULL;
		vertexData = NULL;
		material = NULL;
	}
	~MeshOptimize()
	{
		if(indexData)
			delete[] indexData;
		if(vertexData)
			delete[] vertexData;
		if(material)
			delete material;
	}
	String textureName;
	Material* material;
	LPWORD indexData;
	VERTEXBASE* vertexData;
	DWORD indexCount;
	DWORD vertexCount;
	std::vector<BONEMATRIX> boneMatrixs;
};

struct MESHOPTIMIZE
{
	std::list<String> textureName;
	Material material;
	std::map<VERTEXBASE2,WORD> vertex;
	std::vector<WORD> index;
	std::vector<BONEMATRIX> boneMatrixs;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Camera
// ビュー設定用基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Camera
{
public:
	Camera()
	{
		m_matExt.setIdentity();
		m_cull = 3;
		m_leftHand = true;
	}
	virtual bool setAngle(UINT uDeviceWidth,UINT uDeviceHeight) = 0;
	NMatrix& getView(){return m_matView;}
	NMatrix& getBaseView(){return m_matViewBase;}
	NMatrix& getProjection(){return m_matProjection;}
	NMatrix& getExt(){return m_matExt;}
	void setNMatrix(NMatrix* NMatrix)
	{
		m_matViewBase = *NMatrix;
	}
	INT getCull()const
	{
		return 	m_cull;
	}
	bool isLeftHand()const{ return m_leftHand; }
	void setLeftHand(bool flag)
	{
		m_leftHand = flag;
	}
protected:
	bool m_leftHand;
	NMatrix m_matView;		//ビュー
	NMatrix m_matViewBase;	//基本位置
	NMatrix m_matProjection;	//投影
	NMatrix m_matExt;		//拡張
	INT m_cull;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Camera3D
// 3Dビュー設定用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Camera3D : public Camera
{
public:
	Camera3D();

	bool setAngle(UINT uDeviceWidth,UINT uDeviceHeight);

	void setPosX(float fPoint){m_x=fPoint;} 
	void setY(float fPoint){m_y=fPoint;} 
	void setZ(float fPoint){m_z=fPoint;} 
	float getX(){return m_x;} 
	float getY(){return m_y;} 
	float getZ(){return m_z;} 
	void setRotationX(float fRot){m_rotX=fRot;} 
	void setRotationY(float fRot){m_rotY=fRot;} 
	void setRotationZ(float fRot){m_rotZ=fRot;} 
	float getRotationX(){return m_rotX;} 
	float getRotationY(){return m_rotY;} 
	float getRotationZ(){return m_rotZ;} 
	FLOAT getBaseDepth()const{return m_depth;}

	void getVector(NVector* pVect,FLOAT fX,FLOAT fY);

	void resetPoint();
protected:
	NMatrix m_matBase3D;
	FLOAT m_depth;
	FLOAT m_x,m_y,m_z;
	FLOAT m_rotX,m_rotY,m_rotZ;
	UINT m_deviceWidth,m_deviceHeight;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// VectorObject
// DirectX - 図形用頂点データ管理クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

enum VECTOR_COMMAND
{
	VECTOR_LINE,
	VECTOR_BOX,
	VECTOR_TRIANGLE,
	VECTOR_QUADRANGLE,
};
struct VectorData
{
	VECTOR_COMMAND cmd;
	FLOAT x, y, z;
	FLOAT x2, y2, z2;
	FLOAT x3, y3, z3;
	FLOAT x4, y4, z4;
	FLOAT width;
	FLOAT height;
	FLOAT depth;
	DWORD color1;
	DWORD color2;
	FLOAT bold;
};
struct VERTEXVECTOR
{
	bool operator<(const VERTEXVECTOR& vt) const
	{
		return memcmp(this, &vt, sizeof(VERTEXVECTOR)) < 0;
	}
	bool operator==(const VERTEXVECTOR& vt) const
	{
		return memcmp(this, &vt, sizeof(VERTEXVECTOR)) == 0;
	}
	NVector3 vectPosition;	// オブジェクト座標
	NVector color;			// 色情報			        
};


struct VERTEXVECTOR3D
{
	bool operator=(const VERTEXVECTOR& vt)
	{
		vectPosition = vt.vectPosition;
		color = vt.color;
		return true;
	}
	bool operator<(const VERTEXVECTOR3D& vt) const
	{
		return memcmp(this, &vt, sizeof(VERTEXVECTOR3D)) < 0;
	}
	bool operator==(const VERTEXVECTOR3D& vt) const
	{
		return memcmp(this, &vt, sizeof(VERTEXVECTOR3D)) == 0;
	}
	NVector3 vectPosition;	// オブジェクト座標
	NVector3 vectNormal;
	NVector color;		// 色情報			        
};
class VectorObject
{
public:
	VectorObject();
	void drawBox(FLOAT x, FLOAT y, FLOAT width, FLOAT height, DWORD color);
	void drawLine(FLOAT x, FLOAT y, FLOAT x2, FLOAT y2, DWORD color, FLOAT bold = 1.0f);
	void drawLineBox(FLOAT x, FLOAT y, FLOAT width, FLOAT height, DWORD color, FLOAT bold = 1.0f);
	void drawTriangle(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, DWORD color);
	INT getVertexCount() const;
	VERTEXVECTOR* getVertexData();
	void add(VectorObject* vo);
protected:
	std::list<VectorData> m_vectorData;
};
class VectorObject3D : public VectorObject
{
public:
	void drawQuadrangle(FLOAT x, FLOAT y, FLOAT z, FLOAT x2, FLOAT y2, FLOAT z2, DWORD color, FLOAT bold = 1.0f);
	void drawLine(FLOAT x, FLOAT y, FLOAT z, FLOAT x2, FLOAT y2, FLOAT z2, DWORD color, FLOAT bold = 1.0f);
	void drawTriangle(FLOAT x1, FLOAT y1, FLOAT z1, FLOAT x2, FLOAT y2, FLOAT z2, FLOAT x3, FLOAT y3, FLOAT z3, DWORD color);
	void drawLineBox(FLOAT x, FLOAT y, FLOAT z, FLOAT width, FLOAT height, FLOAT depth, DWORD color, FLOAT bold = 1.0f);
	void drawBox(NVector3 v1, NVector3 v2, NVector3 v3, NVector3 v4, DWORD color, FLOAT bold = 1.0f);
};

}
