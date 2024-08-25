#include "HierarchicalZDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float screenX = screenParam.x * input.uv.x;
	float screenY = screenParam.y * input.uv.y;

	// �~�b�v�͈͂�����.
	// TODO �啪���ʂȂ̂ŁACPU���Ōv�Z���ď��n������������.�������͌Œ�l�Ƃ��ł���������.
	float4 mipRange[DOWN_SAMPLE_COUNT];
	float4 base = float4(0.0f, 0.0f, screenParam.x / 2.0f, screenParam.y / 2.0f);
	for (int idx = 0; idx < DOWN_SAMPLE_COUNT; idx++) {
		mipRange[idx] = base;
		base.y = mipRange[idx].y + mipRange[idx].w;
		base.z = mipRange[idx].z / 2.0f;
		base.w = mipRange[idx].w / 2.0f;
	}

	// �~�b�v�͈͂��v�Z�����̂��A���x���ɍ��킹�Ď��͏����Q�Ƃ��Đ[�x�ő�l���v�Z����.
	float selectDepth = 0.0f;
	for (int n = 0; n < DOWN_SAMPLE_COUNT; n++) {
		if ((mipRange[n].x <= screenX) && (screenX < (mipRange[n].x + mipRange[n].z))) {
			if ((mipRange[n].y <= screenY) && (screenY < (mipRange[n].y + mipRange[n].w))) {
				float baseScreenX = screenX - mipRange[n].x;
				float baseScreenY = screenY - mipRange[n].y;
				int level = pow(2, (n + 1));
				[unroll]
				for (int ox = 0; ox < level; ox++) {
					[unroll]
					for (int oy = 0; oy < level; oy++) {
						float fetchScreenX = (baseScreenX * level) + ox;
						float fetchScreenY = (baseScreenY * level) + oy;
						float2 fetchUV = float2(fetchScreenX / screenParam.x, fetchScreenY / screenParam.y);
						float fetchValue = psDepthTex.Sample(psSamp, fetchUV);
						selectDepth = max(selectDepth, fetchValue);
					}
				}
				break;
			}
		}
	}

	return float4(selectDepth, selectDepth, selectDepth, 1.0f);
}