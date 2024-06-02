struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float4 viewNormal : NORMAL1;
	float4 ray : RAY;
	float2 uv : TEXCOORD;
	uint instID : SV_InstanceID;
};

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	matrix shadow;
	matrix lightView;
	matrix bone[256];
	float3 eye;
};
