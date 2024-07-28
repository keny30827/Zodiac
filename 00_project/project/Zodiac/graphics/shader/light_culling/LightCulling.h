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

public:
	void SetLightInfo(SLightInfo* pInfo, const int infoSize) {
		RETURN((m_lightInfo != pInfo) || (m_lightNum != infoSize));
		m_lightInfo = pInfo;
		m_lightNum = infoSize;
		// TODO GPUリソースへの書き込み.
	}

private:
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	
	ID3DBlob* m_pComputeShader = nullptr;

	ID3D12Resource* m_uavResource = nullptr;
	int m_uavHeapPosition = -1;

	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	// コンスタントバッファーで渡す中身.
	SLightInfo* m_lightInfo = nullptr;
	int m_lightNum = 0;
};