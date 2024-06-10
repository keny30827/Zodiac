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

	m_gaussian1.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_gaussian2.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_colorSprite.Init(graphicsController, 0.0f, 0.0f, 150.0f, 150.0f);
	m_normalSprite.Init(graphicsController, 0.0f, 150.0f, 150.0f, 150.0f);
	m_highBright.Init(graphicsController, 0.0f, 300.0f, 150.0f, 150.0f);
	m_highBrightShrinkBufferForDisp.Init(graphicsController, 0.0f, 450.0f, 150.0f, 150.0f);
	// 縮小バッファに書き込む用.ウィンドウサイズ分で作成して、ビューポートでサイズを調整して描画する.
	m_highBrightShrinkBuffer.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_bloom.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_dofShrinkBufferForDisp.Init(graphicsController, 150.0f, 0.0f, 150.0f, 150.0f);
	// 縮小バッファに書き込む用.ウィンドウサイズ分で作成して、ビューポートでサイズを調整して描画する.
	m_dofShrinkBuffer.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_dof.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_ssao.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	m_ssaoForDisp.Init(graphicsController, 300.0f, 0.0f, 150.0f, 150.0f);

	m_postEffectBuffer.Init(graphicsController, 0.0f, 0.0f, static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
	return true;
}

void CRenderPassSprite::Term()
{
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
	RenderDof(scene, graphicsController, viewPort, scissor);
#endif

#if defined(ENABLE_SSAO)
	RenderSsao(scene, graphicsController, viewPort, scissor, scene.GetMainCamera());
#endif

	// バックバッファに書き込み.
	if (graphicsController.BeginScene(true)) {
#if defined(ENABLE_GBUFFER_TEST)
		{
			ISprite* pSprite = const_cast<ISprite*>(scene.GetFrameBuffer());
			pSprite->SetRenderTarget(&scene.GetColor());
			pSprite->SetNormalRenderTarget(&scene.GetNormal());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#else
		{
			ISprite* pSprite = &m_colorSprite;
			pSprite->SetRenderTarget(&scene.GetColor());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
		{
			ISprite* pSprite = &m_normalSprite;
			scene.GetNormal().SetAccessRootParam(ACCESS_ROOT_PARAM_RENDER_TARGET_SHADER_VIEW);
			pSprite->SetRenderTarget(&scene.GetNormal());
			pSprite->Render(graphicsController, &viewPort, &scissor);
			scene.GetNormal().SetAccessRootParam(ACCESS_ROOT_PARAM_G_BUF_NORMAL);
		}
#if defined(ENABLE_GAUSSIAN_HIGH_BRIGHT)
		{
			ISprite* pSprite = &m_highBright;
			pSprite->SetRenderTarget(&scene.GetGaussian2RT());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
		{
			ISprite* pSprite = &m_highBrightShrinkBufferForDisp;
			pSprite->SetRenderTarget(&scene.GetHighBrightnessShrinkBuffer());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#endif
#if defined(ENABLE_GAUSSIAN_DOF)
		{
			ISprite* pSprite = &m_dofShrinkBufferForDisp;
			pSprite->SetRenderTarget(&scene.GetDofShrinkBuffer());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#endif
#if defined(ENABLE_SSAO)
		{
			ISprite* pSprite = &m_ssaoForDisp;
			pSprite->SetRenderTarget(&scene.GetSsao());
			pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#endif
		{
			ISprite* pSprite = const_cast<ISprite*>(scene.GetFrameBuffer());
			pSprite->SetRenderTarget(&scene.GetTestRT());
			pSprite->EnableSsao(true);
			pSprite->SetSSAORenderTarget(&scene.GetSsao());
			// pSprite->SetRenderTarget(&scene.GetDof());
			// pSprite->SetDepthStencil(&scene.GetShadowMap());
			//pSprite->Render(graphicsController, &viewPort, &scissor);
		}
#endif

		// TODO 新描画設計テスト中.
		ISprite* pSprite = const_cast<ISprite*>(scene.GetFrameBuffer());
		pSprite->SetRenderTarget(&scene.GetBloom());
		pSprite->EnableSsao(false);
		pSprite->Render(graphicsController, &viewPort, &scissor);

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
		auto& rt = scene.GetHighBrightnessShrinkBuffer();
		if (graphicsController.BeginScene(&rt)) {
			m_highBrightShrinkBuffer.SetRenderTarget(&scene.GetGaussian2RT());
			m_highBrightShrinkBuffer.DisableGaussian();
			m_highBrightShrinkBuffer.RenderShrinkBuffer(graphicsController, &viewPort, &scissor);
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

void CRenderPassSprite::RenderDof(CScene& scene, CGraphicsController& graphicsController, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor)
{
	// DOF用のぼかし１枚目.
	{
		auto& rt = scene.GetGaussian1RT();
		if (graphicsController.BeginScene(&rt)) {
			m_gaussian1.SetRenderTarget(&scene.GetTestRT());
			m_gaussian1.EnableGaussianX();
			m_gaussian1.Render(graphicsController, &viewPort, &scissor);
			graphicsController.EndScene(&rt);
		}
	}
	{
		auto& rt = scene.GetGaussian2RT();
		if (graphicsController.BeginScene(&rt)) {
			m_gaussian2.SetRenderTarget(&scene.GetGaussian1RT());
			m_gaussian2.DisableGaussian();
			m_gaussian2.EnableGaussianY();
			m_gaussian2.Render(graphicsController, &viewPort, &scissor);
			graphicsController.EndScene(&rt);
		}
	}
	// ぼかしの縮小バッファを作成.
	{
		auto& rt = scene.GetDofShrinkBuffer();
		if (graphicsController.BeginScene(&rt)) {
			m_dofShrinkBuffer.SetRenderTarget(&scene.GetGaussian2RT());
			m_dofShrinkBuffer.DisableGaussian();
			m_dofShrinkBuffer.RenderShrinkBuffer(graphicsController, &viewPort, &scissor);
			graphicsController.EndScene(&rt);
		}
	}
	// 作成したぼかし２枚を使って、ブルームをかけたスプライトを作成.
	{
		auto& rt = scene.GetDof();
		if (graphicsController.BeginScene(&rt)) {
			m_dof.EnableDof();
			m_dof.SetDepthStencil(&scene.GetDofDepth());
			m_dof.SetRenderTarget(&scene.GetTestRT());
			m_dof.SetDofRenderTarget(&scene.GetGaussian2RT());
			m_dof.SetDofShrinkRenderTarget(&scene.GetDofShrinkBuffer());
			m_dof.Render(graphicsController, &viewPort, &scissor);
			graphicsController.EndScene(&rt);
		}
	}
}

void CRenderPassSprite::RenderSsao(CScene& scene, CGraphicsController& graphicsController, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor, const ICamera* pCamera)
{
	{
		auto& rt = scene.GetSsao();
		if (graphicsController.BeginScene(&rt)) {
			m_ssao.SetDepthStencil(&scene.GetDofDepth());
			m_ssao.SetNormalRenderTarget(&scene.GetNormal());
			m_ssao.RenderSSAO(graphicsController, *pCamera, &viewPort, &scissor);
			graphicsController.EndScene(&rt);
		}
	}
}
