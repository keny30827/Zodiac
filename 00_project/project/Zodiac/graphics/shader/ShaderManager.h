#pragma once
#pragma once

#include "../GraphicsDefs.h"
#include "blur/GaussianBlur.h"

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

private:
	CGaussianBlur m_gaussianBlur;
};