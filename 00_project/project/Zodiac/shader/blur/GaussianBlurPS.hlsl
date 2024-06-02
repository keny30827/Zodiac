#include "GaussianBlurDataSet.hlsli"

// ガウシアンブラー.
float4 GaussianBlur(OutputVSPS input)
{
	// 周辺9近傍（自身を含む）の平均化.
	float w, h, levels = 0.0f;
	psTex.GetDimensions(0, w, h, levels);
	float dx = (1.0f / w) * 2.0f;	// ボケ具合を強くするために係数かけてる.大きくすればボケ度UP.
	float dy = (1.0f / h) * 2.0f;	// ボケ具合を強くするために係数かけてる.大きくすればボケ度UP.
	float4 ret = 0.0f;
	// 自分の位置.
	{
		ret += psTex.Sample(psSamp, input.uv) * weight[0][0];
	}
	if (isEnableX > 0.0f) {		
		// 上下.
		for (uint n = 1; n < 8; n++) {
			ret += psTex.Sample(psSamp, input.uv + float2(dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
			ret += psTex.Sample(psSamp, input.uv + float2(-dx * float(n), 0.0f)) * weight[n >> 2][n % 4];
		}
	}
	if (isEnableY > 0.0f) {
		// 左右.
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