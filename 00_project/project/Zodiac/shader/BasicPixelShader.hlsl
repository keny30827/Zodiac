#include "header/DataSet.hlsli"

OutputRenderTarget BasicPS(OutputVSPS input)
{
	// 平行光源.
	float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);

	// 光源色.
	float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// 法線と内積とった結果を使う.ランバートの余弦則.
	float brightnessValue = dot(normalize(lightPos), normalize(input.normal));
	float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);
	if (isValidToon > 0.0f) {
		float v = saturate(brightnessValue);
		// トゥーン画像は0.0fほど明るい画像なので、輝度1.0fがv0.0fになるようにマッピング.
		brightness = psToonTex.Sample(psSampToon, float2(0.0f, 1.0f - v));
	}

	// ディフューズ.
	float4 diffuseColor = diffuse * brightness;

	// テクスチャ.
	float4 texColor = float4(psTex.Sample(psSamp, input.uv));

	// スフィアマップ用にビュー法線をuv化.
	float2 useUV = float2((input.viewNormal.xy + float2(1.0f, 1.0f)) * float2(0.5f, -0.5f));

	// 乗算スフィアテクスチャ.
	float4 spTexColor = float4(psSpTex.Sample(psSamp, useUV));

	// 加算スフィアテクスチャ.
	float4 spaTexColor = float4(psSpaTex.Sample(psSamp, useUV));

	// アンビエント（入射光は未考慮）.
	float4 ambientBase = float4(ambient, 1.0f) * texColor;
	float4 ambientColor = lightColor * ambientBase;

	// スペキュラー.
	float4 specularColor = lightColor * float4(specular.xyz * pow(saturate(dot(reflect(-lightPos, input.normal), -input.ray)), specular.a), 1.0f);

	float4 result = (lightColor * diffuseColor * texColor * spTexColor + spaTexColor + specularColor + ambientColor);

	// セルフ影用にシャドウマップチェック.
	// クリップ空間を正規化デバイス空間に変換する.
	float3 shadowNormalPos = input.lightViewPos.xyz / input.lightViewPos.w;
	float2 shadowUV = (shadowNormalPos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
	float depth = psShadowTex.SampleCmp(psSampShadow, shadowUV, (shadowNormalPos.z - 0.001f));
	result *= lerp(0.5f, 1.0f, depth);

	OutputRenderTarget output;
	output.final = result;
	output.color = (diffuse * texColor * spTexColor) + spaTexColor;
	output.specular = specular;
	output.normal = (normalize(input.normal) + 1.0f) / 2.0f;
	output.worldPos = input.worldPos;
	float Y = dot(result.rgb, float3(0.299, 0.587, 0.1114));
	output.highBright = (Y > 0.9f) ? result : 0.0f;
	output.objectInfo = float4(1.0f, ambientBase.x, ambientBase.y, ambientBase.z);

	// 地面影用のインスタンスは黒に.
	if (input.instID == 1) {
		output.final = float4(0.0f, 0.0f, 0.0f, 1.0f);
		output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return output;
}