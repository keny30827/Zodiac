#include "header/SpriteDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 color = psTex.Sample(psSamp, input.uv);
	float4 normal = psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f;

	// ���s����.
	float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);

	// �����F.
	float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// �@���Ɠ��ςƂ������ʂ��g��.�����o�[�g�̗]����.
	float brightnessValue = dot(normalize(lightPos), normalize(normal));
	if (brightnessValue <= 0.0f) { brightnessValue = 1.0f; }
	float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);
	
	return color * brightness;
}