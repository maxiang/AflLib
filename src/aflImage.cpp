#include <windows.h>

#include "aflWinTool.h" 
#include "aflImage.h" 

/*
#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define new new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif 

*/

using namespace AFL::WINDOWS;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// DIB
// DIB用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
DIB::DIBFormat DIB::m_dibFormat[] = //フォーマットマスクデータ
{
	{D3DFMT_R8G8B8,  24,0x00000000,0x00ff0000,0x0000ff00,0x000000ff},
	{D3DFMT_X8R8G8B8,32,0x00000000,0x00ff0000,0x0000ff00,0x000000ff},
	{D3DFMT_A8R8G8B8,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff},
	{D3DFMT_X1R5G5B5,16,0x00000000,0x00007c00,0x000003e0,0x0000001f},
	{D3DFMT_R5G6B5  ,16,0x00000000,0x0000f800,0x000007e0,0x0000001f},
	{D3DFMT_A1R5G5B5,16,0x00008000,0x00007c00,0x000003e0,0x0000001f},
	{D3DFMT_A4R4G4B4,16,0x0000f000,0x00000f00,0x000000f0,0x0000000f},
	-1
};

//-----------------------------------------------
//DIB::DIB()
// ---  動作  ---
// 初期値の設定
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
DIB::DIB()
{
	m_reverse = true;
	m_format = -1;
	m_width = 0;
	m_height = 0;
	m_hdc = 0;
	m_bitmap = 0;
	m_oldBitmap = 0;
	m_data = NULL;
}
//-----------------------------------------------
//DIB::~DIB()
// ---  動作  ---
// DIB用に確保したデータの解放
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 無し
//-----------------------------------------------
DIB::~DIB()
{
	releaseDIB();
}

//-----------------------------------------------
//bool DIB::releaseDIB()
// ---  動作  ---
// DIB関連データの解放
// ---  引数  ---
// 無し
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::releaseDIB()
{
	if(m_hdc)
	{
		SelectObject(m_hdc,m_oldBitmap);	//古いビットマップハンドルを戻す
		DeleteDC(m_hdc);					//DC解放
		DeleteObject(m_bitmap);			//ビットマップ解放
	
		m_format = -1;
		m_width = 0;
		m_height = 0;
		m_hdc = 0;
		m_bitmap = 0;
		m_oldBitmap = 0;
		m_data = NULL;
		return true;
	}
	return false;
}
//-----------------------------------------------
//bool DIB::createDIB(INT iWidth,INT iHeight,INT format)
// ---  動作  ---
// DIBの作成
// 対象フォーマットが使用不可能なときは別フォーマットを自動選択
// ---  引数  ---
// iWidth 幅
// iHeight 高さ
// format 対象テクスチャフォーマット
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::createDIB(INT iWidth,INT iHeight,INT format)
{
	//フォーマットに応じて、DIBの作成
	if(createDIB2(iWidth,iHeight,format))
		return true;
	//同一フォーマットで生成不能なとき、別のフォーマットを選ぶ
	if(format == D3DFMT_A1R5G5B5 || format == D3DFMT_A4R4G4B4)
		return createDIB2(iWidth,iHeight,D3DFMT_A8R8G8B8);
	if(format == D3DFMT_R5G6B5)
		return createDIB2(iWidth,iHeight,D3DFMT_X8R8G8B8);
	return false;
}
INT DIB::createBitmapHeader(BITMAPV4HEADER* header,INT width,INT height,INT format)
{
	//フォーマットからマスク情報を取り出す
	DIBFormat* pdibFormat = getFomatTable(format);
	if(!pdibFormat)
		return 0;

	//DIBの一列のバイト数は4の倍数
	INT pitch = (width * pdibFormat->bits/8 + 3)/4*4;


	//構造体初期化
	ZeroMemory(header,sizeof(BITMAPV4HEADER));
	header->bV4Size = sizeof(BITMAPV4HEADER);	//構造体サイズ
	header->bV4Width = width;					//幅
	if(m_reverse)
		header->bV4Height = -(height + 1);		//高さ
	else
		header->bV4Height = height;				//高さ
	header->bV4BitCount  = pdibFormat->bits;
	header->bV4SizeImage = height * pitch;
	header->bV4Planes = 1;						//常に1
	if(format != D3DFMT_X1R5G5B5 && format != D3DFMT_X8R8G8B8 && format != D3DFMT_R8G8B8)
	{
		header->bV4V4Compression = BI_BITFIELDS;	//ビットフィールド使用
		header->bV4RedMask = pdibFormat->redMask;
		header->bV4GreenMask = pdibFormat->greenMask;
		header->bV4BlueMask = pdibFormat->blueMask;
		header->bV4AlphaMask = pdibFormat->alphaMask;	
	}
	return pitch;
}
//-----------------------------------------------
//bool DIB::createDIB2(INT iWidth,INT iHeight,INT format)
// ---  動作  ---
// DIBの作成
// ---  引数  ---
// iWidth 幅
// iHeight 高さ
// format 対象テクスチャフォーマット
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::createDIB2(INT iWidth,INT iHeight,INT format)
{
	//既存DIBの開放
	releaseDIB();

	HDC hDC;
	HBITMAP hBitmap;
	LPBYTE pBitmap;
	INT pitch;
	//ヘッダーの作成
	BITMAPV4HEADER bitmapHeader;
	pitch = createBitmapHeader(&bitmapHeader,iWidth,iHeight,format);
	if(!pitch)
		return false;

	//DIB作成
	hBitmap = CreateDIBSection(NULL,(LPBITMAPINFO)&bitmapHeader
		,DIB_RGB_COLORS,(void**)&pBitmap,0,0);
	if(!hBitmap)
		return false;
	
	//ビットマップイメージのクリア
	ZeroMemory(pBitmap,bitmapHeader.bV4SizeImage);

	//DC作成
	hDC = CreateCompatibleDC(NULL);
	
	//旧ハンドル待避と新ハンドルの設定	
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC,hBitmap);
	
	//既存データの解放
	releaseDIB();

	//メンバに記憶
	m_hdc = hDC;
	m_bitmap = hBitmap;
	m_data = pBitmap;
	m_size = bitmapHeader.bV4SizeImage;
	m_oldBitmap = hOldBitmap;

	m_width = iWidth;
	m_height = iHeight;
	m_format = format;
	m_pitch = pitch;
	
	return true;

}

