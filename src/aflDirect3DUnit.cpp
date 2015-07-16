#include <windows.h>

#include "aflStd.h"
#include "aflDirect3DUnit.h"
#include "aflDirect3DWorld.h"
#ifdef _AFL_D3DX
	#include "aflD3DXFile.h"
#endif

#ifdef _MSC_VER
	#ifdef _DEBUG	//���������[�N�e�X�g
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


namespace AFL{namespace DIRECT3D{
using namespace Gdiplus;
#include "fx/skinnedmesh.h"

#define D3DFVF_VERTEXBASE (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE)

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Unit
// ���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// Unit::Unit()
// -----  ����  -----
// �R���X�g���N�^
// -----  ����  -----
// ����
// ----- �߂�l -----
// ����
//-----------------------------------------------
Unit::Unit()
{
	m_alpha = 100;
	m_point.x = 0.0f;
	m_point.y = 0.0f;
	m_point.z = 0.0f;
	m_w = 0;
	m_scale.x = 1.0f;
	m_scale.y = 1.0f;
	m_scale.z = 1.0f;
	m_rotation.x = 0.0f;
	m_rotation.y = 0.0f;
	m_rotation.z = 0.0f;
	m_relativity.x = 0.0f;
	m_relativity.y = 0.0f;
	m_relativity.z = 0.0f;
	m_clipPoint.x = 0.0f;
	m_clipPoint.y = 0.0f;
	m_clipPoint.z = 0.0f;
	m_clipSize.x = 0.0f;
	m_clipSize.y = 0.0f;
	m_clipSize.z = 0.0f;
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
	m_renderFlag = false;
	m_chainClip = false;
	m_unitParent = NULL;
	m_viewClip.top = 0;
	m_viewClip.bottom = 0;
	m_viewClip.left = 0;
	m_viewClip.right = 0;
	m_center.x = 0;
	m_center.y = 0;
	m_center.z = 0;
	m_textureFilter = D3DTEXF_POINT;
}
//-----------------------------------------------
// Unit::~Unit()
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// void Unit::setTextureFilter(D3DTEXTUREFILTERTYPE filter)
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setTextureFilter(D3DTEXTUREFILTERTYPE filter)
{
	m_textureFilter = filter;
}
//-----------------------------------------------
// D3DTEXTUREFILTERTYPE Unit::getTextureFilter() const
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
D3DTEXTUREFILTERTYPE Unit::getTextureFilter() const
{
	return m_textureFilter;
}
//-----------------------------------------------
// bool Unit::isViewClip() const
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isViewClip() const
{
	return m_clip;
}
//-----------------------------------------------
// void Unit::setViewClip(bool flag)
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setViewClip(bool flag)
{
	m_clip = flag;
}
//-----------------------------------------------
// void Unit::setPosX(FLOAT x)
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPosX(FLOAT x)
{
	setPos(x,getPosY(),getPosZ(),getPosW());
}
//-----------------------------------------------
// void Unit::setPosY(FLOAT y)
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPosY(FLOAT y)
{
	setPos(getPosX(),y,getPosZ(),getPosW());
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPosZ(FLOAT z)
{
	setPos(getPosX(),getPosY(),z,getPosW());
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPosW(FLOAT w)
{
	setPos(getPosX(),getPosY(),getPosZ(),w);
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y)
{
	setPos(x,y,getPosZ(),getPosW());
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y,FLOAT z)
{
	setPos(x,y,z,getPosW());
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setPos(FLOAT x,FLOAT y,FLOAT z,FLOAT w)
{
	m_point.x = x;
	m_point.y = y;
	m_point.z = z;
	m_w = w;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getPosX() const
{
	return m_point.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getPosY() const
{
	return m_point.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getPosZ() const
{
	return m_point.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getPosW() const
{
	return m_w;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setScaleX(FLOAT fScale)
{
	m_scale.x = fScale;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setScaleY(FLOAT fScale)
{
	m_scale.y = fScale;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setScaleZ(FLOAT fScale)
{
	m_scale.z = fScale;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setScale(FLOAT fScale)
{
	m_scale.x = fScale;
	m_scale.y = fScale;
	m_scale.z = fScale;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleX()
{
	return m_scale.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleY()
{
	return m_scale.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getScaleZ()
{
	return m_scale.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterX() const
{
	return m_center.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterY() const
{
	return m_center.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getCenterZ() const
{
	return m_center.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setChainW(bool flag)
{
	m_chainW = flag;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isChainW() const
{
	return m_chainW;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setChainClip(bool flag)
{
	m_chainClip = flag;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isChainClip() const
{
	return m_chainClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setAlpha(DWORD alpha)
{
	m_alpha = alpha;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
DWORD Unit::getAlpha() const
{
	return m_alpha;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRotationX(FLOAT fAngle)
{
	m_rotation.x = fAngle;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRotationY(FLOAT fAngle)
{
	m_rotation.y = fAngle;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRotationZ(FLOAT fAngle)
{
	m_rotation.z = fAngle;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationX()
{
	return m_rotation.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationY()
{
	return m_rotation.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRotationZ()
{
	return m_rotation.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRelativityX(FLOAT fPoint)
{
	m_relativity.x=fPoint;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRelativityY(FLOAT fPoint)
{
	m_relativity.y=fPoint;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setRelativityZ(FLOAT fPoint)
{
	m_relativity.z=fPoint;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityX()
{
	return m_relativity.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityY()
{
	return m_relativity.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getRelativityZ()
{
	return m_relativity.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipX(FLOAT fClip)
{
	m_clipPoint.x=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipY(FLOAT fClip)
{
	m_clipPoint.y=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipZ(FLOAT fClip)
{
	m_clipPoint.z=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipX()
{
	return m_clipPoint.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipY()
{
	return m_clipPoint.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipZ()
{
	return m_clipPoint.z;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipWidth(FLOAT fClip)
{
	m_clipSize.x=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipHeight(FLOAT fClip)
{
	m_clipSize.y=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setClipDepth(FLOAT fClip)
{
	m_clipSize.z=fClip;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipWidth()
{
	return m_clipSize.x;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipHeight()
{
	return m_clipSize.y;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
FLOAT Unit::getClipDepth()
{
	return m_clipSize.z;
}

//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isShadow() const
{
	return m_shadow;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setShadow(bool bShadow)
{
	m_shadow=bShadow;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isLight() const
{
	return m_light;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setLight(bool bLight)
{
	m_light=bLight;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isView2D() const
{
	return m_view2D;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setView2D(bool bView)
{
	m_view2D=bView;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isVisible() const
{
	return m_visible;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isZBuffer() const
{
	return m_zbuffer;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setZBuffer(bool bZBuffer)
{
	m_zbuffer=bZBuffer;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
INT Unit::getBlendMode() const
{
	return m_blendMode;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setBlendMode(INT iMode)
{
	m_blendMode=iMode;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setLimit(bool bFlag)
{
	m_limit=bFlag;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isLimit() const
{
	return m_limit;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setZSort(bool bFlag)
{
	m_zsort=bFlag;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::isZSort() const
{
	return m_zsort;
}

//-----------------------------------------------
// void Unit::add(Unit* unit)
// -----  ����  -----
// �q���j�b�g��ǉ�����
// -----  ����  -----
// unit  �q���j�b�g
// ----- �߂�l -----
// ����
//-----------------------------------------------
void Unit::add(Unit* unit)
{
	unit->m_unitParent = this;
	m_unitChilds.push_back(unit);
}
//-----------------------------------------------
// void Unit::del(Unit* unit)
// -----  ����  -----
// �q���j�b�g���폜����
// -----  ����  -----
// unit  �q���j�b�g
// ----- �߂�l -----
// ����
//-----------------------------------------------
void Unit::del(Unit* unit)
{
	unit->m_unitParent = NULL;
	m_unitChilds.remove(unit);
}

//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::setMatrix(NMatrix* pMatrix,FLOAT fX,FLOAT fY,FLOAT fZ)
{
	NMatrix matrix2;

	//��]�s��쐬
	if(getRotationX() || getRotationY() || getRotationZ())
	{
		FLOAT RX = NPI / 180.0f * getRotationX();
		FLOAT RY = NPI / 180.0f * getRotationY();
		FLOAT RZ = NPI / 180.0f * getRotationZ();
		//���Έړ��s��쐬
		if(getRelativityX() || getRelativityY() || getRelativityZ() || getCenterX()|| getCenterY()|| getCenterZ())
		{
			pMatrix->setTranslation(
				getRelativityX()-getCenterX(),
				getRelativityY()-getCenterY(),
				getRelativityZ()-getCenterZ());
			*pMatrix *= matrix2.setRotationX(RX);		
		}
		else
			*pMatrix =  matrix2.setRotationX(RX);	
		*pMatrix *= matrix2.setRotationY(RY);
		*pMatrix *= matrix2.setRotationZ(RZ);
	}
	else//�ړ��s��쐬
		pMatrix->setTranslation(getRelativityX()-getCenterX(),getRelativityY()-getCenterY(),getRelativityZ()-getCenterZ());


	//�X�P�[�����O
	if(getScaleX()!=1 || getScaleY()!=1 || getScaleZ()!=1)
	{
		matrix2.setScaling(getScaleX(),getScaleY(),getScaleZ());
		*pMatrix = matrix2 * *pMatrix;
	}	
	pMatrix->_41 += getCenterX() + fX;
	pMatrix->_42 += getCenterY() + fY;
	pMatrix->_43 += getCenterZ() + fZ;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
			delete frame;
		}
	}
	m_frame = SP<Frame>(frameParent);


	//�A�j���[�V����
	if(fileObject->anime.size())
	{
		m_animationSet = fileObject->anime;
		setAnimation(m_animationSet.begin()->first.c_str());
	}
	return true;
}
bool Unit::copyImage(Unit* unit)
{
	*(Object*)this = *(Object*)unit;
	m_anime = unit->m_anime;
	m_animationSet = unit->m_animationSet;
	m_frame = unit->m_frame;
	return true;
}
bool Unit::openFile(LPCWSTR fileName)
{
	FileObject fileObject;
	if(!fileObject.load(fileName))
		return false;
	createObject(&fileObject);
	return true;
}
//-----------------------------------------------
// bool Unit::openXFile(LPCSTR pcFileName,D3DXFILEFORMAT xfileFormat)
// -----  ����  -----
// X-File��ǂݍ���
// -----  ����  -----
// pcFileName  �t�@�C����
// xfileFormat �t�@�C���t�H�[�}�b�g
//             _3DS_EXPORTER  MS����3DS�G�N�X�|�[�^
//             _3DS_CONVERTER MS����3DS�R���o�[�^���y�P�|�[�^
// ----- �߂�l -----
// true:���� false:���s
//-----------------------------------------------
#ifdef _AFL_D3DX
bool Unit::openXFile(LPCSTR pcFileName,D3DXFILEFORMAT xfileFormat)
{
	XFile xl;

	AFL::Debug::out("X-File open: %s\n",pcFileName);

	//X�t�@�C���̃I�[�v��
	FileObject* fileObject = xl.load(pcFileName);
	if(!fileObject)
	{
		AFL::Debug::out("X-File error: �t�@�C���̓ǂݍ��݂Ɏ��s\n");
		return false;
	}
	createObject(fileObject);

	delete fileObject;
	return true;
}
#endif
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
		delete frameChild;
	}
	frame->setMatrix(&frameData->matrix);

	readMesh(frame,&frameData->mesh);

	return frame;
}

#define D3DFVF_VERTEXBONE0 (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
struct VERTEXBONE0
{
	VERTEXBONE0& operator=(const VERTEXBASE2& base)
	{
		vectPosition = base.vectPosition;
		vectNormal = base.vectNormal;
		dwColor = *(COLOR4*)&base.color;
		fTu = base.uv.x;
		fTv = base.uv.y;
		return *this;
	}
	NVector3 vectPosition;		// �I�u�W�F�N�g���W
	NVector3 vectNormal;		// �I�u�W�F�N�g�@��
	COLOR4 dwColor;				// �F���
	FLOAT fTu,fTv;				// �e�N�X�`�����W
};
struct VERTE_BONE1
{

	NVector3 vectPosition;		// �I�u�W�F�N�g���W
	union
	{
		BYTE byBlendIndex[4];	//���_�u�����h�C���f�b�N�X
		DWORD dwBlendIndex;
	};
	NVector3 vectNormal;		// �I�u�W�F�N�g�@��
	COLOR4 dwColor;				// �F���
	FLOAT fTu,fTv;				// �e�N�X�`�����W
};
struct VERTE_BONE2
{

	NVector3 vectPosition;		// �I�u�W�F�N�g���W
	FLOAT fBlend[1];			//���_�u�����h�E�G�C�g
	union
	{
		BYTE byBlendIndex[4];	//���_�u�����h�C���f�b�N�X
		DWORD dwBlendIndex;
	};
	NVector3 vectNormal;		// �I�u�W�F�N�g�@��
	COLOR4 dwColor;				// �F���
	FLOAT fTu,fTv;				// �e�N�X�`�����W
};
struct VERTE_BONE3
{

	NVector3 vectPosition;		// �I�u�W�F�N�g���W
	FLOAT fBlend[2];			//���_�u�����h�E�G�C�g
	union
	{
		BYTE byBlendIndex[4];	//���_�u�����h�C���f�b�N�X
		DWORD dwBlendIndex;
	};
	NVector3 vectNormal;		// �I�u�W�F�N�g�@��
	COLOR4 dwColor;				// �F���
	FLOAT fTu,fTv;				// �e�N�X�`�����W
};

//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::readMesh(Frame* frame,MESHOPTIMIZE* xfileMesh,bool shadow)
{
	INT i;
	Mesh* mesh = NEW Mesh();
	INT indexCount = (INT)xfileMesh->index.size();
	INT vertexCount = (INT)xfileMesh->vertex.size();

	//���_�C���f�b�N�X�̐ݒ�
	mesh->createIndexBuffer(indexCount);
	Index* index = mesh->getIndexBuffer();
	LPWORD indexData = (LPWORD)index->lock();
	for(i=0;i<indexCount;i++)
		indexData[i] = (WORD)xfileMesh->index[i];
	index->unlock();
	
	//���_�f�[�^�̐ݒ�

	const static D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 0, 28, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL , 0 },
		{ 0, 40, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 56, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	const static D3DVERTEXELEMENT9 declBone0[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL , 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

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

	Vertex* vertex;
	if(boneCount)
	{
		mesh->createVertexBuffer(vertexCount,sizeof(VERTEXBASE2),D3DFVF_VERTEXBASE);
		vertex = mesh->getVertexBuffer();
		VERTEXBASE2* vertexBase = (VERTEXBASE2*)vertex->lock();
		for(itVertex=xfileMesh->vertex.begin();itVertex!=xfileMesh->vertex.end();++itVertex)
		{
			vertexBase[itVertex->second] = itVertex->first;
		}
		mesh->setDeclaration(decl);
	}
	else
	{
		mesh->createVertexBuffer(vertexCount,sizeof(VERTEXBONE0),D3DFVF_VERTEXBONE0);
		vertex = mesh->getVertexBuffer();
		VERTEXBONE0* vertexBase = (VERTEXBONE0*)vertex->lock();
		for(itVertex=xfileMesh->vertex.begin();itVertex!=xfileMesh->vertex.end();++itVertex)
		{
			vertexBase[itVertex->second] = itVertex->first;
		}
		mesh->setDeclaration(declBone0);
	}
	vertex->unlock();
		
	mesh->setBoneMatrix(xfileMesh->boneMatrixs);


	Material material;
	*(D3DMATERIAL9*)&material = *(D3DMATERIAL9*)&xfileMesh->material;

	std::list<std::string>::iterator itTex;
	for(itTex=xfileMesh->textureName.begin();itTex!=xfileMesh->textureName.end();++itTex)
	{
		if(*itTex != "")	
		{
			Texture* texture = NEW Texture();
			if(itTex->c_str()[0] && texture->openImage(itTex->c_str()))
			{
				mesh->setTexture(texture);
				break;
			}
			else
			{
				delete texture;
			}
		}
	}
	mesh->setMaterial(material);

/*	if(!DIRECT3D::Device::getShader()->isEffect("skinnedmesh"))
		DIRECT3D::Device::getShader()->loadMemory(g_main,sizeof(g_main),"skinnedmesh");
*/
	if(boneCount)
	{
		if(shadow)
			mesh->setVertexShader("s4");
		else
			mesh->setVertexShader("t4");
	}
	else
	{
		if(shadow)
			mesh->setVertexShader("s0");
		else
			mesh->setVertexShader("t0");
	}
	mesh->setShadow(shadow);
	frame->add(mesh);

	delete mesh;
}


//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::optimizeMesh(MESHOPTIMIZE& meshOptimize,std::list<VERTEXBASE2>& listVertex)
{
	INT i;
	INT dataCount = (INT)listVertex.size();
	if(dataCount == 0)
		return false;
	std::map<VERTEXBASE2,WORD>::iterator itMap;
	std::list<VERTEXBASE2>::iterator itVertex;
	INT indexSize = 0;
	INT index;
	meshOptimize.index.resize(dataCount);
	for(itVertex=listVertex.begin(),i=0;i<dataCount;i++,++itVertex)
	{
		itMap = meshOptimize.vertex.find(*itVertex);
		if(itMap != meshOptimize.vertex.end())
		{
			index = (*itMap).second;
		}
		else
		{
			index = indexSize;
			meshOptimize.vertex[*itVertex] = index;
			indexSize++;
		}
		meshOptimize.index[i] = index;
	}

	return true;
}

//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::readMesh(Frame* frame,struct MeshData* meshData)
{
	//�O�p�`�t�F�C�X�̎Z�o
	INT i,j,k;
	
	//�K�p�O�p�`�t�F�C�X��
	INT indexCount = (INT)meshData->vertexIndex.size();


	//�{�[���f�[�^�̓W�J��Ɨp
	const INT BONEMAX = 4;
	INT boneCount = (INT)meshData->boneData.size();
	//�X�L���f�[�^�𒸓_���ƂɍĔz�u <���_INDEX<BONEINDEX,WEIGHT>>
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


	//�ő�}�e���A���̒��o
	INT materialMax = 0;
	for(i=0;i<(INT)meshData->materialIndex.size();i++)
		if((INT)meshData->materialIndex[i] > materialMax)
			materialMax = (INT)meshData->materialIndex[i];
	materialMax++;


	//���_�f�[�^�W�J��Ɨ̈�̊m��
	std::vector<std::list<VERTEXBASE2> >listVertex(materialMax);

	for(i=0;i<(INT)meshData->vertexIndex.size();i++)
	{
		//�}�e���A�����Ƃɕ���
		DWORD materialIndex = 0;
		if(meshData->materialIndex.size())
		{
			materialIndex = meshData->materialIndex[i];
		}
		VERTEXBASE2 vertex;
		for(j=0;j<3;j++)
		{
			//���_�f�[�^�̓W�J
			DWORD dwIndex = meshData->vertexIndex[i].data[j];
			vertex.vectPosition = meshData->vertexData[dwIndex];
			//�{�[���̓W�J
			std::map<INT,FLOAT>::iterator itSkin;
			FLOAT fWeight = 0.0f;
			for(itSkin=vertexSkin[dwIndex].begin(),k=0;itSkin!=vertexSkin[dwIndex].end() && k<BONEMAX;++itSkin,k++)
			{
				vertex.blendIndex[k] = (BYTE)itSkin->first;
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

			//�@���f�[�^�̓W�J
			if(meshData->normalIndex.size())
			{
				DWORD index = meshData->normalIndex[i].data[j];
				vertex.vectNormal = meshData->normalData[index];
			}

			//UV���W�̓W�J
			if(meshData->uvIndex.size())
			{
				DWORD dwIndex = meshData->uvIndex[i].data[j];
				vertex.uv.x = meshData->uvData[dwIndex].u;
				vertex.uv.y = meshData->uvData[dwIndex].v;
			}
			//���_�J���[
			if(meshData->colorData.size())
			{
				DWORD dwIndex = meshData->colorIndex[i].data[j];
				vertex.color =  *(NVector*)&meshData->colorData[dwIndex];

			}
			else if(meshData->materialIndex.size())
			{
				vertex.color =  *(NVector*)&meshData->materialData[materialIndex].material.Diffuse;
			}
			else
			{
				vertex.color = 1.0f;
			}
			listVertex[materialIndex].push_back(vertex);
		}
	}

	//���b�V�������ƍœK��
	for(k=0;k<materialMax;k++)
	{

		MESHOPTIMIZE meshOptimize;
		if(optimizeMesh(meshOptimize,listVertex[k]))
		{
			//�}�e���A���̓W�J
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
			//�œK���ς݃f�[�^��ۑ�
			readMesh(frame,&meshOptimize);
		}

		//�V���h�E�{�����[���̍쐬
		if(isShadow())
		{
			std::list<VERTEXBASE2>::iterator it1,it2;
			int vertexCount = (INT)listVertex[k].size();
			for(j=0,it1=listVertex[k].begin();j<vertexCount-3;j+=3)
			{
				VERTEXBASE2* point1[3] =
				{
					&*it1++,&*it1++,&*it1++
				};
				for(i=j+3,it2=it1;i<vertexCount;i+=3)
				{
					VERTEXBASE2* point2[3] =
					{
						&*it2++,&*it2++,&*it2++
					};
					int l,m;
					bool flag[3][3];
					for(m=0;m<3;m++)
					{
						for(l=0;l<3;l++)
						{
							flag[m][l] = point1[m]->isOverride(*point2[l]);
						}
					}

					static INT index[] = {0,1,1,2,2,0};
					for(m=0;m<6;m+=2)
					{
						for(l=0;l<6;l+=2)
						{
							if(flag[index[m]][index[l+1]] && flag[index[m+1]][index[l]])
							{
								VERTEXBASE2 v;

								v = *point1[index[m]];
								listVertex[k].push_back(v);
								v = *point2[index[l]];
								listVertex[k].push_back(v);
								v = *point1[index[m+1]];
								listVertex[k].push_back(v);
								
								v = *point2[index[l]];
								listVertex[k].push_back(v);
								v = *point1[index[m]];
								listVertex[k].push_back(v);
								v = *point2[index[l+1]];
								listVertex[k].push_back(v);
								break;
							}
						}
					}
				}
			}
			MESHOPTIMIZE meshOptimize;
			if(optimizeMesh(meshOptimize,listVertex[k]))
			{
				//�}�e���A���̓W�J
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
				//�œK���ς݃f�[�^��ۑ�
				readMesh(frame,&meshOptimize,true);
			}

		}
	}

}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::getAnimationMatrix(LPCSTR frameName,NMatrix* matrix)
{
	INT flag = 0;
	NVector position,scale;
	NVector rotation;
	
	NVector position2,scale2;
	NVector rotation2;

	position.x = 0.0f;
	position.y = 0.0f;
	position.z = 0.0f;
	scale.x = 0.0f;
	scale.y = 0.0f;
	scale.z = 0.0f;
	rotation.x = 0.0f;
	rotation.y = 0.0f;
	rotation.z = 0.0f;
	rotation.w = 0.0f;

	ZeroMemory(matrix,sizeof(NMatrix));
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
	//�u�����h�E�G�C�g�̎Z�o
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
					*matrix += matirixAnime;
					break;
				}
				else if(fWeight != 0)
					*matrix += matirixAnime * fWeight;
			}
			else if(ret == 2)
			{
				flag |= 2;
				if(fWeight == 1.0f)
				{
					position += position2;
					scale += scale2;
					rotation += rotation2;
					break;
				}
				else if(fWeight != 0)
				{
					position += position2 * fWeight;
					scale += scale2 * fWeight;
					rotation += rotation2 * fWeight;
				}
			}
		}
	}
	//�s�v�f�[�^�̍폜
	for(i=0;i<(INT)weightList.size();i++)
	{
		if(weightList[i] == 0)
		{
			m_anime.erase(weightIt[i]);
		}
	}
	if(flag & 2)
	{
		NMatrix matrixRotation;
		NMatrix matrixKey = 
		{
			scale.x,0,0,0,
			0,scale.y,0,0,
			0,0,scale.z,0,
			position.x,position.y,position.z,1
		};
		rotation.w *= -1;
		matrixRotation.setRotationQuaternion(rotation);
		*matrix += matrixRotation * matrixKey;

	}
	return flag != 0;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	return true;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::onRenderEnd()
{
	m_renderFlag = true;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::getBoundingBox(NVector* vect)
{
	Frame* frame = m_frame.get();
	if(frame)
	{
		NVector vectSrc[2] = {1000000,1000000,1000000,0,-1000000,-1000000,-1000000,0};

		NMatrix matrix,matrix1;
		setMatrix(&matrix,getPosX(),getPosY(),getPosZ());
		matrix1.setIdentity();
		frame->getBoundingBox(this,&matrix,&matrix1,vectSrc);
		vect[0] = vectSrc[0];
		vect[1] = vectSrc[1];
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::getBoundingBox(Frame* frame,NVector* vect)
{
	if(frame)
	{
		NVector vectSrc[2] = {1000000,1000000,10000000,0,-1000000,-1000000,-1000000,0};

		frame->getBoundingBox(vectSrc);
		vect[0] = vectSrc[0];
		vect[1] = vectSrc[1];
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::onAction(World* world,LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onAction(world,value);
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::onStart(World* world,LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onStart(world,value);
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::onIdel(World* world,LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onIdel(world,value);
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
void Unit::onEnd(World* world,LPVOID value)
{
	std::list<Unit*>::iterator it;
	for(it=m_unitChilds.begin();it!=m_unitChilds.end();++it)
	{
		(*it)->onEnd(world,value);
	}
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
std::list<Unit*>* Unit::getChilds()
{
	return &m_unitChilds;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
Frame* Unit::getFrame()
{
	return m_frame.get();
}
Frame* Unit::getFrame(LPCSTR name)
{
	//if(m_frame.get())
	//	return m_frame->getFrame(name);
	return NULL;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
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
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::onDeviceLost()
{
	return true;
}
//-----------------------------------------------
// 
// -----  ����  -----
// 
// -----  ����  -----
// 
// ----- �߂�l -----
// 
//-----------------------------------------------
bool Unit::onDeviceRestore()
{
	resetRenderFlag();
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitSprite
// 2D�p�X�v���C�g���j�b�g
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
}
void UnitSprite::setPartSize(INT width,INT height)
{
	m_partWidth = width;
	m_partHeight = height;
	resetVertex();
}
INT UnitSprite::getPartWidth() const
{
	if(m_partWidth == 0)
		return getImageWidth();
	return m_partWidth;
}
INT UnitSprite::getPartHeight() const
{
	if(m_partHeight == 0)
		return getImageHeight();
	return m_partHeight;
}
INT UnitSprite::getPartIndex() const
{
	return m_partIndex;
}
void UnitSprite::setPartIndex(INT index)
{
	m_partIndex = index;
	resetVertex();
}
Mesh* UnitSprite::getMesh() const
{
	if(m_frame.get() && m_frame->getMesh())
		return m_frame->getMesh();
	return NULL;
}
bool UnitSprite::createFromTexture(Texture* texture)
{
	return createFromTexture(SP<Texture>(texture));
}
bool UnitSprite::resetVertex()
{
	if(!getMesh())
		return false;
	if(m_frame.getCount() > 1)
	{
		Mesh* mesh = getMesh();
		m_frame = new Frame();;
		createFromTexture(mesh->getTexture());
	}
	Vertex* v = m_frame->getMesh()->getVertexBuffer();
	if(!v)
		return false;

	Texture* texture = getTexture();
	if(!texture)
		return false;

	GETVERTEX vertex;
	if(!getPointVertex(&vertex,texture))
		return false;

	VERTEXSPRITE vertexSprite[] =
	{
		{ vertex.fX1,vertex.fY1, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX,               vertex.fTY},
		{ vertex.fX2,vertex.fY1, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX+vertex.fTWidth,vertex.fTY},
		{ vertex.fX1,vertex.fY2, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX,               vertex.fTY+vertex.fTHeight},
		{ vertex.fX2,vertex.fY2, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX+vertex.fTWidth,vertex.fTY+vertex.fTHeight}
	};
	return v->setData(vertexSprite,sizeof(vertexSprite));
}
bool UnitSprite::createFromTexture(SP<Texture>& texture)
{
	GETVERTEX vertex;
	if(!getPointVertex(&vertex,texture.get()))
		return false;

	VERTEXSPRITE vertexSprite[] =
	{
		{ vertex.fX1,vertex.fY1, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX,               vertex.fTY},
		{ vertex.fX2,vertex.fY1, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX+vertex.fTWidth,vertex.fTY},
		{ vertex.fX1,vertex.fY2, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX,               vertex.fTY+vertex.fTHeight},
		{ vertex.fX2,vertex.fY2, 0.0f, 0,0,-1, 0xffffffff,vertex.fTX+vertex.fTWidth,vertex.fTY+vertex.fTHeight}
	};

	WORD wIndex[]={0,1,2,3,2,1};

	Material material;


	//���b�V���̍쐬
	Mesh mesh;
	mesh.setTexture(texture);
	mesh.createVertexBuffer(vertexSprite,4,sizeof(VERTEXSPRITE),D3DFVF_VERTEXSPRITE);
	mesh.createIndexBuffer(wIndex,6);

	//�t���[���̍쐬
	m_frame = SP<Frame>(NEW Frame());
	m_frame->add(&mesh);


	RECT clipRect = {0,0,texture->getImageWidth(),texture->getImageHeight()};
	setViewClip(&clipRect);
	resetRenderFlag();
	return true;
}
bool UnitSprite::getPointVertex(PGETVERTEX vertexSprite,Texture* textureSrc)
{
	if(!textureSrc)
		return false;

	//�T�C�Y�̕ۑ�
	INT width = textureSrc->getImageWidth();
	INT height = textureSrc->getImageHeight();
	INT textureWidth = textureSrc->getTextureWidth();
	INT textureHeight = textureSrc->getTextureHeight();

	INT widthCount;
	if(m_partWidth)
		widthCount = width / m_partWidth;
	else
		widthCount = 1;

	if(m_partWidth)
		width = m_partWidth; 
	if(m_partHeight)
		height = m_partHeight; 

	INT x,y;
	x = m_partWidth * (m_partIndex%widthCount);
	y = m_partHeight * (m_partIndex/widthCount);


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
	vertexSprite->fX1 -= 0.5f;
	vertexSprite->fY1 -= 0.5f;
	vertexSprite->fX2 -= 0.5f;
	vertexSprite->fY2 -= 0.5f;
	vertexSprite->fTX = x / (FLOAT)textureWidth;
	vertexSprite->fTY = y / (FLOAT)textureHeight;
	vertexSprite->fTWidth = (FLOAT)width/textureWidth;
	vertexSprite->fTHeight = (FLOAT)height/textureHeight;

	return true;
}
typedef struct _D3DKMT_OPENADAPTERFROMDEVICENAME
{
	wchar_t	*pDeviceName;
	HANDLE	hAdapter;
	LUID		AdapterLuid;
} D3DKMT_OPENADAPTERFROMDEVICENAME;

typedef struct _D3DKMT_CLOSEADAPTER
{
	HANDLE	hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct _D3DKMT_QUERYADAPTERINFO
{
	HANDLE			hAdapter;
	unsigned int	Type;
	void			*pPrivateDriverData;
	unsigned int	PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;

typedef struct _D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME
{
	WCHAR	DeviceName[32];
	HANDLE	hAdapter;
	LUID	AdapterLuid;
	unsigned int	VidPnSourceId;
} D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME;

typedef struct _D3DKMT_SEGMENTSIZEINFO
{
	ULONGLONG	DedicatedVideoMemorySize;
	ULONGLONG	DedicatedSystemMemorySize;
	ULONGLONG	SharedSystemMemorySize;
} D3DKMT_SEGMENTSIZEINFO;

typedef struct _D3DKMT_ADAPTERREGISTRYINFO
{
	WCHAR	AdapterString[MAX_PATH];
	WCHAR	BiosString[MAX_PATH];
	WCHAR	DacType[MAX_PATH];
	WCHAR	ChipType[MAX_PATH];
} D3DKMT_ADAPTERREGISTRYINFO;

typedef int	(APIENTRY *PFND3DKMT_CLOSEADAPTER)(IN CONST D3DKMT_CLOSEADAPTER *pData);
typedef int	(APIENTRY *PFND3DKMT_OPENADAPTERFROMGDIDISPLAYNAME)(IN OUT D3DKMT_OPENADAPTERFROMGDIDISPLAYNAME *pData);
typedef int	(__stdcall *PFNDWM_DXGETWINDOWSHAREDSURFACE)(HWND hWnd, LUID adapterLuid, LUID someLuid, DWORD *pD3DFormat,
	HANDLE *pSharedHandle, unsigned __int64 *arg7);

bool UnitSprite::createImage(HWND hwnd)
{
	if(!Device::getInterfaceEx())
		return false;

	LUID luidA={0,0};
	LUID luidN={0,0};
	unsigned __int64 a=0;
	Device::getAdapterLUID(0,&luidA);

	HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
	INT (WINAPI* DwmpDxGetWindowSharedSurface)(HWND,LUID,DWORD,DWORD,D3DFORMAT*,HANDLE*,LPVOID*)
		=(INT (WINAPI*)(HWND,LUID,DWORD,DWORD,D3DFORMAT*,HANDLE*,LPVOID*))GetProcAddress(hDwmApi,(LPCSTR)100);

	INT (WINAPI* DwmpDxUpdateWindowSharedSurface)(HWND, int, int, int, HMONITOR, void*) = 
		(INT (WINAPI*)(HWND, int, int, int, HMONITOR, void*))GetProcAddress(hDwmApi, (LPCSTR)101);

	D3DFORMAT format=D3DFMT_UNKNOWN;
	HANDLE handle=NULL;
	DwmpDxGetWindowSharedSurface(hwnd,luidA,0,0,&format,&handle,(LPVOID*)&luidN);

	if(!handle)
		return false;

	RECT rect;
	::GetWindowRect(hwnd,&rect);
	INT width = rect.right - rect.left;
	INT height = rect.bottom - rect.top;

	Texture* texture = NEW Texture();
	texture->createImage(width,height,D3DFMT_X8R8G8B8,handle);
	createFromTexture(texture);

	DwmpDxUpdateWindowSharedSurface(hwnd,0,0,0,0,0);
	FreeLibrary(hDwmApi);
	return true;
}
bool UnitSprite::createImage(INT width,INT height,D3DFORMAT format,D3DPOOL pool)
{
	//�e�N�X�`���̍쐬
	Texture* texture = NEW Texture();
	texture->setFilter(false);
	texture->createImage(width,height,pool);
	//���_�̍쐬
	createFromTexture(texture);
	return true;
}
bool UnitSprite::createTarget(INT width,INT height,D3DFORMAT format)
{
	//�e�N�X�`���̍쐬
	Texture* texture = NEW Texture();
	texture->createTarget(width,height,format);
	//���_�̍쐬
	createFromTexture(texture);
	return true;
}
bool UnitSprite::openImage(LPCSTR fileName)
{
	return openImage(UCS2(fileName));

}
bool UnitSprite::openImage(LPCWSTR fileName)
{
	//�e�N�X�`���̍쐬
	Texture* texture = NEW Texture();
	texture->setFilter(false);
	if(!texture->openImage(fileName))
	{
		delete texture;
		return false;
	}
	//���_�̍쐬
	createFromTexture(SP<Texture>(texture));
	return true;

}
bool UnitSprite::createText(LPCSTR string,HFONT font,DWORD color,DWORD bcolor,INT limitWidth,bool mline,D3DFORMAT format)
{
	return createText(UCS2(string),font,color,bcolor,limitWidth,mline,format);
}
bool UnitSprite::createText(LPCWSTR string,HFONT font,DWORD color,DWORD bcolor,INT limitWidth,bool mline,D3DFORMAT format)
{
	//�e�N�X�`���̍쐬
	Texture* texture = NEW Texture();
	texture->createText(string,font,color,bcolor,limitWidth,mline,format);
	//���_�̍쐬
	createFromTexture(SP<Texture>(texture));
	return true;
}

bool UnitSprite::createText(LPCSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth,bool mline,D3DFORMAT format)
{
	return createText(UCS2(string),size,color,bcolor,limitWidth,mline,format);
}
bool UnitSprite::createText(LPCWSTR string,INT size,DWORD color,DWORD bcolor,INT limitWidth,bool mline,D3DFORMAT format)
{
	//�e�N�X�`���̍쐬
	Texture* texture = NEW Texture();
	texture->createText(string,size,color,bcolor,limitWidth,mline,format);
	//���_�̍쐬
	createFromTexture(SP<Texture>(texture));
	return true;
}
Texture* UnitSprite::getTexture() const
{
	if(getMesh() && getMesh()->getMaterial())
		return getMesh()->getTexture();
	else
		return NULL;
}
bool UnitSprite::clear(DWORD color)
{
	if(!getTexture())
		return false;
	getTexture()->clear(color);
	return true;
}
IDirect3DSurface9* UnitSprite::getSurface() const
{
	if(getTexture())
		return getTexture()->getSurface();
	return NULL;
}
void UnitSprite::drawText(INT x,INT y,LPCWSTR text,HFONT font,DWORD color,DWORD bcolor)
{
	if(getTexture())
	{
		getTexture()->drawOutlineText(x,y,text,font,color,bcolor);
	}
}

Material* UnitSprite::getMaterial() const
{
	if(getMesh() && getMesh()->getMaterial())
		return getMesh()->getMaterial();
	else
		return NULL;
}
INT UnitSprite::getImageWidth()const
{
	Texture* t = getTexture();
	if(!t)
		return 0;
	return t->getImageWidth();
}
INT UnitSprite::getImageHeight()const
{
	Texture* t = getTexture();
	if(!t)
		return 0;
	return t->getImageHeight();
}
HDC UnitSprite::getDC() const
{
	if(getTexture())
		return getTexture()->getDC();
	return NULL;
}
void UnitSprite::releaseDC(HDC hdc) const
{
	if(getTexture())
		getTexture()->releaseDC(hdc);
}
void UnitSprite::setTarget() const
{
	if(getTexture())
		getTexture()->setTarget();
}
bool UnitSprite::lock(D3DLOCKED_RECT* rect)
{
	Texture* tx = getTexture();
	if(tx)
		return tx->lock(rect);
	return false;
}
bool UnitSprite::unlock()
{
	Texture* tx = getTexture();
	if(tx)
		return tx->unlock();
	return false;
}
D3DFORMAT UnitSprite::getFormat() const
{
	Texture* tx = getTexture();
	if(tx)
		return tx->getFormat();
	return D3DFMT_UNKNOWN;
}



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitText
// TEXT�X�v���C�g���j�b�g
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
bool UnitText::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(!isRenderFlag())
	{
		SIZE s;
		m_font.getFontSize(&s,m_drawString);
		if(getImageWidth() < s.cx+2 || getImageHeight() < s.cy+2)
		{
			createImage(s.cx+2,s.cy+2,D3DFMT_A8R8G8B8);
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
LPCSTR UnitText::getText()
{
	return UTF8(m_drawString);
}
WINDOWS::Font* UnitText::getFont()
{
	return &m_font;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitDIB
// DIB�\���p���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

UnitDIB::UnitDIB()
{
	setBlendMode(2);
}
void UnitDIB::setSize(INT width,INT height)
{
	m_redraw = true;
	m_dibImage.createDIB(width,height,D3DFMT_A8R8G8B8);
	m_dibImage.clear(0x0000ffff);
}
WINDOWS::DIB* UnitDIB::getDIB()
{
	return &m_dibImage;
}
HDC UnitDIB::getDC() const
{
	return m_dibImage.getDC();
}
void UnitDIB::setRedraw()
{
	m_redraw = true;
}
bool UnitDIB::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	if(m_redraw)
	{
		INT height = m_dibImage.getHeight();
		INT width = m_dibImage.getWidth();

		if(width != getImageWidth() || height != getImageHeight())
		{
			createImage(width,height,(D3DFORMAT)m_dibImage.getFormat(),D3DPOOL_DEFAULT);
		}
		m_redraw = false;

		Texture* texture = getTexture();
		if(texture)
		{
			D3DLOCKED_RECT rect;
			if(texture->lock(&rect))
			{

				LPBYTE src = (LPBYTE)m_dibImage.getImage();
				LPBYTE dest = (LPBYTE)rect.pBits;
				if(src && dest)
				{
					INT i;
					for(i=0;i<height;i++)
					{
						CopyMemory(dest,src,width*4);
						dest += rect.Pitch;
						src += m_dibImage.getPitch();
					}
				}
				texture->unlock();
			}
		}
	}
	return true;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GDIPlus
// GDIPlus�������p
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
// GDIPlus�\���p���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
GDIPlus UnitGdip::m_gdiPlus;

void UnitGdip::setSize(INT width,INT height,D3DFORMAT format)
{
	setTextureFilter(D3DTEXF_LINEAR);
	releaseGraphics();
	createImage(width,height,format,D3DPOOL_DEFAULT);
}
UnitGdip::UnitGdip()
{
//	m_bitmap = NULL;
//	m_graphics = NULL;
}
UnitGdip::~UnitGdip()
{
	releaseGraphics();
}
Gdiplus::Graphics* UnitGdip::getGraphics()
{
	if(m_graphics.get())
		return m_graphics.get();


	D3DLOCKED_RECT rect;
	if(!lock(&rect))
		return NULL;

	D3DFORMAT format = getFormat();
	Gdiplus::PixelFormat pixFormat = PixelFormatUndefined;
	switch(format)
	{
	case D3DFMT_X8R8G8B8:
		pixFormat = PixelFormat32bppRGB;
		break;
	case D3DFMT_A8R8G8B8:
		pixFormat = PixelFormat32bppARGB;
		break;
	case D3DFMT_R5G6B5:
		pixFormat = PixelFormat16bppRGB565;
		break;
	case D3DFMT_X1R5G5B5:
		pixFormat = PixelFormat16bppRGB555;
		break;
	case D3DFMT_A1R5G5B5:
		pixFormat = PixelFormat16bppARGB1555;
		break;
	}
	m_bitmap = new Gdiplus::Bitmap(getImageWidth(),getImageHeight(),rect.Pitch,pixFormat,(BYTE*)rect.pBits);
	m_graphics = new Gdiplus::Graphics(m_bitmap.get());
	m_graphics->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	m_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
	return m_graphics.get();
}
void UnitGdip::releaseGraphics()
{
	if(m_graphics.get())
	{
		m_graphics->Flush(FlushIntentionSync);
		//delete m_graphics;
		m_graphics = NULL;
	}
	if(m_bitmap.get())
	{
		//delete m_bitmap;
		m_bitmap = NULL;
		unlock();
	}
}
void UnitGdip::clear(DWORD color)
{
	
	Gdiplus::Graphics* g = getGraphics();
	if(g)
		g->Clear(Color(color));
		
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
bool UnitGdip::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	releaseGraphics();
	return UnitSprite::onRender(world,x,y,z);
}
bool UnitGdip::onDeviceLost()
{
	releaseGraphics();
	return UnitSprite::onDeviceLost();
}
void UnitGdip::drawString(LPCWSTR text,HFONT font,DWORD color)
{
	Graphics* g = getGraphics();
	if(!g)
		return;
	
	HDC hDC = CreateCompatibleDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC,font);
	
	g->DrawString(text,(INT)wcslen(text),&Gdiplus::Font(hDC),
		Gdiplus::PointF(0,0),StringFormat::GenericTypographic(),&Gdiplus::SolidBrush(Gdiplus::Color(color)));
	SelectObject(hDC,oldFont);
	DeleteDC(hDC);
}
void UnitGdip::drawString(INT x,INT y,LPCWSTR text,HFONT font,DWORD color)
{
	Graphics* g = getGraphics();
	if(!g)
		return;
	
	HDC hDC = CreateCompatibleDC(NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC,font);
	
	g->DrawString(text,(INT)wcslen(text),&Gdiplus::Font(hDC),
		Gdiplus::PointF((FLOAT)x,(FLOAT)y),StringFormat::GenericTypographic(),&Gdiplus::SolidBrush(Gdiplus::Color(color)));
	SelectObject(hDC,oldFont);
	DeleteDC(hDC);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitWorld
// �ă����_�����O�p���[���h���j�b�g
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitWorld::UnitWorld()
{
	m_world = NEW World;
}
UnitWorld::~UnitWorld()
{
	delete m_world;
}
void UnitWorld::setUnit(Unit* unit)
{
	m_world->setUnit(unit);
}
World* UnitWorld::getWorld() const
{
	return m_world;
}
void UnitWorld::onAction(World* world,LPVOID value)
{
	m_world->onAction(value);
}
void UnitWorld::onStart(World* world,LPVOID value)
{
	m_world->onStart(value);
}
void UnitWorld::onIdel(World* world,LPVOID value)
{
	m_world->onIdel(value);
}
void UnitWorld::onEnd(World* world,LPVOID value)
{
	m_world->onEnd(value);
}
bool UnitWorld::onRender(World* world,FLOAT& x,FLOAT& y,FLOAT& z)
{
	Texture* texture = getTexture();
	if(texture)
	{
		IDirect3DSurface9* old = Device::getRenderTarget();
		texture->setTarget();
		m_world->render();
		Device::setRenderTarget(old);
	}
	return UnitSprite::onRender(world,x,y,z);
}

}}
