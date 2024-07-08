#include "DecalDataSet.hlsli"

OutputVSPS mainVS(
	float4 pos : POSITION,
	float2 uv : TEXCOORD
	)
{
	OutputVSPS output;
	output.pos = mul(world, pos);
	output.worldPos = output.pos;
	output.pos = mul(mul(proj, view), output.pos);
	output.uv = uv;
	return output;
}