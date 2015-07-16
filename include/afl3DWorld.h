#pragma once

#if !defined(_OPENGL) & !defined(__ANDROID__)
	#include "aflD3DUnit.h"
#else
	#include "aflOpenGLUnit.h"
#endif
namespace AFL{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderFrame
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderUnit
{
	Unit* unit;
	NMatrix matrix;
	FLOAT x,y,z,w;
	FLOAT limit;
	RECT clipRect;
	DWORD alpha;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderFrame
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderFrame
{
	RenderUnit* renderUnit;
	NMatrix matrix;
	Frame* frame;
	FLOAT z;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderMesh
// レンダリング整理用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
struct RenderMesh
{
	RenderFrame* renderFrame;
	Mesh* mesh;
	Material material;
	INT boneData[256];
	INT boneCount;
	bool shadow;
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitShadowDraw
// 影描画用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class UnitShadowDraw : public Unit
{
public:
	UnitShadowDraw();
	~UnitShadowDraw();
	bool onRenderMesh(Mesh* mesh,LPVOID value);
protected:

	Mesh m_shadow;
	SP<Texture> m_screenShadow;
	bool m_softShadow;
#if !defined(_OPENGL) & !defined(__ANDROID__)
	ID3D11DepthStencilState* m_stencilStateSDraw;
#endif
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// World
// レンダリング総合
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class World
{
public:
	World();
	~World();
	void setScreen(Screen* screen);
	Screen* getScreen() const;
	bool clear(FLOAT r=0.0f,FLOAT g=0.0f,FLOAT b=0.0f,FLOAT a=1.0f);
	bool setBackgroundColor(FLOAT r = 0.0f, FLOAT g = 0.0f, FLOAT b = 0.0f, FLOAT a = 1.0f);
	bool present();
	bool draw();

	void setDrawLimit(FLOAT limit);
	FLOAT getDrawLimit()const;
	FLOAT getDrawCenterX()const;
	FLOAT getDrawCenterY()const;
	FLOAT getDrawCenterZ()const;
	Camera3D *getCamera();
	void setCamera(Camera3D* camera);
	void addUnit(Unit* unit);
	void onAction(LPVOID value);
protected:
	bool _render(RenderFrame* pRenderFrame,RenderMesh* pRenderMesh);
	bool _renderShadow(RenderFrame* pRenderFrame,RenderMesh* pRenderMesh);
	bool _render(RenderFrame* pRenderFrame,RenderMesh** ppRenderMesh,INT iMeshCount);
	void _setRenderUnit(Unit* unit,INT parentIndex,RenderUnit* renderUnit,INT& indexUnit);
	void _setRenderUnit(RenderUnit* renderUnit,RenderFrame* renderFrame,INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh);
	void _setRenderUnit(RenderUnit* renderUnit,Frame* frame,RenderFrame* renderFrame,
		INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh,NMatrix& matrix);

	FLOAT m_drawLimit;
	NVector m_drawCenter;
	NVector m_pos;
	COLOR4 m_color;
	Camera3D* m_camera;
	Camera3D m_cameraWorld;
	Screen* m_screen;
	Unit m_unit;
	UnitShadowDraw* m_unitShadowDraw;

#if !defined(_OPENGL) & !defined(__ANDROID__)
	ID3D11DepthStencilState* m_stencilState2D;
	ID3D11SamplerState* m_samplerStage;
#endif
	NMatrix m_constantWorld;
	NMatrix m_constantView;
	NMatrix m_constantProj;
	NMatrix m_constantViewProj;

	NMatrix m_constantView2D;
	NMatrix m_constantProj2D;
	NMatrix m_constantViewProj2D;
};

}
