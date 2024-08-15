#define LIGHT_MAX (256)
#define LIGHT_TILE_WIDTH (16)
#define LIGHT_TILE_HEIGHT (16)
#define LIGHT_TILE_SIZE (LIGHT_TILE_WIDTH * LIGHT_TILE_HEIGHT)

struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

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
	matrix toScreen;
	matrix proj;
	matrix projInv;
	float4 eye;
	float4 screenParam;
	SLightInfo light[LIGHT_MAX];
	uint lightNum;
};

Texture2D<float4> psGBufColor : register(t0);
Texture2D<float4> psGBufNormal : register(t1);
Texture2D<float4> psGBufSSAO : register(t2);
Texture2D<float4> psGBufObjectInfo : register(t3);
Texture2D<float4> psGBufSpecular : register(t4);
Texture2D<float4> psGBufWorldPos : register(t5);

SamplerState psSamp : register(s0);

// タイルライト情報.
RWStructuredBuffer<uint> rwLightIndices : register(u1);