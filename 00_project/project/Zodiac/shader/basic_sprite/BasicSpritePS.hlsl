#include "BasicSpriteDataSet.hlsli"

float4 main(OutputVSPS input) : SV_TARGET
{
	float4 ret = psTex.Sample(psSamp, input.uv);
	return ret;
}