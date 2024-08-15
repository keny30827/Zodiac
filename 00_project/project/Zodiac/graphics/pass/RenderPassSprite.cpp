#include "RenderPassSprite.h"
#include "../GraphicsResourceManager.h"
#include "../GraphicsController.h"
#include "../shader/ShaderManager.h"

#define ENABLE_GAUSSIAN_HIGH_BRIGHT
#define ENABLE_GAUSSIAN_DOF
#define ENABLE_SSAO

bool CRenderPassSprite::Init(CGraphicsController& graphicsController)
{
	IDXGISwapChain4* pSwapChain = graphicsController.GetSwapChain();
	VRETURN_RET(pSwapChain, false);
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	HRESULT ret = pSwapChain->GetDesc1(&scDesc);
	VRETURN_RET(ret == S_OK, false);

	m_gaussian.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_colorSprite.Init(graphicsController, 0.0f, 0.0f, 150.0f, 150.0f);
	m_normalSprite.Init(graphicsController, 0.0f, 150.0f, 150.0f, 150.0f);
	m_highBright.Init(graphicsController, 0.0f, 300.0f, 150.0f, 150.0f);
	m_bloom.Init(graphicsController, 0.0f, 450.0f, 150.0f, 150.0f);
	m_dof.Init(graphicsController, 150.0f, 0.0f, 150.0f, 150.0f);
	m_ssao.Init(graphicsController, 300.0f, 0.0f, 150.0f, 150.0f);

	m_postEffectBuffer.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	return true;
}

void CRenderPassSprite::Term()
{
	m_appRenderTailCallback = nullptr;
	m_postEffectBuffer.Term();
}

