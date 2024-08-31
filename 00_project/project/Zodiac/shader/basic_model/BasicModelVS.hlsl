#include "BasicModelDataSet.hlsli"

OutputVSPS main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD
	)
{
	OutputVSPS output;
	output.pos = mul(world, pos);
	output.normal = mul(world, normal);
	output.normal = normal;
	output.normal.w = 0.0f;
	output.pos = mul(mul(proj, view), output.pos);
	output.uv = uv;
	return output;
}