#include "header/DataSet.hlsli"

OutputVSPS BasicVS(
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
	if (instID == 1) {
		output.pos = mul(shadow, output.pos);
	}
	output.lightViewPos = mul(lightView, output.pos);
	output.worldPos = output.pos;
	output.pos = mul(mul(proj, view), output.pos);
	normal.w = 0.0f;	// ÇªÇ‡ÉfÅ[É^ë§Ç≈1.0fÇ™ì¸ÇÁÇ»Ç¢ÇÊÇ§Ç…Ç∑Ç◊Ç´.
	output.normal = mul(world, normal);
	output.viewNormal = normalize(mul(view, output.normal));
	output.uv = uv;
	output.ray = float4(normalize(mul(world, pos).xyz - eye), 0.0f);
	output.instID = instID;
	return output;
}