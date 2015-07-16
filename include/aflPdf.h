#include "aflStd.h"
#include <list>
using namespace AFL;

struct IMAGEType
{
	INT sample;
	INT width;
	INT height;
	INT colorType;
};
INT getImageWord(LPBYTE data);
INT getImageDWord(LPBYTE data);
bool getJpegHeader(BinaryStream& bs,IMAGEType* imageType);
bool getPngHeader(BinaryStream& bs,IMAGEType* imageType);


class PDFObject;

struct PDFPoint
{
	FLOAT x;
	FLOAT y;
};
struct PDFRect
{
	FLOAT left;
	FLOAT top;
	FLOAT right;
	FLOAT bottom;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFData
// PDFデータ基本クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFData
{
public:
	virtual bool isType(LPCSTR type) = 0;
	virtual INT getInt(){return 0;}
	virtual FLOAT getFloat(){return 0.0f;}
	virtual LPCSTR getString(){return NULL;}
	virtual PDFObject* getObject(){return NULL;}
	virtual std::list<SP<PDFData> >* getArray(){return NULL;}
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFDataArray
// PDFデータ配列
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class PDFDataArray : public PDFData
{
public:
	void add(SP<PDFData> data)
	{
		m_data.push_back(data);
	}
	INT getCount()const{return (INT)m_data.size();}
protected:
	std::list<SP<PDFData> >* getArray(){return &m_data;}
	bool isType(LPCSTR type)
	{
		return strcmp("ARRAY",type) == 0;
	}
	std::list<SP<PDFData> > m_data;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFDataString
// PDFデータ文字列
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFDataString : public PDFData
{
public:
	PDFDataString(LPCSTR value)
	{
		m_value = value;
	}
protected:
	LPCSTR getString(){return m_value;}
	bool isType(LPCSTR type)
	{
		return strcmp("STRING",type) == 0;
	}
	String m_value;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFDataInt
// PDFデータ整数
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFDataInt : public PDFData
{
public:
	PDFDataInt(INT value)
	{
		m_value = value;
	}
protected:
	virtual INT getInt(){return m_value;}
	bool isType(LPCSTR type)
	{
		return strcmp("INT",type) == 0;
	}
	INT m_value;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFDataFloat
// PDFデータ少数
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFDataFloat : public PDFData
{
public:
	PDFDataFloat(FLOAT value)
	{
		m_value = value;
	}
protected:
	virtual FLOAT getFloat(){return m_value;}
	bool isType(LPCSTR type)
	{
		return strcmp("FLOAT",type) == 0;
	}
	FLOAT m_value;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFDataObject
// PDFデータオブジェクト参照
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFDataObject : public PDFData
{
public:
	PDFDataObject(PDFObject* value)
	{
		m_value = value;
	}
protected:
	virtual PDFObject* getObject()
	{
		return m_value;
	}
	bool isType(LPCSTR type)
	{
		return strcmp("OBJECT",type) == 0;
	}
	PDFObject* m_value;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFObject
// PDFオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFObject
{
public:
	PDFObject();
	bool isObjectList()const;
	void setObjectList(bool value);
	void addChild(SP<PDFObject> object);
	void set(LPCSTR name,LPCSTR data);
	void set(LPCSTR name,INT data);
	void set(LPCSTR name,PDFObject* data);
	void set(LPCSTR name,PDFDataArray* data);
	PDFDataArray* getArray(LPCSTR name);
	void add(LPCSTR name,INT value);
	void add(LPCSTR name,FLOAT value);
	void add(LPCSTR name,LPCSTR value);
	void add(LPCSTR name,PDFObject* value);
	void write(BinaryStream& dest,PDFData* data);
	void write(BinaryStream& dest,std::list<INT>& point);
	void setNumber(INT& value);
	INT getNumber()const;
	void call();
	void setStream(BinaryStream& value);
	void del(LPCSTR name);
	BinaryStream* getStream(){return &m_streamBuff;}
protected:
	virtual void callback(){}
	std::list<SP<PDFObject> > m_childObject;
	std::map<String,SP<PDFData> > m_data;
	INT m_number;
	BinaryStream m_streamBuff;
	bool m_objectList;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFImageObject
// PDFイメージオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFImageObject : public PDFObject
{
	PDFObject* m_desc;
public:
	PDFImageObject();
	bool loadImage(BinaryStream& bs);
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFFontObject
// PDFフォントオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFFontObject : public PDFObject
{
public:
	PDFFontObject();

protected:

	PDFObject* m_desc;
	String m_fontName;
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFPageObject
// PDFページオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFPagesObject;
class PDFPageObject : public PDFObject
{
public:
	PDFPageObject(PDFPagesObject* parent);
	PDFObject* getContents(){return m_contents;}
protected:
	PDFPagesObject* m_parent;
	PDFObject* m_contents;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFPagesObject
// PDFページ統合オブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFPagesObject : public PDFObject
{
public:
	PDFPagesObject();
	PDFObject* getResource();
	PDFFontObject* addFont(LPCSTR name);
	PDFImageObject* addImage(LPCSTR name);
	PDFPageObject* addPage();
	void setPageSize(FLOAT width,FLOAT height)
	{
		del("MediaBox");
		add("MediaBox",0);
		add("MediaBox",0);
		add("MediaBox",width);
		add("MediaBox",height);
	}
	INT getPageCount()const{return (INT)m_pages.size();}
	void setProc();
protected:
	void callback();
	std::list<PDFObject*> m_pages;
	std::map<String,PDFFontObject*> m_fonts;
	std::map<String,PDFImageObject*> m_images;
	PDFDataArray* m_pageArray;
	PDFObject* m_resource;

	FLOAT m_pageSize[2];

};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFPageData
// PDFページデータオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class PDFPageData
{
public:
	PDFPageData()
	{
	}
	LPCSTR getData()const{return m_data;}
	void add(LPCSTR src)
	{
		m_data += src;
	}
protected:
	String m_data;

};

class PDFRootObject : public PDFObject
{
public:
	PDFRootObject()
	{
		m_pagesObject = new PDFPagesObject();
		addChild(m_pagesObject);

		set("Type","Catalog");
		set("Pages",m_pagesObject);
	}
	PDFPagesObject* getPages()const{return m_pagesObject;}
protected:
	void callback()
	{

	}
	PDFPagesObject* m_pagesObject;
};
struct LinkData
{
//	INT page;
	FLOAT x,y,width,height;
	String url;
};
struct Bookmark
{
	INT page;
	FLOAT x,y;
	String name;
	std::list<INT> child;
	bool opened;
	PDFObject* object;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// StringData
// PDF化前ページデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class StringData
{
public:
	StringData();
	void setFontName(LPCWSTR fontName);
	void setFontSize(FLOAT size);
	void setPageSize(FLOAT width,FLOAT height)
	{
		m_pageSize.x = width;
		m_pageSize.y = height;
	}
	FLOAT getPageWidth()const{return m_pageSize.x;}
	FLOAT getPageHeight()const{return m_pageSize.y;}
	LPCSTR getData();
	void output();
	FLOAT getLineHeight(INT index);
	void setPoint(FLOAT x,FLOAT y)
	{
		//m_data.appendf("X<%f>Y<%f>",x,y);
		m_point.x = x;
		addLine();
		m_point.y = y;

	}
	void addString(LPCWSTR text,LPCSTR url=NULL);
	void addString(LPCSTR text,LPCSTR url=NULL);
	void setString(FLOAT x,FLOAT y,LPCSTR text,LPCSTR url=NULL);
	void setString(FLOAT x,FLOAT y,LPCWSTR text,LPCSTR url=NULL);
	void addPage();
	void drawBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height);
	void drawLine(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2);
	void loadImage(LPCSTR fileName,FLOAT x,FLOAT y,FLOAT width,FLOAT height);
	void loadImage(LPCSTR fileName);
	void setMarginLeft(FLOAT value)
	{
		m_pageMargin.left = value;
		addLine();
		m_point.x = value;
	}
	void setMargin(FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2)
	{
		m_pageMargin.left = x1;
		m_pageMargin.right = x2;
		m_pageMargin.top = y1;
		m_pageMargin.bottom = x2;
	}
	void addChar(WCHAR data);
	INT setBookmark(LPCSTR name,INT parent=-1,bool opened=true)
	{
		Bookmark bookmark;
		bookmark.page = m_pageNow;
		bookmark.x = m_point.x;
		bookmark.y = m_point.y;
		bookmark.name = name;
		bookmark.opened = opened;
		m_bookmark.push_back(bookmark);
		m_bookmarkNumber.push_back(&m_bookmark.back());

		INT index = (INT)m_bookmark.size()-1;

		if(parent >= 0 && parent < (INT)m_bookmarkNumber.size())
		{
			m_bookmarkNumber[parent]->child.push_back(index);
		}
		m_data.appendf("O<%d>",index);
		return index;
	}
	Bookmark* getBookmark(INT index)
	{
		if(index < (INT)m_bookmarkNumber.size())
			return m_bookmarkNumber[index];
		return NULL;
	}
	INT getBookmarkCount()const{return (INT)m_bookmarkNumber.size();}
	std::map<String,BinaryStream>& getImageFiles()
	{
		return m_imageFiles;
	}
protected:

	void addLine();
	void convertText(String& dest,LPCWSTR text);

	String m_data;
	WString m_buff;
	std::list<FLOAT> m_lineHeight;
	FLOAT m_lineStartX;

	PDFPoint m_pageSize;
	PDFRect m_pageMargin;
	PDFPoint m_point;
	INT m_pageNow;
	INT m_lineNow;
	String m_fontName;
	FLOAT m_fontSize;
	FLOAT m_fontSizeLine;
	FLOAT m_fontHeight;
	std::list<Bookmark> m_bookmark;
	std::vector<Bookmark*> m_bookmarkNumber;
	std::map<String,BinaryStream> m_imageFiles;

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pdf
// PDF変換用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Pdf
{
public:
	Pdf();
	bool save(LPCSTR fileName,StringData* sd);
	bool save(LPCWSTR fileName, StringData* sd);
	bool loadImage(BinaryStream* s, FLOAT x, FLOAT y, FLOAT width, FLOAT height, LPCSTR fileName, BinaryStream* bs);
	void drawBox(BinaryStream* s,FLOAT x,FLOAT y,FLOAT width,FLOAT height);
	void drawLine(BinaryStream* s,FLOAT x,FLOAT y,FLOAT x2,FLOAT y2);
	void convert(String& dest,LPCWSTR text);
	void drawText(BinaryStream* s,FLOAT x,FLOAT y,LPCSTR text,FLOAT size);
	bool output(BinaryStream& dest,StringData* sd);
	FLOAT getDataFloat(LPCSTR src,INT& index);
	INT getDataInt(LPCSTR src,INT& index);
	void getData(String& dest,LPCSTR src,INT& index);
	void setStringData(PDFPagesObject* pages,StringData* sd);
	void getTextSize(PDFPoint& point,LPCWSTR text) const;
	FLOAT getTextWidth(LPCSTR text,FLOAT size) const;

protected:
	void setOutline(StringData* sd,INT index,PDFObject* parent,PDFRootObject* rootObject);

	INT m_pageNow;
	FLOAT m_pointNow;
	String m_fontName;
	FLOAT m_fontSize;
	FLOAT m_pageSize[2];
	PDFRect m_pageMargin;
	std::map<String,BinaryStream> m_imageFiles;
};
