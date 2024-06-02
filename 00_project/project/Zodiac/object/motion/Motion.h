#pragma once
#include <DirectXMath.h>
#include <vector>
#include <map>
#include <string>
#include "../model/Model.h"

struct SMotionHeader {
	char fixString[30] = {};
	char modelName[20] = {};
};

struct SMotion {
	char boneName[15] = {};
	uint32_t flame = 0;
	DirectX::XMFLOAT3 trans = {};
	DirectX::XMFLOAT4 quat = {};
	uint8_t bezierParam[64] = {};
	void Serialize(FILE* fp)
	{
		if (!fp) { return; }
		fread(&boneName, sizeof(boneName), 1, fp);
		fread(&flame, sizeof(flame), 1, fp);
		fread(&trans, sizeof(trans), 1, fp);
		fread(&quat, sizeof(quat), 1, fp);
		fread(&bezierParam, sizeof(bezierParam), 1, fp);
	}
};

class CMotion {
public:
	CMotion() {}
	~CMotion() = default;

public:
	bool Load(const char* fileName);

public:
	void Update(CModel* model);

	void PlayAnimation(bool isLoop = false);
	void StopAnimation();

private:
	void ApplyMotion(CModel* model);
	void ApplyIK(CModel* model);

private:
	void SolveIK_LookAt(CModel* model, const SIkInfo& ikInfo);
	void SolveIK_CosineFormura(CModel* model, const SIkInfo& ikInfo);
	void SolveIK_CCD(CModel* model, const SIkInfo& ikInfo);

private:
	DirectX::XMMATRIX LookAtMatrix(DirectX::XMVECTOR src, DirectX::XMVECTOR dst, DirectX::XMVECTOR up, DirectX::XMVECTOR right);
	DirectX::XMMATRIX LookAtMatrix(DirectX::XMVECTOR dst, DirectX::XMVECTOR up, DirectX::XMVECTOR right);

private:
	float GetYfromXOnBezier(const float x, const SMotion& data, const int approximateNum);

private:
	SMotionHeader m_header = {};
	std::map<std::string, std::vector<SMotion>> m_motionMap = {};
	float m_flame = 0.0f;
	float m_lastFlame = 0.0f;
	bool m_isPlay;
	bool m_isEnd;
	bool m_isLoop;
};