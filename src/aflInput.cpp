//==============================================================
//�y�t �@ �C���zInput.h
//�y  ��  ��  �z���[�U�[����
//�y �R�����g �z���b�z�[
//--------------------------------------------------------------
//�y  ��  ��  �z
//  [2003/11/6]  �쐬�J�n
//==============================================================
#include <windows.h>
#include "aflInput.h"

namespace AFL
{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Input
// ���[�U�[���͎擾
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-----------------------------------------------
// ---  ����  ---
// �R���X�g���N�^
// ---  ����  ---
// ����
// --- �߂�l ---
// 
//-----------------------------------------------
Input::Input()
{
	m_hwnd = NULL;
}

//-----------------------------------------------
// ---  ����  ---
// �L�[���̎擾
// ---  ����  ---
// ����
// --- �߂�l ---
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
// ---  ����  ---
// �L�[���̐ݒ�
// ---  ����  ---
// ����
// --- �߂�l ---
// 
//-----------------------------------------------
void Input::setKey(INT action,INT keyCode)
{
	m_mapKeys.insert(std::pair<int,int>(action,keyCode));
}

}