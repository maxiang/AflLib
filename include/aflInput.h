//==============================================================
//【フ ァ イル】Input.h
//【  種  別  】ユーザー入力
//【 コメント 】ヤッホー
//--------------------------------------------------------------
//【  履  歴  】
//  [2003/11/6]  作成開始
//==============================================================
#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_Input

#include "aflStd.h"
#include <map>
namespace AFL
{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Input
// ユーザー入力取得
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef __ANDROID__
class Input
{
public:
	Input();
	virtual bool isDown(INT action);
	FLOAT getDown(INT action)
	{
		if(isDown(action))
			return 1.0f;
		return 0.0f;
	}
	void setKey(INT action,INT keyCode);
	void setWnd(HWND hwnd)
	{
		m_hwnd = hwnd;
	}

protected:
	std::multimap<INT,INT> m_mapKeys;
	HWND m_hwnd;

};
#else

enum
{
	VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_BUTTON1,VK_BUTTON2,VK_PAGEUP,VK_PAGEDOWN
};

class Input
{
public:
	Input();
	virtual bool isDown(INT action);
	virtual FLOAT getDown(INT action);
	void setKey(INT action,INT keyCode);
	void sendEvent(float* point,int size);
protected:
	std::multimap<INT,INT> m_mapKeys;
	std::map<INT,FLOAT> m_mapKeyPowers;

	bool m_vkUp;
	bool m_vkDown;
	bool m_vkLeft;
	bool m_vkRight;
	bool m_vkZoomUp;
	bool m_vkZoomDown;
	bool m_vkJump;
	bool m_vkAttack;
	float m_x;
	float m_y;
	float m_x2;
	float m_y2;
	float m_downX[2];
	float m_downY[2];
};

#endif
}
#define __INC_Input
#endif
