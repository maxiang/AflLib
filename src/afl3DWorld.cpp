#ifndef __ANDROID__
	#include <windows.h>
#else
	#include <android/log.h>
#endif
#include "afl3DWorld.h"
//----------------------------------------------------
//メモリリークテスト用
#if !defined(CHECK_MEMORY_LEAK)
	#if _MSC_VER && !defined(_WIN32_WCE) && _DEBUG
		#include <crtdbg.h>
		inline static void*  operator new(const size_t size, LPCSTR strFileName, INT iLine)
			{return _malloc_dbg(size,_NORMAL_BLOCK,  strFileName, iLine);}
		inline static void operator delete(void* adr, LPCSTR strFileName, INT iLine)
			{_free_dbg(adr,_NORMAL_BLOCK);}
		#define NEW new(__FILE__, __LINE__)
		#define CHECK_MEMORY_LEAK _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	#else
		#define NEW new
		#define CHECK_MEMORY_LEAK
	#endif
#endif
//----------------------------------------------------
namespace AFL{
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UnitShadowDraw
// 影描画用
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
UnitShadowDraw::UnitShadowDraw()
{
#if !defined(_OPENGL) & !defined(__ANDROID__)
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = false;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.StencilEnable = true;
	desc.StencilWriteMask = 0 ;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL ;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS ;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;

	//影塗り潰し用の設定
	ID3D11DepthStencilState* stencilState = NULL;
	D3DDevice::getDevice()->CreateDepthStencilState(&desc,&m_stencilStateSDraw);

	//塗り潰し用の頂点設定
	const static D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		0
	};
	if(!D3DDevice::getInputElement("FLAT"))
	{
		D3DDevice::addInputElement("FLAT",layout);
	}

	m_shadow.createLayout("SDRAW","FLAT");
	m_shadow.setPShader("SDRAW");

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_shadow.setBlend(&blendDesc);

#else
	//塗り潰し用の頂点設定
	const static GLLayout layout[] =
	{
		"POSITION", 3, GL_FLOAT,
	};

	if(!Vertex::getLayout("SDRAW"))
	{
		Vertex::addLayout("SDRAW",layout,1);
	}


	m_shadow.createLayout("SDRAW","SDRAW");
	m_shadow.setPShader("FLAT");
#endif

	//ソフトシャドウ使用デフォルト
	m_softShadow = false;

	setZBuffer(false);
	setPosW(10.0f);

	const static FLOAT vertex[] = 
	{
		-1.0f,-1.0f, 0.0f,
		-1.0f, 1.0f ,0.0f,
		1.0f, 1.0f,0.0f,
		-1.0f,-1.0f, 0.0f,
		1.0f, 1.0f,0.0f,
		1.0f,-1.0f ,0.0f,
	};

	m_shadow.createMesh(vertex,sizeof(vertex));



	//フレームの作成
	m_frame = SP<Frame>(NEW Frame());
	m_frame->add(&m_shadow);
}

UnitShadowDraw::~UnitShadowDraw()
{
#if !defined(_OPENGL) & !defined(__ANDROID__)
	if(m_stencilStateSDraw)
		m_stencilStateSDraw->Release();
#endif
}