//-----------------------------------------------
//DIB::DIBFormat* DIB::getFomatTable(INT format)
// ---  動作  ---
// テクスチャフォーマットからDIBマスクデータのテーブルを検索
// ---  引数  ---
// format 対象テクスチャフォーマット
// --- 戻り値 ---
// DIBマスクデータテーブルのポインタ
//-----------------------------------------------
DIB::DIBFormat* DIB::getFomatTable(INT format)
{
	//マスク情報を検索
	int i;
	for(i=0;m_dibFormat[i].format != -1 && m_dibFormat[i].format != format;i++);
	if(format == -1)
		return NULL;
	return m_dibFormat + i;
}

//-----------------------------------------------
//bool DIB::xorAlpha()
// ---  動作  ---
// DIBのダミーαの反転
// ---  引数  ---
// 無し
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::xorAlpha()
{
	int i;
	int iSize = getPitch() * getHeight();	//サイズ
	LPBYTE pbyData = (LPBYTE)getImage();	//書き込み用ポインタ

	//想定フォーマットごとのα値反転
	switch(m_format)
	{
	case D3DFMT_A8R8G8B8:
		for(i=3;i<iSize;i+=4)
			pbyData[i] ^= 0xff;
		break;
	case D3DFMT_A1R5G5B5:
		for(i=1;i<iSize;i+=2)
			pbyData[i] ^= 0x80;
		break;
	case D3DFMT_A4R4G4B4:
		for(i=1;i<iSize;i+=2)
			pbyData[i] ^= 0xf0;
		break;
	default:
		return false;
	}
	return true;
}

//-----------------------------------------------
//bool DIB::drawAlpha(BYTE byValue)
// ---  動作  ---
// DIBのダミーαの設定
// ---  引数  ---
// byValue 設定するα値
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::clear(DWORD color)
{
	int i;
	LPBYTE pbyData = (LPBYTE)getImage();	//書き込み用ポインタ
	int iSize = getPitch() * getHeight();	//サイズ


	BYTE byRed = ((LPBYTE)&color)[0];
	BYTE byGreen = ((LPBYTE)&color)[1];
	BYTE byBlue = ((LPBYTE)&color)[2];
	BYTE byAlpha = ((LPBYTE)&color)[3];

	WORD value;

	if(m_format == D3DFMT_R5G6B5)
		value = ((byRed&0xf8) << 8) + ((byGreen&0xfc) << 3) + (byBlue >> 3);
	else if(m_format == D3DFMT_X1R5G5B5)
		value = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
	else if(m_format == D3DFMT_A1R5G5B5)
	{
		value = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
		if(byAlpha)
			value |= 0x8000;
	}
	else if(m_format == D3DFMT_A4R4G4B4)
	{
		value = ((byAlpha&0xf0) << 12) | ((byRed&0xf0) << 8) + (byGreen&0xf0) + (byBlue >> 4);
	}

	//想定フォーマットごとのα値を設定
	switch(m_format)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		for(i=0;i<iSize;i+=4)
			*(LPDWORD)(pbyData+i) = color;
		return true;
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_R5G6B5:
		for(i=0;i<iSize;i+=2)
		{
			*(LPWORD)(pbyData+i) = value;
		}
		break;

	default:
		return false;
	}
	return true;
}

