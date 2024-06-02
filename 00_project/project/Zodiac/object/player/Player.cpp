#include "Player.h"
#include "../../graphics/GraphicsController.h"
#include "../../graphics/GraphicsResourceManager.h"
#include "DirectXTex.h"

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))

bool CPlayer::Init(CGraphicsController& graphicsController, float x, float y, float z)
{
	// �P�ʂ�m.
	m_pos.m128_f32[0] = x;
	m_pos.m128_f32[1] = y;
	m_pos.m128_f32[2] = z;
	m_pos.m128_f32[3] = 1.0f;


	// ���f���ǂݍ���.
	{
		m_model.Load("bin\\model\\player", "player.pmd", graphicsController);
	}

	// ���[�V�����f�[�^�ǂݍ���.
	{
		m_motion.Load(u8"bin\\model\\player\\squat.vmd");
	}

	m_motion.PlayAnimation(true);
	return true;
}

void CPlayer::Update()
{
	// �{�[�����͖��t���[�����������ēK�p������.
	m_model.ClearBoneAll();

	// TODO �Ƃ肠�����펞��]�����Ă݂�.
	// m_rot.m128_f32[1] += 0.05f;
	//m_rot.m128_f32[1] = DirectX::XM_PIDIV2;
	m_rot.m128_f32[1] = (DirectX::XM_PI < m_rot.m128_f32[1]) ? -DirectX::XM_PI : (m_rot.m128_f32[1] < -DirectX::XM_PI) ? DirectX::XM_PI : m_rot.m128_f32[1];
	m_model.SetRot(DirectX::XMMatrixRotationY(m_rot.m128_f32[1]));
	m_model.SetPos(DirectX::XMMatrixTranslationFromVector(m_pos));

	// �A�j���[�V�����K�p.
	m_motion.Update(&m_model);

	// ���f�����X�V.
	m_model.Update();
}

