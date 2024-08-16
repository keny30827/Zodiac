#pragma once
#include <DirectXMath.h>
#include <string>
#include "../model/Model.h"
#include "../motion/Motion.h"

class CGraphicsController;

class CScene;
class CPlayer {
public:
	CPlayer() = default;
	~CPlayer() = default;

public:
	bool Init(CGraphicsController& graphicsController, float x, float y, float z);
	void Update();

public:
	const CModel& GetModel() const { return m_model; }

public:
	DirectX::XMFLOAT3 GetPos() const { return DirectX::XMFLOAT3(m_pos.m128_f32[0], m_pos.m128_f32[1], m_pos.m128_f32[2]); }

#if defined(DEBUG)
public:
	void OnDebugInputPos(DirectX::XMFLOAT3 pos)
	{
		m_pos.m128_f32[0] = pos.x;
		m_pos.m128_f32[1] = pos.y;
		m_pos.m128_f32[2] = pos.z;
	}
#endif

private:
	DirectX::XMVECTOR m_pos = {};
	DirectX::XMVECTOR m_rot = {};
	DirectX::XMVECTOR m_size = {};
	CModel m_model = {};
	CMotion m_motion = {};
};