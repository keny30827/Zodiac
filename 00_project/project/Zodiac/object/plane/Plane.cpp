#include "Plane.h"
#include "../../graphics/GraphicsController.h"

// ------------------------------------------------------.
void C3DQuad::Init(CGraphicsController& rGraphicsController, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT2 size)
{
	m_pos = pos;
	m_rot = rot;
	m_size = size;

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

void C3DQuad::Term()
{
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

void C3DQuad::RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper)
{
	// 処理するプリミティブタイプを設定.
	commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 頂点バッファビューを設定.
	D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
	D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
	commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
	commandWrapper.SetIndexBuffer(&indexBufferInfo);
}

bool C3DQuad::SetupVertex()
{
	m_vertexList.clear();
	
	// 法線はY方向をベースとして、rotを元に回転させて、各頂点位置を決める.
	DirectX::XMMATRIX axis(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_rot));
	axis = DirectX::XMMatrixMultiply(axis, rot);

	DirectX::XMVECTOR halfX = DirectX::XMVectorScale(axis.r[0], (m_size.x * 0.5f));
	DirectX::XMVECTOR halfZ = DirectX::XMVectorScale(axis.r[2], (m_size.y * 0.5f));
	DirectX::XMVECTOR revHalfX = DirectX::XMVectorScale(halfX, -1.0f);
	DirectX::XMVECTOR revhalfZ = DirectX::XMVectorScale(halfZ, -1.0f);

	DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&m_pos);
	DirectX::XMFLOAT3 lb;
	DirectX::XMStoreFloat3(&lb, DirectX::XMVectorAdd(DirectX::XMVectorAdd(posVec, revHalfX), halfZ));
	DirectX::XMFLOAT3 lt;
	DirectX::XMStoreFloat3(&lt, DirectX::XMVectorAdd(DirectX::XMVectorAdd(posVec, revHalfX), revhalfZ));
	DirectX::XMFLOAT3 rb;
	DirectX::XMStoreFloat3(&rb, DirectX::XMVectorAdd(DirectX::XMVectorAdd(posVec, halfX), halfZ));
	DirectX::XMFLOAT3 rt;
	DirectX::XMStoreFloat3(&rt, DirectX::XMVectorAdd(DirectX::XMVectorAdd(posVec, halfX), revhalfZ));

	DirectX::XMFLOAT3 normal;
	DirectX::XMStoreFloat3(&normal, axis.r[1]);

	SVertex vertex[] =
	{
		{ lb, normal, {0.0f, 1.0f} },
		{ lt, normal, {0.0f, 0.0f} },
		{ rb, normal, {1.0f, 1.0f} },
		{ rt, normal, {1.0f, 0.0f} },
	};
	for (uint32_t n = 0; n < _countof(vertex); n++) {
		AddVertex(vertex[n]);
	}
	return true;
}

bool C3DQuad::BuildVertexBuffer(CGraphicsController& graphicsController)
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

bool C3DQuad::BuildIndexBuffer(CGraphicsController& graphicsController)
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

void C3DQuad::MapVertexData()
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

		auto normal = vertex.normal;
		memcpy(pBuffer, &normal, sizeof(normal));
		pBuffer += sizeof(normal);

		auto uv = vertex.uv;
		memcpy(pBuffer, &uv, sizeof(uv));
		pBuffer += sizeof(uv);
	}

	m_pVertexBufferResource->Unmap(0, nullptr);
}

void C3DQuad::MapIndexData()
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
bool C3DPlane::Init(CGraphicsController& rGraphicsController, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT2 size)
{
	m_quad.Init(rGraphicsController, pos, rot, size);
	m_pShader = nullptr;
	return true;
}

void C3DPlane::Term()
{
	m_quad.Term();
}

void C3DPlane::Update()
{
}

void C3DPlane::Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
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