#include"Camera.h"

#include "../../graphics/GraphicsController.h"
#include "../../graphics/GraphicsResourceManager.h"

CCamera::CCamera()
{
}

bool CCamera::Init(uint32_t width, uint32_t height)
{
#if 0
	m_dist = 5.0f;	// m.
#else
	m_dist = 15.0f;	// m.
#endif
	// 原点開始でdist分だけ後ろに下げる.atは原点合わせ.
	m_eye.z = -m_dist;
	// ロールしていない真上向いている状態で開始.
	m_up.y = 1.0f;

	// 透視投影変換用行列.
	m_perspectiveMat = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV2,
		static_cast<float>(width) / static_cast<float>(height),
		1.0f,
		100.0f);

	return true;
}

void CCamera::Update(const DirectX::XMFLOAT3* pFocusPosition)
{
	if (pFocusPosition) {
		DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&m_eye);
		DirectX::XMVECTOR at = DirectX::XMLoadFloat3(&m_at);
		DirectX::XMVECTOR dirToAt = DirectX::XMVectorSubtract(at, eye);
		DirectX::XMVECTOR dirToEye = DirectX::XMVectorSubtract(eye, at);
		dirToAt.m128_f32[3] = 0.0f;
		dirToEye.m128_f32[3] = 0.0f;
		at = DirectX::XMLoadFloat3(&(*pFocusPosition));
		eye = DirectX::XMVectorAdd(at, dirToEye);
		m_at.x = at.m128_f32[0];
		m_at.y = at.m128_f32[1];
		m_at.z = at.m128_f32[2];
		m_eye.x = eye.m128_f32[0];
		m_eye.y = eye.m128_f32[1];
		m_eye.z = eye.m128_f32[2];
	}
	m_view = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&m_eye),
		DirectX::XMLoadFloat3(&m_at),
		DirectX::XMLoadFloat3(&m_up));
}
