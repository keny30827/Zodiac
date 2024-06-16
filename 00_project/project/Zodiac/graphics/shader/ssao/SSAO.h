#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CSSAO : public IShader {
public:
	CSSAO() {}
	~CSSAO() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputNormalRT(IRenderTarget* pTex) { m_inputNormalRT = pTex; }
	void SetInputDepthRT(IDepthStencil* pTex) { m_inputDepthRT = pTex; }

public:
	ID3D12PipelineState* GetPipelineState() override { return m_pPipelineState; }
	ID3D12RootSignature* GetRootSignature() override { return m_pRootSignature; }

private:
	// �K�E�V�A���u���[���s�����߂̎��O�������.
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	
	ID3DBlob* m_pVertexShader = nullptr;
	ID3DBlob* m_pPixelShader = nullptr;

	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	// �R���X�^���g�o�b�t�@�[�œn�����g.
	SShaderSpriteInfo m_sceneInfo = SShaderSpriteInfo();

	// ���͉摜.
	IRenderTarget* m_inputNormalRT = nullptr;
	IDepthStencil* m_inputDepthRT = nullptr;
};