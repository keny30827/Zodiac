#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CDeferredRender : public IShader {
public:
	CDeferredRender() {}
	~CDeferredRender() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputGBufferColor(IRenderTarget* pTex) { m_inputGBufferColor = pTex; }
	void SetInputGBufferNormal(IRenderTarget* pTex) { m_inputGBufferNormal = pTex; }
	void SetInputGBufferSSAO(IRenderTarget* pTex) { m_inputGBufferSSAO = pTex; }
	void SetInputGBufferObjectInfo(IRenderTarget* pTex) { m_inputGBufferObjectInfo = pTex; }
	void SetInputGBufferSpecular(IRenderTarget* pTex) { m_inputGBufferSpecular = pTex; }

	// TODO とりあえず.
	void SetInputEye(const DirectX::XMFLOAT3& eye) 
	{
		m_sceneInfo.eye = eye;
		SShaderSpriteInfo* pBuffer = nullptr;
		HRESULT ret = m_cbvResource->Map(0, nullptr, (void**)(&pBuffer));
		if (ret == S_OK) {
			(*pBuffer) = m_sceneInfo;
			m_cbvResource->Unmap(0, nullptr);
		}
	}

public:
	ID3D12PipelineState* GetPipelineState() override { return m_pPipelineState; }
	ID3D12RootSignature* GetRootSignature() override { return m_pRootSignature; }

private:
	// ガウシアンブラーを行うための事前準備情報.
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	
	ID3DBlob* m_pVertexShader = nullptr;
	ID3DBlob* m_pPixelShader = nullptr;

	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	// コンスタントバッファーで渡す中身.
	SShaderSpriteInfo m_sceneInfo = SShaderSpriteInfo();

	// 入力画像.
	IRenderTarget* m_inputGBufferColor = nullptr;
	IRenderTarget* m_inputGBufferNormal = nullptr;
	IRenderTarget* m_inputGBufferSSAO = nullptr;
	IRenderTarget* m_inputGBufferObjectInfo = nullptr;
	IRenderTarget* m_inputGBufferSpecular = nullptr;
};