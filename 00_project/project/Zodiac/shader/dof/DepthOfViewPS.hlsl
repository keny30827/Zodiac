#include "DepthOfViewDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 ret;

	float base = psDepthTex.Sample(psSamp, float2(0.5f, 0.5f));
	float now = psDepthTex.Sample(psSamp, input.uv);
	float diffDepth = saturate(pow(abs(base - now), 0.6f)) * 8.0f;
	float nPos, fPos;
	fPos = modf(diffDepth, nPos);
	float4 color[2];
	if (nPos == 0) {
		color[0] = psTex.Sample(psSamp, input.uv);
		color[1] = psDof.Sample(psSamp, input.uv);
	}
	else if (nPos == 1) {
		color[0] = psDof.Sample(psSamp, input.uv);
		color[1] = psDofShrink.Sample(psSamp, input.uv * 0.5f);
	}
	else {
		float2 sizeUV = input.uv * 0.5f;
		float2 posUV = float2(0.0f, 0.0f);
		for (int n = 2; n <= nPos; n++) {
			posUV.y = posUV.y + sizeUV.y;
			sizeUV = sizeUV * 0.5f;
		}
		color[0] = psDofShrink.Sample(psSamp, posUV + sizeUV);
		{
			posUV.y = posUV.y + sizeUV.y;
			sizeUV = sizeUV * 0.5f;
		}
		color[1] = psDofShrink.Sample(psSamp, posUV + sizeUV);
	}
	ret = lerp(color[0], color[1], fPos);

	return ret;
}