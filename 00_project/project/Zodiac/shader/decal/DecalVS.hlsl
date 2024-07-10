#include "DecalDataSet.hlsli"

OutputVSPS mainVS(
	float4 pos : POSITION,
	float2 uv : TEXCOORD
	)
{
	OutputVSPS output;
	output.pos = pos;//mul(world, pos);
	output.worldPos = output.pos;
	output.projPos = mul(mul(proj, view), output.pos);
	output.pos = output.projPos;
	output.uv = uv;
	return output;
}