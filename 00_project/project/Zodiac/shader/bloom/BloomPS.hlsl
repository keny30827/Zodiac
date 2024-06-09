#include "BloomDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 color = psTex.Sample(psSamp, input.uv);

	// 1���ڂ͓����𑜓x�Ȃ̂ŁA���ʂɉ��Z.
	color = color + saturate(psBloom.Sample(psSamp, input.uv));

	// 2���ڂ́A�k���o�b�t�@.�c���������ɂȂ��Ă�̂ŁAuv�l�A�N�Z�X�𔼕��ɂ��Ȃ���A�N�Z�X����.
	float4 addBloomShrink = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float2 sizeUV = input.uv * 0.5f;
	float2 posUV = float2(0.0f, 0.0f);
	for (int n = 0; n < 8; n++) {
		addBloomShrink = saturate(addBloomShrink) + saturate(psBloomShrink.Sample(psSamp, posUV + sizeUV));
		posUV.y = posUV.y + sizeUV.y;
		sizeUV = sizeUV * 0.5f;
	}
	color = color + saturate(addBloomShrink);

	return color;
}