bool DIB::drawAlpha(BYTE byValue)
{
	int i;
	BYTE byDrawValue;						//描画する値
	LPBYTE pbyData = (LPBYTE)getImage();	//書き込み用ポインタ
	int iSize = getPitch() * getHeight();	//サイズ


	//想定フォーマットごとのα値を設定
	switch(m_format)
	{
	case D3DFMT_A8R8G8B8:
		for(i=2;i<iSize;i+=4)
			pbyData[i] = byValue;
		break;
	case D3DFMT_A1R5G5B5:
		if(byValue)
			byDrawValue = 0x80;
		else
			byDrawValue = 0x00;
		for(i=1;i<iSize;i+=2)
		{
			pbyData[i] &= 0x7f;
			pbyData[i] |= byDrawValue;
		}
		break;
	case D3DFMT_A4R4G4B4:
		byDrawValue = byValue & 0xf0;
		for(i=1;i<iSize;i+=2)
		{
			pbyData[i] &= 0x0f;
			pbyData[i] |= byDrawValue;
		}
		break;
	default:
		return false;
	}
	return true;
}

//-----------------------------------------------
//bool DIB::drawText(int iX,int iY,LPCSTR pText,INT iSize,
//	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
// ---  動作  ---
// テキストを描画、フォント指定無し
// ---  引数  ---
// iX 描画座標X
// iY 描画座標Y
// pTest 描画テキスト
// iSize 文字サイズ
// colText 文字色
// colBack 縁取り、-1で無し
// iLimitWidth 最大幅、到達すると改行
// pSize 描画範囲を返すための領域へのポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::drawText(int iX,int iY,LPCSTR pText,INT iSize,COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{
	//フォントハンドルの取得
	Font font;
	font.setSize(iSize);
	font.createFont();
	return drawText(iX,iY,UCS2(pText),font,colText,colBack,iLimitWidth,pSize); 
}
bool DIB::drawText(int iX,int iY,LPCWSTR pText,INT iSize,COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{
	//フォントハンドルの取得
	Font font;
	font.setSize(iSize);
	font.createFont();
	return drawText(iX,iY,pText,font,colText,colBack,iLimitWidth,pSize); 
}

//-----------------------------------------------
//bool DIB::drawText(int iX,int iY,LPCSTR pText,HFONT hFont,
//	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
// ---  動作  ---
// テキストを描画、フォント指定有り
// ---  引数  ---
// iX 描画座標X
// iY 描画座標Y
// pTest 描画テキスト
// hFont フォントハンドル
// colText 文字色
// colBack 縁取り、-1で無し
// iLimitWidth 最大幅、到達すると改行
// pSize 描画範囲を返すための領域へのポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::drawText(int iX,int iY,LPCSTR pText,HFONT hFont,COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{	
	return drawText(iX,iY,UCS2(pText),hFont,colText,colBack,iLimitWidth,pSize);
}
bool DIB::drawText(int iX,int iY,LPCWSTR pText,HFONT hFont,COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{	
	if(!pText)
		return false;

	INT i;
	HDC hDC = getDC();
	HFONT hFontBack = (HFONT)SelectObject(hDC,hFont);

	SIZE sizeMax = {0,0};
	INT iWidth = 0;
	INT iHeight = 0;
	SIZE sizeFont;
	sizeFont.cy = 0;
	for(i=0;pText[i];i++)
	{
		//タブ
		if(pText[i] == '\t')
		{
			GetTextExtentPoint32W(hDC,L" ",1,&sizeFont);
			iWidth += sizeFont.cx*4;
			iWidth -= iWidth % (sizeFont.cx*4);
			continue;
		}
		//改行
		if(pText[i] == '\n')
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
			continue;
		}
		//サイズ取得
		GetTextExtentPoint32W(hDC,pText+i,1,&sizeFont);

		//改行
		if(iLimitWidth > 0 && sizeFont.cx + iWidth > iLimitWidth)
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
		}
		//描画処理
		SetBkMode(hDC,TRANSPARENT);
		if((int)colBack != -1)
		{
			SetTextColor(hDC,colBack);
			TextOutW(hDC,iWidth+iX+0,iHeight+iY+1,pText+i,1);
			TextOutW(hDC,iWidth+iX+0,iHeight+iY+2,pText+i,1);
			TextOutW(hDC,iWidth+iX+2,iHeight+iY+0,pText+i,1);
			TextOutW(hDC,iWidth+iX+0,iHeight+iY+1,pText+i,1);
		}
		SetTextColor(hDC,colText);
		TextOutW(hDC,iWidth+iX+1,iHeight+iY+1,pText+i,1);
		
		iWidth += sizeFont.cx;
		if(sizeMax.cx < iWidth)
			sizeMax.cx = iWidth;
	}
	sizeMax.cy = iHeight+sizeFont.cy;
	if(pSize)
		*pSize = sizeMax;
	SelectObject(hDC,hFontBack);
	
	return true;
}


