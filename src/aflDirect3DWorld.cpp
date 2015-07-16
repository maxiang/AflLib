#include <windows.h>

#include "aflDirect3DUnit.h"
#include "aflDirect3DWorld.h"

#ifdef _MSC_VER
	#ifdef _DEBUG	//メモリリークテスト
		#include <crtdbg.h>
		#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
		inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
		inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
			{_free_dbg(pVoid, _NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif //_DEBUG
#else
		#define CHECK_MEMORY_LEAK
#endif

namespace AFL{namespace DIRECT3D{

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// World
// DirectX - レンダリング総合
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
World::World()
{
	m_cameraDefault = NEW Camera3D();
	m_camera = m_cameraDefault;
	m_colorClear = 0x3355bb;
	m_drawCenter.x = 0;
	m_drawCenter.y = 0;
	m_drawCenter.z = 0;
	m_drawLimit = 0.0f;
	m_unit = NULL;

	Light light;
	m_light.push_back(light); 
	m_fog.setEnable(false);
	m_shadow = false;
}
World::~World()
{
	delete m_cameraDefault;
}
void World::setLight(INT index,Light* light)
{
	m_light[index] = *light;	
}
void World::setUnit(Unit* unit)
{
	m_unit = unit;
}
//-----------------------------------------------
// void World::setClearColor(D3DCOLOR value)
// -----  動作  -----
// 背景色を設定する
// -----  引数  -----
// D3DCOLOR 背景色
// ----- 戻り値 -----
// 無し
//-----------------------------------------------
void World::setClearColor(D3DCOLOR value)
{
	m_colorClear = value;
}
//-----------------------------------------------
// bool World::setRenderSize(INT width,INT height,D3DFORMAT format=D3DFMT_UNKNOWN)
// -----  動作  -----
// レンダリングサイズを指定
// -----  引数  -----
// width   幅
// height	高さ
// format	フォーマット
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool World::setRenderSize(INT width,INT height,D3DFORMAT format)
{
	return m_screen.create(width,height,format);
}
INT World::getRenderWidth() const
{
	return m_screen.getWidth();
}
INT World::getRenderHeight() const
{
	return m_screen.getHeight();
}
Screen* World::getScreen()
{
	return &m_screen;
}
bool World::isShadow()const
{
	return m_shadow;
}
void World::setShadow(bool flag)
{
	m_shadow = flag;
}
//-----------------------------------------------
// bool World::present(HWND hWnd)
// -----  動作  -----
// サーフェイスの内容を描画
// -----  引数  -----
// hWnd   レンダリング先のウインドウハンドル
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool World::present(HWND hWnd,bool scale)
{
	return m_screen.present(hWnd,scale);
}
Camera* World::getCamera()
{
	return m_camera;
}
void World::setCamera(Camera* camera)
{
	m_camera=camera;
}
void World::setDrawLimit(FLOAT limit)
{
	m_drawLimit = limit;
}
FLOAT World::getDrawLimit() const
{
	return m_drawLimit;
}
FLOAT World::getDrawCenterX() const
{
	return m_drawCenter.x;
}
FLOAT World::getDrawCenterY() const
{
	return m_drawCenter.y;
}
FLOAT World::getDrawCenterZ() const
{
	return m_drawCenter.z;
}
void World::onAction(LPVOID value)
{
	if(m_unit)
		m_unit->onAction(this,value);
}
void World::onIdel(LPVOID value)
{
	if(m_unit)
		m_unit->onIdel(this,value);
}

void World::onStart(LPVOID value)
{
	if(m_unit)
		m_unit->onStart(this,value);
}
void World::onEnd(LPVOID value)
{
	if(m_unit)
		m_unit->onEnd(this,value);
}

bool World::render()
{
	if(!m_unit)
	{
		//デバイスの状況チェック
		
			return false;
		IDirect3DDevice9* pd3dDevice = Device::getInterface();


		//描画先のクリア
		if(Device::getCaps()->StencilCaps)
			pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET | D3DCLEAR_STENCIL, m_colorClear, 1.0f, 0 );
		else
			pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, m_colorClear, 1.0f, 0 );
		return true;
	}
	return render(m_unit);
}
//-----------------------------------------------
// bool World::render(Unit* unit)
// -----  動作  -----
// ユニットを描画
// -----  引数  -----
// unit  描画ユニット
// ----- 戻り値 -----
// true:成功 false:失敗
//-----------------------------------------------
bool World::render(Unit* unit)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;

	Camera3D* camera = (Camera3D*)getCamera();
	if(camera)
	{
		NVector vect = {0.0f,1.0f,0.0f};
		vect = vect.transformNormal(camera->getView());
		vect = vect.normal3();

		float pointX = camera->getX();
		float pointY = camera->getY();
		float drawPointX = pointX - vect.x * m_drawLimit*0.1f;
		float drawPointY = pointY - vect.y * m_drawLimit*0.1f;
		m_drawCenter.x = drawPointX;
		m_drawCenter.y = drawPointY;
		m_drawCenter.z = camera->getZ();
	}


	//ユニットデータの展開
	INT iUnitCount = unit->getAllUnitCount();
	RenderUnit* renderUnit = NEW RenderUnit[iUnitCount];

	INT iIndexUnit = 0;
	setRenderUnit(unit,-1,renderUnit,iIndexUnit);

	INT iFrameCount = unit->getAllFrameCount();
	INT iMeshCount = unit->getAllMeshCount();
	RenderFrame* renderFrame = NEW RenderFrame[iFrameCount];
	RenderMesh* renderMesh = NEW RenderMesh[iMeshCount];



	INT iIndexFrame = 0;
	INT iIndexMesh = 0;

	INT i;
	for(i=0;i<iIndexUnit;i++)
		setRenderUnit(renderUnit+i,renderFrame,iIndexFrame,renderMesh,iIndexMesh);


	//ソート
	INT j;
	RenderMesh** ppMesh = NEW RenderMesh*[iMeshCount];
	for(i=0;i<iIndexMesh;i++)
	{
		ppMesh[i] = renderMesh + i;
	}
	for(j=0;j<iIndexMesh-1;j++)
	{
		for(i=j+1;i<iIndexMesh;i++)
		{
			bool bFlag = false;
			RenderMesh* pUnit1 = ppMesh[i];
			RenderMesh* pUnit2 = ppMesh[j];
			RenderUnit* renderUnit1 = pUnit1->renderFrame->renderUnit;
			RenderUnit* renderUnit2 = pUnit2->renderFrame->renderUnit;

			if(renderUnit1->unit->isView2D() < renderUnit2->unit->isView2D())
				bFlag = true;
			else if(renderUnit1->unit->isView2D() == renderUnit2->unit->isView2D())
			{
				if(renderUnit1->w < renderUnit2->w)
					bFlag = true;
				else if(renderUnit1->w == renderUnit2->w)
				{
					if(pUnit1->shadow < pUnit2->shadow)
						bFlag = true;
					else if(pUnit1->shadow == pUnit2->shadow &&
							pUnit1->renderFrame->z > pUnit2->renderFrame->z)
						bFlag = true;
				}
			}
			if(bFlag)
			{
				ppMesh[j] = pUnit1;
				ppMesh[i] = pUnit2;
			}
		}
	}


	render(renderFrame,ppMesh,iIndexMesh);

	for(i=0;i<iIndexUnit;i++)
		renderUnit[i].unit->onRenderEnd();

	delete[] renderUnit;
	delete[] renderFrame;
	delete[] renderMesh;
	delete[] ppMesh;
	return true;
}
void World::setRenderUnit(Unit* unit,INT parentIndex,RenderUnit* renderUnit,INT& indexUnit)
{
	FLOAT fX = unit->getPosX();
	FLOAT fY = unit->getPosY();
	FLOAT fZ = unit->getPosZ();
	FLOAT fW = unit->getPosW();
	FLOAT px = 0;
	FLOAT py = 0;
	FLOAT pz = 0;
	DWORD alpha = 100;
	if(parentIndex >= 0)
	{
		alpha = renderUnit[parentIndex].alpha;
		px = renderUnit[parentIndex].matrix._41;//renderUnit[parentIndex].x;
		py = renderUnit[parentIndex].matrix._42;//renderUnit[parentIndex].y;
		pz = renderUnit[parentIndex].matrix._43;//renderUnit[parentIndex].z;

		fX += px;
		fY += py;
		fZ += pz;
		if(unit->isChainW())
			fW += renderUnit[parentIndex].w;
	}
	
	//可視属性なら以降描画リストに
	if(unit->isVisible())
	{
		if(!unit->onRender(this,fX,fY,fZ))
			return;

		NMatrix matrix2;
		unit->setMatrix(&matrix2,fX-px,fY-py,fZ-pz);
		if(parentIndex >= 0)
		{
			//親ユニット分移動
			matrix2 *= renderUnit[parentIndex].matrix;
		}

		renderUnit[indexUnit].alpha = unit->getAlpha() * alpha / 100;
		renderUnit[indexUnit].x = fX;
		renderUnit[indexUnit].y = fY;
		renderUnit[indexUnit].z = fZ;
		renderUnit[indexUnit].w = fW;

		//クリッピング領域の設定
		RECT& rectSrc = *unit->getViewClip();
		RECT& rectDest = renderUnit[indexUnit].clipRect;
		rectDest.top = rectSrc.top;
		rectDest.bottom = rectSrc.bottom;
		rectDest.left = rectSrc.left;
		rectDest.right = rectSrc.right;

		if(parentIndex >= 0 && renderUnit[parentIndex].unit->isViewClip())
		{
			//クリッピング領域の設定
			if(unit->isChainClip())
			{
				RECT rectParent = renderUnit[parentIndex].clipRect;
				rectParent.top -= (INT)unit->getPosY();
				rectParent.bottom -= (INT)unit->getPosY();
				rectParent.left -= (INT)unit->getPosX();
				rectParent.right -= (INT)unit->getPosX();
				if(rectParent.top > rectDest.top)
					rectDest.top = rectParent.top;
				if(rectParent.left > rectDest.left)
					rectDest.left = rectParent.left;
				if(rectParent.bottom < rectDest.bottom)
					rectDest.bottom = rectParent.bottom;
				if(rectParent.right < rectDest.right)
					rectDest.right = rectParent.right;

				if(rectDest.left >= rectDest.right || rectDest.top >= rectDest.bottom)
				{
					rectDest.left = 0;
					rectDest.right = 0;
					rectDest.top = 0;
					rectDest.bottom = 0;
				}
			}
		}
		renderUnit[indexUnit].unit = unit;
		renderUnit[indexUnit].matrix = matrix2;

		INT myIndex = indexUnit;
		indexUnit++;

		//子クラスのメッシュリストの設定
		std::list<Unit*>::iterator itUnit;
		for(itUnit=unit->getChilds()->begin();itUnit!=unit->getChilds()->end();++itUnit)
		{
			setRenderUnit(*itUnit,myIndex,renderUnit,indexUnit);
		}
	}
}
void World::setRenderUnit(RenderUnit* renderUnit,RenderFrame* renderFrame,INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh)
{
	//自クラスのメッシュリストの設定
	Unit* unit = renderUnit->unit;
	Frame* frame = unit->getFrame();
	float x = renderUnit->x;
	float y = renderUnit->y;
	float z = renderUnit->z;
	if(frame)
	{
		if(unit->isLimit() && !unit->isView2D())
		{

			Camera* camera = m_camera;
			NVector vect = {x,y,z,0};
			vect.project(0.0f,0.0f,(FLOAT)m_screen.getWidth(),(FLOAT)m_screen.getHeight(),0.0f,1.0f,
				camera->getProjection(),camera->getView(),NMatrix().setIdentity());


			NVector vectLimit;
			vectLimit = 2.0f;
			if(vect.inBounds(vectLimit))
				return;

			if(getDrawLimit())
			{
				FLOAT fLimit;
				FLOAT fRX = getDrawCenterX() - x;
				FLOAT fRY = getDrawCenterY() - y;
				FLOAT fRZ = getDrawCenterZ() - z;
				fLimit = getDrawLimit();
				fLimit = fLimit * fLimit;
				fLimit -= fRX*fRX + fRY*fRY + fRZ*fRZ;
				if(fLimit < 0)
					return;
			}
		}

		//メッシュリストを再起的に取得
		INT iStartFrame = indexFrame;
		INT iStartMesh = indexMesh;

		setRenderUnit(renderUnit,frame,renderFrame,indexFrame,renderMesh,indexMesh,&renderUnit->matrix);

		//ボーンフレーム番号を設定
		INT i,j,k;
		for(j=iStartMesh;j<indexMesh;j++)
		{
			Mesh* mesh = renderMesh[j].mesh;
			std::vector<BONEMATRIX>* boneMatrixs = mesh->getBoneMatrix();
			for(k=0;k<(int)boneMatrixs->size();++k)
			{
				for(i=iStartFrame;i<indexFrame;i++)
				{
					if((*boneMatrixs)[k].name == renderFrame[i].frame->getFrameName())
					{
						renderMesh[j].boneData[k] = i;
					}
				}
			}
			renderMesh[j].boneCount = k;
		}
	}

}
void World::setRenderUnit(RenderUnit* renderUnit,Frame* frame,RenderFrame* renderFrame,INT& indexFrame,
		RenderMesh* renderMesh,INT& indexMesh,NMatrix* matrix)
{
	NMatrix matrixWork,matAnimation;

	Unit* unit = renderUnit->unit;
	if(unit->getAnimationMatrix(frame->getFrameName(),&matAnimation))
		matrixWork = matAnimation * *matrix;
	else
		matrixWork = frame->getMatrix() * *matrix;

	std::list<Frame>::iterator itFrame;
	for(itFrame=frame->getFrameChilds().begin();itFrame!=frame->getFrameChilds().end();++itFrame)
	{
		setRenderUnit(renderUnit,&*itFrame,renderFrame,indexFrame,renderMesh,indexMesh,&matrixWork);
	}

	FLOAT fSortZ=0;
	if(unit->isZSort() && getCamera())
	{
		NMatrix matrixSort = matrixWork * getCamera()->getView();
		fSortZ = matrixSort._43;
	}

	RenderFrame* rFrame = &renderFrame[indexFrame];
	++indexFrame;
	rFrame->matrix = matrixWork;
	rFrame->z = fSortZ;
	rFrame->renderUnit = renderUnit;
	rFrame->frame = frame;


	std::list<Mesh>::iterator itMesh;
	RenderMesh* pUnit;
	for(itMesh=frame->getMeshes().begin();itMesh!=frame->getMeshes().end();++itMesh)
	{
		pUnit = &renderMesh[indexMesh++];
		pUnit->renderFrame = rFrame;
		pUnit->mesh = &*itMesh;
		pUnit->shadow = itMesh->isShadow();
	}
}
bool World::render(RenderFrame* pRenderFrame,RenderMesh** ppRenderMesh,INT iMeshCount)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;

	m_screen.setTarget();
 
	//描画先のクリア
	if(Device::getCaps()->StencilCaps)
	{
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, m_colorClear, 1.0f, 0 );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET , m_colorClear, 1.0f, 0 );
	}
	else
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, m_colorClear, 1.0f, 0 );

	if(m_camera)
	{
		//カメラ位置設定
		m_camera->setAngle(Device::getTargetWidth(),Device::getTargetHeight());
		//投影設定
		pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&m_camera->getProjection());
	}
	//ライト設定
	std::vector<Light>::iterator itLight;
	INT i;
	for(i=0,itLight=m_light.begin();itLight != m_light.end();++itLight,i++)
	{
		if((*itLight).isEnable())
		{
			pd3dDevice->SetLight( i, &*itLight);
			pd3dDevice->LightEnable(i,true);
		}
		else
			pd3dDevice->LightEnable(i,false);
	}
	//フォグの設定
	if(m_fog.isEnable())
	{
		FLOAT fStart = m_fog.getFogStart();
		FLOAT fEnd = m_fog.getFogEnd();
		DWORD dwColor = m_fog.getFogColor();

		pd3dDevice->SetRenderState(D3DRS_FOGENABLE, true);
		pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
		pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
		pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(LPDWORD)&fStart);
		pd3dDevice->SetRenderState(D3DRS_FOGEND,   *(LPDWORD)&fEnd);
	}
	else
		pd3dDevice->SetRenderState(D3DRS_FOGENABLE, false);

	//描画開始
	pd3dDevice->BeginScene();
	if(ppRenderMesh)
	{
		for(i=0;i<iMeshCount;i++)
		{
			if(ppRenderMesh[i]->renderFrame->renderUnit->unit->isView2D())
				break;
			drawImage(pRenderFrame,ppRenderMesh[i]);
		}
	}

	//影描画処理
	if(isShadow() && Device::getCaps()->StencilCaps && i)
	{
		DWORD color = 0x30050505;
		struct VECTCOLOR
		{
			D3DVECTOR vect;
			FLOAT w;
			DWORD col;
		}vectDraw[]=
		{
			0,0,0,1,color,
			(FLOAT)Device::getTargetWidth()-1,0,0,1,color,
			(FLOAT)Device::getTargetWidth()-1,(FLOAT)Device::getTargetHeight()-1,0,1,color,
			0,(FLOAT)Device::getTargetHeight()-1,0,1,color
		};
		pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA   );
		pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA  );
		
		pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,true);
		pd3dDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
		pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
		pd3dDevice->SetRenderState(D3DRS_STENCILREF, 1);
		pd3dDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
		pd3dDevice->SetTexture(0,NULL);
		pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN ,2,vectDraw,sizeof(VECTCOLOR));
		pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,false);
	}
	
	for(;i<iMeshCount;i++)
	{
		if(!ppRenderMesh[i]->renderFrame->renderUnit->unit->isView2D())
			break;
		drawImage(pRenderFrame,ppRenderMesh[i]);
	}


	pd3dDevice->EndScene();

	return true;
}

