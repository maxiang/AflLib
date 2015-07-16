#if _MSC_VER >= 1100
#pragma once
#endif // _MSC_VER >= 1100
#ifndef __INC_AFLDIRECT3DWORLD

namespace AFL{namespace DIRECT3D{

class Unit;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// World
// DirectX - ÉåÉìÉ_ÉäÉìÉOëççá
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class World
{
public:
	World();
	~World();
	bool render();
	bool render(Unit* unit);
	bool persent(HWND hWnd);
	void getDrawRect(HWND hWnd,LPRECT pRect);
	void getDrawPoint(HWND hWnd,LPPOINT point);
	Camera* getCamera();
	void setCamera(Camera* camera);

	void setDrawLimit(FLOAT limit);
	FLOAT getDrawLimit()const;
	FLOAT getDrawCenterX()const;
	FLOAT getDrawCenterY()const;
	FLOAT getDrawCenterZ()const;

	void setLight(INT index,Light* light);
	void setUnit(Unit* unit);
	void setClearColor(D3DCOLOR value);
	virtual void onAction(LPVOID value);
	virtual void onIdel(LPVOID value);
	virtual void onStart(LPVOID value);
	virtual void onEnd(LPVOID value);

	bool setRenderSize(INT width,INT height,D3DFORMAT format=D3DFMT_UNKNOWN);
	INT getRenderWidth() const;
	INT getRenderHeight() const;
	bool present(HWND hWnd,bool scale=false);
	Screen* getScreen();
	bool isShadow()const;
	void setShadow(bool flag = true);
protected:
	bool render(RenderFrame* pRenderFrame,RenderMesh** ppRenderMesh,INT iCount);
	bool drawImage(RenderFrame* pRenderFrame,RenderMesh* pRenderUnit);
	void setRenderUnit(Unit* unit,INT parentIndex,RenderUnit* renderUnit,INT& indexUnit);
	void setRenderUnit(RenderUnit* renderUnit,RenderFrame* renderFrame,INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh);
	void setRenderUnit(RenderUnit* renderUnit,Frame* frame,RenderFrame* renderFrame,
		INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh,NMatrix* matrix);


	bool m_shadow;
	D3DCOLOR m_colorClear;
	Camera* m_camera;
	Camera* m_cameraDefault;
	FLOAT m_drawLimit;
	D3DVECTOR m_drawCenter;
	std::vector<Light> m_light;
	Fog m_fog;

	Unit* m_unit;
	Screen m_screen;

};

}}

#define __INC_AFLDIRECT3DWORLD
#endif