#if !defined(_WIN32_WCE)
//-----------------------------------------------
//bool DIB::drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,
//	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
// ---  動作  ---
// テキストを描画、アンチエイリアス付き、フォント指定無し
// ---  引数  ---
// iX 描画座標X
// iY 描画座標Y
// pTest 描画テキスト
// iSize 文字サイズ
// colText 文字色
// colBack 縁取り、-1で無し
// iLimitWidth 最大幅、到達すると改行
// pSize 描画範囲を返すための領域へのポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::drawOutlineText(int iX,int iY,LPCSTR pText,INT iSize,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{
	//フォントハンドルの取得
	Font font;
	font.setSize(iSize);
	font.createFont();
	return drawOutlineText(iX,iY,UCS2(pText),font,colText,colBack,iLimitWidth,pSize); 
}

bool DIB::createOutlineText(LPCSTR text,INT size,COLORREF color,COLORREF bcolor,INT limitWidth)
{
	return createOutlineText(UCS2(text),size,color,bcolor,limitWidth);
}
bool DIB::createOutlineText(LPCWSTR text,INT size,COLORREF color,COLORREF bcolor,INT limitWidth)
{
	//フォントハンドルの取得
	Font font;
	font.setSize(size);
	font.createFont();

	SIZE sizeImage;
	font.getFontSize(&sizeImage,text,-1,limitWidth);
	createDIB(sizeImage.cx,sizeImage.cy,D3DFMT_A8R8G8B8);
	return drawOutlineText(0,0,text,font,color,bcolor,limitWidth); 

}
bool DIB::createOutlineText(LPCSTR text,HFONT font,COLORREF color,COLORREF bcolor,INT limitWidth)
{
	return createOutlineText(UCS2(text),font,color,bcolor,limitWidth);
}
bool DIB::createOutlineText(LPCWSTR text,HFONT font,COLORREF color,COLORREF bcolor,INT limitWidth)
{
	SIZE sizeImage;
	Font::getFontSize(&sizeImage,font,text,-1,limitWidth);
	createDIB(sizeImage.cx+8,sizeImage.cy+8,D3DFMT_A8R8G8B8);
	return drawOutlineText(0,0,text,font,color,bcolor,limitWidth); 
}

//-----------------------------------------------
//bool DIB::drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,
//	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
// ---  動作  ---
// テキストを描画、アンチエイリアス付き、フォント指定あり
// ---  引数  ---
// iX 描画座標X
// iY 描画座標Y
// pTest 描画テキスト
// hFont フォントハンドル
// colText 文字色
// colBack 縁取り、-1で無し
// iLimitWidth 最大幅、到達すると改行
// pSize 描画範囲を返すための領域へのポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//----------------------------------------------
bool DIB::drawOutlineText(int iX,int iY,LPCSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{
	return drawOutlineText(iX,iY,UCS2(pText),hFont,colText,colBack,iLimitWidth,pSize);
}
bool DIB::drawOutlineText(int iX,int iY,LPCWSTR pText,HFONT hFont,
	COLORREF colText,COLORREF colBack,INT iLimitWidth,LPSIZE pSize)
{
	if(!pText)
		return false;
	HDC hDC = getDC();
	if(!hDC)
		return false;
	GLYPHMETRICS metInfo;
	INT i;
	TEXTMETRIC tm; 
	HFONT hFontBack = (HFONT)SelectObject(hDC,hFont);
	GetTextMetrics(hDC,&tm);

	//回転無し
	MAT2 mx2 = {1,1,0,0,0,0,1,1};

	SIZE sizeMax = {0,0};
	INT iWidth = 0;
	INT iHeight = 0;
	SIZE sizeFont={0,0};
	for(i=0;pText[i];i++)
	{
		//タブ
		if(pText[i] == '\t')
		{
			GetTextExtentPoint32W(hDC,L" ",1,&sizeFont);
			iWidth += sizeFont.cx*4;
			iWidth -= iWidth % (sizeFont.cx*4);
			continue;
		}
		//改行
		if(pText[i] == '\n')
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
			continue;
		}
		//サイズ取得
		GetTextExtentPoint32W(hDC,pText+i,1,&sizeFont);
		//改行
		if(iLimitWidth > 0 && sizeFont.cx + iWidth > iLimitWidth)
		{
			iHeight += sizeFont.cy;
			iWidth = 0;
		}
		WORD wCode;
		wCode = pText[i];

		INT iBuffSize = GetGlyphOutline(hDC,wCode,GGO_GRAY8_BITMAP,&metInfo,NULL,NULL,&mx2);
		if(iBuffSize > 0)
		{
			LPBYTE pbyBitmap = new BYTE[iBuffSize]; 
			GetGlyphOutline(hDC,wCode,GGO_GRAY8_BITMAP,&metInfo,iBuffSize,pbyBitmap,&mx2);

			if((int)colBack != -1)
			{
				drawGlyphOutline(iWidth+iX+0,iHeight+iY+2,colBack,tm.tmAscent,&metInfo,pbyBitmap);
				drawGlyphOutline(iWidth+iX+2,iHeight+iY+0,colBack,tm.tmAscent,&metInfo,pbyBitmap);
				drawGlyphOutline(iWidth+iX+1,iHeight+iY+0,colBack,tm.tmAscent,&metInfo,pbyBitmap);
				drawGlyphOutline(iWidth+iX+0,iHeight+iY+1,colBack,tm.tmAscent,&metInfo,pbyBitmap);
			}
			drawGlyphOutline(iWidth+iX+1,iHeight+iY+1,colText,tm.tmAscent,&metInfo,pbyBitmap);
			delete pbyBitmap;
		}

		iWidth += sizeFont.cx;
		if(sizeMax.cx < iWidth)
			sizeMax.cx = iWidth;
	}
	sizeMax.cy = iHeight+sizeFont.cy;
	if(pSize)
		*pSize = sizeMax;
	SelectObject(hDC,hFontBack);
	return true;
}