void CRenderPassSprite::Render(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr)
{
	// UI用パス.
	// バックバッファへの書き出し.

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

#if defined(ENABLE_GAUSSIAN_HIGH_BRIGHT)
	RenderBloom(scene, graphicsController, shaderMgr, viewPort, scissor);
#endif

#if defined(ENABLE_GAUSSIAN_DOF)
	RenderDof(scene, graphicsController, shaderMgr, viewPort, scissor);
#endif

#if defined(ENABLE_SSAO)
	RenderSsao(scene, graphicsController, shaderMgr, viewPort, scissor, scene.GetMainCamera());
#endif

	// バックバッファに書き込み.
	if (graphicsController.BeginScene(true)) {
		auto* pShader = shaderMgr.GetBasicSpriteShader();
#if defined(ENABLE_GBUFFER_TEST)
		{
			ISprite* pSprite = const_cast<ISprite*>(scene.GetFrameBuffer());
			pSprite->SetRenderTarget(&scene.GetColor());
			pSprite->SetNormalRenderTarget(&scene.GetNormal());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#else
		{
			pShader->SetInputBaseRT(&scene.GetColor());
			m_colorSprite.SetShader(pShader);
			m_colorSprite.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
		{
			pShader->SetInputBaseRT(&scene.GetNormal());
			m_normalSprite.SetShader(pShader);
			m_normalSprite.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
#if defined(ENABLE_GAUSSIAN_HIGH_BRIGHT)
		{
			pShader->SetInputBaseRT(&scene.GetHighBrightnessShrinkBuffer());
			m_highBright.SetShader(pShader);
			m_highBright.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
#endif
#if defined(ENABLE_GAUSSIAN_DOF)
		{
			pShader->SetInputBaseRT(&scene.GetDofShrinkBuffer());
			m_dof.SetShader(pShader);
			m_dof.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
#endif
#if defined(ENABLE_SSAO)
		{
			pShader->SetInputBaseRT(&scene.GetSsao());
			m_ssao.SetShader(pShader);
			m_ssao.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
#endif
		{
			pShader->SetInputBaseRT(&scene.GetTestRT());
			m_ssao.SetShader(pShader);
			m_ssao.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
		}
#endif

		// TODO 新描画設計テスト中.
		// ここで、ディファードでSSAO込みの絵を書く.
		{
			auto* pDeferredShader = shaderMgr.GetDeferredRenderShader();
			pDeferredShader->SetInputGBufferColor(&scene.GetColor());
			pDeferredShader->SetInputGBufferNormal(&scene.GetNormal());
			pDeferredShader->SetInputGBufferSSAO(&scene.GetSsao());
			pDeferredShader->SetInputGBufferObjectInfo(&scene.GetObjectInfo());
			pDeferredShader->SetInputGBufferSpecular(&scene.GetSpecular());
			pDeferredShader->SetInputGBufferWorldPos(&scene.GetWorldPos());
			pDeferredShader->SetInputEye(scene.GetMainCamera()->GetEye());
			if (auto* pLightCullingShader = shaderMgr.GetLightCullingShader()) {
				pDeferredShader->SetInputTileLight(pLightCullingShader->GetTileLightHeapPosition());
			}
			pDeferredShader->SetScreenParam(static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
			pDeferredShader->SetLightInfo(scene.GetLight(0), scene.GetLightNum());
			CSprite* pSprite = static_cast<CSprite*>(const_cast<ISprite*>(scene.GetFrameBuffer()));
			pSprite->SetShader(pDeferredShader);
			pSprite->Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
	}
#if 0
		ISprite* pSprite = const_cast<ISprite*>(scene.GetFrameBuffer());
		pSprite->SetRenderTarget(&scene.GetDof());
		pSprite->EnableSsao(false);
		pSprite->Render(graphicsController, &viewPort, &scissor);
#endif

		if (m_appRenderTailCallback) {
			(m_appRenderTailCallback)(&graphicsController);
		}

		graphicsController.EndScene();
	}
}

void CRenderPassSprite::RenderBloom(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor)
{
	// 大本の１枚目にガウシアンかける.
	{
		auto* pShader = shaderMgr.GetGaussianBlurShader();
		{
			auto& rt = scene.GetGaussian1RT();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputTexture(&scene.GetHighBrightness());
				pShader->EnableGaussianX();
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
		{
			auto& rt = scene.GetGaussian2RT();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputTexture(&scene.GetGaussian1RT());
				pShader->EnableGaussianY();
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
	}
	// ぼかしの縮小バッファを作成.
	{
		auto* pShader = shaderMgr.GetBasicSpriteShader();
		auto& rt = scene.GetHighBrightnessShrinkBuffer();
		if (graphicsController.BeginScene(&rt)) {
			pShader->SetInputBaseRT(&scene.GetGaussian2RT());
			m_postEffectBuffer.SetShader(pShader);
			m_postEffectBuffer.SetDownSample(true);
			m_postEffectBuffer.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
			m_postEffectBuffer.SetDownSample(false);
			graphicsController.EndScene(&rt);
		}
	}
	// 作成したぼかし２枚を使って、ブルームをかけたスプライトを作成.
	{
		auto* pShader = shaderMgr.GetBloomShader();
		{
			auto& rt = scene.GetBloom();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputBaseRT(&scene.GetTestRT());
				pShader->SetInputBloomMipTopRT(&scene.GetGaussian2RT());
				pShader->SetInputBloomMipOtherRT(&scene.GetHighBrightnessShrinkBuffer());
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
	}
}

void CRenderPassSprite::RenderDof(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor)
{
	// 大本の１枚目にガウシアンかける.
	{
		auto* pShader = shaderMgr.GetGaussianBlurShader();
		{
			auto& rt = scene.GetGaussian1RT();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputTexture(&scene.GetTestRT());
				pShader->EnableGaussianX();
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
		{
			auto& rt = scene.GetGaussian2RT();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputTexture(&scene.GetGaussian1RT());
				pShader->EnableGaussianY();
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
	}
	// ぼかしの縮小バッファを作成.
	{
		auto* pShader = shaderMgr.GetBasicSpriteShader();
		auto& rt = scene.GetDofShrinkBuffer();
		if (graphicsController.BeginScene(&rt)) {
			pShader->SetInputBaseRT(&scene.GetGaussian2RT());
			m_postEffectBuffer.SetShader(pShader);
			m_postEffectBuffer.SetDownSample(true);
			m_postEffectBuffer.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
			m_postEffectBuffer.SetDownSample(false);
			graphicsController.EndScene(&rt);
		}
	}
	// 作成したぼかし２枚を使って、ブルームをかけたスプライトを作成.
	{
		auto* pShader = shaderMgr.GetDepthOfViewShader();
		{
			auto& rt = scene.GetDof();
			if (graphicsController.BeginScene(&rt)) {
				pShader->SetInputBaseRT(&scene.GetTestRT());
				pShader->SetInputBlurMipTopRT(&scene.GetGaussian2RT());
				pShader->SetInputBlurMipOtherRT(&scene.GetDofShrinkBuffer());
				pShader->SetInputDepthRT(&scene.GetDofDepth());
				m_postEffectBuffer.SetShader(pShader);
				m_postEffectBuffer.Render(
					graphicsController.GetCommandWrapper(),
					graphicsController.GetHeapWrapper(),
					&viewPort,
					&scissor);
				graphicsController.EndScene(&rt);
			}
		}
	}
}

void CRenderPassSprite::RenderSsao(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor, const ICamera* pCamera)
{
	{
		auto* pShader = shaderMgr.GetSSAOShader();
		auto& rt = scene.GetSsao();
		if (graphicsController.BeginScene(&rt)) {
			pShader->SetInputDepthRT(&scene.GetDofDepth());
			pShader->SetInputNormalRT(&scene.GetNormal());
			pShader->SetCameraInfo(*pCamera);
			m_postEffectBuffer.SetShader(pShader);
			m_postEffectBuffer.Render(
				graphicsController.GetCommandWrapper(),
				graphicsController.GetHeapWrapper(),
				&viewPort,
				&scissor);
			graphicsController.EndScene(&rt);
		}
	}
}
