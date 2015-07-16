#ifndef __ANDROID__
	#include <windows.h>
#else
	#include <android/log.h>
#endif

#include <map>
#include <vector>
#if !defined(_OPENGL) & !defined(__ANDROID__)
	#include "aflD3DUnit.h"
#else
	#include "aflOpenGLUnit.h"
#endif

using namespace AFL;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Frame
// Frameデータ管理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Frame::Frame()
{
	m_frameParent = NULL;
	m_matrix.setIdentity();
}
NMatrix Frame::getMatrix() const
{
	return m_matrix;
}

void Frame::setName(LPCSTR name)
{
	m_frameName = name;
}
void Frame::add(Frame* frame)
{
	frame->m_frameParent = this;
	m_frameChilds.push_back(frame);
}
void Frame::add(Mesh* mesh)
{
	m_meshes.push_back(*mesh);
}
void Frame::addShadow(Mesh* mesh)
{
	m_meshesShadow.push_back(*mesh);
}
INT Frame::getFrameCount()
{
	return (INT)m_frameChilds.size();
}
INT Frame::getMeshCount()
{
	return (INT)m_meshes.size();
}
INT Frame::getAllFrameCount()
{
	INT iCount = 0;
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it)->getAllFrameCount();
	}
	iCount += getFrameCount();
	return iCount;
}
INT Frame::getAllMeshCount()
{
	INT iCount = 0;
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		iCount += (*it)->getAllMeshCount();
	}
	iCount += getMeshCount();
	return iCount;
}
void Frame::getBoundingBox(NVector* vect,Unit* unit)
{
	NMatrix matrix;
	getLocalMatrix(matrix,unit);
	matrix = matrix * unit->getMatrix();

	INT i;
	for(i=0;i<8;i++)
	{
		NVector vectSrc = m_boundingBox[i].transformCoord(matrix);
	
		vect[0] = vect[0].minimum(vectSrc);
		vect[1] = vect[1].maximum(vectSrc);
	}

}
Mesh* Frame::getMesh()
{
	return m_meshes.size()?&m_meshes.front():NULL;
}

bool Frame::createShadow()
{
	bool flag = false;
	size_t count = m_meshes.size();
	if(count)
	{
		m_meshesShadow.clear();
		m_meshesShadow.resize(m_meshes.size());

		std::list<Mesh>::iterator itMesh;
		std::list<Mesh>::iterator itMeshShadow;
		for(itMesh=m_meshes.begin(),itMeshShadow=m_meshesShadow.begin();
			itMesh!=m_meshes.end();++itMesh,++itMeshShadow)
		{
//			itMesh->createShadow(*itMeshShadow);
		}
		flag = true;
	}
	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		flag |= (*itFrame)->createShadow();
	}

	return flag;
}

void Frame::createBoundingBox()
{

	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		(*itFrame)->createBoundingBox();
	}

	if(m_meshes.size())
	{
		NVector vect[2];
		std::list<Mesh>::iterator itMesh;
		itMesh = m_meshes.begin();
		vect[0] = itMesh->getVertexRange()[0];
		vect[1] = itMesh->getVertexRange()[1];

		for(;itMesh!=m_meshes.end();++itMesh)
		{
			NVector v[2] = {itMesh->getVertexRange()[0],itMesh->getVertexRange()[1]};
			vect[0] = vect[0].minimum(v[0]);
			vect[1] = vect[1].maximum(v[1]);
		}

		m_bounding[0] = vect[0];
		m_bounding[1] = vect[1];
	}
	else
	{
		ZeroMemory(m_bounding,sizeof(m_bounding));
	}

	NVector vectBox[8] = 
	{
		m_bounding[0].x,m_bounding[0].y,m_bounding[0].z,
		m_bounding[0].x,m_bounding[0].y,m_bounding[1].z,
		m_bounding[0].x,m_bounding[1].y,m_bounding[0].z,
		m_bounding[1].x,m_bounding[0].y,m_bounding[0].z,
		m_bounding[1].x,m_bounding[1].y,m_bounding[0].z,
		m_bounding[0].x,m_bounding[1].y,m_bounding[1].z,
		m_bounding[1].x,m_bounding[0].y,m_bounding[1].z,
		m_bounding[1].x,m_bounding[1].y,m_bounding[1].z,
	};
	CopyMemory(m_boundingBox,vectBox,sizeof(NVector)*8);
}
void Frame::setBoundingBox(NVector* vect)
{
	m_bounding[0] = vect[0];
	m_bounding[1] = vect[1];
	NVector vectBox[8] = 
	{
		m_bounding[0].x,m_bounding[0].y,m_bounding[0].z,0.0f,
		m_bounding[0].x,m_bounding[0].y,m_bounding[1].z,0.0f,
		m_bounding[0].x,m_bounding[1].y,m_bounding[0].z,0.0f,
		m_bounding[1].x,m_bounding[0].y,m_bounding[0].z,0.0f,
		m_bounding[1].x,m_bounding[1].y,m_bounding[0].z,0.0f,
		m_bounding[0].x,m_bounding[1].y,m_bounding[1].z,0.0f,
		m_bounding[1].x,m_bounding[0].y,m_bounding[1].z,0.0f,
		m_bounding[1].x,m_bounding[1].y,m_bounding[1].z,0.0f
	};
	CopyMemory(m_boundingBox,vectBox,sizeof(NVector)*8);
}