//-----------------------------------------------
//bool DIB::drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,
//	LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
// ---  動作  ---
// アンチエイリアス付きテキストの描画、内部利用版
// ---  引数  ---
// iX 描画座標X
// iY 描画座標Y
// colText テキストカラー
// tmAscent アウトラインテキストのアスペクト情報
// pmetInfo 文字列の回転パラメータ
// pbyBitmap 形状が記憶されている領域のポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::drawGlyphOutline(INT iX,INT iY,COLORREF colText,LONG tmAscent,LPGLYPHMETRICS pmetInfo,LPBYTE pbyBitmap)
{
	INT i,j;
	BYTE byAlpha;
	DWORD dwColor;
	WORD wColor;
	iX += pmetInfo->gmptGlyphOrigin.x;
	iY += tmAscent - pmetInfo->gmptGlyphOrigin.y;
	INT countY;
	INT countX;
	if(iX + pmetInfo->gmBlackBoxX > (UINT)getWidth())
		countX = getWidth() - iX;
	else
		countX = pmetInfo->gmBlackBoxX;
	if(iY + pmetInfo->gmBlackBoxY > (UINT)getHeight())
		countY = getHeight() - iY;
	else
		countY = pmetInfo->gmBlackBoxY;
	const INT iCountX = countX;
	const INT iCountY = countY;
	INT iSrcPitch = (pmetInfo->gmBlackBoxX + 3) / 4 * 4;
	INT iDestPitch = getPitch();
	LPBYTE pbyDest = (LPBYTE)getImage();	//書き込み用ポインタ
	BYTE byRed = ((LPBYTE)&colText)[0];
	BYTE byGreen = ((LPBYTE)&colText)[1];
	BYTE byBlue = ((LPBYTE)&colText)[2];

	//想定フォーマットごとのα値反転
	switch(m_format)
	{
	case D3DFMT_X8R8G8B8:
		pbyDest += iDestPitch * iY + iX * 4;
		dwColor = (byRed << 16) + (byGreen << 8) + byBlue;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					*(LPDWORD)(pbyDest+i*4) = dwColor;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_A8R8G8B8:
		pbyDest += iDestPitch * iY + iX * 4;
		dwColor = (byRed << 16) + (byGreen << 8) + byBlue;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					byAlpha = ((byAlpha-1) << 2) + 3;
					*(LPDWORD)(pbyDest+i*4) = dwColor;
					pbyDest[i*4+3] = byAlpha;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5:
		if(m_format == D3DFMT_R5G6B5)
			wColor = ((byRed&0xf8) << 8) + ((byGreen&0xfc) << 3) + (byBlue >> 3);
		else if(m_format == D3DFMT_X1R5G5B5)
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
		else
		{
			wColor = ((byRed&0xf8) << 7) + ((byGreen&0xf8) << 2) + (byBlue >> 3);
			wColor |= 0x8000;
		}
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					*(LPWORD)(pbyDest+i*2) = wColor;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	case D3DFMT_A4R4G4B4:
		wColor = ((byRed&0xf0) << 4) + (byGreen&0xf0) + (byBlue >> 4);
		pbyDest += iDestPitch * iY + iX * 2;
		for(j=0;j<iCountY;j++)
		{
			for(i=0;i<iCountX;i++)
			{
				byAlpha = pbyBitmap[i];
				if(byAlpha)
				{
					byAlpha = ((byAlpha-1) << 2) & 0xf0;
					*(LPWORD)(pbyDest+i*2) = wColor;
					pbyDest[i*2+1] |= byAlpha;
				}
			}
			pbyBitmap += iSrcPitch;
			pbyDest += iDestPitch;
		}
		break;
	default:
		return false;
	}
	return true;
}
#endif
/*
//-----------------------------------------------
//bool DIB::pushImage(LPDIRECT3DTEXTURE9 pTexture)
// ---  動作  ---
// DIBにテクスチャ情報を保存
// ---  引数  ---
// pTexture テクスチャ情報のポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::pushImage(LPDIRECT3DTEXTURE9 pTexture)
{
	INT i,j;

	//DIBの存在をチェック
	if(!isData())
		return false;
	
	//テクスチャフォーマットの取得
	D3DSURFACE_DESC d3dsurfaceDesc;
	pTexture->GetLevelDesc(0,&d3dsurfaceDesc);
	//テクスチャをロック
	RECT rect = {0,0,getWidth(),getHeight()};
	D3DLOCKED_RECT d3dlockRect;
	pTexture->LockRect(0,&d3dlockRect,&rect,D3DLOCK_READONLY);

	//アドレスとピッチを取得
	INT	iPitchDest = getPitch();
	INT iPitchSrc = d3dlockRect.Pitch;
	LPBYTE pDest = (LPBYTE)getImage();
	LPBYTE pSrc = (LPBYTE)d3dlockRect.pBits;

	if(d3dsurfaceDesc.Format == getFormat())
	{
		//テクスチャとDIBのフォーマットが一致
		for(i=0;i<getHeight();i++)
		{
			CopyMemory(pDest,pSrc,iPitchDest);
			pSrc += iPitchSrc;
			pDest += iPitchDest;
		}
	}
	else if(getFormat() == D3DFMT_A8R8G8B8)
	{
		if(d3dsurfaceDesc.Format == D3DFMT_A4R4G4B4)
		{
			//D3DFMT_A8R8G8B8とD3DFMT_A4R4G4B4を変換
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wData = ((LPWORD)pSrc)[j];
					pDest[j*4+3] = (wData&0xf000)>>8;
					pDest[j*4+2] = (wData&0x0f00)>>4;
					pDest[j*4+1] = (wData&0x00f0);
					pDest[j*4+0] = (wData&0x000f)<<4;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
		else if(d3dsurfaceDesc.Format == D3DFMT_A1R5G5B5)
		{
			//D3DFMT_A8R8G8B8とD3DFMT_A4R4G4B4を変換
			static BYTE byData[2] = {0xff,0x00};
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wData = ((LPWORD)pSrc)[j];
					pDest[j*4+3] = byData[(wData&0x8000)==0];
					pDest[j*4+2] = (wData&0x7c00)>>7;
					pDest[j*4+1] = (wData&0x03e0)>>2;
					pDest[j*4+0] = (wData&0x001f)<<3;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
	}
	else if(getFormat() == D3DFMT_X8R8G8B8)
	{
		if(d3dsurfaceDesc.Format == D3DFMT_R5G6B5)
		{
			//D3DFMT_A8R8G8B8とD3DFMT_A1R5G5B5を変換
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wData = ((LPWORD)pSrc)[j];
					pDest[j*4+2] = (wData&0xf800)>>8;
					pDest[j*4+1] = (wData&0x07e0)>>3;
					pDest[j*4+0] = (wData&0x001f)<<3;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
	}
	pTexture->UnlockRect(0);
	return true;
}

//-----------------------------------------------
//bool DIB::popImage(LPDIRECT3DTEXTURE9 pTexture)
// ---  動作  ---
// DIBからテクスチャへイメージを描き戻す
// ---  引数  ---
// pTexture テクスチャ情報のポインタ
// --- 戻り値 ---
// true:成功 false:失敗
//-----------------------------------------------
bool DIB::popImage(LPDIRECT3DTEXTURE9 pTexture)
{
	INT i,j;
	
	//DIBの存在をチェック
	if(!isData())
		return false;
	
	//テクスチャフォーマットの取得
	D3DSURFACE_DESC d3dsurfaceDesc;
	pTexture->GetLevelDesc(0,&d3dsurfaceDesc);
	//テクスチャをロック
	D3DLOCKED_RECT d3dlockRect;
	RECT rect = {0,0,getWidth(),getHeight()};
	if(pTexture->LockRect(0,&d3dlockRect,&rect,D3DLOCK_NOSYSLOCK ) != D3D_OK)
		return false;

	//アドレスとピッチを取得
	INT	iPitchDest = d3dlockRect.Pitch;
	INT iPitchSrc = getPitch();
	LPBYTE pSrc = (LPBYTE)getImage();
	LPBYTE pDest = (LPBYTE)d3dlockRect.pBits;

	if(d3dsurfaceDesc.Format == getFormat())
	{
		//テクスチャとDIBのフォーマットが一致
		for(i=0;i<getHeight();i++)
		{
			CopyMemory(pDest,pSrc,iPitchSrc);
			pSrc += iPitchSrc;
			pDest += iPitchDest;
		}
	}
	else if(getFormat() == D3DFMT_X8R8G8B8)
	{
		if(d3dsurfaceDesc.Format == D3DFMT_R5G6B5)
		{
			//D3DFMT_X8R8G8B8とD3DFMT_R5G6B5を変換
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wRed = ((WORD)pSrc[j*4+2]&0xf8) << 8;
					WORD wGreen = ((WORD)pSrc[j*4+1]&0xfc) << 3;
					WORD wBlue = ((WORD)pSrc[j*4+0]&0xf8) >> 3;
					((LPWORD)pDest)[j] = wRed | wGreen | wBlue;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
	}
	else if(getFormat() == D3DFMT_A8R8G8B8)
	{
		if(d3dsurfaceDesc.Format == D3DFMT_A4R4G4B4)
		{
			//D3DFMT_A8R8G8B8とD3DFMT_A4R4G4B4を変換
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wAlpha = ((WORD)pSrc[j*4+3]&0xf0) << 8;
					WORD wRed = ((WORD)pSrc[j*4+2]&0xf0) << 4;
					WORD wGreen = ((WORD)pSrc[j*4+1]&0xf0);
					WORD wBlue = ((WORD)pSrc[j*4]&0xf0) >> 4;
					((LPWORD)pDest)[j] = wAlpha | wRed | wGreen | wBlue;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
		else if(d3dsurfaceDesc.Format == D3DFMT_A1R5G5B5)
		{
			//D3DFMT_A8R8G8B8とD3DFMT_A1R5G5B5を変換
			for(i=0;i<getHeight();i++)
			{
				for(j=0;j<getWidth();j++)
				{
					WORD wAlpha = (pSrc[j*4+3]!=0) << 15;
					WORD wRed = ((WORD)pSrc[j*4+2]&0xf8) << 7;
					WORD wGreen = ((WORD)pSrc[j*4+1]&0xf8) << 2;
					WORD wBlue = ((WORD)pSrc[j*4]&0xf8) >> 3;
					((LPWORD)pDest)[j] = wAlpha | wRed | wGreen | wBlue;
				}
				pSrc += iPitchSrc;
				pDest += iPitchDest;
			}
		}
	}
	pTexture->UnlockRect(0);
	return true;
}
*/
//-----------------------------------------------
//HBITMAP DIB::getBitmap() const
// ---  動作  ---
// DIBのビットマップハンドルを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// ビットマップハンドル
//-----------------------------------------------
HBITMAP DIB::getBitmap() const
{
	return m_bitmap;
}

