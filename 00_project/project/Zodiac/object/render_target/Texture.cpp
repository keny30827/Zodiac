#include "Texture.h"
#include "../../graphics/GraphicsController.h"

CTexture::~CTexture()
{
	SAEF_RELEASE(m_pTextureCPUResource);
	SAEF_RELEASE(m_pTextureGPUResource);
}

bool CTexture::Load(const std::string& path, CGraphicsController& graphicsController, const HEAP_CATEGORY category, const ACCESS_ROOT_PARAM rootParam)
{
	VRETURN_RET(!path.empty(), false);

	m_rootParam = rootParam;

	std::wstring wTexPath = util::C2W(path);
	RETURN_RET(!wTexPath.empty(), false);

	HRESULT ret = DirectX::LoadFromWICFile(
		wTexPath.c_str(),
		DirectX::WIC_FLAGS_NONE,
		&(m_metaData),
		m_image);

	// シェーダー用のリソース作成.
	BuildTexture(graphicsController, category);

	// リソースに固定データをマッピング.
	MapTextureData();

	return (ret == S_OK);
}

void CTexture::MapTextureData()
{
	VRETURN(m_pTextureCPUResource);

	const DirectX::Image* pImg = GetImage().GetImage(0, 0, 0);
	VRETURN(pImg);
	
	uint8_t* pBuff = nullptr;
	auto ret = m_pTextureCPUResource->Map(0, nullptr, (void**)&pBuff);
	VRETURN(ret == S_OK);

	const uint32_t rowPitch = util::AdjustRowPitch(static_cast<uint32_t>(pImg->rowPitch), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	uint8_t* pStart = pImg->pixels;
	for (int n = 0; n < pImg->height; n++) {
		std::copy_n(pStart, pImg->rowPitch, pBuff);
		pStart += pImg->rowPitch;
		pBuff += rowPitch;
	}

	m_pTextureCPUResource->Unmap(0, nullptr);
	return;
}

bool CTexture::BuildTexture(CGraphicsController& graphicsController, const HEAP_CATEGORY category)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	const DirectX::Image* pImg = GetImage().GetImage(0, 0, 0);
	VRETURN_RET(pImg, false);

	HRESULT ret = S_FALSE;

	const uint32_t rowPitch = util::AdjustRowPitch(static_cast<uint32_t>(pImg->rowPitch), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	// CPU側からアクセスできるアップロード用バッファを用意.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = (rowPitch * pImg->height);
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
			IID_PPV_ARGS(&m_pTextureCPUResource));

		VRETURN_RET(ret == S_OK, false);
	}

	// GPUからアクセスする用のバッファを用意.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(GetMetaData().dimension);
		resourceDesc.Alignment = 0;
		resourceDesc.Width = static_cast<uint32_t>(GetMetaData().width);
		resourceDesc.Height = static_cast<uint32_t>(GetMetaData().height);
		resourceDesc.DepthOrArraySize = static_cast<uint16_t>(GetMetaData().arraySize);
		resourceDesc.MipLevels = static_cast<uint16_t>(GetMetaData().mipLevels);
		resourceDesc.Format = GetMetaData().format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pTextureGPUResource));

		VRETURN_RET(ret == S_OK, false);
	}

	// コピー元のバッファ情報設定.
	{
		m_copySrc.pResource = m_pTextureCPUResource;
		m_copySrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		m_copySrc.PlacedFootprint.Offset = 0;
		m_copySrc.PlacedFootprint.Footprint.Width = static_cast<uint32_t>(GetMetaData().width);
		m_copySrc.PlacedFootprint.Footprint.Height = static_cast<uint32_t>(GetMetaData().height);
		m_copySrc.PlacedFootprint.Footprint.Depth = static_cast<uint32_t>(GetMetaData().depth);
		m_copySrc.PlacedFootprint.Footprint.Format = GetMetaData().format;
		m_copySrc.PlacedFootprint.Footprint.RowPitch = rowPitch;
	}

	// コピー先のバッファ情報設定.
	{
		m_copyDst.pResource = m_pTextureGPUResource;
		m_copyDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		m_copyDst.SubresourceIndex = 0;
	}

	// リソースバリアの設定.
	{
		m_copyResBarrier.Transition.pResource = m_pTextureGPUResource;
		m_copyResBarrier.Transition.Subresource = 0;
		m_copyResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		m_copyResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		m_ResetResBarrier.Transition.pResource = m_pTextureGPUResource;
		m_ResetResBarrier.Transition.Subresource = 0;
		m_ResetResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		m_ResetResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;	
	}

	// ヒープへの紐づけ作業.
	{
		// 割り当て.
		m_textureHeapCategory = category;
		m_textureHeapPosition = graphicsController.AllocateHeapPosition(m_textureHeapCategory);
		VRETURN_RET(m_textureHeapPosition >= 0, false);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(m_textureHeapCategory, m_textureHeapPosition);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = (m_copyDst.pResource) ? m_copySrc.PlacedFootprint.Footprint.Format : DXGI_FORMAT_R8G8B8A8_UNORM;
		pDevice->CreateShaderResourceView(
			m_copyDst.pResource,
			&srvDesc,
			handle);
	}

	return true;
}