bool UnitShadowDraw::onRenderMesh(Mesh* mesh,LPVOID value)
{
	World* world = (World*)value;
#if !defined(_OPENGL) & !defined(__ANDROID__)
	Screen* screen = world->getScreen();
	ID3D11DeviceContext* context;
	context = screen->getContext();

	//トライアングルリストを設定
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );


	if(m_softShadow)
	{
		INT width = screen->getWidth();
		INT height = screen->getHeight();
		if(m_screenShadow->getImageWidth() != width || m_screenShadow->getImageHeight() != height)
		{
			m_screenShadow->create(width,height,D3D11_USAGE_DEFAULT);
		}


		//描画ターゲットの変更
		m_screenShadow->setTarget(context);
		m_screenShadow->clear(0x00000000);
	
		UINT offset = 0;
		UINT stride = m_shadow.getStrideSize();
		ID3D11Buffer* vertexBuffer = *m_shadow.getVertex();
		context->OMSetDepthStencilState(m_stencilStateSDraw,1);
		context->IASetInputLayout( *m_shadow.getInputLayout() );
		context->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );
		context->VSSetShader(*D3DDevice::getVShader("FLAT"), NULL, 0 );
		context->PSSetShader(*D3DDevice::getPShader("FLAT"), NULL, 0 );
		context->OMSetBlendState(D3DDevice::getBlendSet(),(FLOAT*)&NVector::set(0,0,0,0),0xffffffff );
		context->IASetIndexBuffer(*m_shadow.getIndex(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed( m_shadow.getIndex()->getSize()/2, 0,0 );	
		context->OMSetDepthStencilState(NULL,1);
		//描画ターゲットの復元
		screen->setTarget();

		//context->OMSetDepthStencilState(NULL,1);
		context->OMSetDepthStencilState(m_stencilStateSDraw,1);
		ID3D11ShaderResourceView* resourceView = m_screenShadow->getResourceView();
		context->PSSetShaderResources( 0, 1,&resourceView);
		//context->PSSetSamplers( 0, 1, &m_samplerStage );


		context->IASetInputLayout( *m_shadow.getInputLayout() );
		context->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );

		context->VSSetShader(*m_shadow.getVertexShader(), NULL, 0 );
		context->PSSetShader(*m_shadow.getPixelShader(), NULL, 0 );
	
		context->IASetIndexBuffer(*m_shadow.getIndex(), DXGI_FORMAT_R16_UINT, 0);
		context->OMSetBlendState(D3DDevice::getBlendAlpha(),(FLOAT*)&NVector::set(0,0,0,0),0xffffffff );
		context->DrawIndexed( m_shadow.getIndex()->getSize()/2, 0,0 );
	}
	else
	{
		//ノーマルシャドウ
		UINT offset = 0;
		UINT stride = m_shadow.getStrideSize();
		ID3D11Buffer* vertexBuffer = *m_shadow.getVertex();
		context->OMSetDepthStencilState(m_stencilStateSDraw,1);
		context->IASetInputLayout( *m_shadow.getInputLayout() );
		context->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );
		context->VSSetShader(*D3DDevice::getVShader("FLAT"), NULL, 0 );
		context->PSSetShader(*D3DDevice::getPShader("FLAT2"), NULL, 0 );
		context->OMSetBlendState(m_shadow.getBlend(),(FLOAT*)&NVector::set(0,0,0,0),0xffffffff );
		context->IASetIndexBuffer(*m_shadow.getIndex(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed( m_shadow.getIndex()->getSize()/2, 0,0 );	
		context->OMSetDepthStencilState(NULL,1);
	}
#else
	//影の描画
	if (m_shadow.getIndex())
	{
		m_shadow.useProgram();
		m_shadow.bindLayout();

		glEnable(GL_BLEND);
		glCullFace(GL_FRONT);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_LEQUAL, 1, ~0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *m_shadow.getIndex());
		glDrawElements(GL_TRIANGLES, m_shadow.getIndex()->getSize() / 2, GL_UNSIGNED_SHORT, 0);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
	}
#endif
	return false;
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// World
// レンダリング総合
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
World::World()
{

#if !defined(_OPENGL) & !defined(__ANDROID__)
	//2Dオブジェクト用ステンシル設定
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = false;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.StencilEnable = true;
	desc.StencilWriteMask = 0 ;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL ;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS ;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	desc.StencilEnable = false;
	D3DDevice::getDevice()->CreateDepthStencilState(&desc,&m_stencilState2D);

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // サンプリング時に使用するフィルタ。ここでは異方性フィルターを使用する。
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある v テクスチャー座標
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標
	samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
	samplerDesc.MaxAnisotropy = 16;                        // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。
	samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
	D3DDevice::getDevice()->CreateSamplerState( &samplerDesc, &m_samplerStage );

	D3DDevice::loadShaders(L"Shaders/");
#endif

	m_screen = NULL;
	m_camera = &m_cameraWorld;

	m_color = COLOR4(0.0f, 0.0f, 0.0f, 1.0f);
	m_drawCenter.x = 0;
	m_drawCenter.y = 0;
	m_drawCenter.z = 0;
	m_drawLimit = 0.0f;
	m_unitShadowDraw = new UnitShadowDraw();
	m_unit.add(m_unitShadowDraw);
}

World::~World()
{
	delete m_unitShadowDraw;
}

