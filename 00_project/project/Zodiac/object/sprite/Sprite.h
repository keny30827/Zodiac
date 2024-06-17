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

class CSprite : public ISprite
{
public:
	CSprite() = default;
	virtual ~CSprite() {}

public:
	bool Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h);
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