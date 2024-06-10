#pragma once

#include "../../graphics/GraphicsDefs.h"
#include "../render_target/Texture.h"

class CGraphicsController;
class CScene;

class CQuad
{
private:
	struct SVertex {
		DirectX::XMFLOAT3 pos = {};	//px.
		DirectX::XMFLOAT2 uv = {};
		int GetDataSize()
		{
			return sizeof(decltype(pos)) + sizeof(decltype(uv));
		}
	};

public:
	CQuad() = default;
	virtual ~CQuad() {};

public:
	void Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h);
	void Term();
	void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper);

public:
	ID3D12Resource* GetVertexBufferResource()
	{
		return m_pVertexBufferResource;
	}
	const D3D12_VERTEX_BUFFER_VIEW GetVertexBufferViewInfo() const
	{
		return m_vertexBufferViewInfo;
	}
	ID3D12Resource* GetIndexBufferResource()
	{
		return m_pIndexBufferResource;
	}
	const D3D12_INDEX_BUFFER_VIEW GetIndexBufferViewInfo() const
	{
		return m_indexBufferViewInfo;
	}
	int GetIndexNum() const { return static_cast<int>(m_indexList.size()); }

private:
	bool BuildVertexBuffer(CGraphicsController& graphicsController);
	bool BuildIndexBuffer(CGraphicsController& graphicsController);

	void MapVertexData();
	void MapIndexData();

	void AddVertex(const SVertex& vertex) { m_vertexList.push_back(vertex); }
	void AddIndex(const uint16_t index) { m_indexList.push_back(index); }

private:
	std::vector<SVertex> m_vertexList = {};
	std::vector<uint16_t> m_indexList = {};

	// GPUリソース関連.
	ID3D12Resource* m_pVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViewInfo = {};
	ID3D12Resource* m_pIndexBufferResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferViewInfo = {};
};

class CSpriteNew : public ISpriteNew
{
public:
	CSpriteNew() = default;
	virtual ~CSpriteNew() {}

public:
	void Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h);
	void Term();

public:
	virtual void Update() override;
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;

public:
	void SetShader(IShader* pShader) { m_pShader = pShader; }
	void SetDownSample(bool b) { m_isDownSample = b; };

private:
	DirectX::XMFLOAT2 m_pos = DirectX::XMFLOAT2();
	DirectX::XMFLOAT2 m_size = DirectX::XMFLOAT2();

	CQuad m_quad;
	IShader* m_pShader = nullptr;
	bool m_isDownSample = false;
};

class CSprite : public ISprite {
private:
	struct SVertex {
		DirectX::XMFLOAT3 pos = {};	//px.
		DirectX::XMFLOAT2 uv = {};
		int GetDataSize()
		{
			return sizeof(decltype(pos)) + sizeof(decltype(uv));
		}
	};

public:
	CSprite() = default;
	virtual ~CSprite();

public:
	bool Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h);

