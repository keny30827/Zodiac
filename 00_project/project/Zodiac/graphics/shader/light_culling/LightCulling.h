#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CLightCulling : public IShader {
public:
	CLightCulling() {}
	~CLightCulling() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	ID3D12PipelineState* GetPipelineState() override { return m_pPipelineState; }
	ID3D12RootSignature* GetRootSignature() override { return m_pRootSignature; }
	int GetTileLightHeapPosition() const { return m_uavHeapPosition; }

public:
	void SetInputDepthRT(IDepthStencil* pTex) { m_inputDepthRT = pTex; }
	void SetCameraInfo(const ICamera& rCamera)
	{
		SShaderLightInfo* pBuffer = nullptr;
		HRESULT ret = m_cbvResource->Map(0, nullptr, (void**)(&pBuffer));
		if (ret == S_OK) {
			pBuffer->view = rCamera.GetViewMatrix();
			pBuffer->proj = rCamera.GetPerspectiveMatrix();
			pBuffer->projInv = XMMatrixInverse(nullptr, rCamera.GetPerspectiveMatrix());
			m_cbvResource->Unmap(0, nullptr);
		}
	}
	void SetLightInfo(const SLightInfo* pInfo, const int infoSize)
	{
		SShaderLightInfo* pBuffer = nullptr;
		HRESULT ret = m_cbvResource->Map(0, nullptr, (void**)(&pBuffer));
		if (ret == S_OK) {
			for (int n = 0; n < infoSize; n++) {
				pBuffer->light[n] = pInfo[n];
			}
			pBuffer->lightNum = infoSize;
			m_cbvResource->Unmap(0, nullptr);
		}
	}
	void SetScreenParam(const float w, const float h)
	{
		SShaderLightInfo* pBuffer = nullptr;
		HRESULT ret = m_cbvResource->Map(0, nullptr, (void**)(&pBuffer));
		if (ret == S_OK) {
			pBuffer->screenParam.x = w;
			pBuffer->screenParam.y = h;
			m_cbvResource->Unmap(0, nullptr);
		}
		m_screenParam.x = w;
		m_screenParam.y = h;
	}

#if defined(DEBUG)
public:
	void DB_DispLightInfo();
#endif

private:
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	
	ID3DBlob* m_pComputeShader = nullptr;

	ID3D12Resource* m_uavResource = nullptr;
	int m_uavHeapPosition = -1;

	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	IDepthStencil* m_inputDepthRT = nullptr;

	DirectX::XMFLOAT2 m_screenParam = DirectX::XMFLOAT2(0.0f, 0.0f);
};