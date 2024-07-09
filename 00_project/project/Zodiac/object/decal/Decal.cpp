#include "Decal.h"
#include "../../graphics/GraphicsController.h"

// ------------------------------------------------------.
void CDecalQuad::Init(CGraphicsController& rGraphicsController, float x, float y, float z, float w, float h)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
	m_size.x = w;
	m_size.y = h;

	// 頂点.
	{
		SetupVertex();
	}

	// 頂点インデックス.
	{
		uint16_t index[] =
		{
			0, 1, 2,
			2, 1, 3,
		};
		for (uint32_t n = 0; n < _countof(index); n++) {
			AddIndex(index[n]);
		}
	}

	VRETURN(BuildVertexBuffer(rGraphicsController));
	VRETURN(BuildIndexBuffer(rGraphicsController));

	MapVertexData();
	MapIndexData();
}

void CDecalQuad::Term()
{
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

void CDecalQuad::RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper)
{
	// 処理するプリミティブタイプを設定.
	commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 頂点バッファビューを設定.
	D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
	D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
	commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
	commandWrapper.SetIndexBuffer(&indexBufferInfo);
}

bool CDecalQuad::SetupVertex()
{
	m_vertexList.clear();
	SVertex vertex[] =
	{
		{ {m_pos.x, m_pos.y + m_size.y, m_pos.z},				{0.0f, 1.0f} },
		{ {m_pos.x, m_pos.y, m_pos.z},							{0.0f, 0.0f} },
		{ {m_pos.x + m_size.x, m_pos.y + m_size.y, m_pos.z},	{1.0f, 1.0f} },
		{ {m_pos.x + m_size.x, m_pos.y, m_pos.z},				{1.0f, 0.0f} },
	};
	for (uint32_t n = 0; n < _countof(vertex); n++) {
		AddVertex(vertex[n]);
	}
	return true;
}

bool CDecalQuad::BuildVertexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_vertexList.size() > 0, false);
	VRETURN_RET(!m_pVertexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_vertexList[0].GetDataSize() * m_vertexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_vertexBufferViewInfo.BufferLocation = m_pVertexBufferResource->GetGPUVirtualAddress();
		m_vertexBufferViewInfo.SizeInBytes = m_vertexList[0].GetDataSize() * static_cast<int>(m_vertexList.size());
		m_vertexBufferViewInfo.StrideInBytes = m_vertexList[0].GetDataSize();
	}

	return true;
}

bool CDecalQuad::BuildIndexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_indexList.size() > 0, false);
	VRETURN_RET(!m_pIndexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = sizeof(m_indexList[0]) * m_indexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pIndexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_indexBufferViewInfo.BufferLocation = m_pIndexBufferResource->GetGPUVirtualAddress();
		m_indexBufferViewInfo.SizeInBytes = sizeof(m_indexList[0]) * static_cast<int>(m_indexList.size());
		m_indexBufferViewInfo.Format = DXGI_FORMAT_R16_UINT;
	}

	return true;
}

void CDecalQuad::MapVertexData()
{
	RETURN(m_pVertexBufferResource);

	uint8_t* pBuffer = nullptr;
	HRESULT ret = m_pVertexBufferResource->Map(0, nullptr, (void**)(&pBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_vertexList.size(); n++) {
		const auto& vertex = m_vertexList[n];

		auto pos = vertex.pos;
		memcpy(pBuffer, &pos, sizeof(pos));
		pBuffer += sizeof(pos);

		auto uv = vertex.uv;
		memcpy(pBuffer, &uv, sizeof(uv));
		pBuffer += sizeof(uv);
	}

	m_pVertexBufferResource->Unmap(0, nullptr);
}

void CDecalQuad::MapIndexData()
{
	RETURN(m_pIndexBufferResource);

	uint16_t* pIndexBuffer = nullptr;
	HRESULT ret = m_pIndexBufferResource->Map(0, nullptr, (void**)(&pIndexBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_indexList.size(); n++) {
		pIndexBuffer[n] = m_indexList[n];
	}

	m_pIndexBufferResource->Unmap(0, nullptr);
}

// ------------------------------------------------------.
bool C2DDecal::Init(CGraphicsController& rGraphicsController, float x, float y, float z, float w, float h)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
	m_quad.Init(rGraphicsController, x, y, z, w, h);
	m_pShader = nullptr;
	return true;
}

void C2DDecal::Term()
{
	m_quad.Term();
}

void C2DDecal::Update()
{
}

void C2DDecal::Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// シェーダーが設定されていない場合は描画できないのでダメ.
	VRETURN(m_pShader);

	// 描画時の環境情報.
	{
		// ビューポートとシザリングを設定.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}
	}

	// メッシュ情報.
	{
		m_quad.RenderSetup(commandWrapper, heapWrapper);
	}

	// シェーダー関連の情報は任せる.
	{
		m_pShader->RenderSetup(commandWrapper, heapWrapper);
	}

	// 描画.
	commandWrapper.DrawIndexedInstanced(m_quad.GetIndexNum(), 1, 0, 0, 0);
}