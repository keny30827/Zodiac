struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 worldPos : WPOS;
	float4 projPos : PPOS;
	float2 uv : TEXCOORD;
};

struct OutputRenderTarget {
	float4 color : SV_TARGET0;
	float4 objInfo : SV_TARGET1;
};

Texture2D<float4> psTex : register(t0);
Texture2D<float4> psObjInfo : register(t1);
Texture2D<float4> psWorldPos : register(t2);
SamplerState psSamp : register(s0);

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
};
