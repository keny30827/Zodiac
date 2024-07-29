
#define LIGHT_MAX (256)
#define LIGHT_TILE_WIDTH (16)
#define LIGHT_TILE_HEIGHT (16)
#define LIGHT_CULLING_INDEX_MAX (300000)

struct SLightInfo {
	float4 pos;
	float4 dir;
	float4 color;
	float attenuationDistance;
	float limitAngle;
};

cbuffer cbuff0 : register(b0) {
	matrix world;
	matrix view;
	matrix proj;
	matrix projInv;
	float2 screenParam;
	SLightInfo light[LIGHT_MAX];
	int lightNum;
};

// �[�x.
Texture2D<float> depthTex : register(t0);

// �o�͗p�̃��C�g�C���f�b�N�X�o�b�t�@�[.
RWStructuredBuffer<uint> rwLightIndices : register(u0);

// �X���b�h�Ԃł̋��L������.
groupshared uint sMinZ; //�^�C���̍ŏ��[�x.
groupshared uint sMaxZ; //�^�C���̍ő�[�x.
groupshared uint sTileLightIndices[LIGHT_MAX]; //�^�C���ɐڐG���Ă���|�C���g���C�g�̃C���f�b�N�X.
groupshared uint sTileNumLights; //�^�C���ɐڐG���Ă���|�C���g���C�g�̐�.