#include "header/SpriteDataSet.hlsli"

OutputVSPS main(
	float4 pos : POSITION,
	float2 uv : TEXCOORD
	)
{
	OutputVSPS output;
	output.pos = mul(toScreen, pos);
	output.uv = uv;
	return output;
}