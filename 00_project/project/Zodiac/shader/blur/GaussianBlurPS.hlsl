#include "GaussianBlurDataSet.hlsli"

// �K�E�V�A���u���[.
float4 GaussianBlur(OutputVSPS input)
{
	// ����9�ߖT�i���g���܂ށj�̕��ω�.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float dy = (1.0f / h) * 2.0f;	// �{�P����������邽�߂ɌW�������Ă�.�傫������΃{�P�xUP.
	float4 ret = 0.0f;
	// �����̈ʒu.
	{
		ret += psTex.Sample(psSamp, input.uv) * weight[0][0];
	}
	if (isEnableX > 0.0f) {		
		// �㉺.
		for (uint n = 1; n < 8; n++) {
			ret += psTex.Sample(psSamp, input.uv + float2(dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
			ret += psTex.Sample(psSamp, input.uv + float2(-dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
		}
	}
	if (isEnableY > 0.0f) {
		// ���E.
		for (uint n = 1; n < 8; n++) {
			ret += psTex.Sample(psSamp, input.uv + float2(0.0f, dy * float(n))) * weight[n >> 2][n % 4];
			ret += psTex.Sample(psSamp, input.uv + float2(0.0f, -dy * float(n))) * weight[n >> 2][n % 4];
		}
	}
	return ret;
}

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 ret = 0.0f;
	if ((isEnableX > 0.0f) || (isEnableY > 0.0f)) {
		ret = GaussianBlur(input);
	}
	return ret;
}