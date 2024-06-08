#include "RenderPassTest.h"
#include "../GraphicsResourceManager.h"
#include "../GraphicsController.h"

void CRenderPassTest::Render(CScene& scene, CGraphicsController& graphicsController)
{
	// �e�X�g�p�p�X.
	// ���f�����e�X�g�p�����_�[�^�[�Q�b�g�ɏ����o��.

	// �K�v�ȏ��̎擾.���߂Ȃ�ł��؂�.
	IDXGISwapChain4* pSwapChain = graphicsController.GetSwapChain();
	VRETURN(pSwapChain);
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	HRESULT ret = pSwapChain->GetDesc1(&scDesc);
	VRETURN(ret == S_OK);
	
	// �r���[�|�[�g�ƃV�U�����O���.
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

	// ��U�e�������Ă݂�.
	auto& rt = scene.GetTestRT();
	if (graphicsController.BeginScene(&rt, &scene.GetShadowMap())) {
		// �I�u�W�F�N�g�`��.
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
	};
	GAME_COLOR pColor[] = {
		GAME_COLOR::GAME_COLOR_WHITE,
		GAME_COLOR::GAME_COLOR_WHITE,
		GAME_COLOR::GAME_COLOR_WHITE,
		GAME_COLOR::GAME_COLOR_BLACK,
	};
	if (graphicsController.BeginScene(pList, COUNTOF(pList), pColor, &scene.GetDofDepth())) {
		// �I�u�W�F�N�g�`��.
		for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
			IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
			pModel->SetDepthStencil(&scene.GetShadowMap());
			pModel->Render(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
		}
		graphicsController.EndScene(pList, COUNTOF(pList), pColor, &scene.GetDofDepth());
	}
}