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
	// ���[�J�����W�ł̈ʒu�v�Z.
	float fBoneWeight = boneW / 100.0f;
	matrix ipBone = (bone[boneNo[0]] * fBoneWeight) + (bone[boneNo[1]] * (1.0f - fBoneWeight));
	output.pos = mul(ipBone, pos);
	normal.w = 0.0f;	// �����f�[�^����1.0f������Ȃ��悤�ɂ��ׂ�.
	// �A�E�g���C���p�Ȃ̂ŁA�@�������ɏ��������o��.���[�J�����_�ŉ����o�����Ⴄ.
	float4 dirEyeToVertex = mul(world, output.pos) - float4(eye, 1.0f);
	float distEyeToVertex = length(dirEyeToVertex);
	float rate = distEyeToVertex * tan(2.0f / fov);
	output.pos += normalize(normal) * outlineScale * rate;
	// --- ���f���̊O���̃A�E�g���C�������o���悤�ɁA����Z�����ɉ����o��.
	output.pos += normalize(dirEyeToVertex);
	// ���[���h���W��.
	output.pos = mul(world, output.pos);
	output.normal = mul(world, normal);
	// �N���b�v���W��O�܂�.
	output.pos = mul(mul(proj, view), output.pos);
	output.uv = uv;
	return output;
}