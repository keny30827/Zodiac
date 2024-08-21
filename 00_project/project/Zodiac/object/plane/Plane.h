#pragma once

#include "../../graphics/GraphicsDefs.h"

// 3D平面.
class C3DQuad
{
private:
	struct SVertex {
		DirectX::XMFLOAT3 pos = {};	//worldPos.
		DirectX::XMFLOAT3 normal = {};
		DirectX::XMFLOAT2 uv = {};
		int GetDataSize()
		{
			return sizeof(decltype(pos)) + sizeof(decltype(normal)) + sizeof(decltype(uv));
		}
	};

public:
	C3DQuad() = default;
	virtual ~C3DQuad() {};

public:
	void Init(CGraphicsController& rGraphicsController, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT2 size);
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
	void SetPos(const DirectX::XMFLOAT3& pos)
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
	DirectX::XMFLOAT3 m_pos = DirectX::XMFLOAT3();
	DirectX::XMFLOAT3 m_rot = DirectX::XMFLOAT3();
	DirectX::XMFLOAT2 m_size = DirectX::XMFLOAT2();

	std::vector<SVertex> m_vertexList = {};
	std::vector<uint16_t> m_indexList = {};

	// GPUリソース関連.
	ID3D12Resource* m_pVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViewInfo = {};
	ID3D12Resource* m_pIndexBufferResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferViewInfo = {};
};

// 3D平面.
class C3DPlane : public IPlane {
public:
	C3DPlane() = default;
	virtual ~C3DPlane() {};

public:
	bool Init(CGraphicsController& rGraphicsController, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT2 size);
	void Term();

public:
	virtual void Update() override;
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;

public:
	void SetShader(IShader* pShader) { m_pShader = pShader; }
	void SetPos(const DirectX::XMFLOAT3& pos)
	{
		m_quad.SetPos(pos);
	}

private:
	C3DQuad m_quad;
	IShader* m_pShader = nullptr;
};