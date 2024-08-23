#include "RenderPassTest.h"
#include "../GraphicsResourceManager.h"
#include "../GraphicsController.h"
#include "../shader/ShaderManager.h"

#include "../../object/decal/Decal.h"
#include "../../object/plane/Plane.h"

void CRenderPassTest::Render(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr)
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

	// �f�v�X�v���p�X.
	{
		auto& rt = scene.GetTestRT();
		if (graphicsController.BeginScene(&rt, &scene.GetDepthPrepass())) {
			// �I�u�W�F�N�g�`��.
			for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
				IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
				pModel->RenderDepthPrepass(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
			}
			graphicsController.EndScene(&rt, &scene.GetDepthPrepass());
		}
	}

	// ���C�g�J�����O.
	{
		// �R�}���h���X�g���g�����������Ȃ̂����A�ЂƂ܂��K����RT�ւ̏������݈����ɂ��Ă���.
		auto& rt = scene.GetTestRT();
		if (graphicsController.BeginScene(&rt)) {
			auto* pShader = shaderMgr.GetLightCullingShader();
			pShader->SetInputDepthRT(&scene.GetDepthPrepass());
			pShader->SetScreenParam(static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
			pShader->SetLightInfo(scene.GetLight(0), scene.GetLightNum());
			pShader->SetCameraInfo(*scene.GetMainCamera());
			pShader->RenderSetup(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper());
			graphicsController.EndScene(&rt);
		}
#if 0
		if (auto* pShader = shaderMgr.GetLightCullingShader()) {
			pShader->DB_DispLightInfo();
		}
#endif
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

	// �I�u�W�F�N�g�`��.
	{
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
			for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
				IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
				pModel->SetDepthStencil(&scene.GetShadowMap());
				pModel->Render(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
			}
			graphicsController.EndScene(pList, COUNTOF(pList), pColor, &scene.GetDofDepth());
		}
	}

	// ����.
	{
		IRenderTarget* pList[] = {
			&scene.GetTestRT(),
			&scene.GetColor(),
			&scene.GetNormal(),
			&scene.GetHighBrightness(),
			&scene.GetObjectInfo(),
			&scene.GetSpecular(),
		};
		GAME_COLOR pColor[] = {
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
		};
		if (graphicsController.BeginScene(pList, COUNTOF(pList), pColor, &scene.GetDepthPrepass(), false)) {
			for (uint32_t n = 0; n < scene.GetPlaneNum(); n++) {
				auto* pShader = shaderMgr.GetSSRShader();
				pShader->SetCameraInfo(*scene.GetMainCamera());
				pShader->SetWindowInfo(static_cast<float>(scDesc.Width), static_cast<float>(scDesc.Height));
				pShader->SetInputDepthRT(&scene.GetDepthPrepass());
				pShader->SetInputBaseRT(&scene.GetColor());
				pShader->SetInputNormalRT(&scene.GetNormal());
				pShader->SetInputObjectInfoRT(&scene.GetObjectInfo());
				pShader->SetInputSpecularRT(&scene.GetSpecular());
				C3DPlane* pPlane = static_cast<C3DPlane*>(const_cast<IPlane*>(scene.GetPlane(n)));
				pPlane->SetShader(pShader);
				pPlane->Render(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), &viewPort, &scissor);
			}
			graphicsController.EndScene(pList, COUNTOF(pList), pColor, &scene.GetDepthPrepass());
		}
	}

	// �A�E�g���C��.
	// �w�ʖ@�Ȃ̂Ń��f���`��.
	{
		IRenderTarget* pOutlineList[] = {
			&scene.GetColor(),
			&scene.GetObjectInfo(),
		};
		GAME_COLOR pOutlineColor[] = {
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
		};
		if (graphicsController.BeginScene(pOutlineList, COUNTOF(pOutlineList), pOutlineColor, &scene.GetDepthPrepass(), false)) {
			for (uint32_t n = 0; n < scene.GetModelNum(); n++) {
				IModel* pModel = const_cast<IModel*>(scene.GetModel(n));
				pModel->SetDepthStencil(&scene.GetShadowMap());
				pModel->RenderOutline(graphicsController.GetCommandWrapper(), graphicsController.GetHeapWrapper(), *(scene.GetMainCamera()), &viewPort, &scissor);
			}
			graphicsController.EndScene(pOutlineList, COUNTOF(pOutlineList), pOutlineColor, &scene.GetDepthPrepass());
		}
	}

	// �f�J�[��.
	{
		IRenderTarget* pDecalList[] = {
			&scene.GetColor(),
			&scene.GetObjectInfo(),
		};
		GAME_COLOR pDecalColor[] = {
			GAME_COLOR::GAME_COLOR_INVALID,
			GAME_COLOR::GAME_COLOR_INVALID,
		};
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
}
