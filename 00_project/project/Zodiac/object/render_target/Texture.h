#pragma once

#include "../../graphics/GraphicsDefs.h"

class CGraphicsController;
class CTexture : public ITexture {
public:
	CTexture() = default;
	virtual ~CTexture();

	CTexture(const CTexture& from)
	{
		m_metaData = from.m_metaData;
		m_image = std::move(const_cast<CTexture&>(from).m_image);
		m_pTextureCPUResource = from.m_pTextureCPUResource;
		m_pTextureGPUResource = from.m_pTextureGPUResource;
		m_copySrc = from.m_copySrc;
		m_copyDst = from.m_copyDst;
		m_copyResBarrier = from.m_copyResBarrier;
		m_ResetResBarrier = from.m_ResetResBarrier;		
		m_textureHeapPosition = from.m_textureHeapPosition;
		const_cast<CTexture&>(from).m_pTextureCPUResource = nullptr;
		const_cast<CTexture&>(from).m_pTextureGPUResource = nullptr;
	}
	CTexture& operator=(const CTexture& from)
	{
		m_metaData = from.m_metaData;
		m_image = std::move(const_cast<CTexture&>(from).m_image);
		m_pTextureCPUResource = from.m_pTextureCPUResource;
		m_pTextureGPUResource = from.m_pTextureGPUResource;
		m_copySrc = from.m_copySrc;
		m_copyDst = from.m_copyDst;
		m_copyResBarrier = from.m_copyResBarrier;
		m_ResetResBarrier = from.m_ResetResBarrier;
		m_textureHeapPosition = from.m_textureHeapPosition;
		const_cast<CTexture&>(from).m_pTextureCPUResource = nullptr;
		const_cast<CTexture&>(from).m_pTextureGPUResource = nullptr;
		return *this;
	}

public:
	bool Load(
		const std::string& path,
		CGraphicsController& graphicsController,
		const HEAP_CATEGORY category = HEAP_CATEGORY_TEXTURE,
		const ACCESS_ROOT_PARAM rootParam = ACCESS_ROOT_PARAM_TEXTURE);

public:
	virtual const DirectX::TexMetadata& GetMetaData() const override
	{
		return m_metaData;
	}
	virtual const DirectX::ScratchImage& GetImage() const override
	{
		return m_image;
	}
	virtual const ID3D12Resource* GetCPUResource() const override
	{
		return m_pTextureCPUResource;
	}
	virtual const ID3D12Resource* GetGPUResource() const override
	{
		return m_pTextureGPUResource;
	}
	virtual const HEAP_CATEGORY GetHeapCategory() const override
	{
		return m_textureHeapCategory;
	}
	virtual const int GetHeapPosition() const override
	{
		return m_textureHeapPosition;
	}
	virtual const D3D12_TEXTURE_COPY_LOCATION& GetCopySrc() const override
	{
		return m_copySrc;
	}
	virtual const D3D12_TEXTURE_COPY_LOCATION& GetCopyDst() const override
	{
		return m_copyDst;
	}
	virtual const D3D12_RESOURCE_BARRIER& GetCopyResourceBarrier() const override
	{
		return m_copyResBarrier;
	}
	virtual const D3D12_RESOURCE_BARRIER& GetResetResourceBarrier() const override
	{
		return m_ResetResBarrier;
	}
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() override
	{
		return m_rootParam;
	}

private:
	void MapTextureData();

private:
	bool BuildTexture(CGraphicsController& graphicsController, const HEAP_CATEGORY category);

private:
	DirectX::TexMetadata m_metaData = {};
	DirectX::ScratchImage m_image = {};

	ID3D12Resource* m_pTextureCPUResource = nullptr;
	ID3D12Resource* m_pTextureGPUResource = nullptr;
	D3D12_TEXTURE_COPY_LOCATION m_copySrc = {};
	D3D12_TEXTURE_COPY_LOCATION m_copyDst = {};
	D3D12_RESOURCE_BARRIER m_copyResBarrier = {};
	D3D12_RESOURCE_BARRIER m_ResetResBarrier = {};
	HEAP_CATEGORY m_textureHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_INVALID;
	int m_textureHeapPosition = -1;

	ACCESS_ROOT_PARAM m_rootParam = ACCESS_ROOT_PARAM::ACCESS_ROOT_PARAM_INVALID;
};