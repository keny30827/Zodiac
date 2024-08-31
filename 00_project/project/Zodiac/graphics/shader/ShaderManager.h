#pragma once
#pragma once

#include "../GraphicsDefs.h"
#include "blur/GaussianBlur.h"
#include "bloom/Bloom.h"
#include "basic_sprite/BasicSprite.h"
#include "dof/DepthOfView.h"
#include "ssao/SSAO.h"
#include "deferred/DeferredRender.h"
#include "decal/2DDecal.h"
#include "light_culling/LightCulling.h"
#include "ssr/SSR.h"
#include "hierarchical_z/HierarchicalZ.h"
#include "basic_model/BasicModel.h"

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
	CSSAO* GetSSAOShader() { return &m_ssao; }
	CDeferredRender* GetDeferredRenderShader() { return &m_deferredRender; }
	C2DDecalShader* Get2DDecalShader() { return &m_2dDecal; }
	CLightCulling* GetLightCullingShader() { return &m_lightCulling; }
	CSSR* GetSSRShader() { return &m_ssr; }
	CHierarchicalZ* GetHierarchicalZShader() { return &m_hierarchicalZ; }
	CBasicModel* GetBasicModel() { return &m_basicModel; }

private:
	CGaussianBlur m_gaussianBlur;
	CBloom m_bloom;
	CBasicSprite m_basicSprite;
	CDepthOfView m_dof;
	CSSAO m_ssao;
	CDeferredRender m_deferredRender;
	C2DDecalShader m_2dDecal;
	CLightCulling m_lightCulling;
	CSSR m_ssr;
	CHierarchicalZ m_hierarchicalZ;
	CBasicModel m_basicModel;
};