void World::setScreen(Screen* screen)
{
	m_screen = screen;
	m_screen->setCamera((Camera3D*)&m_camera);
	
}
Screen* World::getScreen() const
{
	return m_screen;
}
Camera3D* World::getCamera()
{
	return m_camera;
}
void World::setCamera(Camera3D* camera)
{
	if(m_screen)
		m_screen->setCamera((Camera3D*)&m_camera);
	m_camera = camera;
}
void World::addUnit(Unit* unit)
{
	m_unit.add(unit);
}
bool World::clear(FLOAT r,FLOAT g,FLOAT b,FLOAT a)
{
	if(!m_screen)
		return false;
	return m_screen->clear(r,g,b,a);
}
bool World::setBackgroundColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
{
	m_color = COLOR4(r,g,b,a);
	return true;
}

bool World::present()
{
	if(!m_screen)
		return false;
	return m_screen->present();
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

bool World::draw()
{
	Unit* unit = &m_unit;
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
	INT unitCount = unit->getAllUnitCount();
	INT iIndexUnit = 0;
	RenderUnit* renderUnit = new RenderUnit[unitCount];
	_setRenderUnit(unit,-1,renderUnit,iIndexUnit);

	INT iFrameCount = unit->getAllFrameCount();
	INT iMeshCount = unit->getAllMeshCount();
	INT iIndexFrame = 0;
	INT iIndexMesh = 0;
	RenderFrame* renderFrame = new RenderFrame[iFrameCount];
	RenderMesh* renderMesh = new RenderMesh[iMeshCount];

	//ユニットからフレームとメッシュ情報を取り出す
	INT i;
	for(i=0;i<iIndexUnit;i++)
		_setRenderUnit(renderUnit+i,renderFrame,iIndexFrame,renderMesh,iIndexMesh);

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

			FLOAT w1 = renderUnit1->w + (pUnit1->shadow?0.5f:0.0f);
			FLOAT w2 = renderUnit2->w + (pUnit2->shadow?0.5f:0.0f);
			if(w1 < w2)
				bFlag = true;
			else if(w1 == w2)
			{

				if(pUnit1->renderFrame->z > pUnit2->renderFrame->z)
					bFlag = true;
			}
			if(bFlag)
			{
				ppMesh[j] = pUnit1;
				ppMesh[i] = pUnit2;
			}
		}
	}

#if !defined(_OPENGL) & !defined(__ANDROID__)
#else
	//デバイス修復処理
	GLDevice::lost2();
	GLDevice::restore();
#endif
	//クリア
	clear(m_color.r, m_color.g, m_color.b, m_color.a);

	//メッシュの描画
	_render(renderFrame,ppMesh,iIndexMesh);


	for(i=0;i<iIndexUnit;i++)
		renderUnit[i].unit->onRenderEnd();

	delete[] ppMesh;

	delete[] renderMesh;
	delete[] renderFrame;
	delete[] renderUnit;
	return true;
}

