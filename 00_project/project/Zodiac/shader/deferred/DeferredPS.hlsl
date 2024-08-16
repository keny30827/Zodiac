#include "DeferredDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 objectInfo = psGBufObjectInfo.Sample(psSamp, input.uv);
	
	if (objectInfo.x < 1.0f) {
		discard;
	}

	float4 color = psGBufColor.Sample(psSamp, input.uv);
	float4 normal = normalize(psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f);
	float4 specular = psGBufSpecular.Sample(psSamp, input.uv);
	float4 ambient = float4(objectInfo.yzw, 0.0f);
	float depth = psGBufDepth.Sample(psSamp, input.uv);

	// ワールド位置計算.
	//float4 worldPos = psGBufWorldPos.Sample(psSamp, input.uv);
	float4 worldPos = float4(input.uv * 2.0f - 1.0f, depth, 1.0f);
	{
		worldPos = mul(projInv, worldPos);
		worldPos /= worldPos.w;
		worldPos = mul(viewInv, worldPos);
	}

	float4 ray = float4(normalize(worldPos.xyz - eye.xyz), 1.0f);

	// タイル位置の計算.
	const int tileX = floor(input.pos.x / LIGHT_TILE_WIDTH);
	const int tileY = floor(input.pos.y / LIGHT_TILE_HEIGHT);
	const int tileW = int((screenParam.x + LIGHT_TILE_WIDTH - 1) / LIGHT_TILE_WIDTH);
	const int tileIndex = (tileY * tileW) + tileX;

	// 平行光源.
	//float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);
	// 光源色.
	//float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// ヒットしているライトの分だけ計算する.
	float4 resultColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	for (uint n = 0; n < lightNum; n++) {
		const int lightIndex = n;// int(rwLightIndices[tileIndex + n]);
		if (lightIndex < 0) {
			break;
		}

		SLightInfo lightInfo = light[lightIndex];
		float dist = abs(length(lightInfo.pos - worldPos));
		float lightRate = max(lightInfo.attenuationDistance - dist, 0.0f) / lightInfo.attenuationDistance;
		float4 lightColor = lightInfo.color * lightRate;

		// 法線と内積とった結果を使う.ランバートの余弦則.
		float brightnessValue = dot(normalize(lightInfo.pos - worldPos), normal);
		if (brightnessValue >= 0.0f) {
			float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);
			// ディフューズ.
			float4 diffuseColor = lightColor * color * brightness;
			// スペキュラ.
			float4 specularColor = lightColor * float4(specular.xyz * pow(saturate(dot(reflect(-(lightInfo.pos - worldPos), normal), -ray)), specular.a), 1.0f);
			// 環境光.
			float4 ambColor = lightColor * ambient;

			resultColor += (diffuseColor + specularColor + ambColor);
		}
	}

	resultColor /= lightNum;

	// SSAOの結果も入れる.
	//float4 ssao = psGBufSSAO.Sample(psSamp, input.uv);

	return resultColor;//* ssao;
}