//-----------------------------------------------
//LPVOID DIB::getImage() const
// ---  動作  ---
// ビットマップイメージのアドレスを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// ビットマップイメージのアドレス
//-----------------------------------------------
LPVOID DIB::getImage() const
{
	return m_data;
}

//-----------------------------------------------
//INT DIB::getFormat()const
// ---  動作  ---
// DIBに割り当てられているテクスチャフォーマットの番号を返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// フォーマット番号
//-----------------------------------------------
INT DIB::getFormat()const
{
	return m_format;
}

//-----------------------------------------------
//HDC DIB::getDC()const
// ---  動作  ---
// DIBに関連づけられているデバイスコンテキストを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// デバイスコンテキスト
//-----------------------------------------------
HDC DIB::getDC()const
{
	return m_hdc;
}

//-----------------------------------------------
//INT DIB::getPitch()const
// ---  動作  ---
// DIBの一行あたりのサイズを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// 一行のサイズ（byte）
//-----------------------------------------------
INT DIB::getPitch()const
{
	return m_pitch;
}

//-----------------------------------------------
//INT DIB::getWidth()const
// ---  動作  ---
// DIBの幅を返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// DIBの幅
//-----------------------------------------------
INT DIB::getWidth()const
{
	return m_width;
}

//-----------------------------------------------
//INT DIB::getHeight()const
// ---  動作  ---
// DIBの高さを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// DIBの高さ
//-----------------------------------------------
INT DIB::getHeight()const
{
	return m_height;
}

