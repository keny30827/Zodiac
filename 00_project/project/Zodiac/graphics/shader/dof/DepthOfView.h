#pragma once

#include "../../GraphicsDefs.h"

/*
* シェーダーの性質上、ブラーをかけたRT、またそれをダウンサンプルしてミップマップ化したRTは他で作られる前提.
*/

class CGraphicsController;
class CDepthOfView : public IShader {
public:
	CDepthOfView() {}
	~CDepthOfView() {}

public:
	void Init(CGraphicsController& graphicsController);
	void Term();
	void Update() override;
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) override;

public:
	void SetInputBaseRT(IRenderTarget* pTex) { m_inputBaseRT = pTex; }
	void SetInputBlurMipTopRT(IRenderTarget* pTex) { m_inputBlurMipTopRT = pTex; }
	void SetInputBlurMipOtherRT(IRenderTarget* pTex) { m_inputBlurMipOtherRT = pTex; }
	void SetInputDepthRT(IRenderTarget* pTex) { m_inputDepthRT = pTex; }

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
	IRenderTarget* m_inputBaseRT = nullptr;
	IRenderTarget* m_inputBlurMipTopRT = nullptr;
	IRenderTarget* m_inputBlurMipOtherRT = nullptr;
	IRenderTarget* m_inputDepthRT = nullptr;
};