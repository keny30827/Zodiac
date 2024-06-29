struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float4 viewNormal : NORMAL1;
	float4 ray : RAY;
	float4 lightViewPos : TPOS;
	float2 uv : TEXCOORD;
	uint instID : SV_InstanceID;
};

struct OutputRenderTarget {
	float4 final : SV_TARGET0;
	float4 color : SV_TARGET1;
	float4 normal : SV_TARGET2;
	float4 highBright : SV_TARGET3;
	float4 objectInfo : SV_TARGET4;
};

Texture2D<float4> psTex : register(t0);
Texture2D<float4> psSpTex : register(t1);
Texture2D<float4> psSpaTex : register(t2);
Texture2D<float4> psToonTex : register(t3);
Texture2D<float4> psShadowTex : register(t4);

SamplerState psSamp : register(s0);
SamplerState psSampToon : register(s1);
SamplerComparisonState psSampShadow : register(s2);

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	matrix shadow;
	matrix lightView;
	matrix bone[256];
	float3 eye;
};
cbuffer cbuff1 : register(b1) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
	float isValidToon;
};