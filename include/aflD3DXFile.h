#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLXFILE

#include <string>
#include <list>
#include <vector>
#include <dxfile.h>
#include <D3DX9Xof.h>
#include "afl3DBase.h"
namespace AFL{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFileBinaryReader
// X-File用バイナリデータ読み出し用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class XFileBinaryReader
{
public:
	XFileBinaryReader(LPCVOID data);
	void set(LPCVOID data);
	INT getInt();
	DWORD getDWord();
	FLOAT getFloat();
	DWORD getLast() const;
	LPCVOID getAddress(INT dataSize=0);
protected:
	const BYTE* m_dataAddress;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFileLibraly
// X-File用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class XFileLibraly
{
public:
	XFileLibraly();
	virtual ~XFileLibraly();

	ID3DXFile* getInterface() const;
protected:
	ID3DXFile* m_xfile;	//インタフェイス
};

class XFileInterface
{
public:
	static ID3DXFile* getInterface(){return m_xfilLibraly.getInterface();}
protected:
	static XFileLibraly m_xfilLibraly;
};



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XFile
// X-File用クラス
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class XFile
{
public:
	XFile();
	FileObject* load(LPCWSTR fileName);
	FileObject* load(LPCSTR fileName);
	bool save(LPCSTR fileName,FileObject* worldObject,DXFILEFORMAT format=DXFILEFORMAT_TEXT,bool flag3DX=true);
	bool save(LPCWSTR fileName,FileObject* worldObject,DXFILEFORMAT format=DXFILEFORMAT_TEXT,bool flag3DX=true);

	void setNormal(bool flag){m_setNormal=flag;}
	void setUV(bool flag){m_setUV=flag;}
	void setSkin(bool flag){m_setSkin=flag;}
	void setVColor(bool flag){m_setVColor=flag;}
	void setTextureFullPath(bool flag){m_setTextureFullPath=flag;}
	void setAnime(bool flag){m_setAnime=flag;}

protected:
	FileObject* loadMemory(LPBYTE fileData,INT length);

	bool readFrame(ID3DXFileData* xfileData,FrameData* frameData);
	bool readAnimation(ID3DXFileData* xfileData,FileObject* worldObject);
	bool readMesh(ID3DXFileData* xfileData,MeshData* meshData);
	bool readMaterial(ID3DXFileData* xfileData,MeshData* meshData);

	ID3DXFileSaveData* writeAnimation(ID3DXFileSaveObject* xfileSave,LPCSTR name,ANIMATIONSET* anime);
	ID3DXFileSaveData* writeFrame(ID3DXFileSaveObject* xfileSave,ID3DXFileSaveData* xfileSaveData,FrameData* frameData);
	ID3DXFileSaveData* writeMesh(ID3DXFileSaveData* xfileSave,MeshData* meshData);

	INT getIndexSize(std::vector<INDEX4>& index);
	void setIndex(LPVOID dest,std::vector<INDEX4>& index);

	bool convertD3DX(MeshData* meshData,bool flag3DX);
	bool convertD3DX(FrameData* frameData,bool flag3DX);
	FileObject* convertD3DX(FileObject* worldObject,bool flag3DX);

	bool clearData(MeshData* meshData);
	bool clearData(FrameData* frameData);
	void clearData(FileObject* worldObject);

	bool m_setNormal;
	bool m_setUV;
	bool m_setTextureFullPath;
	bool m_setSkin;
	bool m_setAnime;
	bool m_setVColor;

};
}


#define __INC_AFLXFILE
#endif

