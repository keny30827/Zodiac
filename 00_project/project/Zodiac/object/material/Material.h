#pragma once

#include "../../graphics/GraphicsDefs.h"
#include "../render_target/Texture.h"

// 光情報.
struct SMatarialLight {
	DirectX::XMFLOAT3 diffuse = {};
	float diffuse_alpha = 0.0f;
	DirectX::XMFLOAT3 specular = {};
	float specularity = 0.0f;
	DirectX::XMFLOAT3 ambient = {};
	float isValidToon = 0.0f;
};

// テクスチャ情報.
struct SMatarialTexture {
	CTexture texture = {};
	CTexture sphereTexture = {};
	CTexture sphereAddTexture = {};
	CTexture toonTexture = {};
};

// その他情報.
struct SMatarialOption {
	uint8_t toonIdx = 0;
	uint8_t edgeOption = 0;
};

class CGraphicsController;
class CMatarial : public IMaterial {
public:
	CMatarial() = default;
	virtual ~CMatarial();

	CMatarial(const CMatarial& from)
	{
		m_indicesNum = from.m_indicesNum;
		m_light = from.m_light;
		m_texture = from.m_texture;
		m_option = from.m_option;
		m_pLightResource = from.m_pLightResource;
		m_lightHeapPosition = from.m_lightHeapPosition;
		const_cast<CMatarial&>(from).m_pLightResource = nullptr;
	}
	CMatarial& operator=(const CMatarial& from)
	{
		m_indicesNum = from.m_indicesNum;
		m_light = from.m_light;
		m_texture = from.m_texture;
		m_option = from.m_option;
		m_pLightResource = from.m_pLightResource;
		m_lightHeapPosition = from.m_lightHeapPosition;
		const_cast<CMatarial&>(from).m_pLightResource = nullptr;
		return *this;
	}

public:
	bool Load(const SMMDMatarial& readData, const std::string& foulderName, CGraphicsController& graphicsController);

public:
	virtual const uint32_t GetIndicesNum() const override
	{
		return m_indicesNum;
	}
	virtual const DirectX::XMFLOAT3& GetDiffuse() const override
	{
		return m_light.diffuse;
	}
	virtual const float GetDiffuseAlpha() const override
	{
		return m_light.diffuse_alpha;
	}
	virtual const DirectX::XMFLOAT3& GetSpecular() const override
	{
		return m_light.specular;
	}
	virtual const float GetSpecularity() const override
	{
		return m_light.specularity;
	}
	virtual const DirectX::XMFLOAT3& GetAmbient() const override
	{
		return m_light.ambient;
	}
	virtual const float IsValidToon() const override
	{
		return m_light.isValidToon;
	}
	virtual const ITexture* GetTexture() const override
	{
		return &m_texture.texture;
	}
	virtual const ITexture* GetSphereTexture() const override
	{
		return &m_texture.sphereTexture;
	}
	virtual const ITexture* GetSphereAddTexture() const override
	{
		return &m_texture.sphereAddTexture;
	}
	virtual const ITexture* GetToonTexture() const override
	{
		return &m_texture.toonTexture;
	}
	virtual const uint8_t GetToonIdx() const override
	{
		return m_option.toonIdx;
	}
	virtual const uint8_t GetEdgeOption() const override
	{
		return m_option.edgeOption;
	}
	virtual const uint32_t GetShaderUseDataSize() const override
	{
		return sizeof(decltype(m_light));
	}
	virtual const ID3D12Resource* GetLightResource() const override
	{
		return m_pLightResource;
	}
	virtual const int GetLightHeapPosition() const override
	{
		return m_lightHeapPosition;
	}
	virtual const uint32_t GetTotalHeapUseSize() const override
	{
		// ディスクリプタヒープを使う数.テクスチャ数＋ライト情報.
		return GetTexHeapUseSize() + GetLightHeapUseSize();
	}
	virtual const uint32_t GetLightHeapUseSize() const override
	{
		return GetLightHeapSize();
	}
	virtual const uint32_t GetTexHeapUseSize() const override
	{
		return GetTexHeapSize();
	}
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() const override
	{
		return ACCESS_ROOT_PARAM_MATERIAL;
	}

public:
	static uint32_t GetLightHeapSize() { return 1; }
	static uint32_t GetTexHeapSize() { return (sizeof(SMatarialTexture) / sizeof(CTexture)); }

private:
	void MapLightData();

private:
	bool BuildLight(CGraphicsController& graphicsController);

private:
	uint32_t m_indicesNum = 0;
	SMatarialLight m_light = {};
	SMatarialTexture m_texture = {};
	SMatarialOption m_option = {};

	ID3D12Resource* m_pLightResource = nullptr;
	int m_lightHeapPosition = -1;
};
