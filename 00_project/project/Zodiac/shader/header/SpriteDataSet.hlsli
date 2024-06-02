struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
	float isEnableDof;
	float isEnableSsao;
};

cbuffer cbuff1 : register(b1) {
	float4 weight[2];
	float isEnableX;
	float isEnableY;
};

Texture2D<float4> psTex : register(t0);
Texture2D<float4> psNormalTex : register(t1);
Texture2D<float> psDepthTex : register(t2);
Texture2D<float4> psGBufNormal : register(t3);
Texture2D<float4> psBloom : register(t4);
Texture2D<float4> psBloomShrink : register(t5);
Texture2D<float4> psDof : register(t6);
Texture2D<float4> psDofShrink : register(t7);
Texture2D<float4> psSSAO : register(t8);

SamplerState psSamp : register(s0);