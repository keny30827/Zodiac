#include "LightCullingDataSet.hlsli"

// タイルごとのフラスタムを取得する.
// Intelのサンプル「Deferred Rendering for Current and Future Rendering Pipelines 」.
void GetTileFrustumPlane(out float4 frustumPlanes[6], uint3 groupId)
{
    // タイルの最大・最小深度を浮動小数点に変換.
    float minTileZ = asfloat(sMinZ);
    float maxTileZ = asfloat(sMaxZ);

    // タイル幅で割ることで、座標系における1を1タイルとするタイル座標に変換している.
    // タイル幅の倍で割ることで、0を原点として、-〇〇 ~ +〇〇までの値を取るようにしている.
    float2 tileScale = screenParam.xy * rcp(float(2 * LIGHT_TILE_WIDTH));

    // groupIdは、screenParam / (LIGHT_TILE_WIDTH, LIGHT_TILE_HEIGHT)で求められた範囲になっている.
    // なので、tileScaleが中点となるような範囲になっている（例：tileScaleが50ならgroupIdの最大は100）.
    // つまり、groupIdを0 ~ MAXまで回した場合、tileBiasはtileScale ~ -tileScaleまでをとる.
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

    // 法線が正規化されていない4面についてだけ正規化する
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }
}

/*!
 * @brief カメラ空間での座標を計算する。
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