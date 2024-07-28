#include "header/ShadowDataSet.hlsli"

OutputVSPS main(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneNo : BONE_NO,
	min16uint boneW : BONE_W,
	min16uint edge : EDGE,
	uint instID : SV_InstanceID
	)
{
	OutputVSPS output;
	float fBoneWeight = boneW / 100.0f;
	matrix ipBone = (bone[boneNo[0]] * fBoneWeight) + (bone[boneNo[1]] * (1.0f - fBoneWeight));
	output.pos = mul(ipBone, pos);
	output.pos = mul(world, output.pos);
	output.pos = mul(view, output.pos);
	output.pos = mul(proj, output.pos);
	return output;
}