#pragma once
#pragma once

#include "../GraphicsDefs.h"
#include "blur/GaussianBlur.h"
#include "bloom/Bloom.h"
#include "basic_sprite/BasicSprite.h"
#include "dof/DepthOfView.h"

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
	CBasicSprite* GetBasicSpriteShader() { return &m_basicSprite; }
	CDepthOfView* GetDepthOfViewShader() { return &m_dof; }

private:
	CGaussianBlur m_gaussianBlur;
	CBloom m_bloom;
	CBasicSprite m_basicSprite;
	CDepthOfView m_dof;
};