#include "SSAODataSet.hlsli"

// �^�������𐶐��ł���Ƃ�.���`�����@���������Ǝv�����A�����Ȍ����͂킩�炸.
float random(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(OutputVSPS input) : SV_TARGET
{
	// ���̃s�N�Z���̐[�x.
	float z = psDepthTex.Sample(psSamp, input.uv);

	// �[�x�ƍ��̃s�N�Z����UV�l����A����3D���W�𕜌�����.
	// �N���b�v��Ԃ��ƁAx = -1.0f ~ 1.0f, y = -1.0f ~ 1.0f, z = 0.0f ~ 1.0f�ɂȂ�͂��Ȃ̂�,.
	// UV�l��XY�Ɍ����ĂĔ͈͂𒲐����Ă���.
	float3 clipPos = float3(input.uv * 2.0f - 1.0f, z);
	clipPos.y *= -1.0f;
	float4 threeDPos = mul(projInv, float4(clipPos, 1.0f));
	threeDPos /= threeDPos.w;

	// ���̃s�N�Z���̖@��.
	float4 normal = psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f;

	// 1/�� �烶�@V(p, v) cos�� d��.V()�͓K�p������֐��itrue or false�j.cos���̓����o�[�g��.
	// ��́A1/�� ��0,2�� ��0,1/2�΁@V(p, v) cos�� (rd��) (rsin��d��) �Ƃ�������.���ʐϕ�.
	float radius = 0.5f;
	float ao = 0.0f;
	float ao_norm = 0.0f;
	if (z < 1.0f) {
		[unroll]
		for (int n = 0; n < 256; n++) {
			float rand_x = random(float2(n, n)) * 2.0f - 1.0f;
			float rand_y = random(float2(rand_x, n)) * 2.0f - 1.0f;
			float rand_z = random(float2(rand_x, rand_y)) * 2.0f - 1.0f;
			float4 omega = normalize(float4(rand_x, rand_y, rand_z, 0.0f));

			// �@���ƃ����_�������x�N�g���Ƃ̓��ς��Ƃ��āA�����ɂ����܂��Ă��邩�̔��聕�����o�[�g�̌��ʂƂ��Ďg��.
			float _dot = dot(omega, normal);

			// �����𔲂��o��.
			float unit = sign(_dot);
			if (unit < 0.0f) {
				// �����Ɏ��߂邽�߂ɕ������].
				omega *= unit;
				// �����o�[�g�Ŏg���̂ňꉞ�v�Z������.
				_dot = dot(omega, normal);
			}

			// ��΂����x�N�g�����̑��a�͌v�Z���Ă����āA�Ō�ɐ��K���ړI�Ŏg��.
			// V(p, v) cos�� �� V(p, v) �� 1.0f �Ƒz�肵�����̌v�Z.
			ao_norm += _dot;

			// ���̈ʒu�ƃ����_�������x�N�g��(������radius)����A�`�F�b�N����ʒu���v�Z����.
			float4 checkPos = threeDPos + (omega * radius);
			float4 clipPos = mul(proj, checkPos);
			clipPos /= clipPos.w;

			// V(p, v) cos��.
			float2 uv = (clipPos.xy + 1.0f) * 0.5f;
			uv.y *= -1.0f;
			ao += step(psDepthTex.Sample(psSamp, uv), clipPos.z) * _dot;
		}
		if (ao_norm > 0.0f) {
			ao /= ao_norm;
		}
	}

	return (1.0f - ao);
}