void World::_setRenderUnit(Unit* unit,INT parentIndex,RenderUnit* renderUnit,INT& indexUnit) 
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
		px = renderUnit[parentIndex].matrix._41;
		py = renderUnit[parentIndex].matrix._42;
		pz = renderUnit[parentIndex].matrix._43;

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

		NMatrix matrix;
		matrix = unit->getMatrix(fX-px,fY-py,fZ-pz);
		if(parentIndex >= 0)
		{
			//親ユニット分移動
			matrix *= renderUnit[parentIndex].matrix;
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
		renderUnit[indexUnit].matrix = matrix;

		INT myIndex = indexUnit;
		indexUnit++;

		//子クラスのメッシュリストの設定
		std::list<Unit*>::iterator itUnit;
		for(itUnit=unit->getChilds()->begin();itUnit!=unit->getChilds()->end();++itUnit)
		{
			_setRenderUnit(*itUnit,myIndex,renderUnit,indexUnit);
		}
	}
}
void World::_setRenderUnit(RenderUnit* renderUnit,RenderFrame* renderFrame,INT& indexFrame,RenderMesh* renderMesh,INT& indexMesh)
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
			vect.project(0.0f,0.0f,(FLOAT)m_screen->getWidth(),(FLOAT)m_screen->getHeight(),0.0f,1.0f,
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

		_setRenderUnit(renderUnit,frame,renderFrame,indexFrame,renderMesh,indexMesh,renderUnit->matrix);

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
void World::_setRenderUnit(RenderUnit* renderUnit,Frame* frame,RenderFrame* renderFrame,INT& indexFrame,
		RenderMesh* renderMesh,INT& indexMesh,NMatrix& matrix)
{
	NMatrix matrixWork,matAnimation;

	Unit* unit = renderUnit->unit;
	if(unit->getAnimationMatrix(frame->getFrameName(),matAnimation))
		matrixWork = matAnimation * matrix;
	else
		matrixWork = frame->getMatrix() * matrix;

	std::list<SP<Frame> >::iterator itFrame;
	for(itFrame=frame->getFrameChilds().begin();itFrame!=frame->getFrameChilds().end();++itFrame)
	{
		_setRenderUnit(renderUnit,itFrame->get(),renderFrame,indexFrame,renderMesh,indexMesh,matrixWork);
	}

	FLOAT fSortZ=0;
	if(unit->isZSort())
	{
		NMatrix matrixSort = matrixWork * m_camera->getView();
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
		pUnit->shadow = false;
	}
	for(itMesh=frame->getShadows().begin();itMesh!=frame->getShadows().end();++itMesh)
	{
		pUnit = &renderMesh[indexMesh++];
		pUnit->renderFrame = rFrame;
		pUnit->mesh = &*itMesh;
		pUnit->shadow = true;
	}
}

void World::onAction(LPVOID value)
{
	m_unit.onAction(value);
}
#if !defined(_OPENGL) & !defined(__ANDROID__)
bool World::_render(RenderFrame* pRenderFrame,RenderMesh* pRenderMesh)
{
	Mesh* mesh = pRenderMesh->mesh;
	if(!mesh)
		return false;
	Unit* unit = pRenderMesh->renderFrame->renderUnit->unit;
	if(!unit->onRenderMesh(mesh,this))
		return false;

	if(!mesh->getVertex())
		return true;

	RenderFrame* renderFrame = pRenderMesh->renderFrame;
	Frame* frame = renderFrame->frame;

	ID3D11DeviceContext* context = m_screen->getContext();


	Shader* ps = mesh->getPixelShader();

	NVector c = {1.0f,1.0f,1.0f,(FLOAT)unit->getAlpha()/100.0f};
	ps->update("Color",&c);
/*
	//カスタムシェーダ定数設定
	std::map<int,ConstantBuffer>::iterator it;
	std::map<int,ConstantBuffer>& vsConstant = unit->getVSConstant();
	for(it = vsConstant.begin();it!=vsConstant.end();++it)
	{
		context->VSSetConstantBuffers( it->first, 1, it->second );
	}
	std::map<int,ConstantBuffer>& psConstant = unit->getPSConstant();
	for(it = psConstant.begin();it!=psConstant.end();++it)
	{
		context->PSSetConstantBuffers( it->first, 1, it->second );
	}

	*/

	Shader* vs = mesh->getVertexShader();
	//ビューの設定
	if(renderFrame->renderUnit->unit->isView2D())
	{
		vs->update("View",m_constantView2D);
		vs->update("Projection",m_constantProj2D);
		vs->update("ViewProj",m_constantViewProj2D);
	}
	else
	{
		vs->update("View",m_constantView);
		vs->update("Projection",m_constantProj);
		vs->update("ViewProj",m_constantViewProj);
	}
	//ワールド行列の設定
	NMatrix matrix = renderFrame->matrix;
	matrix.setTranspose();
	vs->update("World",&matrix);

	//ボーンの設定
	if(pRenderMesh->boneCount > 0)
	{
		INT i;
		NMatrix matrix[24];
		BONEMATRIX* boneMatrixs = &(*mesh->getBoneMatrix())[0];
		for(i=0;i<pRenderMesh->boneCount;i++)
		{
			matrix[i] = boneMatrixs[i].matrix * pRenderFrame[pRenderMesh->boneData[i]].matrix;
			matrix[i].setTranspose();
		}
		vs->update("ConstantWorldBone",&matrix);
	}

	//シェーダ定数の確定
	vs->setConstant(context);
	ps->setConstant(context);


	UINT offset = 0;
	UINT stride = mesh->getStrideSize();
	ID3D11Buffer* vertexBuffer = *mesh->getVertex();

	context->IASetInputLayout( *mesh->getInputLayout() );
	context->IASetVertexBuffers( 0, 1,&vertexBuffer, &stride, &offset );

	context->VSSetShader(*mesh->getVertexShader(), NULL, 0 );
	context->PSSetShader(*mesh->getPixelShader(), NULL, 0 );
	context->PSSetSamplers( 0, 1, &m_samplerStage );

	//トライアングルリストを設定
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//ラスタライザーの設定
	if(mesh->getRasterizer())
	{
		context->RSSetState(mesh->getRasterizer());
	}
	//深度バッファ設定
	if (mesh->getDepthStencil())
	{
		context->OMSetDepthStencilState(mesh->getDepthStencil(),1);
	}
	else if (!renderFrame->renderUnit->unit->isZBuffer())
		context->OMSetDepthStencilState(m_stencilState2D, 1);
	//ブレンド設定
	if (mesh->getBlend())
	{
		context->OMSetBlendState(mesh->getBlend(), (FLOAT*)&NVector::set(0, 0, 0, 0), 0xffffffff);
	}
	else
		context->OMSetBlendState(D3DDevice::getBlendAlpha(), (FLOAT*)&NVector::set(0, 0, 0, 0), 0xffffffff);

	if (mesh->getTexture())
	{
		INT count = mesh->getTextureCount();
		ID3D11ShaderResourceView** resourceView = new ID3D11ShaderResourceView*[count];
		INT i;
		for(i=0;i<count;i++)
			resourceView[i] = mesh->getTexture(i)->getResourceView();
		context->PSSetShaderResources( 0, count, resourceView);
		context->VSSetShaderResources( 0, count, resourceView);
		delete[] resourceView;
	}

	if(mesh->getIndex())
	{
		context->IASetIndexBuffer(*mesh->getIndex(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed( mesh->getIndex()->getSize()/2, 0,0 );
	}
	else
	{
		context->Draw( mesh->getVertex()->getSize()/mesh->getStrideSize(), 0 );
	}
	context->RSSetState(NULL);
	context->OMSetDepthStencilState(NULL,1);

	return true;
}

bool World::_render(RenderFrame* pRenderFrame,RenderMesh** ppRenderMesh,INT iMeshCount)
{
	if(!m_screen)
		return false;

	//カメラ位置設定
	m_camera->setAngle(m_screen->getWidth(),m_screen->getHeight());

	NMatrix matrix[3];
	//ビューと射影行列の設定
	matrix[0] = m_camera->getView();
	matrix[1] = m_camera->getProjection();
	matrix[2] = m_camera->getView()*m_camera->getProjection();
	matrix[0].setTranspose();
	matrix[1].setTranspose();
	matrix[2].setTranspose();

	m_constantWorld = matrix[0];
	m_constantView = matrix[0];
	m_constantProj = matrix[1];
	m_constantViewProj = matrix[2];


	matrix[0] = m_camera->getBaseView();
	matrix[2] = m_camera->getBaseView()*m_camera->getProjection();
	matrix[0].setTranspose();
	matrix[2].setTranspose();

	m_constantView2D = matrix[0];
	m_constantProj2D = matrix[1];
	m_constantViewProj2D = matrix[2];



	INT i;
	if(ppRenderMesh)
	{
		for(i=0;i<iMeshCount;i++)
		{
			_render(pRenderFrame,ppRenderMesh[i]);
		}
	}

	return true;
}

#else
bool World::_render(RenderFrame* pRenderFrame,RenderMesh* pRenderMesh)
{
	Mesh* mesh = pRenderMesh->mesh;
	if(!mesh)
		return false;



	//シェーダの選択
	mesh->useProgram();
	RenderFrame* renderFrame = pRenderMesh->renderFrame;
	Frame* frame = renderFrame->frame;

	//メッシュのコールバック
	Unit* unit = pRenderMesh->renderFrame->renderUnit->unit;
	if (!unit->onRenderMesh(mesh, this))
		return false;


	//ビューの設定
	if(renderFrame->renderUnit->unit->isView2D())
	{
		mesh->updateVS("Projection",&m_constantProj);
		mesh->updateVS("View",&m_constantView2D);
		mesh->updateVS("ViewProj",&m_constantViewProj2D);
	}
	else
	{
		mesh->updateVS("Projection",&m_constantProj);
		mesh->updateVS("View",&m_constantView);
		mesh->updateVS("ViewProj",&m_constantViewProj);
	}
	
	//ライトの設定
	float light = (float)renderFrame->renderUnit->unit->isLight();
	mesh->getProgram()->uniformFloat("Light",&light);


	//ワールド行列の設定
	NMatrix matrix = renderFrame->matrix;
	matrix.setTranspose();
	mesh->updateVS("World",&matrix);


	//ボーンの設定
	if(pRenderMesh->boneCount > 0)
	{
		INT i;
		NMatrix matrix[24];
		BONEMATRIX* boneMatrixs = &(*mesh->getBoneMatrix())[0];
		for(i=0;i<pRenderMesh->boneCount;i++)
		{
			matrix[i] = boneMatrixs[i].matrix * pRenderFrame[pRenderMesh->boneData[i]].matrix;
			matrix[i].setTranspose();
		}
		mesh->updateVS("WorldMatrixArray",matrix);
	}

	mesh->bindLayout();


	if(mesh->getBlendMode() == 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	//テクスチャ設定
	if (mesh->getTexture())
	{
		INT count = mesh->getTextureCount();
		if (count == 2)
		{
			INT a = 1;
		}
		INT i;
		for (i = 0; i<count; i++)
		{
			String s;
			s.printf("texture%d", i);
			glActiveTexture(GL_TEXTURE0 + i);
			glEnable(GL_TEXTURE_2D);
			mesh->getProgram()->uniformInt(s, &i);
			glBindTexture(GL_TEXTURE_2D, *mesh->getTexture(i));
		}
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if(!pRenderMesh->shadow)
	{

		if(unit->isBothSide())		//片面・両面
			glDisable(GL_FRONT);
		else
		{
			glEnable(GL_CULL_FACE);
			if (renderFrame->renderUnit->unit->isView2D() || m_camera->isLeftHand())
				glCullFace(GL_FRONT);
			else
				glCullFace(GL_BACK);
		}
		//通常メッシュの描画
		if(renderFrame->renderUnit->unit->isZBuffer())
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);


		//両面同時
		glDisable(GL_CULL_FACE);
		glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 0, ~0); //両面の設定
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP); //裏面の設定
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP); //表面の設定
	}

	//描画処理
	if (mesh->getIndex())
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *mesh->getIndex());
		glDrawElements(GL_TRIANGLES, mesh->getIndex()->getSize() / 2, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, mesh->getVertex()->getSize() / mesh->getStrideSize());
	}

	//テクスチャ設定の解除
	if (mesh->getTexture())
	{
		INT i;
		INT count = mesh->getTextureCount();
		for (i = 1; i<count; i++)
		{
			String s;
			glActiveTexture(GL_TEXTURE0 + i);
			glDisable(GL_TEXTURE_2D);
		}
		glActiveTexture(GL_TEXTURE0);

	}

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	return true;
}

