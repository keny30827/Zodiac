#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class C2DDecalShader : public IShader {
public:
	C2DDecalShader() {}
	~C2DDecalShader() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputBaseRT(IRenderTarget* pTex) { m_inputBaseRT = pTex; }
	void SetInputObjInfoRT(IRenderTarget* pTex) { m_inputObjInfoRT = pTex; }
	void SetInputWorldPosRT(IRenderTarget* pTex) { m_inputWorldPosRT = pTex; }
	void SetInputColorRT(IRenderTarget* pTex) { m_inputColorRT = pTex; }
	void SetShaderInfo(const ICamera& rCamera)
	{
		m_sceneInfo = SShaderDecalInfo();
		m_sceneInfo.view *= rCamera.GetViewMatrix();
		m_sceneInfo.proj *= rCamera.GetPerspectiveMatrix();
		SShaderDecalInfo* pBuffer = nullptr;
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
	SShaderDecalInfo m_sceneInfo = SShaderDecalInfo();

	// 入力画像.
	IRenderTarget* m_inputBaseRT = nullptr;
	IRenderTarget* m_inputObjInfoRT = nullptr;
	IRenderTarget* m_inputWorldPosRT = nullptr;
	IRenderTarget* m_inputColorRT = nullptr;
};