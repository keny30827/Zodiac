struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
	float4 eye;
};

Texture2D<float4> psGBufColor : register(t0);
Texture2D<float4> psGBufNormal : register(t1);
Texture2D<float4> psGBufSSAO : register(t2);
Texture2D<float4> psGBufObjectInfo : register(t3);
Texture2D<float4> psGBufSpecular : register(t4);
Texture2D<float4> psGBufWorldPos : register(t5);

SamplerState psSamp : register(s0);