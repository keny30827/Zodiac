#include "RenderTarget.h"
#include "../../graphics/GraphicsController.h"

void CRenderTarget::Init(CGraphicsController& rGraphicsController, uint32_t width, uint32_t height, ACCESS_ROOT_PARAM rootParam)
{
	auto* pDevice = rGraphicsController.GetGraphicsDevice();
	VRETURN(pDevice);

	m_width = width;
	m_height = height;
	m_accessRootParam = rootParam;

	// リソースの実体を作成.
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
		resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		HRESULT ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			nullptr,
			IID_PPV_ARGS(&m_pResource));

		RETURN(ret == S_OK);
	}

	// RTV作成.
	{
		// ヒープに割り当てる.
		m_rtvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_RENDER_TARGET;
		m_rtvHeapPosition = rGraphicsController.AllocateHeapPosition(m_rtvHeapCategory);
		VRETURN(m_rtvHeapPosition >= 0);
		// ビュー.
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rGraphicsController.GetCPUDescriptorHandle(m_rtvHeapCategory, m_rtvHeapPosition);
		pDevice->CreateRenderTargetView(m_pResource, nullptr, handle);
	}

	// SRV作成.
	{
		// ヒープに割り当てる.
		m_srvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_RENDER_TARGET_SHADER_VIEW;
		m_srvHeapPosition = rGraphicsController.AllocateHeapPosition(m_srvHeapCategory);
		VRETURN(m_srvHeapPosition >= 0);
		// ビュー.
		D3D12_CPU_DESCRIPTOR_HANDLE srvStartHandle = rGraphicsController.GetCPUDescriptorHandle(m_srvHeapCategory, m_srvHeapPosition);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		m_resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
}

void CRenderTarget::Term()
{
	SAEF_RELEASE(m_pResource);
}