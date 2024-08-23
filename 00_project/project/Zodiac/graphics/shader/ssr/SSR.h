#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CSSR : public IShader {
public:
	CSSR() {}
	~CSSR() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputDepthRT(IDepthStencil* pTex) { m_inputDepthRT = pTex; }

	void SetInputBaseRT(IRenderTarget* pTex) { m_inputBaseRT = pTex; }
	void SetInputNormalRT(IRenderTarget* pTex) { m_inputNormalRT = pTex; }
	void SetInputObjectInfoRT(IRenderTarget* pTex) { m_inputObjectInfoRT = pTex; }
	void SetInputSpecularRT(IRenderTarget* pTex) { m_inputSpecularRT = pTex; }

	void SetCameraInfo(const ICamera& rCamera)
	{
		m_sceneInfo.eye = rCamera.GetEye();
		m_sceneInfo.view = rCamera.GetViewMatrix();
		m_sceneInfo.viewInv = XMMatrixInverse(nullptr, rCamera.GetViewMatrix());
		m_sceneInfo.proj = rCamera.GetPerspectiveMatrix();
		m_sceneInfo.projInv = XMMatrixInverse(nullptr, rCamera.GetPerspectiveMatrix());
		SShaderPlaneInfo* pBuffer = nullptr;
		HRESULT ret = m_cbvResource->Map(0, nullptr, (void**)(&pBuffer));
		if (ret == S_OK) {
			(*pBuffer) = m_sceneInfo;
			m_cbvResource->Unmap(0, nullptr);
		}
	}
	void SetWindowInfo(const float w, const float h)
	{
		m_sceneInfo.screenParam.x = w;
		m_sceneInfo.screenParam.y = h;
		SShaderPlaneInfo* pBuffer = nullptr;
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
	SShaderPlaneInfo m_sceneInfo = SShaderPlaneInfo();

	// 入力画像.
	IDepthStencil* m_inputDepthRT = nullptr;
	IRenderTarget* m_inputBaseRT = nullptr;
	IRenderTarget* m_inputNormalRT = nullptr;
	IRenderTarget* m_inputObjectInfoRT = nullptr;
	IRenderTarget* m_inputSpecularRT = nullptr;
};