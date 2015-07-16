#pragma once

#include "aflD3DUnit.h"
#include "aflDShow.h"


namespace AFL{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitVideo
// DirectX - 動画再生用ユニット
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitVideo : public UnitGdip,public MediaSampler
{
public:
	UnitVideo();
	virtual ~UnitVideo();
	void releaseBuffer();
	void addEvent(ClassProc& classProc)
	{
		m_classProc = classProc;
	}
protected:
	virtual void onStatChange()
	{
		m_classProc.call(this);
	}
	virtual void onImageInit();
	bool onRender(LPVOID world,FLOAT& x,FLOAT& y,FLOAT& z);
	virtual void onImageDraw(LPVOID data,DWORD size);
	LPVOID m_dataPtr;
	INT m_dataSize;
	Critical m_critical;
	ClassProc m_classProc;
	bool m_initImage;
};

}

