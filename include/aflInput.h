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

}
#define __INC_Input
#endif