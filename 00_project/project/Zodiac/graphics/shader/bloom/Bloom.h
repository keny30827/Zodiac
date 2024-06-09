#pragma once

#include "../../GraphicsDefs.h"

/*
* シェーダーの性質上、一定輝度値を抜き出したRT、またそれをダウンサンプルしてミップマップ化したRTは他で作られる前提.
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
	IRenderTarget* m_inputBloomMipTopRT = nullptr;
	IRenderTarget* m_inputBloomMipOtherRT = nullptr;
};