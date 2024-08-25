#define DOWN_SAMPLE_COUNT (4)

struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
	matrix viewInv;
	float4 eye;
	float4 screenParam;
};

Texture2D<float> psDepthTex : register(t0);
SamplerState psSamp : register(s0);