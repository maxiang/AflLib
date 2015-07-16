#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECTDRAW

namespace AFL{namespace DirectDraw{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device
// DirectX - DirectDraw基本ラッパー
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Device
{
public:
	Device();
	~Device();
	bool setFullMode(HWND hWnd,int nWidth,int nHeight,int nBits);
	bool setWindowMode();
protected:
	struct IDirectDraw* m_pDirectDraw;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Screen
// DirectX - DirectDrawスクリーンモード設定
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Screen
{
public:
	Screen();
	~Screen();
	bool setScreenMode(HWND hWnd,bool bMode,INT iWidth=0,INT iHeight=0);
	bool changeDirectMode(HWND hWnd,INT iWidth,INT iHeight);
	bool setFullScreenMode(INT iWidth,INT iHeight);
	bool changeWindowMode();
	bool isScreenMode()const{return m_bFullScreen;}
	Device* getDevice();
protected:
	Device* m_pDDraw;
	HWND m_hWnd;
	HWND m_hWndParent;
	HMENU m_hMenu;
	DWORD m_dwStyle;
	DWORD m_dwStyleEx;
	RECT m_rectWindow;
	bool m_bFullScreen;
};

}}
#define __INC_AFLDIRECTDRAW
#endif