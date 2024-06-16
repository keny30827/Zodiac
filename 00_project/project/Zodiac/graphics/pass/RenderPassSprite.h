#pragma once

#include "../../object/sprite/Sprite.h"

class CGraphicsController;
class CScene;
class CShaderManager;
class CRenderPassSprite {
public:
	CRenderPassSprite() = default;
	~CRenderPassSprite() = default;

public:
	bool Init(CGraphicsController& graphicsController);
	void Term();

public:
	void Render(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr);

private:
	void RenderBloom(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor);
	void RenderDof(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor);
	void RenderSsao(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor, const ICamera* pCamera);

private:
	CSprite m_gaussian1;
	CSprite m_gaussian2;
	CSprite m_colorSprite;
	CSprite m_normalSprite;
	CSprite m_highBright;
	CSprite m_highBrightShrinkBuffer;
	CSprite m_highBrightShrinkBufferForDisp;
	CSprite m_bloom;
	CSprite m_dofShrinkBuffer;
	CSprite m_dofShrinkBufferForDisp;
	CSprite m_dof;
	CSprite m_ssao;
	CSprite m_ssaoForDisp;

	CSpriteNew m_postEffectBuffer;
};