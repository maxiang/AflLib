
#ifndef __ANDROID__
	#include <windows.h>
#else
	#include <android/log.h>
#endif
#include "aflInput.h"

namespace AFL
{

#ifndef __ANDROID__
Input::Input()
{
	m_hwnd = NULL;
}


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
#else

Input::Input()
{
	m_vkLeft = false;
	m_vkRight = false;
	m_vkUp = false;
	m_vkDown = false;
	m_vkZoomUp = false;
	m_vkZoomDown = false;
	m_vkJump = false;
	m_vkAttack = false;
	m_x = 0.0f;
	m_y = 0.0f;
}


bool Input::isDown(INT action)
{
	if(action == VK_UP && m_vkUp)
		return true;
	if(action == VK_DOWN && m_vkDown)
		return true;
	if(action == VK_LEFT && m_vkLeft)
		return true;
	if(action == VK_RIGHT && m_vkRight)
		return true;
	if(action == 4 && m_vkAttack)
		return true;
	if(action == 5 && m_vkJump)
		return true;
	if(action == 6 && m_vkZoomUp)
		return true;
	if(action == 7 && m_vkZoomDown)
		return true;

	return false;
}
FLOAT Input::getDown(INT action)
{
	return m_mapKeyPowers[action];
}
void Input::sendEvent(float* point,int size)
{
	int count = size /5;


	m_mapKeyPowers[(INT)VK_UP] = 0;
	m_mapKeyPowers[(INT)VK_DOWN] = 0;
	m_mapKeyPowers[(INT)VK_LEFT] = 0;
	m_mapKeyPowers[(INT)VK_RIGHT] = 0;
	m_mapKeyPowers[(INT)VK_BUTTON1] = 0;
	m_mapKeyPowers[(INT)VK_BUTTON2] = 0;
	m_mapKeyPowers[(INT)VK_PAGEUP] = 0;
	m_mapKeyPowers[(INT)VK_PAGEDOWN] = 0;

	m_vkRight = false;
	m_vkLeft = false;
	m_vkUp = false;
	m_vkDown = false;
	m_vkJump = false;
	m_vkAttack = false;
	m_vkZoomUp = false;
	m_vkZoomDown = false;

	if(count < 2)
		return;
	float x1,y1,x2,y2;
	if(point[1] < point[5+1])
	{
		x1 = point[1];
		y1 = point[2];
		x2 = point[5+1];
		y2 = point[5+2];
	}
	else
	{
		x2 = point[1];
		y2 = point[2];
		x1 = point[5+1];
		y1 = point[5+2];
	}


	static int atackCount = 0;

	static bool flag = false;


	FLOAT margin = 0.2f;
	FLOAT f = y1 - y2;
	if(f > margin)
	{
		m_mapKeyPowers[(INT)VK_LEFT] = (f-margin)*2.0f;
        m_vkLeft = true;
	}
	if(f < -margin)
	{
		m_mapKeyPowers[(INT)VK_RIGHT] = (-f+margin)*2.0f;
 		m_vkRight = true;
	}


	if(y1 < -0.0f)// && y2 < -0.07f)
	{
		f = -(y1<y2?y1:y2)/0.5f;
		if(f > 1.0f)
			f = 1.0f;
		m_mapKeyPowers[(INT)VK_UP] = f;
 		m_vkUp = true;
	}

	if(y1 > 0.2f)// && y2 > 0.07f)
	{
		f = ((y1>y2?y1:y2)-0.2)/0.5f;
		if(f > 1.0f)
			f = 1.0f;
		m_mapKeyPowers[(INT)VK_DOWN] = f;
 		m_vkDown = true;
	}

	if(x1 > -0.3f)
		m_vkZoomUp = true;
	else if(x1 > -0.6f)
		m_vkJump = true;

	if(x2 < 0.3f)
		m_vkZoomDown = true;
	else if(x2 < 0.6f)
		m_vkAttack = true;



	/*
	if(x2 >= 0)
	{
		float zx1 = pow(m_x - m_x2,2) + pow(m_y - m_y2,2);
		float zx2 = pow(x - x2,2) + pow(y - y2,2);
		if(zx1 - zx2 > 12.0f)
			m_vkZoomUp = true;
		else
			m_vkZoomUp = false;

		if(zx1 - zx2 < -12.0f)
			m_vkZoomDown = true;
		else
			m_vkZoomDown = false;
	}
	else
	{
		if(action == 0)
			m_flag = true;
		else if(action == 1)
			m_flag = false;

		if(x - m_x > 2.0f)
			m_vkLeft = true;
		else
			m_vkLeft = false;

		if(x - m_x < -2.0f)
			m_vkRight = true;
		else
			m_vkRight = false;
	}

	m_x = x;
	m_y = y;
	m_x2 = x2;
	m_y2 = y2;
*/
}

#endif


void Input::setKey(INT action,INT keyCode)
{
	m_mapKeys.insert(std::pair<int,int>(action,keyCode));
}

}
