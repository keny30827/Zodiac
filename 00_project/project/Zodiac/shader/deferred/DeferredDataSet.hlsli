struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
};

Texture2D<float4> psGBufColor : register(t0);
Texture2D<float4> psGBufNormal : register(t1);
Texture2D<float4> psGBufSSAO : register(t2);
Texture2D<float4> psGBufObjectInfo : register(t3);

SamplerState psSamp : register(s0);