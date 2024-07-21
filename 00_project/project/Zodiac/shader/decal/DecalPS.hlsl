#include "DecalDataSet.hlsli"

OutputRenderTarget mainPS(OutputVSPS input)
{
	float3 screenPos = input.projPos.xyz / input.projPos.w;
	float2 uv = (float2(screenPos.x, screenPos.y) + float2(1.0f, 1.0f)) * float2(0.5f, -0.5f);

	float4 objInfo = psObjInfo.Sample(psSamp, uv);
	if (objInfo.r < 1.0f) {
		discard;
	}

	float4 worldInfo = psWorldPos.Sample(psSamp, uv);
	if (worldInfo.z < input.worldPos.z) {
		discard;
	}

	float4 baseColor = psColor.Sample(psSamp, uv);

	float2 localUV = input.uv * float2(1.0f, -1.0f);
	float4 color = psTex.Sample(psSamp, localUV);
	// 黒はマスクしておく.
	if (!any(color.xyz)) {
		discard;
	}

	// ベースの色とブレンドする.
	float3 blendColor = (color.xyz * min(color.a, 1.0f)) + (baseColor.xyz * max(1.0f - color.a, 0.0f));

	OutputRenderTarget output;
	output.color = float4(blendColor.x, blendColor.y, blendColor.z, min(color.a + baseColor.a, 1.0f));
	output.objInfo = float4(objInfo.r, 0.0f, 0.0f, 0.0f);

	return output;
}