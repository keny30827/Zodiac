#pragma once

#include "../../GraphicsDefs.h"

class CGraphicsController;
class CGaussianBlur : public IShader {
public:
	CGaussianBlur() {}
	~CGaussianBlur() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void EnableGaussianX()
	{
		m_info.isEnableX = 1.0f;
		m_info.isEnableY = 0.0f;
	}
	void EnableGaussianY()
	{
		m_info.isEnableX = 0.0f;
		m_info.isEnableY = 1.0f;
	}
	void DisableGaussian()
	{
		m_info.isEnableX = 0.0f;
		m_info.isEnableY = 0.0f;
	}

public:
	ID3D12PipelineState* GetPipelineState() override { return m_pPipelineState; }
	ID3D12RootSignature* GetRootSignature() override { return m_pRootSignature; }

private:
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	ID3DBlob* m_pVertexShader = nullptr;
	ID3DBlob* m_pPixelShader = nullptr;
	ID3D12Resource* m_cbvResource = nullptr;
	int m_cbvHeapPosition = -1;

	// コンスタントバッファーで渡す中身.
	SShaderGaussianInfo m_info = SShaderGaussianInfo();
};