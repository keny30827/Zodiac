#include "header/ModelOutlineDataSet.hlsli"

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
	// ローカル座標での位置計算.
	float fBoneWeight = boneW / 100.0f;
	matrix ipBone = (bone[boneNo[0]] * fBoneWeight) + (bone[boneNo[1]] * (1.0f - fBoneWeight));
	output.pos = mul(ipBone, pos);
	normal.w = 0.0f;	// そもデータ側で1.0fが入らないようにすべき.
	// アウトライン用なので、法線方向に少し押し出し.ローカル時点で押し出しちゃう.
	float4 dirEyeToVertex = mul(world, output.pos) - float4(eye, 1.0f);
	float distEyeToVertex = length(dirEyeToVertex);
	float rate = distEyeToVertex * tan(2.0f / fov);
	output.pos += normalize(normal) * outlineScale * rate;
	// --- モデルの外縁のアウトラインだけ出すように、少しZ方向に押し出す.
	output.pos += normalize(dirEyeToVertex);
	// ワールド座標に.
	output.pos = mul(world, output.pos);
	output.normal = mul(world, normal);
	// クリップ座標手前まで.
	output.pos = mul(mul(proj, view), output.pos);
	output.uv = uv;
	return output;
}