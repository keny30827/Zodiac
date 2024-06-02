#include "Player.h"
#include "../../graphics/GraphicsController.h"
#include "../../graphics/GraphicsResourceManager.h"
#include "DirectXTex.h"

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))

bool CPlayer::Init(CGraphicsController& graphicsController, float x, float y, float z)
{
	// 単位はm.
	m_pos.m128_f32[0] = x;
	m_pos.m128_f32[1] = y;
	m_pos.m128_f32[2] = z;
	m_pos.m128_f32[3] = 1.0f;


	// モデル読み込み.
	{
		m_model.Load("bin\\model\\player", "player.pmd", graphicsController);
	}

	// モーションデータ読み込み.
	{
		m_motion.Load(u8"bin\\model\\player\\squat.vmd");
	}

	m_motion.PlayAnimation(true);
	return true;
}

void CPlayer::Update()
{
	// ボーン情報は毎フレーム初期化して適用し直し.
	m_model.ClearBoneAll();

	// TODO とりあえず常時回転させてみる.
	// m_rot.m128_f32[1] += 0.05f;
	//m_rot.m128_f32[1] = DirectX::XM_PIDIV2;
	m_rot.m128_f32[1] = (DirectX::XM_PI < m_rot.m128_f32[1]) ? -DirectX::XM_PI : (m_rot.m128_f32[1] < -DirectX::XM_PI) ? DirectX::XM_PI : m_rot.m128_f32[1];
	m_model.SetRot(DirectX::XMMatrixRotationY(m_rot.m128_f32[1]));
	m_model.SetPos(DirectX::XMMatrixTranslationFromVector(m_pos));

	// アニメーション適用.
	m_motion.Update(&m_model);

	// モデル情報更新.
	m_model.Update();
}

