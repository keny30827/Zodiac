#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CBasicModel : public IShader {
public:
	CBasicModel() {}
	~CBasicModel() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
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
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	
	ID3DBlob* m_pVertexShader = nullptr;

	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	// コンスタントバッファーで渡す中身.
	SShaderPlaneInfo m_sceneInfo = SShaderPlaneInfo();
};