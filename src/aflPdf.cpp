#include <map>
#include <list>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "aflStd.h"
#include "aflPdf.h"

using namespace AFL;



INT getImageWord(LPBYTE data)
{
	return (data[0] << 8) + data[1];
}
INT getImageDWord(LPBYTE data)
{
	return (data[0] << 24) +(data[1] << 16) +(data[2] << 8)  + data[3];
}
bool getJpegHeader(BinaryStream& bs,IMAGEType* imageType)
{
	bs.setSeek(0,SEEK_SET);
	bool flag = false;
	INT code;

	code = bs.getImageWord();
	if(code == 0xffd8)
	{
		while(code = bs.getImageWord())
		{
			if((code & 0xff00)!=0xff00)
				break;
			INT length = bs.getImageWord()-2;
			LPBYTE data = (LPBYTE)bs.getData();
			bs.read(data,length);
			if(code == 0xFFC0)
			{
				imageType->sample = data[0];
				imageType->height = (data[1] << 8) + data[2];
				imageType->width = (data[3] << 8) + data[4];
				imageType->colorType = data[5];
				flag = true;
				break;
			}
		}
	}
	return flag;
}
bool getPngHeader(BinaryStream& bs,IMAGEType* imageType)
{
	bs.setSeek(0,SEEK_SET);
	static BYTE pngSignature[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};

	bool flag = false;
	if(memcmp(pngSignature,bs.getData(),8)==0)
	{
		bs.setSeek(8,SEEK_CUR);
		INT length;
		while(length = bs.getImageDWord())
		{
			DWORD code = bs.getImageDWord();
			if(code == 0)
				break;

			if(code == 0x49484452)
			{
				BYTE* buff = (LPBYTE)bs.getData();

				imageType->width = (buff[0] <<24) + (buff[1] <<16) + (buff[2] <<8) + buff[3];
				imageType->height = (buff[4] <<24) + (buff[5] <<16) + (buff[6] <<8) + buff[7];
				imageType->sample = buff[8];
				imageType->colorType = buff[9];
				
				if(imageType->colorType <= 6)
					flag = true;
				else
					flag = false;
				break;
			}
		}
	}
	return flag;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFObject
// PDFオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PDFObject::PDFObject()
{
	m_number = 0;
	m_objectList = true;
}
bool PDFObject::isObjectList()const
{
	return m_objectList;
}
void PDFObject::setObjectList(bool value)
{
	m_objectList = value;
}
void PDFObject::addChild(SP<PDFObject> object)
{
	m_childObject.push_back(object);
}
void PDFObject::set(LPCSTR name,LPCSTR data)
{
	m_data[name] = new PDFDataString(data);
}
void PDFObject::set(LPCSTR name,INT data)
{
	m_data[name] = new PDFDataInt(data);
}
void PDFObject::set(LPCSTR name,PDFObject* data)
{
	m_data[name] = new PDFDataObject(data);
}

void PDFObject::set(LPCSTR name,PDFDataArray* data)
{
	m_data[name] = data;
}

void PDFObject::del(LPCSTR name)
{
	m_data.erase(name);
}
PDFDataArray* PDFObject::getArray(LPCSTR name)
{
	PDFDataArray* dataArray;
	std::map<String,SP<PDFData> >::iterator it = m_data.find(name);
	if(it == m_data.end())
	{
		dataArray = new PDFDataArray();
		m_data[name] = dataArray;
	}
	else
	{
		if(it->second->isType("ARRAY"))
		{
			dataArray = (PDFDataArray*)m_data[name].get();
		}
		else
		{
			dataArray = new PDFDataArray();
			m_data[name] = dataArray;
		}
	}
	return dataArray;
}

void PDFObject::add(LPCSTR name,FLOAT value)
{
	PDFDataArray* dataArray = getArray(name);
	dataArray->add(new PDFDataFloat(value));
}
void PDFObject::add(LPCSTR name,INT value)
{
	PDFDataArray* dataArray = getArray(name);
	dataArray->add(new PDFDataInt(value));
}
void PDFObject::add(LPCSTR name,LPCSTR value)
{
	PDFDataArray* dataArray = getArray(name);
	dataArray->add(new PDFDataString(value));
}

void PDFObject::add(LPCSTR name,PDFObject* value)
{
	PDFDataArray* dataArray = getArray(name);
	dataArray->add(new PDFDataObject(value));
}

void PDFObject::write(BinaryStream& dest,PDFData* data)
{
	if(data->isType("STRING"))
	{
		if(data->getString()[0] == '<' || data->getString()[0] == '('|| data->getString()[0] == '[')
			dest.printf(" %s",data->getString());
		else
			dest.printf(" /%s",data->getString());
	}
	else if(data->isType("INT"))
		dest.printf(" %d",data->getInt());
	else if(data->isType("FLOAT"))
		dest.printf(" %g",data->getFloat());
	if(data->isType("OBJECT"))
	{
		if(data->getObject()->isObjectList())
			dest.printf(" %d 0 R",data->getObject()->getNumber());
		else
		{
			std::map<String,SP<PDFData> >::iterator it;
			dest.write(" <<");
			PDFObject* object = data->getObject();
			foreach(it,object->m_data)
			{
				BinaryStream data;
				write(data,it->second.get());
				dest.printf(" /%s",it->first.c_str());
				dest.write(data.getData(),data.getSize());
			}

			dest.write(" >>");
		}
	}
	else if(data->isType("ARRAY"))
	{
		dest.write(" [ ");
		std::list<SP<PDFData> >::iterator it;
		foreach(it,(*data->getArray()))
		{
			write(dest,it->get());
		}
		dest.write(" ]");
	}

}
void PDFObject::write(BinaryStream& dest,std::list<INT>& point)
{
	if(m_objectList)
	{
		m_streamBuff.setSeek(0,SEEK_SET);

		point.push_back((INT)(INT)dest.getSize());
		dest.printf("%d 0 obj\n",m_number);
		dest.write("<<\n");
	
		std::map<String,SP<PDFData> >::iterator it;
		foreach(it,m_data)
		{
			BinaryStream data;
			write(data,it->second.get());
			dest.printf(" /%s",it->first.c_str());
			dest.write(data.getData(),data.getSize());
			dest.write("\n");
		}

		if(m_streamBuff.getSize())
		{
			dest.printf(" /Length %d\n",m_streamBuff.getSize());
		}
		dest.write(">>\n");

		if(m_streamBuff.getSize())
		{
			dest.write("stream\n");
			dest.write(m_streamBuff.getData(),m_streamBuff.getSize());
			dest.write("\nendstream\n");
		}
		dest.printf("endobj\n\n");
	}

	std::list<SP<PDFObject> >::iterator itObject;
	for(itObject=m_childObject.begin();itObject!=m_childObject.end();++itObject)
	{
		(*itObject)->write(dest,point);
	}
}
void PDFObject::setNumber(INT& value)
{
	if(m_objectList)
	{
		m_number = value;
		++value;
	}
	std::list<SP<PDFObject> >::iterator it;
	for(it=m_childObject.begin();it!=m_childObject.end();++it)
	{
		(*it)->setNumber(value);
	}
}
INT PDFObject::getNumber()const
{
	return m_number;
}
void PDFObject::call()
{
	callback();

	std::list<SP<PDFObject> >::iterator it;
	for(it=m_childObject.begin();it!=m_childObject.end();++it)
	{
		(*it)->call();
	}
}
void PDFObject::setStream(BinaryStream& value)
{
	m_streamBuff = value;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFFontObject
// PDFフォントオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PDFFontObject::PDFFontObject()
{
	m_fontName = "KozMinStd-Regular";
	//m_fontName = "#82l#82r#83S#83V#83b#83N";

	PDFObject* desc = new PDFObject();
	addChild(desc);
	m_desc = desc;
	
	PDFObject* font1 = new PDFObject();
	addChild(font1);

	font1->set("Type","Font");
	font1->set("Subtype","CIDFontType0");
	font1->set("BaseFont",m_fontName);

	font1->set("CIDSystemInfo","<< /Registry (Adobe) /Ordering (Japan1) /Supplement 3 >>");
	font1->set("FontDescriptor",desc);
	font1->add("W",1);
	font1->add("W",255);
	font1->add("W",500);

	desc->set("Type","FontDescriptor");
	desc->set("FontName",m_fontName);
	desc->set("Flags",6);
/*	desc->add("FontBBox",-437);
	desc->add("FontBBox",-340);
	desc->add("FontBBox",1144);
	desc->add("FontBBox",1317);
*/
	desc->add("FontBBox",0);
	desc->add("FontBBox",0);
	desc->add("FontBBox",0);
	desc->add("FontBBox",0);


	desc->set("StemV",80);
	desc->set("StemH",80);
	desc->set("ItalicAngle",0);
	desc->set("CapHeight",500); //明朝
	desc->set("XHeight",501);
	desc->set("Ascent",897);	//明朝
	desc->set("MaxWidth",1000);
	desc->set("AvgWidth",500);
	desc->set("MissingWidth",500);
	desc->set("Descent",-147);

	set("Style","<< /Panose <0805020B0609000000000000> >>");

	set("Type","Font"); 
	set("BaseFont",m_fontName);
	set("Subtype","Type0"); 
	set("Encoding","UniJIS-UTF16-H"); 
	add("DescendantFonts",font1); 
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFImageObject
// PDFイメージオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PDFImageObject::PDFImageObject()
{
	set("Type","XObject");
	set("Subtype","Image");
}
bool PDFImageObject::loadImage(BinaryStream& bsSrc)
{
	INT kind;
	IMAGEType imageType;
	if(getJpegHeader(bsSrc,&imageType))
		kind = 0;
	else if(getPngHeader(bsSrc,&imageType))
		kind = 1;
	else
		return false;
	
	
	set("Width",imageType.width);
	set("Height",imageType.height);
	set("BitsPerComponent",imageType.sample);

	switch(imageType.colorType)
	{
	case 1:
		set("ColorSpace","DeviceGray");
		break;
	case 2:
	case 3:
		set("ColorSpace","DeviceRGB");
		break;
	case 4:
		set("ColorSpace","DeviceCMYK");
		set("Decode","[1 0 1 0 1 0 1 0]");
		break;
	default:
		return false;
	}
	if(kind == 0)
	{
		set("Filter","DCTDecode");
		setStream(bsSrc);
	}
	else if(kind == 1)
	{
		set("Filter","FlateDecode");
		String work;
		work.printf("<< /Predictor 15 /Colors %d  /Columns %d>>",
			imageType.colorType==2?3:1,imageType.width);
		set("DecodeParms",work);

		static BYTE pngSignature[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};

		bool flag = false;
		bsSrc.setSeek(0,SEEK_SET);
		BYTE buff[8];
		if(bsSrc.read(buff,8) && memcmp(pngSignature,buff,8)==0)
		{
			INT length;
			BinaryStream bs;
			while(length = bsSrc.getImageDWord())
			{
				CHAR code[5];
				code[4] = 0;
				if(bsSrc.read(code,4)==0)
					break;

				if(strcmp(code,"PLTE")==0 ||
					strcmp(code,"tRNS")==0)
				{
					bsSrc.setSeek(length+4,SEEK_CUR);
				}
				else if(strcmp(code,"IEND")==0)
				{
					break;
				}
				else if(strcmp(code,"IDAT")==0)
				{
					BYTE* buff = new BYTE[length];
					if(bsSrc.read(buff,length) == 0)
					{
						delete[] buff;
						break;
					}
					bs.write(buff,length);
					delete[] buff;
					flag = true;
					bsSrc.setSeek(4,SEEK_CUR);
					break;
				}
				else
					bsSrc.setSeek(length+4,SEEK_CUR);

			}
			setStream(bs);
		}
	}

	return true;
}

class PDFPagesObject;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFPageObject
// PDFページオブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PDFPageObject::PDFPageObject(PDFPagesObject* parent)
{
	set("Type","Page");
	m_parent = parent;
	set("Parent",(PDFObject*)m_parent);

	m_contents = new PDFObject;
	addChild(m_contents);
	set("Contents",m_contents);
	set("Resources",parent->getResource());
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// PDFPagesObject
// PDFページ統合オブジェクト
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

PDFPagesObject::PDFPagesObject()
{
	set("Type","Pages");


	m_resource = new PDFObject();
	addChild(m_resource);

	addFont("F1");
	m_pageArray = new PDFDataArray;
	set("Kids",m_pageArray);



	m_resource->add("ProcSet","PDF"); 
	m_resource->add("ProcSet","Text"); 
	m_resource->add("ProcSet","ImageB"); 
	m_resource->add("ProcSet","ImageC"); 
	m_resource->add("ProcSet","ImageI"); 

}
void PDFPagesObject::setProc()
{
	PDFObject* procSetFont = new PDFObject();
	procSetFont->setObjectList(false);
	addChild(procSetFont);
	m_resource->set("Font",procSetFont); 
	std::map<String,PDFFontObject*>::iterator itFont;
	foreach(itFont,m_fonts)
	{
		procSetFont->set(itFont->first,itFont->second);
	}

	PDFObject* procSetImage = new PDFObject();
	procSetImage->setObjectList(false);
	addChild(procSetImage);
	m_resource->set("XObject",procSetImage); 
	std::map<String,PDFImageObject*>::iterator itImage;
	foreach(itImage,m_images)
	{
		procSetImage->set(itImage->first,itImage->second);
	}
}
PDFObject* PDFPagesObject::getResource()
{
	return m_resource;
}
PDFFontObject* PDFPagesObject::addFont(LPCSTR name)
{
	PDFFontObject* font = new PDFFontObject();
	addChild(font);
	m_fonts[name] = font;
	return font;
}
PDFImageObject* PDFPagesObject::addImage(LPCSTR name)
{
	PDFImageObject* object = new PDFImageObject();
	addChild(object);
	m_images[name] = object;
	return object;
}
PDFPageObject* PDFPagesObject::addPage()
{
	PDFPageObject* page = new PDFPageObject(this);
	addChild(page);
	m_pages.push_back(page);
	m_pageArray->add(new PDFDataObject(page));
	return page;
}

void PDFPagesObject::callback()
{
	set("Count",(INT)m_pages.size());
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// StringData
// PDF化前ページデータ保存用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
StringData::StringData()
{
	m_pageSize.x = 595.27559f;
	m_pageSize.y = 841.88976f;
	m_pageMargin.left = 25.0f;
	m_pageMargin.right = 25.0f;
	m_pageMargin.top = 25.0f;
	m_pageMargin.bottom = 25.0f;
	m_lineStartX = m_pageMargin.left;

	m_fontHeight = 1.2f;
	m_fontName = "F1";
	m_fontSize = 24.0f;
	m_fontSizeLine = m_fontSize;
	m_pageNow = 0;
	m_lineNow = 1;
	m_point.x = m_pageMargin.left;
	m_point.y = m_pageMargin.top;
}

void StringData::setFontName(LPCWSTR fontName)
{
	String work;
	convertText(work,fontName);
	m_data.appendf("F<%s>",work.c_str());
}
void StringData::setFontSize(FLOAT size)
{
	addLine();
	m_fontSize= size;
	m_data.appendf("S<%g>",size);
}
LPCSTR StringData::getData()
{
	addLine();
	return m_data.c_str();
}
void StringData::output()
{
	addLine();
	puts(m_data.c_str());
}
FLOAT StringData::getLineHeight(INT index)
{
	INT i;
	std::list<FLOAT>::iterator it;
	for(i=0,it=m_lineHeight.begin();i<=index && it!=m_lineHeight.end();i++,it++)
	{
		if(i==index)
			return *it;
	}
	return 0;
}
void StringData::addString(LPCWSTR text,LPCSTR url)
{
	INT i;
	if(url)
		m_data.appendf("A<%s>",url);
	for(i=0;text[i];i++)
		addChar(text[i]);
	if(url)
		m_data.appendf("E");
}
void StringData::addString(LPCSTR text,LPCSTR url)
{
	INT i;

	if(url)
		m_data.appendf("A<%s>",url);

	std::wstring dest;
	UTF8toUCS2(dest,text);
	for(i=0;dest[i];i++)
		addChar(dest[i]);
	if(url)
		m_data.appendf("E");
}
void StringData::setString(FLOAT x,FLOAT y,LPCSTR text,LPCSTR url)
{
	setPoint(x,y);
	INT i;

	if(url)
		m_data.appendf("A<%s>",url);

	std::wstring dest;
	UTF8toUCS2(dest,text);
	for(i=0;dest.c_str()[i];i++)
		addChar(dest[i]);
	if(url)
		m_data.appendf("E");
	addLine();
}
void StringData::setString(FLOAT x,FLOAT y,LPCWSTR text,LPCSTR url)
{
	setPoint(x,y);
	INT i;

	if(url)
		m_data.appendf("A<%s>",url);

	for(i=0;text[i];i++)
		addChar(text[i]);
	if(url)
		m_data.appendf("E");
	addLine();
}
void StringData::addPage()
{
	m_point.x = m_pageMargin.left;
	addLine();
	m_data.appendf("P");
	++m_pageNow;
	m_point.y = m_pageMargin.top;
}
void StringData::drawBox(FLOAT x,FLOAT y,FLOAT width,FLOAT height)
{
	m_data.appendf("X<%g>Y<%g>W<%g>H<%g>B",x,y,width,height);

}
void StringData::drawLine(FLOAT x,FLOAT y,FLOAT x2,FLOAT y2)
{
	m_data.appendf("X<%g>Y<%g>W<%g>H<%g>N",x,y,x2,y2);

}
void StringData::loadImage(LPCSTR fileName,FLOAT x,FLOAT y,FLOAT width,FLOAT height)
{
	BinaryStream bs;
	if(!bs.load(fileName))
		return;
	m_imageFiles[fileName] = bs;
	m_data.appendf("X<%g>Y<%g>W<%g>H<%g>I<%s>",x,y,width,height,fileName);

}
void StringData::loadImage(LPCSTR fileName)
{
	BinaryStream bs;
	if(!bs.load(fileName))
		return;
	IMAGEType imageType;
	if(!getJpegHeader(bs,&imageType))
		if(!getPngHeader(bs,&imageType))
			return;

	FLOAT width = (FLOAT)imageType.width;
	FLOAT height = (FLOAT)imageType.height;
	FLOAT heightLimit = m_pageSize.y - m_pageMargin.bottom;
	
	m_point.y += m_fontSizeLine;
//	addLine();
	if(m_point.y + height > heightLimit)
		addPage();
	m_data.appendf("X<%g>Y<%g>W<%g>H<%g>I<%s>",m_point.x,m_point.y,width,height,fileName);
	m_point.y += height;
	addLine();
	m_imageFiles[fileName] = bs;
}

void StringData::addLine()
{
	m_lineHeight.push_back(m_fontSizeLine);


	if(m_buff.size())
	{
		m_data.appendf("X<%g>Y<%g>L<%d>",m_lineStartX,m_point.y,m_lineNow);

		String work;
		convertText(work,m_buff);
		m_data.appendf("T<%s>",work.c_str());
		m_buff.clear();
	}
	if(m_point.x < m_pageMargin.left)
		m_point.x = m_pageMargin.left;
	m_lineStartX = m_point.x;
	m_fontSizeLine = m_fontSize;
	m_lineNow++;
}
void StringData::addChar(WCHAR data)
{
	FLOAT size;
	FLOAT widthLimit;
	FLOAT widthMax = 0;

	FLOAT heightLimit = m_pageSize.y - m_pageMargin.bottom;

	if(m_fontSize > m_fontSizeLine)
		m_fontSizeLine = m_fontSize;

	if(m_point.y + m_fontSizeLine > heightLimit)
		addPage();

	widthLimit = m_pageSize.x - m_pageMargin.right;
	if(data == '\n')
	{
		m_point.x = m_pageMargin.left;
		addLine();
		m_point.y += m_fontSizeLine*m_fontHeight;

		size = 0;

	}
	else
	{
		if(data < 0x100)
			size = m_fontSize / 2;
		else
			size = m_fontSize;


		if(m_point.x + size > widthLimit)
		{	
			m_point.x = m_pageMargin.left;
			addLine();
			m_point.y += m_fontSizeLine*m_fontHeight;

		}
		if(data == '\t')
			m_point.x += size*4;
		else
			m_point.x += size;
		m_buff += data;
	}
}

void StringData::convertText(String& dest,LPCWSTR text)
{
	INT i;
	for(i=0;text[i];i++)
	{
		dest.appendf("%04X",text[i]);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pdf
// PDF変換用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Pdf::Pdf()
{
	m_pageNow = 0;
	m_pageSize[0] = 595;
	m_pageSize[1] = 842;
	m_pageMargin.left = 10.0f;
	m_pageMargin.right = 10.0f;
	m_pageMargin.top = 10.0f;
	m_pageMargin.bottom = 10.0f;

	m_fontName = "F1";
	m_fontSize = 24.0f;
}
bool Pdf::save(LPCSTR fileName,StringData* sd)
{
	BinaryStream stream;

	output(stream,sd);

	FILE* file = fopen(fileName,"wb");
	if(!file)
		return false;
	fwrite(stream.getData(),stream.getSize(),1,file);
	fclose(file);
	return true;
}
bool Pdf::save(LPCWSTR fileName, StringData* sd)
{
	BinaryStream stream;

	output(stream, sd);

	FILE* file = _wfopen(fileName, L"wb");
	if (!file)
		return false;
	fwrite(stream.getData(), stream.getSize(), 1, file);
	fclose(file);
	return true;
}
bool Pdf::loadImage(BinaryStream* s,FLOAT x,FLOAT y,FLOAT width,FLOAT height,LPCSTR fileName,BinaryStream* bs)
{
	IMAGEType imageType;
	if(!getJpegHeader(*bs,&imageType))
		if(!getPngHeader(*bs,&imageType))
			return false;

	if(width == 0)
		width = (FLOAT)imageType.width;
	if(height == 0)
		height = (FLOAT)imageType.height;
	int num;
	auto it=m_imageFiles.begin();
	for(num=0;it->first!=fileName;num++,++it);

	s->printf("q %g 0 0 %g %g %g cm /I%d Do Q\n",
		width,height,x,m_pageSize[1]-y-height,num);
	return true;
}

void Pdf::drawBox(BinaryStream* s,FLOAT x,FLOAT y,FLOAT width,FLOAT height)
{
	y = m_pageSize[1]-y-height-1;
	String work;
	work.printf("q .5 w 1 0 0 1 %g %g cm 0 0 0.5 rg 0 0 %g %g re S Q\n",
		x,y,width,height);
	s->write(work);
}
void Pdf::drawLine(BinaryStream* s,FLOAT x1,FLOAT y1,FLOAT x2,FLOAT y2)
{
	FLOAT py1 = m_pageSize[1]-y1-1;
	FLOAT py2 = m_pageSize[1]-y2-1;
	String work;
	work.printf("q .5 w 0 0 0.5 rg %g %g m %g %g l S Q\n",
		x1,py1,x2,py2);
	s->write(work);
}

void Pdf::convert(String& dest,LPCWSTR text)
{
	INT i;
	for(i=0;text[i];i++)
	{
		dest.appendf("%04x",text[i]);
	}
}
FLOAT Pdf::getTextWidth(LPCSTR text,FLOAT size) const
{
	INT count=0;
	INT code;
	while(*text)
	{
		sscanf(text,"%04x",&code);
		if(code < 0x100)
			count++;
		else
			count+=2;
		text+=4;
	}
	return size*count/2;
}

void Pdf::drawText(BinaryStream* s,FLOAT x,FLOAT y,LPCSTR text,FLOAT size)
{
	y = m_pageSize[1]-y;
	
	String work;
	work.printf("BT /%s %g Tf %g %g Td <%s> Tj ET\n",
		m_fontName.c_str(),size,
		x,y,text);
	s->write(work);

}
bool Pdf::output(BinaryStream& dest,StringData* sd)
{
	INT count;
	m_imageFiles = sd->getImageFiles();
	PDFRootObject rootObject;

	PDFObject* info = new PDFObject;
	info->set("CreationDate","(D:19991115)");
	info->set("Title","(PDF)");
	info->set("Author","(Mofo)");
	rootObject.addChild(info);

	m_pageSize[0] = sd->getPageWidth();
	m_pageSize[1] = sd->getPageHeight();

	PDFPagesObject* pages = rootObject.getPages();
	pages->setPageSize(m_pageSize[0],m_pageSize[1]);
	
	//記述データの展開
	setStringData(pages,sd);

	//イメージデータを読み込む
	INT numImage = 0;
	std::map<String,BinaryStream>::iterator itImage;
	foreach(itImage,m_imageFiles)
	{
		String name;
		name.printf("I%d",numImage++);
		PDFImageObject* image = pages->addImage(name);
		image->loadImage(itImage->second);
	}

	//アウトラインの作成
	if(sd->getBookmarkCount())
	{
		PDFObject* outlineObject = new PDFObject;
		PDFObject* outline = sd->getBookmark(0)->object;
		setOutline(sd,0,outlineObject,&rootObject);

		outlineObject->set("Type","Outlines");
		outlineObject->set("First",outline);
		outlineObject->set("Last",outline);

		rootObject.set("Outlines",outlineObject);
		rootObject.addChild(outlineObject);
		rootObject.set("PageMode","UseOutlines");
	}


	INT i=1;
	rootObject.setNumber(i);
	rootObject.call();
	pages->setProc();


	std::list<INT> pointer;
	dest.write("%PDF-1.4\n\n");
	rootObject.write(dest,pointer);

	String work;

	count = (INT)pointer.size();
	INT pointXRef = (INT)dest.getSize();
	work.printf("xref\n0 %d\n%010d 65535 f \n",count+1,0);

	std::list<INT>::iterator it;
	for(it=pointer.begin();it!=pointer.end();it++)
	{
		work.appendf("%010d 00000 n \n",*it);
	}


	work.appendf("trailer\n<<\n /Root %d 0 R\n /Size %d\n /Info %d 0 R\n>>\nstartxref\n",
		rootObject.getNumber(),
		count+1,info->getNumber());
	work.appendf("%d\n%%%%EOF\n",pointXRef);
	dest.write(work.c_str());

	return true;
}
void Pdf::setOutline(StringData* sd,INT index,PDFObject* parent,PDFRootObject* rootObject)
{
	Bookmark* bookmark = sd->getBookmark(index);
	rootObject->addChild(bookmark->object);
	bookmark->object->set("Parent",parent);
	
	if(bookmark->child.size())
	{
		bookmark->object->set("First",sd->getBookmark(bookmark->child.front())->object);
		bookmark->object->set("Last",sd->getBookmark(bookmark->child.back())->object);

		PDFObject* br = NULL;
		std::list<INT>::iterator it;
		foreach(it,bookmark->child)
		{
			if(br)
			{
				br->set("Next",sd->getBookmark(*it)->object);
				sd->getBookmark(*it)->object->set("Prev",br);
			}
			setOutline(sd,*it,bookmark->object,rootObject);
			br = sd->getBookmark(*it)->object;
		}
	}

}
FLOAT Pdf::getDataFloat(LPCSTR src,INT& index)
{
	String work;
	getData(work,src,index);
	return (FLOAT)atof(work);
}
INT Pdf::getDataInt(LPCSTR src,INT& index)
{
	String work;
	getData(work,src,index);
	return atoi(work);
}
void Pdf::getData(String& dest,LPCSTR src,INT& index)
{
	if(src[index] != '<')
		return;
	for(index++;src[index] != '>' && src[index];index++)
		dest += src[index];
}
void Pdf::setStringData(PDFPagesObject* pages,StringData* sd)
{
	INT i;
	INT index;
	LPCSTR src = sd->getData();
	FLOAT x,y;
	FLOAT height;
	FLOAT width;
	FLOAT size;
	String url;

	PDFPageObject* page = pages->addPage();
	PDFObject* contents = page->getContents();
	BinaryStream* stream = contents->getStream();

	for(i=0;src[i];i++)
	{
		String work;
		CHAR c = src[i];
		i++;
		switch(c)
		{
		case 'A':
			getData(url,src,i);
			break;
		case 'P':
			page = pages->addPage();
			contents = page->getContents();
			stream = contents->getStream();
			i--;
			break;
		case 'W':
			width = getDataFloat(src,i);
			break;
		case 'H':
			height = getDataFloat(src,i);
			break;
		case 'L':
			height = sd->getLineHeight(getDataInt(src,i));
			break;
		case 'X':
			x = getDataFloat(src,i);
			break;
		case 'Y':
			y = getDataFloat(src,i);
			break;
		case 'S':
			size = getDataFloat(src,i);
			break;
		case 'O':
			index = getDataInt(src,i);
			{
				PDFObject* outline = new PDFObject;
				INT i;
				String work;
				WString bookName;
				Bookmark* bookmark = sd->getBookmark(index);
				bookmark->object = outline;
				UTF8toUCS2(bookName,bookmark->name);
				for(i=0;bookName.c_str()[i];i++)
				{
					LPCBYTE data = (LPCBYTE)(bookName.c_str() + i);
					work.appendf("%02x%02x",data[1],data[0]);
				}
				String title;
				title.printf("<feff%s>",work.c_str());

				work.printf("XYZ 0 %g null",m_pageSize[1] - sd->getBookmark(index)->y);
				outline->set("Title",title.c_str());
				outline->add("Dest",page);
				outline->add("Dest",work);
				outline->set("Count",sd->getBookmark(index)->opened);
			}
			break;
		case 'T':
			getData(work,src,i);
			if(url.length())
			{
				String link;
				link.printf("Annot /Subtype /Link /Rect [%g %g %g %g] /Border[0 0 0] /A <</S /URI /URI (%s)>>",
					x,m_pageSize[1]-y-height,x+getTextWidth(work,size),m_pageSize[1]-y,url.c_str());
				PDFObject* linkObject = new PDFObject;
				linkObject->set("Type",link.c_str());
				linkObject->setObjectList(false);
				page->add("Annots",linkObject);
			}
			drawText(stream,x,y+height,work,size);
			break;
		case 'B':
			drawBox(stream,x,y,width,height);
			i--;
			break;
		case 'N':
			drawLine(stream,x,y,width,height);
			i--;
			break;
		case 'I':
			getData(work,src,i);
			loadImage(stream,x,y,width,height,work.c_str(),&m_imageFiles[work.c_str()]);
			break;
		default:
			getData(work,src,i);
			break;
		}
	}


}

void Pdf::getTextSize(PDFPoint& point,LPCWSTR text) const
{
	INT i;
	FLOAT widthLimit;
	FLOAT x = 0;
	FLOAT y = 0;
	FLOAT widthMax = 0;
	FLOAT size;

	widthLimit = m_pageSize[0] - m_pageMargin.left - m_pageMargin.right;
	for(i=0;text[i];i++)
	{
		if(text[i] == '\n')
		{
			size = 0;
			y += m_fontSize;

		}
		else if(text[i] < 0x100)
			size = m_fontSize / 2;
		else
			size = m_fontSize;
			
		if(x > widthLimit + size)
		{	
			x = size;
			y += size;
			widthMax = widthLimit;
		}
		else 
		{
			x += size;
			if(x > widthMax)
				widthMax = x;
		}
	}
	point.x = widthMax;
	point.y = y + m_fontSize;
}
