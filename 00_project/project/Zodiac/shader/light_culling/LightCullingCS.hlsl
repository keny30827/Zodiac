#include "LightCullingDataSet.hlsli"

// �^�C�����Ƃ̃t���X�^�����擾����.
// Intel�̃T���v���uDeferred Rendering for Current and Future Rendering Pipelines�v.
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

    // c1�̓t���X�^���̍��ʂ̖@��.[-tileBias.x, 0, proj._11 * tileScale.x]��90�x��]�������`�ɂȂ��Ă���.
    // proj._11��1/tan(2/fovX)�݂����Ȑ��l.�����炭�����A�r���[��Ԃŕ������s�����ۂ̊p�x�ɍ��킹���`��̂䂪�݂��z�����邽�߂̎w��.
    // c2�̓t���X�^���̏�ʂ̖@��.[0, tileBias.y, proj._22 * tileScale.y]��90�x��]�������`�ɂȂ��Ă���.
    // proj._22��1/tan(2/fovY)�݂����Ȑ��l.�����炭�����A�r���[��Ԃŕ������s�����ۂ̊p�x�ɍ��킹���`��̂䂪�݂��z�����邽�߂̎w��.
    float4 c1 = float4(proj._11 * tileScale.x, 0.0, tileBias.x, 0.0);
    float4 c2 = float4(0.0, -proj._22 * tileScale.y, tileBias.y, 0.0);
    float4 c4 = float4(0.0, 0.0, 1.0, 0.0);

    // c4��1��1�^�C������\���Ă���.
    // c1���ɂ���ƁAc1�̋t�Ƃ������Ƃ́A���ʂ�-90�x��]�������x�N�g���ɂȂ�.
    // ���̏�ԂŐ���]������1�^�C�����i�߂��x�N�g���A�Ƃ������Ƃ́A.
    // ���ʂ�1�^�C������]�����ɉ�]�������ʁi�܂�E�ʁj�ɑ΂��āA-90�x�����ɉ�]�������x�N�g���A�Ƃ������ƂɂȂ�̂ŁA.
    // �܂�A�E�ʂ̖@���Ƃ������ƂɂȂ�.
    frustumPlanes[0] = c4 - c1; // �E��.
    frustumPlanes[1] = c1; // ����.
    frustumPlanes[2] = c4 - c2; // ����.
    frustumPlanes[3] = c2; // ���.
    // ���ʂƓ_�̋����̌v�Z�Ŏg�����߂ɁAd�����Ƃ���z�ʒu��n���Ă���.
    frustumPlanes[4] = float4(0.0, 0.0, 1.0, -minTileZ);
    frustumPlanes[5] = float4(0.0, 0.0, -1.0, maxTileZ);

    // �@�������K������Ă��Ȃ�4�ʂɂ��Ă������K������
    [unroll]
    for (uint i = 0; i < 4; ++i) {
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
    uint3 groupId          : SV_GroupID,            // �f�B�X�p�b�`���Ɏw�肵���O���[�v�����ł̍��WID.
    uint3 dispatchThreadId : SV_DispatchThreadID,   // �O���[�vx�O���[�v���X���b�h����+SV_GroupThreadID.�܂�A�S�X���b�h���ł̍��WID.
    uint3 groupThreadId : SV_GroupThreadID       // �O���[�v���X���b�h(numthreads)�ł̍��WID.
)
{
    // �^�C�����ł̃C���f�b�N�X�����߂�.
	uint groupIndex = groupThreadId.y * LIGHT_TILE_WIDTH + groupThreadId.x;

    // ���L������������������.
    if (groupIndex == 0) {
        sTileNumLights = 0;
        sMinZ = 0x7F7FFFFF; // float�̍ő�l.
        sMaxZ = 0;
    }

    uint2 frameUV = dispatchThreadId.xy;

    //�r���[��Ԃł̍��W���v�Z����.
    float3 posInView = ComputePositionInCamera(frameUV);

    // ����.
    GroupMemoryBarrierWithGroupSync();

    // �^�C���̍ő�E�ŏ��[�x�����߂�.
    // ���̏����͕��񂷂�X���b�h�S�ĂŔr���I�ɏ��������.
    InterlockedMin(sMinZ, asuint(posInView.z));
    InterlockedMax(sMaxZ, asuint(posInView.z));

    // �����œ�������邱�ƂŃ^�C���̍ő�E�ŏ��[�x�𐳂������̂ɂ���.
    GroupMemoryBarrierWithGroupSync();

    // �^�C���̐�������߂�.
    float4 frustumPlanes[6];
    GetTileFrustumPlane(frustumPlanes, groupId);

    // �^�C���ƃ|�C���g���C�g�̏Փ˔���.
    // LIGHT_TILE_SIZE����1�^�C���Ƃ��ĕ���œ������Ă��āA�^�C���P�ʂł̗̍p���C�g�����߂Ă���̂ŁA.
    // ���܂����������U�����悤�ɊJ�n�ʒu�̓O���[�v����ID�A��������LIGHT_TILE_SIZE����΂��`�ɂ��Ă���.
    for (uint lightIndex = groupIndex; lightIndex < lightNum; lightIndex += LIGHT_TILE_SIZE) {
        SLightInfo lightInfo = light[lightIndex];

        // �^�C���Ƃ̔���
        bool inFrustum = true;
        for (uint i = 0; i < 6; ++i) {
            // ���ʂ̕������炵��.
            // ���A���ʈʒu����������Ă��Ȃ��ɂ�������炸�A���܂��s�����R���킩�炸.
            float4 lp = float4(lightInfo.posInView.xyz, 1.0f);
            float d = dot(frustumPlanes[i], lp);
            // -light.range�̎��́A���C�g���_���ʂƓ�����W�ɂ��鎞�A������傫���Ȃ�A���Ȃ��Ƃ��ʂ̓����ɂ͂���.
            // �������ɐ������������Ă��Ȃ����A�t���X�^���O�ɏo����傫���ꍇ�́A���̖ʂƓ��ς����������-light.range�������͂�.
            // (�Ⴆ�΁A���ʂɑ΂��đ傫�Ȑ��̒l��������ꍇ�A�E�ʂƓ��ς����Ɩ@�����t�ɂȂ�̂ŁA�傫�ȕ��̒l�ƂȂ���-light.range�������).
            inFrustum = inFrustum && (d >= -lightInfo.attenuationDistance);
        }

        // �^�C���ƏՓ˂��Ă���ꍇ
        if (inFrustum) {
            uint listIndex;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    // �����œ��������ƁAsTileLightIndices�Ƀ^�C���ƏՓ˂��Ă��郉�C�g�̃C���f�b�N�X���ς܂�Ă���
    GroupMemoryBarrierWithGroupSync();

    // ���C�g�C���f�b�N�X���o�̓o�b�t�@�ɏo��.
    {
        // ��ʑS�̂ł̃^�C�����i���j.
        uint numCellX = (screenParam.x + LIGHT_TILE_WIDTH - 1) / LIGHT_TILE_WIDTH;
        // ���̃X���b�h�̉�f�ʒu����^�C�����W������o������ł̒ʂ��ԍ�.
        uint tileIndex = floor(frameUV.x / LIGHT_TILE_WIDTH) + floor(frameUV.y / LIGHT_TILE_WIDTH) * numCellX;
        // �P�^�C��������̔z�񐔂͍ő吔�����C�g���Ȃ̂ŁA�q�b�g���Ă��邵�Ă��Ȃ��Ɋ֌W�Ȃ��o�b�t�@�Ƃ��Ă̓��C�g�����m�ۂ��Ă���.
        // �ŁA�^�C��ID���Ɋi�[���Ă���.
        uint lightStart = lightNum * tileIndex;
        for (uint lightIndex = groupIndex; lightIndex < sTileNumLights; lightIndex += LIGHT_TILE_SIZE) {
            rwLightIndices[lightStart + lightIndex] = sTileLightIndices[lightIndex];
        }

        // ���̎��_��sTileNumLights�Ȃǂ̋��L�ϐ��̓����͎���Ă���̂ŁA�Ō�ɏI���̈������.
        // �^�C�����Ƃɐ擪�X���b�h�����ōs����.
        if ((groupIndex == 0) && (sTileNumLights < lightNum)) {
            //-1�Ŕԕ��B
            rwLightIndices[lightStart + sTileNumLights] = 0xffffffff;
        }
    }
}