public:
	void Update();
	virtual void Render(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderShrinkBuffer(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderBloom(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderSSAO(CGraphicsController& graphicsController, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;

private:
	void MapVertexData();
	void MapIndexData();
	void MapGaussianData();
	void MapShaderInfo(CGraphicsController& graphicsController, const ICamera* pCamera = nullptr);

public:
	virtual ID3D12Resource* GetVertexBufferResource() override
	{
		return m_pVertexBufferResource;
	}
	virtual const D3D12_VERTEX_BUFFER_VIEW GetVertexBufferViewInfo() const override
	{
		return m_vertexBufferViewInfo;
	}
	virtual ID3D12Resource* GetIndexBufferResource() override
	{
		return m_pIndexBufferResource;
	}
	virtual const D3D12_INDEX_BUFFER_VIEW GetIndexBufferViewInfo() const override
	{
		return m_indexBufferViewInfo;
	}
	virtual ID3D12Resource* GetShaderInfoResource() override
	{
		return m_pShaderInfoResource;
	}
	virtual const int GetShaderInfoHeapPosition() const override
	{
		return m_shaderInfoHeapPosition;
	}
	virtual ID3D12PipelineState* GetPipelineState() override
	{
		return m_pPipelineState;
	}
	virtual ID3D12RootSignature* GetRootSignature() override
	{
		return m_pRootSignature;
	}
	virtual ID3DBlob* GetVertexShader() override
	{
		return m_pVertexShader;
	}
	virtual ID3DBlob* GetPixelShader() override
	{
		return m_pPixelShader;
	}
	virtual IRenderTarget* GetRenderTarget() override
	{
		return m_pRenderTarget;
	}
	virtual int GetIndexNum() const override
	{
		return static_cast<int>(m_indexList.size());
	}
	virtual void EnableGaussianX() override
	{
		m_isEnableGaussianX = true;
		m_isEnableGaussianY = false;
		MapGaussianData();
	}
	virtual void EnableGaussianY()  override
	{
		m_isEnableGaussianX = false;
		m_isEnableGaussianY = true;
		MapGaussianData();
	}
	virtual void DisableGaussian() override
	{
		m_isEnableGaussianX = false;
		m_isEnableGaussianY = false;
		MapGaussianData();
	}
	virtual ID3D12Resource* GetGaussianResource() override
	{
		return m_pGaussianResource;
	}
	virtual const int GetGaussianHeapPosition() const override
	{
		return m_gaussianHeapPosition;
	}
	virtual void SetRenderTarget(IRenderTarget* pRT) override
	{
		m_pRenderTarget = pRT;
	}
	virtual ITexture& GetNormalTexture() override
	{
		return m_normalFilter;
	}
	virtual void SetDepthStencil(IDepthStencil* pDS) override
	{
		m_pDepthStencil = pDS;
	}
	virtual IDepthStencil* GetDepthStencil() override
	{
		return m_pDepthStencil;
	}
	virtual void SetNormalRenderTarget(IRenderTarget* pRT) override
	{
		m_pNormalRenderTarget = pRT;
	}
	virtual IRenderTarget* GetNormalRenderTarget() override
	{
		return m_pNormalRenderTarget;
	}
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() override
	{
		return ACCESS_ROOT_PARAM_SHADER_INFO;
	}
	virtual ID3D12PipelineState* GetBloomPipelineState() override
	{
		return m_pBloomPipelineState;
	}
	virtual void SetBloomRenderTarget(IRenderTarget* pRT) override
	{
		m_pBloom = pRT;
	}
	virtual IRenderTarget* GetBloomRenderTarget() override
	{
		return m_pBloom;
	}
	virtual void SetBloomShrinkRenderTarget(IRenderTarget* pRT) override
	{
		m_pBloomShrink = pRT;
	}
	virtual IRenderTarget* GetBloomShrinkRenderTarget() override
	{
		return m_pBloomShrink;
	}
	virtual void EnableDof() override
	{
		m_isEnableDof = true;
	}
	virtual void DisableDof() override
	{
		m_isEnableDof = false;
	}
	virtual bool IsEnableDof() override
	{
		return m_isEnableDof;
	}
	virtual void SetDofRenderTarget(IRenderTarget* pRT) override
	{
		m_pDof = pRT;
	}
	virtual IRenderTarget* GetDofRenderTarget() override
	{
		return m_pDof;
	}
	virtual void SetDofShrinkRenderTarget(IRenderTarget* pRT) override
	{
		m_pDofShrink = pRT;
	}
	virtual IRenderTarget* GetDofShrinkRenderTarget() override
	{
		return m_pDofShrink;
	}
	virtual ID3D12PipelineState* GetSSAOPipelineState() override
	{
		return m_pSSAOPipelineState;
	}
	virtual void SetSSAORenderTarget(IRenderTarget* pRT) override
	{
		m_pSSAO = pRT;
	}
	virtual IRenderTarget* GetSSAORenderTarget() override
	{
		return m_pSSAO;
	}
	virtual void EnableSsao(bool b) override
	{
		m_isEnableSsao = b;
	}
	virtual bool IsEnableSsao() override
	{
		return m_isEnableSsao;
	}

private:
	void AddVertex(const SVertex& vertex) { m_vertexList.push_back(vertex); }
	void AddIndex(const uint16_t index) { m_indexList.push_back(index); }

private:
	bool BuildVertexBuffer(CGraphicsController& graphicsController);
	bool BuildIndexBuffer(CGraphicsController& graphicsController);
	bool BuildShaderInfoBuffer(CGraphicsController& graphicsController);
	bool BuildGaussianBuffer(CGraphicsController& graphicsController);
	bool BuildShader(CGraphicsController& graphicsController);
	bool BuildRootSignature(CGraphicsController& graphicsController);
	bool BuildPipelineState(CGraphicsController& graphicsController);

	// ブルーム関連.
	bool BuildBloomShader(CGraphicsController& graphicsController);
	bool BuildBloomPipelineState(CGraphicsController& graphicsController);

	// SSAO関連.
	bool BuildSSAOShader(CGraphicsController& graphicsController);
	bool BuildSSAOPipelineState(CGraphicsController& graphicsController);

private:
	// CPU側で使うモデル情報.
	std::vector<SVertex> m_vertexList = {};
	std::vector<uint16_t> m_indexList = {};

	DirectX::XMFLOAT2 m_pos = DirectX::XMFLOAT2();
	DirectX::XMFLOAT2 m_size = DirectX::XMFLOAT2();

	CTexture m_normalFilter;

	// GPUリソース関連.
	ID3D12Resource* m_pVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViewInfo = {};

	ID3D12Resource* m_pIndexBufferResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferViewInfo = {};

	ID3D12Resource* m_pShaderInfoResource = nullptr;
	int m_shaderInfoHeapPosition = -1;

	ID3D12Resource* m_pGaussianResource = nullptr;
	int m_gaussianHeapPosition = -1;

	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;

	ID3DBlob* m_pVertexShader = nullptr;
	ID3DBlob* m_pPixelShader = nullptr;

	IRenderTarget* m_pRenderTarget = nullptr;
	IRenderTarget* m_pNormalRenderTarget = nullptr;
	IDepthStencil* m_pDepthStencil = nullptr;

	// ガウシアン関連.
	bool m_isEnableGaussianX = false;
	bool m_isEnableGaussianY = false;

	// ブルーム関連.
	ID3DBlob* m_pBloomPS = nullptr;
	IRenderTarget* m_pBloom = nullptr;
	IRenderTarget* m_pBloomShrink = nullptr;
	ID3D12PipelineState* m_pBloomPipelineState = nullptr;

	// 被写界深度.
	bool m_isEnableDof = false;
	IRenderTarget* m_pDof = nullptr;
	IRenderTarget* m_pDofShrink = nullptr;

	// SSAO.
	bool m_isEnableSsao = false;
	IRenderTarget* m_pSSAO = nullptr;
	ID3DBlob* m_pSSAOPS = nullptr;
	ID3D12PipelineState* m_pSSAOPipelineState = nullptr;
};