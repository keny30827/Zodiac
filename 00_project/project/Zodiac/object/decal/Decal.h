#pragma once

#include "../../graphics/GraphicsDefs.h"

// 2Dデカール用頂点情報.
class CDecalQuad
{
private:
	struct SVertex {
		DirectX::XMFLOAT4 pos = {};	//worldPos.
		DirectX::XMFLOAT2 uv = {};
		int GetDataSize()
		{
			return sizeof(decltype(pos)) + sizeof(decltype(uv));
		}
	};

public:
	CDecalQuad() = default;
	virtual ~CDecalQuad() {};

public:
	void Init(CGraphicsController& rGraphicsController, float x, float y, float z, float w, float h);
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

public:
	void SetPos(const DirectX::XMFLOAT4& pos)
	{
		m_pos = pos;
		SetupVertex();
	}

private:
	bool SetupVertex();

	bool BuildVertexBuffer(CGraphicsController& graphicsController);
	bool BuildIndexBuffer(CGraphicsController& graphicsController);

	void MapVertexData();
	void MapIndexData();

	void AddVertex(const SVertex& vertex) { m_vertexList.push_back(vertex); }
	void AddIndex(const uint16_t index) { m_indexList.push_back(index); }

private:
	DirectX::XMFLOAT4 m_pos = DirectX::XMFLOAT4();
	DirectX::XMFLOAT2 m_size = DirectX::XMFLOAT2();

	std::vector<SVertex> m_vertexList = {};
	std::vector<uint16_t> m_indexList = {};

	// GPUリソース関連.
	ID3D12Resource* m_pVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViewInfo = {};
	ID3D12Resource* m_pIndexBufferResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferViewInfo = {};
};

// 2Dデカールオブジェクト.
class C2DDecal : public IDecal {
public:
	C2DDecal() = default;
	virtual ~C2DDecal();

public:
	bool Init(CGraphicsController& rGraphicsController, float x, float y, float z, float w, float h);
	void Term();

public:
	virtual void Update() override;
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;

public:
	void SetShader(IShader* pShader) { m_pShader = pShader; }
	void SetPos(const DirectX::XMFLOAT4& pos)
	{
		m_pos = pos;
		m_quad.SetPos(pos);
	}

private:
	DirectX::XMFLOAT4 m_pos = DirectX::XMFLOAT4();

	CDecalQuad m_quad;
	IShader* m_pShader = nullptr;
};