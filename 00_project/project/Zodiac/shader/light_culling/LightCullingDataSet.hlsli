
#define LIGHT_MAX (256)
#define LIGHT_TILE_WIDTH (16)
#define LIGHT_TILE_HEIGHT (16)
#define LIGHT_CULLING_INDEX_MAX (300000)

struct SLightInfo {
	float4 pos;
	float4 dir;
	float4 color;
	float attenuationDistance;
	float limitAngle;
};

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	SLightInfo light[LIGHT_MAX];
	int lightNum;
};

// 出力用のライトインデックスバッファー.
RWStructuredBuffer<uint> rwLightIndices : register(u0);