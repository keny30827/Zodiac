#include "DeferredDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 objectInfo = psGBufObjectInfo.Sample(psSamp, input.uv);
	
	if (objectInfo.x < 1.0f) {
		// �A�E�g���C���h�Ȃ̂ŁA�J���[�P�F�ł��̂܂܏o��.
		if (objectInfo.x >= 0.5f) {
			return psGBufColor.Sample(psSamp, input.uv);
		}
		else {
			discard;
		}
	}

	float4 color = psGBufColor.Sample(psSamp, input.uv);
	float4 normal = psGBufNormal.Sample(psSamp, input.uv) * 2.0f - 1.0f;
	float4 specular = psGBufSpecular.Sample(psSamp, input.uv);
	float4 ambient = float4(objectInfo.yzw, 0.0f);
	float depth = psGBufDepth.Sample(psSamp, input.uv);

	// ���[���h�ʒu�v�Z.
	// TODO GBuffer�ɓ���Ă郏�[���h�ʒu���o�O���Ă���.�����A�i�[���鎞�_�ŕςȕ�Ԃ�����������Ă���ۂ�.
	// �������������������v�Z�����̂ŁA�ЂƂ܂���������g��.
	//float4 worldPos = psGBufWorldPos.Sample(psSamp, input.uv);
	float4 worldPos = float4(input.uv * 2.0f - 1.0f, depth, 1.0f);
	{
		// UV���W����N���b�v���W�ɂ���ۂ�Y�����]����̂�.
		worldPos.y *= -1.0f;
		worldPos = mul(projInv, worldPos);
		worldPos /= worldPos.w;
		worldPos = mul(viewInv, worldPos);
	}

	float4 ray = float4(normalize(worldPos.xyz - eye.xyz), 1.0f);

	// �^�C���ʒu�̌v�Z.
	const int tileX = floor((screenParam.x * input.uv.x) / LIGHT_TILE_WIDTH);
	const int tileY = floor((screenParam.y * input.uv.y) / LIGHT_TILE_HEIGHT);
	const int tileW = int((screenParam.x + LIGHT_TILE_WIDTH - 1) / LIGHT_TILE_WIDTH);
	const int tileIndex = ((tileY * tileW) + tileX) * lightNum;

	// ���s����.
	//float4 lightPos = float4(-1.0f, 1.0f, -1.0f, 0.0f);
	// �����F.
	//float4 lightColor = float4(0.8f, 0.8f, 0.8f, 1.0f);

	// �q�b�g���Ă��郉�C�g�̕������v�Z����.
	float4 resultColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint n = 0; n < lightNum; n++) {
		const int lightIndex = int(rwLightIndices[tileIndex + n]);
		if (lightIndex < 0) {
			break;
		}

		SLightInfo lightInfo = light[lightIndex];
		float dist = abs(length(lightInfo.pos - worldPos));
		float lightRate = max(lightInfo.attenuationDistance - dist, 0.0f) / lightInfo.attenuationDistance;
		float4 lightColor = lightInfo.color;;// *lightRate;

		// �@���Ɠ��ςƂ������ʂ��g��.�����o�[�g�̗]����.
		float brightnessValue = dot(normalize(lightInfo.pos - worldPos), normal);
		if (brightnessValue >= 0.0f) {
			float4 brightness = float4(brightnessValue, brightnessValue, brightnessValue, 1.0f);
			// �f�B�t���[�Y.
			float4 diffuseColor = lightColor * color * brightness;
			// �X�y�L����.
			float4 specularColor = min(max(lightColor * float4(specular.xyz * pow(saturate(dot(reflect(-(lightInfo.pos - worldPos), normal), -ray)), specular.a), 1.0f), 0.0f), 1.0f);
			// ����.
			float4 ambColor = lightColor * ambient;

			resultColor += (diffuseColor + specularColor + ambColor);
		}
	}

	resultColor /= lightNum;

	// SSAO�̌��ʂ������.
	float4 ssao = psGBufSSAO.Sample(psSamp, input.uv);
	return resultColor * ssao;
}