#include "DecalDataSet.hlsli"

OutputRenderTarget mainPS(OutputVSPS input)
{
	float2 useUV = input.uv * float2(1.0f, -1.0f);

	// TODO ここのUVは全画面でのUV値計算をしないといけない.
	float4 objInfo = psObjInfo.Sample(psSamp, useUV);
	if (objInfo.r < 1.0f) {
		discard;
	}

	// TODO ここのUVは全画面でのUV値計算をしないといけない.
	float4 worldInfo = psWorldPos.Sample(psSamp, useUV);
	if (worldInfo.z < input.worldPos.z) {
		discard;
	}

	float4 color = psTex.Sample(psSamp, useUV);

	OutputRenderTarget output;
	output.color = color;

	return output;
}