//-----------------------------------------------
//INT DIB::getSize()const
// ---  動作  ---
// DIBイメージの総サイズを返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// DIBイメージのサイズ
//-----------------------------------------------
INT DIB::getSize()const
{
	return m_size;
}

//-----------------------------------------------
//bool DIB::isData()const
// ---  動作  ---
// DIBイメージが確保されているか返す
// ---  引数  ---
// 無し
// --- 戻り値 ---
// true:有り false:無し
//-----------------------------------------------
bool DIB::isData()const
{
	return m_bitmap!=false;
}

bool DIB::save(LPCSTR fileName)
{
	BITMAPV4HEADER bitmapInfo;
	createBitmapHeader(&bitmapInfo,getWidth(),getHeight(),D3DFMT_R8G8B8);
	bitmapInfo.bV4Height = -bitmapInfo.bV4Height-1;
	
	BITMAPFILEHEADER BFHeader;
	ZeroMemory(&BFHeader,sizeof(BITMAPFILEHEADER));
	BFHeader.bfType = 0x4d42;
	BFHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV4HEADER);
	BFHeader.bfSize = BFHeader.bfOffBits+getPitch()*getHeight();
	
	FILE* file = fopen(fileName,"wb");
	if(!file)
		return false;
	fwrite(&BFHeader,sizeof(BITMAPFILEHEADER),1,file);
	fwrite(&bitmapInfo,sizeof(BITMAPV4HEADER),1,file);

	int i;
	LPBYTE pDest = (LPBYTE)getImage() + (getHeight()-1)*getPitch();
	for(i=0;i<getHeight();i++)
	{
		fwrite(pDest,getPitch(),1,file);
		pDest -= getPitch();
	}
	fclose(file);
	return true;
}
/*
bool DIB::savePNG(LPCTSTR fileName)
{
	FILE* file = fopen(fileName,"wb");
	if(!file)
		return false;

	png_structp png_ptr;
	png_infop info_ptr;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		return false;
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);

	png_init_io(png_ptr, file);
	png_set_IHDR(png_ptr, info_ptr, getWidth(), getHeight(), 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

	png_color_8 sig_bit;
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	sig_bit.alpha = 0;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	png_text text_ptr;
	text_ptr.key = "";
	text_ptr.text = "";
	text_ptr.compression = PNG_TEXT_COMPRESSION_NONE;
	//png_set_text(png_ptr, info_ptr, &text_ptr, 1);

	png_write_info(png_ptr, info_ptr);
	png_set_bgr(png_ptr);

	png_bytep *row_pointers;
	row_pointers = new png_bytep[getHeight()];
	INT i;
	for (i=0; i < getHeight(); i++)
		row_pointers[i] = (png_bytep)getImage()+getPitch()*i;

	png_write_image(png_ptr, row_pointers);
	free(row_pointers);
	png_write_end(png_ptr, info_ptr);


	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(file);
	return true;

}
*/