bool World::drawImage(RenderFrame* pRenderFrame,RenderMesh* pRenderMesh)
{
	//デバイスの状況チェック
	if(!isDeviceActive())
		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();
	if(!pd3dDevice)
		return false;

	Mesh* mesh = pRenderMesh->mesh;
	if(!mesh)
		return false;

	RenderFrame* renderFrame = pRenderMesh->renderFrame;

	Vertex* vertex = mesh->getVertexBuffer();
	if(!vertex)
		return false;
	IDirect3DVertexBuffer9* pVertexBuffer = vertex->getInterface();
	if(!pVertexBuffer)
		return false;
	Index* index = mesh->getIndexBuffer();
	IDirect3DIndexBuffer9* pIndexBuffer = index?index->getInterface():NULL;
	

	RenderUnit* renderUnit = renderFrame->renderUnit;
	Unit* unit = renderUnit->unit;
	bool shaderFlag = mesh->isShader();

	//ビュークリップ
	if(unit->isViewClip())
	{
		RECT& viewRect = renderFrame->renderUnit->clipRect;
		if(viewRect.top >= viewRect.bottom || viewRect.left >= viewRect.right)
			return true;

		NVector box1 = {(FLOAT)viewRect.left,(FLOAT)viewRect.top,0,1};
		NVector	box2 = {(FLOAT)viewRect.right,(FLOAT)viewRect.top,0,1};
		NVector	box3 = {(FLOAT)viewRect.left,(FLOAT)viewRect.bottom,0,1};
		NVector out1 = box1.transformCoord(renderFrame->matrix);
		NVector out2 = box2.transformCoord(renderFrame->matrix);
		NVector out3 = box3.transformCoord(renderFrame->matrix);

		Device::setClipper(
			out1.x,out1.y,out1.z,
			out2.x,out2.y,out2.z,
			out3.x,out3.y,out3.z);
	}

	//座標の設定
	if(shaderFlag)
	{
		VertexShader* vertexShader = mesh->getVertexShader();
		Device::getInterface()->SetVertexShader(*vertexShader);
		//シェーダ
		NMatrix matrix[100];	
		NMatrix* matView;
		NMatrix* matProj;
		NMatrix matWork;
		if(m_camera)
		{
			if(unit->isView2D())
			{
				matView = &m_camera->getBaseView();
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
			}
			else
			{
				matView = &m_camera->getView();
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,m_camera->getCull());
			}
			matProj = &m_camera->getProjection();
		}
		vertexShader->setDefaultParam();

		Material* material = mesh->getMaterial();
		vertexShader->setParam("MaterialDiffuse",&material->Diffuse);
		vertexShader->setParam("MaterialAmbient",&material->Ambient);
		vertexShader->setParam("View",matView);
		vertexShader->setParam("Proj",matProj);
		vertexShader->setParam("World",&renderFrame->matrix);
		
		if(vertexShader->isParam("LightDir"))
		{
			if(unit->isLight())
			{
				NVector v = {0.1f,-0.5f,-0.5f,0.0f};
				vertexShader->setParam("LightDir",&v);
			}
			else
			{
				static NVector v = {0,0,0};
				vertexShader->setParam("LightDir",&v);
			}
		}
		if(vertexShader->isParam("WorldView") || vertexShader->isParam("WorldViewProj"))
		{
			
			matWork = renderFrame->matrix * *matView;
			vertexShader->setParam("WorldView",&matWork);
			if(vertexShader->isParam("WorldViewProj"))
			{
				matWork *= *matProj;
				vertexShader->setParam("WorldViewProj",&matWork);
			}
			
		}
		if(vertexShader->isParam("ViewProj"))
		{
			matWork = *matView * *matProj;
			vertexShader->setParam("ViewProj",&matWork);
		}
		if(pRenderMesh->boneCount > 0)
		{
			INT i;
			BONEMATRIX* boneMatrixs = &(*mesh->getBoneMatrix())[0];
			for(i=0;i<pRenderMesh->boneCount;i++)
			{
				matrix[i] = boneMatrixs[i].matrix * pRenderFrame[pRenderMesh->boneData[i]].matrix;
			}
			vertexShader->setParam("WorldMatrixArray",matrix,pRenderMesh->boneCount);
		}
	}
	else
	{
		//固定機能
		if(pRenderMesh->boneCount == 0)
		{
			pd3dDevice->SetSoftwareVertexProcessing(false);
			pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND ,D3DVBF_DISABLE);
			pd3dDevice->SetTransform(D3DTS_WORLD ,(D3DMATRIX*)&renderFrame->matrix);
		}
		else
		{
			//ハードウエアの能力が足りるかどうか
			const D3DCAPS9* desc = Device::getCaps();
			if(Device::getCaps()->MaxVertexBlendMatrixIndex < 3
				||(INT)Device::getCaps()->MaxVertexBlendMatrices < pRenderMesh->boneCount)
				pd3dDevice->SetSoftwareVertexProcessing(true);
			pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE,TRUE);
			pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND ,D3DVBF_3WEIGHTS );

			INT i;
			std::vector<BONEMATRIX>* boneMatrixs = mesh->getBoneMatrix();
			for(i=0;i<pRenderMesh->boneCount;i++)
			{
				pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i) ,(D3DMATRIX*)&pRenderFrame[pRenderMesh->boneData[i]].matrix);
				pd3dDevice->MultiplyTransform(D3DTS_WORLDMATRIX(i) ,(D3DMATRIX*)&(*boneMatrixs)[i].matrix);
			}
		}
		//ビュー設定
		if(m_camera)
		{
			if(unit->isView2D())
			{
				pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&m_camera->getBaseView());
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
			}
			else
			{
				pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&m_camera->getView());
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,m_camera->getCull());
			}
		}
		//----------------------------
		//ライティング
		if(unit->isLight())
			pd3dDevice->SetRenderState(D3DRS_LIGHTING,true);
		else
			pd3dDevice->SetRenderState(D3DRS_LIGHTING,false);
		//----------------------------
		Material* material = pRenderMesh->mesh->getMaterial();
		if(unit->isLight())
		{
			if(material)
			{
				pd3dDevice->SetMaterial((D3DMATERIAL9*)material);
				pd3dDevice->SetRenderState(D3DRS_COLORVERTEX,true);
				pd3dDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_COLOR1);
				pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			}
		}
	}	
	

	Texture* pTexture = pRenderMesh->mesh->getTexture();
	IDirect3DTexture9* pITexture = NULL;
	if(pTexture)
	{
		if(renderUnit->alpha != 100)
		{
			DWORD alpha = (0xff * renderUnit->alpha / 100) << 24;
			pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR,alpha);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		}
		else
		{
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		}
		pITexture = pTexture->getInterface();
	}




	//----------------------------
	//Zバッファの設定
	if(unit->isZBuffer())
	{
		pd3dDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
		pd3dDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	}
	else
	{
		pd3dDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	//	pd3dDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
	//	pd3dDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	}

	D3DTEXTUREFILTERTYPE filter = unit->getTextureFilter();
	pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,filter);
	pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,filter);
	pd3dDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,filter);
	if(filter == D3DTEXF_ANISOTROPIC)
	{
		pd3dDevice->SetSamplerState(0,D3DSAMP_MAXANISOTROPY,16);
	}

	//クリッピングの設定
	if(unit->getClipWidth() || unit->getClipHeight() || unit->getClipDepth())
	{
		Device::setClipper(unit->getClipX(),unit->getClipY(),unit->getClipZ(),
			unit->getClipWidth(),unit->getClipHeight(),unit->getClipDepth());

	}
	if(pRenderMesh->shadow)
	{
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	else if(renderUnit->alpha != 100 && !pTexture)
	{
		BYTE a = (BYTE)(0xff * renderUnit->alpha / 100);
		BYTE alpha[4] = {a,a,a,0};
		pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR,*(LPDWORD)&alpha);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);
		pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 8);
	}
	else
	{
		switch(unit->getBlendMode())
		{
		case 0: //ブレンド設定
			pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 8);
			break;
		case 1:
			pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_ONE);
			break;
		case 2:
			pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
			pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 247);
			break;
		}
	}

	if(shaderFlag)
	{
		Device::getInterface()->SetVertexDeclaration(mesh->getDeclaration());
	}

		
	//描画
	pd3dDevice->SetTexture(0,pITexture);

	INT iPrimitiveCount;
	INT iVertexCount = vertex->getStrideCount();

	if(index)
	{
		if(mesh->getPrimitiveType() == D3DPT_TRIANGLELIST)
			iPrimitiveCount = index->getCount() / 3;
		else
			iPrimitiveCount = index->getCount() / 2;
	}
	else
	{
		if(mesh->getPrimitiveType() == D3DPT_TRIANGLELIST)
			iPrimitiveCount = iVertexCount / 3;
		else
			iPrimitiveCount = iVertexCount / 2;
	}

	pd3dDevice->SetStreamSource(0,pVertexBuffer,0,vertex->getStrideSize());
	if(!shaderFlag)
		pd3dDevice->SetFVF(vertex->getFVF());
	pd3dDevice->SetIndices(pIndexBuffer);
	if(pRenderMesh->shadow)
	{
		if(Device::getCaps()->StencilCaps)
		{
			pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);

			//pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, false);
			pd3dDevice->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);
			pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,true);
			pd3dDevice->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);

			if(false && Device::getCaps()->StencilCaps & D3DSTENCILCAPS_TWOSIDED)
			{
				//両面可能
				pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE,true);
				pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC,D3DCMP_ALWAYS);
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
				pd3dDevice->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_KEEP);
				pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL,D3DSTENCILOP_DECR);
				pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS,D3DSTENCILOP_KEEP);
				pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL,D3DSTENCILOP_INCR);
				pd3dDevice->DrawIndexedPrimitive(mesh->getPrimitiveType() ,0,0 ,iVertexCount,0,iPrimitiveCount);
				pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE,false);
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
			}
			else
			{
				pd3dDevice->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_KEEP);
				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
				pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL,D3DSTENCILOP_INCR);
				pd3dDevice->DrawIndexedPrimitive(mesh->getPrimitiveType() ,0,0 ,iVertexCount,0,iPrimitiveCount);

				pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
				pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL,D3DSTENCILOP_DECR);
				pd3dDevice->DrawIndexedPrimitive(mesh->getPrimitiveType() ,0,0 ,iVertexCount,0,iPrimitiveCount);

			}
			pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,false);
			pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
			pd3dDevice->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
			//pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, true);
		}

	}
	else
	{
	//pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,true);


		pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP);
		pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL,D3DSTENCILOP_KEEP);
		pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		if(pIndexBuffer)
			pd3dDevice->DrawIndexedPrimitive(mesh->getPrimitiveType() ,0,0 ,iVertexCount,0,iPrimitiveCount);
		else
			pd3dDevice->DrawPrimitive(mesh->getPrimitiveType() ,0,iPrimitiveCount);

	//	pd3dDevice->SetRenderState(D3DRS_STENCILENABLE,false);
	}

	if(shaderFlag)
	{
		Device::getInterface()->SetVertexShader(NULL);
		Device::getInterface()->SetPixelShader(NULL);
		Device::getInterface()->SetVertexDeclaration(NULL);
	}
	else
		pd3dDevice->SetFVF(NULL);

	Device::getInterface()->SetRenderState( D3DRS_SCISSORTESTENABLE, false );

	//クリッピングの解除
	if(unit->isViewClip() || unit->getClipWidth() || unit->getClipHeight() || unit->getClipDepth())
	{
		Device::setClipper(0,0,0,0,0,0);
	}
	return true;
}
bool World::persent(HWND hWnd)
{
	//デバイスがロストしていないかチェック
	if(Device::isLost())
	{
		//ロストデバイスの修復
		if(!Device::resetDevice())
			return false;
	}
	//デバイスの状況チェック
//	if(!isDeviceActive())
//		return false;
	IDirect3DDevice9* pd3dDevice = Device::getInterface();

	if(Device::getParams()->SwapEffect & D3DSWAPEFFECT_COPY)
	{
		//アスペクト比を保って描画
		AFL::WINDOWS::Rect3D rect;
		getDrawRect(hWnd,&rect);

		//描画済みサーフェイスを表示
		//pd3dDevice->Present( NULL, rect, hWnd, NULL );
		m_screen.present(hWnd);
	}
	else
	{
		//描画済みサーフェイスを表示
		//pd3dDevice->Present( NULL, NULL, hWnd, NULL );
		m_screen.present(hWnd);
	}
	//領域を更新済みに設定
	ValidateRect(hWnd,NULL);
	return true;
}

