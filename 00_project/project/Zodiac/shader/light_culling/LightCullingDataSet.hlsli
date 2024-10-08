
#define LIGHT_MAX (256)
#define LIGHT_TILE_WIDTH (16)
#define LIGHT_TILE_HEIGHT (16)
#define LIGHT_TILE_SIZE (LIGHT_TILE_WIDTH * LIGHT_TILE_HEIGHT)
#define LIGHT_CULLING_INDEX_MAX (300000)

struct SLightInfo {
	float4 pos;
	float4 posInView;
	float4 dir;
	float4 color;
	float attenuationDistance;
	float limitAngle;
	float2 padding_;
};

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	matrix projInv;
	float2 screenParam;
	float2 padding;
	SLightInfo light[LIGHT_MAX];
	uint lightNum;
};

// 深度.
Texture2D<float> depthTex : register(t0);

// 出力用のライトインデックスバッファー.
RWStructuredBuffer<uint> rwLightIndices : register(u0);

// スレッド間での共有メモリ.
groupshared uint sMinZ; //タイルの最小深度.
groupshared uint sMaxZ; //タイルの最大深度.
groupshared uint sTileLightIndices[LIGHT_MAX]; //タイルに接触しているポイントライトのインデックス.
groupshared uint sTileNumLights; //タイルに接触しているポイントライトの数.