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
	float4 worldPos = psGBufWorldPos.Sample(psSamp, input.uv);
	
	float4 ray = float4(normalize(worldPos.xyz - eye.xyz), 1.0f);

	// ���s����.
	float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);

	// �����F.
	float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// �@���Ɠ��ςƂ������ʂ��g��.�����o�[�g�̗]����.
	float brightnessValue = dot(normalize(lightPos), normal);
	float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);

	// �f�B�t���[�Y.
	float4 diffuseColor = lightColor * color * brightness;

	// �X�y�L����.
	float4 specularColor = lightColor * float4(specular.xyz * pow(saturate(dot(reflect(-lightPos, normal), -ray)), specular.a), 1.0f);

	// ����.
	float4 ambColor = lightColor * ambient;

	// SSAO�̌��ʂ������.
	float4 ssao = psGBufSSAO.Sample(psSamp, input.uv);

	return (diffuseColor + specularColor + ambColor) * ssao;
}