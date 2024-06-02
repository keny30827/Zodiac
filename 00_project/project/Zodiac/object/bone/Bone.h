#pragma once

#include "../../graphics/GraphicsDefs.h"

struct CBone : public IBone {
public:
	CBone() = default;
	virtual ~CBone() = default;

public:
	bool Load(const SMMDBone& readData);
	void ClearMatrix() {
		m_trans = DirectX::XMMatrixIdentity();
		m_rot = DirectX::XMMatrixIdentity();
		m_affine = DirectX::XMMatrixIdentity();
	}

public:
	virtual const DirectX::XMMATRIX& GetAffineMatrix() const override
	{
		return m_affine;
	}

public:
	DirectX::XMMATRIX& GetRowTrans() { return m_trans; }
	DirectX::XMMATRIX& GetRowRot() { return m_rot; }
	DirectX::XMMATRIX& GetRowAffine() { return m_affine; }

	std::vector<uint16_t>& GetChildren() { return m_children; }

public:	// データ読み込み後のオリジンデータ.
	const char* GetName() const { return m_name; }
	uint16_t GetParentNo() const { return m_parentNo; }
	uint16_t GetNextNo() const { return m_nextNo; }
	uint16_t GetIkBoneNo() const { return m_ikBoneNo; }
	uint8_t GetType() const { return m_type; }
	const DirectX::XMFLOAT3& GetOriginPos() const { return m_pos; }

public:
	char m_name[20] = {};
	uint16_t m_parentNo = 0;
	uint16_t m_nextNo = 0;
	uint8_t m_type = 0;
	uint16_t m_ikBoneNo = 0;
	DirectX::XMFLOAT3 m_pos = {};
	
	// 各ボーンで毎フレーム適用するアフィン変換行列.
	DirectX::XMMATRIX m_trans = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_rot = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_affine = DirectX::XMMatrixIdentity();

	// 自分の子ボーン.
	std::vector<uint16_t> m_children = {};
};
