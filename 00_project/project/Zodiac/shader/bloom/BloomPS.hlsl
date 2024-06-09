#include "BloomDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 color = psTex.Sample(psSamp, input.uv);

	// 1枚目は同じ解像度なので、普通に加算.
	color = color + saturate(psBloom.Sample(psSamp, input.uv));

	// 2枚目は、縮小バッファ.縦横半分ずつになってるので、uv値アクセスを半分にしながらアクセスする.
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