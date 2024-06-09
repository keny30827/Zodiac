#pragma once

#include "../../GraphicsDefs.h"

/*
* �V�F�[�_�[�̐�����A���P�x�l�𔲂��o����RT�A�܂�������_�E���T���v�����ă~�b�v�}�b�v������RT�͑��ō����O��.
*/

class CGraphicsController;
class CBloom : public IShader {
public:
	CBloom() {}
	~CBloom() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputBaseRT(IRenderTarget* pTex) { m_inputBaseRT = pTex; }
	void SetInputBloomMipTopRT(IRenderTarget* pTex) { m_inputBloomMipTopRT = pTex; }
	void SetInputBloomMipOtherRT(IRenderTarget* pTex) { m_inputBloomMipOtherRT = pTex; }

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
	IRenderTarget* m_inputBaseRT = nullptr;
	IRenderTarget* m_inputBloomMipTopRT = nullptr;
	IRenderTarget* m_inputBloomMipOtherRT = nullptr;
};