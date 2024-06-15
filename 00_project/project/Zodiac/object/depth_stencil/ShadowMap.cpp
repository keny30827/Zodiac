#include "ShadowMap.h"
#include "../../graphics/GraphicsController.h"

void CShadowMap::Init(CGraphicsController& rGraphicsController, uint32_t width, uint32_t height)
{
	auto* pDevice = rGraphicsController.GetGraphicsDevice();
	VRETURN(pDevice);

	m_width = width;
	m_height = height;

	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_width;
		resourceDesc.Height = m_height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		HRESULT ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			nullptr,
			IID_PPV_ARGS(&m_pResource));
		VRETURN(ret == S_OK);
	}

	// DSV作成.
	{
		// ヒープに割り当てる.
		m_dsvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_DEPTH_STENCIL;
		m_dsvHeapPosition = rGraphicsController.AllocateHeapPosition(m_dsvHeapCategory);
		VRETURN(m_dsvHeapPosition >= 0);
		// ビュー.
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rGraphicsController.GetCPUDescriptorHandle(m_dsvHeapCategory, m_dsvHeapPosition);
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		pDevice->CreateDepthStencilView(m_pResource, &dsvDesc, handle);
	}

	// SRV作成.
	{
		// ヒープに割り当てる.
		m_srvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_HUGE;
		m_srvHeapPosition = rGraphicsController.AllocateHeapPosition(m_srvHeapCategory);
		VRETURN(m_srvHeapPosition >= 0);
		// ビュー.
		D3D12_CPU_DESCRIPTOR_HANDLE srvStartHandle = rGraphicsController.GetCPUDescriptorHandle(m_srvHeapCategory, m_srvHeapPosition);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		pDevice->CreateShaderResourceView(
			m_pResource,
			&srvDesc,
			srvStartHandle);
	}

	// リソースバリアを用意.
	{
		m_resourceBarrier.Transition.pResource = m_pResource;
		m_resourceBarrier.Transition.Subresource = 0;
		m_resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		m_resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
}

void CShadowMap::Term()
{
	SAEF_RELEASE(m_pResource);
}