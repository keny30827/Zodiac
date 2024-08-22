struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float2 uv : TEXCOORD;
};

struct OutputRenderTarget {
	float4 final : SV_TARGET0;
	float4 objectInfo : SV_TARGET1;
};

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	matrix shadow;
	matrix lightView;
	matrix bone[256];
	float3 eye;
	float outlineScale;
	float fov;
};
