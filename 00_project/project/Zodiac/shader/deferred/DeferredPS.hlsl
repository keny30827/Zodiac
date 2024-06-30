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

	// ���ˌv�Z�Ɏg�����߂̃x�N�g����������3D�ʒu����v�Z���Ă���.
	// ���̃s�N�Z���̐[�x.
	float z = psGBufNormal.Sample(psSamp, input.uv).x;
	// �[�x�ƍ��̃s�N�Z����UV�l����A����3D���W�𕜌�����.
	// �N���b�v��Ԃ��ƁAx = -1.0f ~ 1.0f, y = -1.0f ~ 1.0f, z = 0.0f ~ 1.0f�ɂȂ�͂��Ȃ̂�,.
	// UV�l��XY�Ɍ����ĂĔ͈͂𒲐����Ă���.
	float4 threeDPos = mul(projInv, float4(input.uv * 2.0f - 1.0f, z, 1.0f));
	threeDPos /= threeDPos.w;
	float4 ray = float4(normalize(threeDPos.xyz - eye.xyz), 0.0f);

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

	// SSAO�̌��ʂ������.
	float4 ssao = psGBufSSAO.Sample(psSamp, input.uv);

	return (diffuseColor + specularColor) * ssao;
}