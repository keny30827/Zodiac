#include "RenderPassTest.h"
#include "../GraphicsResourceManager.h"
#include "../GraphicsController.h"
#include "../shader/ShaderManager.h"

#include "../../object/decal/Decal.h"

void CRenderPassTest::Render(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr)
{
	// テスト用パス.
	// モデルをテスト用レンダーターゲットに書き出す.

	// 必要な情報の取得.だめなら打ち切り.
	IDXGISwapChain4* pSwapChain = graphicsController.GetSwapChain();
	VRETURN(pSwapChain);
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	HRESULT ret = pSwapChain->GetDesc1(&scDesc);
	VRETURN(ret == S_OK);
	
	// ビューポートとシザリング作る.
	D3D12_VIEWPORT viewPort = {};
	{
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width = static_cast<float>(scDesc.Width);
		viewPort.Height = static_cast<float>(scDesc.Height);
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
	}
	D3D12_RECT scissor = {};
	{
		scissor.top = 0;
		scissor.left = 0;
		scissor.bottom = scissor.top + scDesc.Height;
		scissor.right = scissor.left + scDesc.Width;
	}

	// デプスプリパス.
	{
		auto& rt = scene.GetTestRT();
		if (graphicsController.BeginScene(&rt, &scene.GetDepthPrepass())) {
			// オブジェクト描画.
			for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
				IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
				pModel->RenderDepthPrepass(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
			}
			graphicsController.EndScene(&rt, &scene.GetDepthPrepass());
		}
	}

	// ライトカリング.
	{
		// コマンドリストを使いたいだけなのだが、ひとまず適当なRTへの書き込み扱いにしておく.
		auto& rt = scene.GetTestRT();
		if (graphicsController.BeginScene(&rt)) {
			auto* pShader = shaderMgr.GetLightCullingShader();
			pShader->SetInputDepthRT(&scene.GetDepthPrepass());
			pShader->SetScreenParam(static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
			pShader->SetLightInfo(scene.GetLight(0), scene.GetLightNum());
			pShader->SetCameraInfo(*scene.GetMainCamera());
			graphicsController.EndScene(&rt);
		}
	}

	// 一旦影を書いてみる.
	auto& rt = scene.GetTestRT();
	if (graphicsController.BeginScene(&rt, &scene.GetShadowMap())) {
		// オブジェクト描画.
		for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
			IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
			pModel->RenderShadow(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
		}
		graphicsController.EndScene(&rt, &scene.GetShadowMap());
	}

	IRenderTarget* pList[] = {
		&scene.GetTestRT(),
		&scene.GetColor(),
		&scene.GetNormal(),
		&scene.GetHighBrightness(),
		&scene.GetObjectInfo(),
		&scene.GetSpecular(),
		&scene.GetWorldPos(),
	};
	GAME_COLOR pColor[] = {
		GAME_COLOR::GAME_COLOR_WHITE,
		GAME_COLOR::GAME_COLOR_WHITE,
		GAME_COLOR::GAME_COLOR_BLACK,
		GAME_COLOR::GAME_COLOR_BLACK,
		GAME_COLOR::GAME_COLOR_BLACK,
		GAME_COLOR::GAME_COLOR_BLACK,
		GAME_COLOR::GAME_COLOR_BLACK,
	};
	if (graphicsController.BeginScene(pList, COUNTOF(pList), pColor, &scene.GetDofDepth())) {
		// オブジェクト描画.
		for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
			IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
			pModel->SetDepthStencil(&scene.GetShadowMap());
			pModel->Render(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
		}
		graphicsController.EndScene(pList, COUNTOF(pList), pColor, &scene.GetDofDepth());
	}

	IRenderTarget* pDecalList[] = {
		&scene.GetColor(),
		&scene.GetObjectInfo(),
	};
	GAME_COLOR pDecalColor[] = {
		GAME_COLOR::GAME_COLOR_INVALID,
		GAME_COLOR::GAME_COLOR_INVALID,
	};
	// デカール.
	if (graphicsController.BeginScene(pDecalList, COUNTOF(pDecalList), pDecalColor)) {
		auto* pShader = shaderMgr.Get2DDecalShader();
		pShader->SetShaderInfo(*scene.GetMainCamera());
		pShader->SetInputBaseRT(&scene.GetNormal());
		pShader->SetInputObjInfoRT(&scene.GetObjectInfo());
		pShader->SetInputWorldPosRT(&scene.GetWorldPos());
		pShader->SetInputColorRT(&scene.GetColor());
		C2DDecal* pDecal = static_cast<C2DDecal*>(const_cast<IDecal*>(scene.Get2DDecal()));
		pDecal->SetShader(pShader);
		pDecal->Render(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), &viewPort, &scissor);
		graphicsController.EndScene(pDecalList, COUNTOF(pDecalList), pDecalColor);
	}
}
