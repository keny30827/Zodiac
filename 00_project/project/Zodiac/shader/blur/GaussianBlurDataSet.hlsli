struct OutputVSPS {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer cbuff0 : register(b0) {
	matrix toScreen;
	matrix proj;
	matrix projInv;
};

cbuffer cbuff1 : register(b1) {
	float4 weight[2];
	float isEnableX;
	float isEnableY;
};

Texture2D<float4> psTex : register(t0);
SamplerState psSamp : register(s0);