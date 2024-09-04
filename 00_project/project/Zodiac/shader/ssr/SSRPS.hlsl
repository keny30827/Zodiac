#include "SSRDataSet.hlsli"

OutputRenderTarget main(OutputVSPS input)
{
	OutputRenderTarget output;

	// �X�N���[�����W����uv���v�Z����.
	float2 displayUV = float2(input.pos.x / screenParam.x, input.pos.y / screenParam.y);

	// UV�Ɛ[�x���烏�[���h��ԂɈړ�������.
	float4 basePos = float4(displayUV, input.pos.z, 1.0f);
	{
		// �N���b�v��Ԃ�.
		basePos.x = basePos.x * 2.0f - 1.0f;
		basePos.y = (basePos.y * 2.0f - 1.0f) * -1.0f;
		// �r���[���.
		basePos = mul(projInv, basePos);
		basePos /= basePos.w;
		// ���[���h���.
		basePos = mul(viewInv, basePos);
	}

	// ���[���h��ԏ�Ŗ@�����甽�˕��������߂�.
	float4 ray = float4(0.0f, 0.0f, 0.0f, 0.0f);
	{
		float4 dirEyeToPos = float4(basePos.xyz - eye.xyz, 0.0f);
		ray = normalize(reflect(dirEyeToPos, normalize(input.normal)));
	}

	// ���˕����ɃX�e�b�v�𓥂܂��āA�����̈ʒu���X�N���[�����W�ɂ��Đ[�x���`�F�b�N.
	// �����̗����ɓ����Ă�����Ƃ肠�����������g��.�����ĂȂ��Ȃ玟�̃X�e�b�v.
	// ���񂩃X�e�b�v�𓥂݁A���ɂ��q�b�g���Ȃ������畨�̐F���o��.
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 pos = basePos;
	[unroll]
	for (int n = 0; n < 128; ++n) {
		pos = pos + (ray * 0.20f);
		// �r���[���.
		float4 viewPos = mul(view, pos);
		// �N���b�v���.
		float4 clipPos = mul(proj, viewPos);
		clipPos /= clipPos.w;
		// uv.
		float2 checkUV = float2((clipPos.x + 1.0f) * 0.5f, (clipPos.y * -1.0f + 1.0f) * 0.5f);
		// �[�x.
		float checkDepth = depth.Sample(psSamp, checkUV);
		// �����̗��ɉB��Ă�.
		if (checkDepth < clipPos.z) {
			// �O�ʂɂ���ʒu�����[���h�ʒu�ɖ߂��āAZ�̍������ł�������ꍇ�͖�������.
			float4 frontPos = float4(checkUV, checkDepth, 1.0f);
			// �N���b�v��Ԃ�.
			frontPos.x = frontPos.x * 2.0f - 1.0f;
			frontPos.y = (frontPos.y * 2.0f - 1.0f) * -1.0f;
			// �r���[���.
			frontPos = mul(projInv, frontPos);
			frontPos /= frontPos.w;
			// ���[���h���.
			frontPos = mul(viewInv, frontPos);
			if ((pos.z - frontPos.z) <= 2.75f) {
				color = texColor.Sample(psSamp, checkUV);
				break;
			}
		}
	}

	output.color = color;
	output.normal = (normalize(input.normal) + 1.0f) / 2.0f;
	output.objectInfo = float4(1.0f, 0.5f, 0.5f, 0.5f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return output;
}