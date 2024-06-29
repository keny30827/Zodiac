#include "DeferredDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 color = psGBufColor.Sample(psSamp, input.uv);
	float4 normal = psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f;
	float4 objectInfo = psGBufObjectInfo.Sample(psSamp, input.uv);
	
	if (objectInfo.x < 1.0f) {
		discard;
	}

	// 平行光源.
	float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);

	// 光源色.
	float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// 法線と内積とった結果を使う.ランバートの余弦則.
	float brightnessValue = dot(normalize(lightPos), normalize(normal));
	float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);

	// SSAOの結果も入れる.
	float4 ssao = psGBufSSAO.Sample(psSamp, input.uv);

	return lightColor * color * brightness * ssao;
}