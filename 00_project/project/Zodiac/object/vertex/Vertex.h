#pragma once

#include "../../graphics/GraphicsDefs.h"

class CVertex : public IVertex {
public:
	CVertex() = default;
	~CVertex() = default;

public:
	bool Load(const SMMDVertex& readData);

public:
	virtual const DirectX::XMFLOAT3& GetPosition() const override
	{
		return m_position;
	}
	virtual const DirectX::XMFLOAT3& GetNormal() const override
	{
		return m_normal;
	}
	virtual const DirectX::XMFLOAT2& GetUV() const override
	{
		return m_uv;
	}
	virtual const uint32_t GetBoneNumberListNum() const override
	{
		return COUNTOF(m_boneNo);
	}
	virtual const uint16_t* GetBoneNumberList() const override
	{
		return m_boneNo;
	}
	virtual const uint8_t GetBoneWeight() const override
	{
		return m_boneWeight;
	}
	virtual const uint8_t GetEdgeOption() const override
	{
		return m_edgeOption;
	}
	virtual const uint32_t GetDataSize() const override
	{
		return
			sizeof(decltype(m_position)) +
			sizeof(decltype(m_normal)) +
			sizeof(decltype(m_uv)) +
			(sizeof(decltype(m_boneNo[0])) * GetBoneNumberListNum()) +
			sizeof(decltype(m_boneWeight)) +
			sizeof(decltype(m_edgeOption));
	}

private:
	DirectX::XMFLOAT3 m_position = {};
	DirectX::XMFLOAT3 m_normal = {};
	DirectX::XMFLOAT2 m_uv = {};
	uint16_t m_boneNo[2] = {};
	uint8_t m_boneWeight = 0;
	uint8_t m_edgeOption = 0;
};