void World::getDrawRect(HWND hWnd,LPRECT pRect)
{
	AFL::WINDOWS::Rect3D rect;
	GetClientRect(hWnd,rect);
	FLOAT fAspect = (FLOAT)Device::getTargetWidth() / (FLOAT)Device::getTargetHeight();
	FLOAT fAspect2 = (FLOAT)rect.getWidth() / (FLOAT)rect.getHeight();
	if(fAspect > fAspect2)
	{
		rect.bottom = (INT)(rect.getWidth() / fAspect);
	}
	else if(fAspect < fAspect2)
	{
		rect.right = (INT)(rect.getHeight() * fAspect);
	}
	*pRect = rect;
}
void World::getDrawPoint(HWND hWnd,LPPOINT point)
{
	AFL::WINDOWS::Rect3D rect;
	GetClientRect(hWnd,rect);
	FLOAT fAspect = (FLOAT)Device::getTargetWidth() / (FLOAT)Device::getTargetHeight();
	FLOAT fAspect2 = (FLOAT)rect.getWidth() / (FLOAT)rect.getHeight();
	if(fAspect > fAspect2)
	{
		rect.bottom = (INT)(rect.getWidth () / fAspect);
	}
	else if(fAspect < fAspect2)
	{
		rect.right = (INT)(rect.getHeight () * fAspect);
	}
	point->x = point->x * Device::getTargetWidth()/rect.getWidth();
	point->y = point->y * Device::getTargetHeight()/rect.getHeight();
}

}}