struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
};

Texture2D<float4> psGBufNormal : register(t0);
Texture2D<float> psDepthTex : register(t1);

SamplerState psSamp : register(s0);