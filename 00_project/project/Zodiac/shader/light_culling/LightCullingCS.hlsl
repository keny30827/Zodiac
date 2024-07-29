#include "LightCullingDataSet.hlsli"

// �^�C�����Ƃ̃t���X�^�����擾����.
// Intel�̃T���v���uDeferred Rendering for Current and Future Rendering Pipelines �v.
void GetTileFrustumPlane(out float4 frustumPlanes[6], uint3 groupId)
{
    // �^�C���̍ő�E�ŏ��[�x�𕂓������_�ɕϊ�.
    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);

    // �^�C�����Ŋ��邱�ƂŁA���W�n�ɂ�����1��1�^�C���Ƃ���^�C�����W�ɕϊ����Ă���.
    // �^�C�����̔{�Ŋ��邱�ƂŁA0�����_�Ƃ��āA-�Z�Z ~ +�Z�Z�܂ł̒l�����悤�ɂ��Ă���.
    float2 tileScale = screenParam.xy * rcp(float(2 * LIGHT_TILE_WIDTH));

    // groupId�́AscreenParam / (LIGHT_TILE_WIDTH, LIGHT_TILE_HEIGHT)�ŋ��߂�ꂽ�͈͂ɂȂ��Ă���.
    // �Ȃ̂ŁAtileScale�����_�ƂȂ�悤�Ȕ͈͂ɂȂ��Ă���i��FtileScale��50�Ȃ�groupId�̍ő��100�j.
    // �܂�AgroupId��0 ~ MAX�܂ŉ񂵂��ꍇ�AtileBias��tileScale ~ -tileScale�܂ł��Ƃ�.
    float2 tileBias = tileScale - float2(groupId.xy);

    // .
    float4 c1 = float4(proj._11 * tileScale.x, 0.0, -tileBias.x, 0.0);
    float4 c2 = float4(0.0, -proj._22 * tileScale.y, -tileBias.y, 0.0);
    float4 c4 = float4(0.0, 0.0, 1.0, 0.0);

    frustumPlanes[0] = c4 - c1; // Right
    frustumPlanes[1] = c1; // Left
    frustumPlanes[2] = c4 - c2; // Top
    frustumPlanes[3] = c2; // Bottom
    frustumPlanes[4] = float4(0.0, 0.0, 1.0, -minTileZ);
    frustumPlanes[5] = float4(0.0, 0.0, -1.0, maxTileZ);

    // �@�������K������Ă��Ȃ�4�ʂɂ��Ă������K������
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
}

/*!
 * @brief �J������Ԃł̍��W���v�Z����B
 */
float3 ComputePositionInCamera(uint2 globalCoords)
{
    float2 st = ((float2)globalCoords + 0.5) * rcp(screenParam.xy);
    st = st * float2(2.0, -2.0) - float2(1.0, -1.0);
    float3 screenPos;
    screenPos.xy = st.xy;
    screenPos.z = depthTex.Load(uint3(globalCoords, 0.0f));
    float4 cameraPos = mul(projInv, float4(screenPos, 1.0f));

    return cameraPos.xyz / cameraPos.w;
}

[numthreads(LIGHT_TILE_WIDTH, LIGHT_TILE_HEIGHT, 1)]
void main(
	uint3 groupId          : SV_GroupID,
	uint3 dispatchThreadId : SV_DispatchThreadID,
	uint3 groupThreadId : SV_GroupThreadID
)
{
	uint groupIndex = groupThreadId.y * LIGHT_TILE_WIDTH + groupThreadId.x;
	rwLightIndices[groupIndex] = 0;
}