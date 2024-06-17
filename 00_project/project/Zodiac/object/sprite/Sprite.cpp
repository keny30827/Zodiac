#include "Sprite.h"
#include "../../graphics/GraphicsController.h"
#include "../../graphics/GraphicsResourceManager.h"

// ------------------------------------------------------.
void CQuad::Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h)
{
	// ���_.
	{
		SVertex vertex[] =
		{
			{ {x, y + h, 0.0f},		{0.0f, 1.0f} },
			{ {x, y, 0.0f},			{0.0f, 0.0f} },
			{ {x + w, y + h, 0.0f},	{1.0f, 1.0f} },
			{ {x + w, y, 0.0f},		{1.0f, 0.0f} },
		};
		for (uint32_t n = 0; n < _countof(vertex); n++) {
			AddVertex(vertex[n]);
		}
	}

	// ���_�C���f�b�N�X.
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

void CQuad::Term()
{
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

void CQuad::RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper)
{
	// ��������v���~�e�B�u�^�C�v��ݒ�.
	commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ���_�o�b�t�@�r���[��ݒ�.
	D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
	D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
	commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
	commandWrapper.SetIndexBuffer(&indexBufferInfo);
}

bool CQuad::BuildVertexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_vertexList.size() > 0, false);
	VRETURN_RET(!m_pVertexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPU���\�[�X���m��.
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

	// GPU�����猩���悤�Ƀr���[��p��.
	{
		m_vertexBufferViewInfo.BufferLocation = m_pVertexBufferResource->GetGPUVirtualAddress();
		m_vertexBufferViewInfo.SizeInBytes = m_vertexList[0].GetDataSize() * static_cast<int>(m_vertexList.size());
		m_vertexBufferViewInfo.StrideInBytes = m_vertexList[0].GetDataSize();
	}

	return true;
}

bool CQuad::BuildIndexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_indexList.size() > 0, false);
	VRETURN_RET(!m_pIndexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPU���\�[�X���m��.
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

	// GPU�����猩���悤�Ƀr���[��p��.
	{
		m_indexBufferViewInfo.BufferLocation = m_pIndexBufferResource->GetGPUVirtualAddress();
		m_indexBufferViewInfo.SizeInBytes = sizeof(m_indexList[0]) * static_cast<int>(m_indexList.size());
		m_indexBufferViewInfo.Format = DXGI_FORMAT_R16_UINT;
	}

	return true;
}

void CQuad::MapVertexData()
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

void CQuad::MapIndexData()
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
bool CSprite::Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h)
{
	m_pos.x = x;
	m_pos.y = y;
	m_size.x = w;
	m_size.y = h;
	m_quad.Init(rGraphicsController, x, y, w, h);
	m_pShader = nullptr;
	return true;
}

void CSprite::Term()
{
	m_quad.Term();
}

void CSprite::Update()
{
}

void CSprite::Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// �V�F�[�_�[���ݒ肳��Ă��Ȃ��ꍇ�͕`��ł��Ȃ��̂Ń_��.
	VRETURN(m_pShader);

	// �`�掞�̊����.
	{
		// �r���[�|�[�g�ƃV�U�����O��ݒ�.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}
	}

	// ���b�V�����.
	{
		m_quad.RenderSetup(commandWrapper, heapWrapper);
	}

	// �V�F�[�_�[�֘A�̏��͔C����.
	{
		m_pShader->RenderSetup(commandWrapper, heapWrapper);
	}

	// �`��.
	if (m_isDownSample) {
		VRETURN(pViewPort);
		VRETURN(pScissor);
		// �r���[�|�[�g�ƃV�U�����O��؂�ւ��Ȃ���k���o�b�t�@���쐬����.
		D3D12_VIEWPORT tmpViewPort = *pViewPort;
		D3D12_RECT tmpScissor = *pScissor;
		for (int n = 0; n < 8; n++) {
			// �T�C�Y�𔼕���.
			tmpViewPort.Width *= 0.5f;
			tmpViewPort.Height *= 0.5f;
			const int scissorOffsetY = (tmpScissor.bottom - tmpScissor.top) / 2;
			tmpScissor.right = tmpScissor.left + ((tmpScissor.right - tmpScissor.left) / 2);
			tmpScissor.bottom = tmpScissor.top + scissorOffsetY;
			// ���̃T�C�Y�ŕ`��.
			commandWrapper.SetViewports(&tmpViewPort, 1);
			commandWrapper.SetScissorRects(&tmpScissor, 1);
			commandWrapper.DrawIndexedInstanced(m_quad.GetIndexNum(), 1, 0, 0, 0);
			// �ʒu���炷.
			tmpViewPort.TopLeftY += tmpViewPort.Height;
			tmpScissor.top += scissorOffsetY;
			tmpScissor.bottom += scissorOffsetY;
		}
	}
	else {
		commandWrapper.DrawIndexedInstanced(m_quad.GetIndexNum(), 1, 0, 0, 0);
	}
}
