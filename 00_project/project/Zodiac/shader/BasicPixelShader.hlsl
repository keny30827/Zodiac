#include "header/DataSet.hlsli"

OutputRenderTarget BasicPS(OutputVSPS input)
{
	// ���s����.
	float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);

	// �����F.
	float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// �@���Ɠ��ςƂ������ʂ��g��.�����o�[�g�̗]����.
	float brightnessValue = dot(normalize(lightPos), normalize(input.normal));
	float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);
	if (isValidToon > 0.0f) {
		float v = saturate(brightnessValue);
		// �g�D�[���摜��0.0f�قǖ��邢�摜�Ȃ̂ŁA�P�x1.0f��v0.0f�ɂȂ�悤�Ƀ}�b�s���O.
		brightness = psToonTex.Sample(psSampToon, float2(0.0f, 1.0f - v));
	}

	// �f�B�t���[�Y.
	float4 diffuseColor = diffuse * brightness;

	// �e�N�X�`��.
	float4 texColor = float4(psTex.Sample(psSamp, input.uv));

	// �X�t�B�A�}�b�v�p�Ƀr���[�@����uv��.
	float2 useUV = float2((input.viewNormal.xy + float2(1.0f, 1.0f)) * float2(0.5f, -0.5f));

	// ��Z�X�t�B�A�e�N�X�`��.
	float4 spTexColor = float4(psSpTex.Sample(psSamp, useUV));

	// ���Z�X�t�B�A�e�N�X�`��.
	float4 spaTexColor = float4(psSpaTex.Sample(psSamp, useUV));

	// �A���r�G���g�i���ˌ��͖��l���j.
	float4 ambientBase = float4(ambient, 1.0f) * texColor;
	float4 ambientColor = lightColor * ambientBase;

	// �X�y�L�����[.
	float4 specularColor = lightColor * float4(specular.xyz * pow(saturate(dot(reflect(-lightPos, input.normal), -input.ray)), specular.a), 1.0f);

	float4 result = (lightColor * diffuseColor * texColor * spTexColor + spaTexColor + specularColor + ambientColor);

	// �Z���t�e�p�ɃV���h�E�}�b�v�`�F�b�N.
	// �N���b�v��Ԃ𐳋K���f�o�C�X��Ԃɕϊ�����.
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

	// �n�ʉe�p�̃C���X�^���X�͍���.
	if (input.instID == 1) {
		output.final = float4(0.0f, 0.0f, 0.0f, 1.0f);
		output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return output;
}