NMatrix Frame::getMatrix(Unit* unit)
{
	NMatrix matrix;
	if(!unit->getAnimationMatrix(getFrameName(),matrix))
		return m_matrix;
	return matrix;
}
void Frame::getLocalMatrix(NMatrix& matrix,Unit* unit)
{
	matrix = getMatrix();
	Frame* parent = this;
	while((parent = parent->getParentFrame())!=NULL)
	{
		matrix = matrix * parent->getMatrix(unit);
	}
}

void Frame::getBoundingBox(Unit* unit,NMatrix* pTopMatrix,NMatrix* pMatrix,NVector* vect)
{
	NMatrix matrix,matAnimation;
	
	if(unit->getAnimationMatrix(getFrameName(),matAnimation))
		matrix = matAnimation * *pMatrix;
	else
		matrix = (NMatrix)m_matrix * *pMatrix;

	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=m_frameChilds.begin();itFrame!=m_frameChilds.end();++itFrame)
	{
		(*itFrame)->getBoundingBox(unit,pTopMatrix,&matrix,vect);
	}

	if(m_meshes.size() == 0)
		return;

	NMatrix matrixCache = matrix * *pTopMatrix;

	INT i;
	for(i=0;i<8;i++)
	{
		NVector vectSrc;
		vectSrc = m_boundingBox[i].transformCoord(matrixCache);
		vect[0] = vect[0].minimum(vectSrc);
		vect[1] = vect[1].maximum(vectSrc);
	}

}
void Frame::setParent()
{
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		(*it)->m_frameParent = this;
		(*it)->setParent();
	}
}
void Frame::setMatrix(NMatrix* matrix)
{
	m_matrix = *matrix;
}
LPCSTR Frame::getFrameName() const
{
	return m_frameName.c_str();
}
Frame* Frame::getFrame(LPCSTR name)
{
	if(strcmp(name,getFrameName()) == 0)
		return this;
	std::list<SP<Frame> >::iterator it;
	for(it=m_frameChilds.begin();it!=m_frameChilds.end();++it)
	{
		Frame* frame = (*it)->getFrame(name);
		if(frame)
			return frame;
	}
	return NULL;
}
Frame* Frame::getParentFrame() const
{
	return m_frameParent;
}
Frame& Frame::operator=(const Frame& frame)
{
	m_matrix = frame.m_matrix;
	m_meshes = frame.m_meshes;
	m_meshesShadow = frame.m_meshesShadow;
	m_frameChilds = frame.m_frameChilds;
	m_frameName = frame.m_frameName;
	CopyMemory(&m_bounding,&frame.m_bounding,sizeof(m_bounding));
	CopyMemory(&m_boundingBox,&frame.m_boundingBox,sizeof(m_boundingBox));
	setParent();
	return *this;
}

std::list<SP<Frame> >& Frame::getFrameChilds()
{
	return m_frameChilds;
}
std::list<Mesh>& Frame::getMeshes()
{
	return m_meshes;
}
std::list<Mesh>& Frame::getShadows()
{
	return m_meshesShadow;
}
