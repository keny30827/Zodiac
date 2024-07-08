#include "DecalDataSet.hlsli"

OutputRenderTarget mainPS(OutputVSPS input)
{
	float4 objInfo = psObjInfo.Sample(psSamp, input.uv);
	if (objInfo.r <= 0.0f) {
		discard;
	}

	float4 worldInfo = psWorldPos.Sample(psSamp, input.uv);
	if (worldInfo.z < input.worldPos.z) {
		discard;
	}

	float4 color = psTex.Sample(psSamp, input.uv);

	OutputRenderTarget output;
	output.color = color;

	return output;
}