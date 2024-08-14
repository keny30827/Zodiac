#include "LightCullingDataSet.hlsli"

// タイルごとのフラスタムを取得する.
// Intelのサンプル「Deferred Rendering for Current and Future Rendering Pipelines」.
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

    // c1はフラスタムの左面の法線.[-tileBias.x, 0, proj._11 * tileScale.x]を90度回転させた形になっている.
    // proj._11は1/tan(2/fovX)みたいな数値.おそらくだが、ビュー空間で分割を行った際の角度に合わせた形状のゆがみを吸収するための指定.
    // c2はフラスタムの上面の法線.[0, tileBias.y, proj._22 * tileScale.y]を90度回転させた形になっている.
    // proj._22は1/tan(2/fovY)みたいな数値.おそらくだが、ビュー空間で分割を行った際の角度に合わせた形状のゆがみを吸収するための指定.
    float4 c1 = float4(proj._11 * tileScale.x, 0.0, tileBias.x, 0.0);
    float4 c2 = float4(0.0, -proj._22 * tileScale.y, tileBias.y, 0.0);
    float4 c4 = float4(0.0, 0.0, 1.0, 0.0);

    // c4の1は1タイル分を表している.
    // c1を例にすると、c1の逆ということは、左面を-90度回転させたベクトルになる.
    // その状態で正回転方向に1タイル分進めたベクトル、ということは、.
    // 左面を1タイル正回転方向に回転させた面（つまり右面）に対して、-90度方向に回転させたベクトル、ということになるので、.
    // つまり、右面の法線ということになる.
    frustumPlanes[0] = c4 - c1; // 右面.
    frustumPlanes[1] = c1; // 左面.
    frustumPlanes[2] = c4 - c2; // 下面.
    frustumPlanes[3] = c2; // 上面.
    // 平面と点の距離の計算で使うために、d成分としてz位置を渡している.
    frustumPlanes[4] = float4(0.0, 0.0, 1.0, -minTileZ);
    frustumPlanes[5] = float4(0.0, 0.0, -1.0, maxTileZ);

    // 法線が正規化されていない4面についてだけ正規化する
    [unroll]
    for (uint i = 0; i < 4; ++i) {
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
    uint3 groupId          : SV_GroupID,            // ディスパッチ時に指定したグループ数内での座標ID.
    uint3 dispatchThreadId : SV_DispatchThreadID,   // グループxグループ内スレッド総数+SV_GroupThreadID.つまり、全スレッド内での座標ID.
    uint3 groupThreadId : SV_GroupThreadID       // グループ内スレッド(numthreads)での座標ID.
)
{
    // タイル内でのインデックスを求める.
	uint groupIndex = groupThreadId.y * LIGHT_TILE_WIDTH + groupThreadId.x;

    // 共有メモリを初期化する.
    if (groupIndex == 0) {
        sTileNumLights = 0;
        sMinZ = 0x7F7FFFFF; // floatの最大値.
        sMaxZ = 0;
    }

    uint2 frameUV = dispatchThreadId.xy;

    //ビュー空間での座標を計算する.
    float3 posInView = ComputePositionInCamera(frameUV);

    // 同期.
    GroupMemoryBarrierWithGroupSync();

    // タイルの最大・最小深度を求める.
    // この処理は並列するスレッド全てで排他的に処理される.
    InterlockedMin(sMinZ, asuint(posInView.z));
    InterlockedMax(sMaxZ, asuint(posInView.z));

    // ここで同期を取ることでタイルの最大・最小深度を正しいものにする.
    GroupMemoryBarrierWithGroupSync();

    // タイルの錘台を求める.
    float4 frustumPlanes[6];
    GetTileFrustumPlane(frustumPlanes, groupId);

    // タイルとポイントライトの衝突判定.
    // LIGHT_TILE_SIZE分を1タイルとして並列で動かしていて、タイル単位での採用ライトを決めているので、.
    // うまく処理が分散されるように開始位置はグループ内のID、そこからLIGHT_TILE_SIZEずつ飛ばす形にしている.
    for (uint lightIndex = groupIndex; lightIndex < lightNum; lightIndex += LIGHT_TILE_SIZE) {
        SLightInfo lightInfo = light[lightIndex];

        // タイルとの判定
        bool inFrustum = true;
        for (uint i = 0; i < 6; ++i) {
            // 平面の方程式らしい.
            // が、平面位置が加味されていないにもかかわらず、うまく行く理由がわからず.
            float4 lp = float4(lightInfo.posInView.xyz, 1.0f);
            float d = dot(frustumPlanes[i], lp);
            // -light.rangeの時は、ライト原点が面と同一座標にある時、それより大きいなら、少なくとも面の内部にはある.
            // 正方向に制限がかかっていないが、フラスタム外に出る程大きい場合は、他の面と内積を取った時に-light.rangeを下回るはず.
            // (例えば、左面に対して大きな正の値を取った場合、右面と内積を取ると法線が逆になるので、大きな負の値となって-light.rangeを下回る).
            inFrustum = inFrustum && (d >= -lightInfo.attenuationDistance);
        }

        // タイルと衝突している場合
        if (inFrustum) {
            uint listIndex;
            InterlockedAdd(sTileNumLights, 1, listIndex);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    // ここで同期を取ると、sTileLightIndicesにタイルと衝突しているライトのインデックスが積まれている
    GroupMemoryBarrierWithGroupSync();

    // ライトインデックスを出力バッファに出力.
    {
        // 画面全体でのタイル数（横）.
        uint numCellX = (screenParam.x + LIGHT_TILE_WIDTH - 1) / LIGHT_TILE_WIDTH;
        // 今のスレッドの画素位置からタイル座標を割り出した上での通し番号.
        uint tileIndex = floor(frameUV.x / LIGHT_TILE_WIDTH) + floor(frameUV.y / LIGHT_TILE_WIDTH) * numCellX;
        // １タイルあたりの配列数は最大数がライト数なので、ヒットしているしていないに関係なくバッファとしてはライト数を確保しておく.
        // で、タイルID順に格納しておく.
        uint lightStart = lightNum * tileIndex;
        for (uint lightIndex = groupIndex; lightIndex < sTileNumLights; lightIndex += LIGHT_TILE_SIZE) {
            rwLightIndices[lightStart + lightIndex] = sTileLightIndices[lightIndex];
        }

        // この時点でsTileNumLightsなどの共有変数の同期は取られているので、最後に終了の印を入れる.
        // タイルごとに先頭スレッドだけで行われる.
        if ((groupIndex == 0) && (sTileNumLights < lightNum)) {
            //-1で番兵。
            rwLightIndices[lightStart + sTileNumLights] = 0xffffffff;
        }
    }
}