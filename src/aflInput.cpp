//==============================================================
//【フ ァ イル】Input.h
//【  種  別  】ユーザー入力
//【 コメント 】ヤッホー
//--------------------------------------------------------------
//【  履  歴  】
//  [2003/11/6]  作成開始
//==============================================================
#include <windows.h>
#include "aflInput.h"

namespace AFL
{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Input
// ユーザー入力取得
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// ---  動作  ---
// コンストラクタ
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
Input::Input()
{
	m_hwnd = NULL;
}

//-----------------------------------------------
// ---  動作  ---
// キー情報の取得
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
bool Input::isDown(INT action)
{
	if(m_hwnd && m_hwnd != GetForegroundWindow())
		return false;
	std::multimap<INT,INT>::iterator itKey;
	std::pair<std::multimap<INT,INT>::iterator,std::multimap<INT,INT>::iterator> pair;
	pair = m_mapKeys.equal_range(action);

	for(itKey=pair.first;itKey!=pair.second;++itKey)
	{
		if(GetAsyncKeyState((*itKey).second)<0)
			return true;
	}
	return false;
}


//-----------------------------------------------
// ---  動作  ---
// キー情報の設定
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 
//-----------------------------------------------
void Input::setKey(INT action,INT keyCode)
{
	m_mapKeys.insert(std::pair<int,int>(action,keyCode));
}

}