bool World::_render(RenderFrame* pRenderFrame,RenderMesh** ppRenderMesh,INT iMeshCount)
{
	if(!m_screen)
		return false;
	//カメラ位置設定
	m_camera->setAngle(m_screen->getWidth(),m_screen->getHeight());

	//ビューと射影行列の設定
	m_constantView = m_camera->getView();
	m_constantProj = m_camera->getProjection();
	m_constantViewProj = m_camera->getView()*m_camera->getProjection();
	m_constantView.setTranspose();
	m_constantProj.setTranspose();
	m_constantViewProj.setTranspose();


	m_constantView2D = m_camera->getBaseView();
	m_constantViewProj2D = m_camera->getBaseView()*m_camera->getProjection();
	m_constantView2D.setTranspose();
	m_constantViewProj2D.setTranspose();

	/*
	INT i;
	if(ppRenderMesh)
	{
		for(i=0;i<iMeshCount;i++)
		{
			Unit* unit = ppRenderMesh[i]->renderFrame->renderUnit->unit;
			if(unit->isView2D())
				break;
			_render(pRenderFrame,ppRenderMesh[i]);
		}
	}

	for(;i<iMeshCount;i++)
	{
		_render(pRenderFrame,ppRenderMesh[i]);
	}
*/
	INT i;
	if (ppRenderMesh)
	{
		for (i = 0; i<iMeshCount; i++)
		{
			_render(pRenderFrame, ppRenderMesh[i]);
		}
	}

	return true;
}
#endif

}
