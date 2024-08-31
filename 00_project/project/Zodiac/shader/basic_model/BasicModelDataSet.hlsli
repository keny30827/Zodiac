struct OutputVSPS {
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

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