bool DIB::bitBlt(HDC hDC,INT destX,INT destY,INT width,INT height,INT srcX,INT srcY,DWORD rop)
{
	if(width < 0)
		width = getWidth();
	if(height < 0)
		height = getHeight();
	return BitBlt(getDC(),destX,destY,width,height,hDC,srcX,srcY,rop) != false;
}
bool DIB::stretchBlt(HDC hDC,INT destX,INT destY,INT width,INT height,INT srcX,INT srcY,INT srcWidth,INT srcHeight,DWORD rop)
{
	if(width < 0)
		width = getWidth();
	if(height < 0)
		height = getHeight();

	if(srcWidth == -1 || srcHeight == -1)
	{
		HBITMAP hBitmap;
		BITMAP bitmap;
		//HDCからHBITMAPを取得
		hBitmap = (HBITMAP)::GetCurrentObject( hDC , OBJ_BITMAP );
		//HBITMAPからBITMAPを取得
		if(::GetObject( hBitmap , sizeof( BITMAP ) , &bitmap ))
		{
			srcWidth = bitmap.bmWidth;
			srcHeight = bitmap.bmHeight;
		}
		else
		{
			srcWidth = ::GetDeviceCaps( hDC , HORZRES );
			srcHeight = ::GetDeviceCaps( hDC , VERTRES );
		}
	}
	SetStretchBltMode( getDC(), COLORONCOLOR );
	return StretchBlt(getDC(),destX,destY,width,height,hDC,srcX,srcHeight,srcWidth,-srcHeight,rop) != false;
}
DWORD DIB::getPixel(INT x,INT y)
{
	LPDWORD adr = (LPDWORD)((LPBYTE)m_data + y * getPitch() + x * sizeof(DWORD));
	return *adr;
}
void DIB::drawBox(INT x,INT y,INT width,INT height,COLORREF color)
{
	HBRUSH hbrush = CreateSolidBrush(color);
	HBRUSH old = (HBRUSH)SelectObject(m_hdc,hbrush);
	PatBlt(m_hdc,x,y,width,height,PATCOPY);
	SelectObject(m_hdc,old);
	DeleteObject(hbrush);
}
void DIB::drawLine(INT x1,INT y1,INT x2,INT y2,COLORREF color)
{
	HPEN hpen = CreatePen( PS_SOLID, 1, color); 
	HPEN old = (HPEN)SelectObject(m_hdc,hpen);
	MoveToEx(m_hdc,x1,y1,NULL);
	LineTo(m_hdc,x2,y2);
	SelectObject(m_hdc,old);
	DeleteObject(hpen);
}
HRGN DIB::setClip(INT x1,INT y1,INT x2,INT y2)
{
	HRGN hrgn = CreateRectRgn(x1,y1,x2,y2);
	SelectClipRgn(m_hdc , hrgn);
	return hrgn;
}
void DIB::delClip(HRGN hrgn)
{
	SelectClipRgn(m_hdc , NULL);
	DeleteObject(hrgn);
}
