struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct OutputRenderTarget {
	float4 final : SV_TARGET0;
	float4 color : SV_TARGET1;
	float4 normal : SV_TARGET2;
	float4 highBright : SV_TARGET3;
	float4 objectInfo : SV_TARGET4;
	float4 specular : SV_TARGET5;
};

Texture2D<float> depth : register(t0);

Texture2D<float4> texColor : register(t1);
Texture2D<float4> texNormal : register(t2);
Texture2D<float4> texObjectInfo : register(t3);
Texture2D<float4> texSpecular : register(t4);

SamplerState psSamp : register(s0);

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix viewInv;
	matrix proj;
	matrix projInv;
	float2 screenParam;
	float2 padding;
	float4 eye;
};
