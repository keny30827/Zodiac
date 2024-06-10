#pragma once
#pragma once

#include "../GraphicsDefs.h"
#include "blur/GaussianBlur.h"
#include "bloom/Bloom.h"

class CGraphicsController;
class CShaderManager {
public:
	CShaderManager() {}
	~CShaderManager() {}

public:
	bool Init(CGraphicsController& graphicsController);
	void Term();

public:
	CGaussianBlur* GetGaussianBlurShader() { return &m_gaussianBlur; }
	CBloom* GetBloomShader() { return &m_bloom; }

private:
	CGaussianBlur m_gaussianBlur;
	CBloom m_bloom;
};