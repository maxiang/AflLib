#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_Image


namespace AFL{namespace WINDOWS{
#ifndef _D3DFORMAT
	typedef enum 
	{

		D3DFMT_R8G8B8               = 20,
		D3DFMT_A8R8G8B8             = 21,
		D3DFMT_X8R8G8B8             = 22,
		D3DFMT_R5G6B5               = 23,
		D3DFMT_X1R5G5B5             = 24,
		D3DFMT_A1R5G5B5             = 25,
		D3DFMT_A4R4G4B4             = 26
	}_D3DFORMAT;
#endif

class DDB
{
public:
	DDB()
	{
		m_bitmap = NULL;
	}
	~DDB()
	{
		release();
	}
	void release()
	{
		if(m_hdc)
		{
			DeleteDC(m_hdc);
		}
		if(m_bitmap)
		{
			DeleteObject(m_bitmap);
			m_bitmap = NULL;
		}
	}
	bool create(INT width,INT height)
	{
		if(m_size.cx == width && m_size.cy == height)
			return false;
		release();
		HDC hdc = GetDC(NULL);
		m_bitmap = CreateCompatibleBitmap(hdc, width, height);
		if(m_bitmap)
		{
			m_hdc = CreateCompatibleDC(hdc);
			SelectObject(m_hdc,m_bitmap);
			m_size.cx = width;
			m_size.cy = height;
		}
		ReleaseDC(NULL,hdc);
		return m_bitmap != NULL;

	}
	bool clear(COLORREF color)
	{
		if(!m_hdc)
			return false;
		RECT rect = {0,0,m_size.cx,m_size.cy};
		HBRUSH b = CreateSolidBrush(color);
		FillRect(m_hdc,&rect,b);
		DeleteObject(b);
		return true;
	}
	HDC getDC() const
	{
		return m_hdc;
	}
protected:
	HBITMAP m_bitmap;
	HDC m_hdc;
	SIZE m_size;
};


class DIB
{
	struct DIBFormat
	{
		INT format;
		INT bits;
		DWORD alphaMask;
		DWORD redMask;
		DWORD greenMask;
		DWORD blueMask;
	};
public:
	DIB();
	~DIB();
	void setReverse(bool value){m_reverse = value;}
	bool createDIB(INT iWidth,INT iHeight,INT iFormat);
	bool releaseDIB();

	HBITMAP getBitmap()const;
	LPVOID getImage()const;
	INT getFormat()const;
	HDC getDC()const;
	INT getPitch()const;
	INT getWidth()const;
	INT getHeight()const;
	INT getSize()const;
	bool isData()const;

#if !defined(_WIN32_WCE)
	bool drawGlyphOutline(INT iX,INT iY,COLORREF refColor,LONG tmAscent,LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap);
	bool createOutlineText(LPCSTR text,INT size,COLORREF color,COLORREF bcolor=-1,INT limitWidth=0);
	bool createOutlineText(LPCSTR text,HFONT font,COLORREF color,COLORREF bcolor=-1,INT limitWidth=0);
	bool createOutlineText(LPCWSTR text,INT size,COLORREF color,COLORREF bcolor=-1,INT limitWidth=0);
	bool createOutlineText(LPCWSTR text,HFONT font,COLORREF color,COLORREF bcolor=-1,INT limitWidth=0);
	bool drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,COLORREF nColor,COLORREF nBColor=0,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,COLORREF nColor,COLORREF nBColor=0,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawOutlineText(int iX,int iY,LPCWSTR pText,HFONT hFont,COLORREF nColor,COLORREF nBColor=0,INT iLimitWidth=0,LPSIZE pSize=NULL);
#endif

	bool drawText(int iX,int iY,LPCSTR pText,INT iSize,COLORREF nColor,COLORREF nBColor=-1,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawText(int iX,int iY,LPCSTR pText,HFONT hFont,COLORREF nColor,COLORREF nBColor=-1,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawText(int iX,int iY,LPCWSTR pText,INT iSize,COLORREF nColor,COLORREF nBColor=-1,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawText(int iX,int iY,LPCWSTR pText,HFONT hFont,COLORREF nColor,COLORREF nBColor=-1,INT iLimitWidth=0,LPSIZE pSize=NULL);
	bool drawAlpha(BYTE byValue);
	bool xorAlpha();
	bool clear(DWORD color=0);

	bool save(LPCSTR fileName);
	bool savePNG(LPCSTR fileName);

	bool bitBlt(HDC hDC,INT destX=0,INT destY=0,INT width=-1,INT height=-1,INT srcX=0,INT srcY=0,DWORD rop=SRCCOPY);
	bool stretchBlt(HDC hDC,INT destX=0,INT destY=0,INT width=-1,INT height=-1,INT srcX=0,INT srcY=0,INT srcWidth=-1,INT srcHeight=-1,DWORD rop=SRCCOPY);
	DWORD getPixel(INT x,INT y);
	void drawBox(INT x,INT y,INT width,INT height,COLORREF color);
	void drawLine(INT x1,INT y1,INT x2,INT y2,COLORREF color);
	HRGN setClip(INT x1,INT y1,INT x2,INT y2);
	void delClip(HRGN hrgn);
protected:
	INT createBitmapHeader(BITMAPV4HEADER* header,INT width,INT height,INT format);
	bool createDIB2(INT iWidth,INT iHeight,INT iFormat);
	DIBFormat* getFomatTable(INT iFormat);

	bool m_reverse;			//DIB上下反転回避
	INT m_format;			//テクスチャフォーマット
	INT m_width;			//イメージ幅
	INT m_height;			//イメージ高さ
	INT m_size;				//データサイズ
	INT m_pitch;			//一列のバイト数
	HDC m_hdc;				//デバイスコンテキスト
	HBITMAP m_oldBitmap;	//旧ビットマップハンドル
	HBITMAP m_bitmap;		//ビットマップハンドル
	LPVOID m_data;			//ビットマップ配列へのポインタ
	
	//テクスチャフォーマット定義用
	static DIBFormat m_dibFormat[];
};

}}
#define __INC_Image
#endif
