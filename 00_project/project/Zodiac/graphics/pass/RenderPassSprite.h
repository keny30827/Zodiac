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

public:
	void SetRenderTailCallback(FFuncCallbackRenderTail f) { m_appRenderTailCallback = f; }

private:
	void RenderBloom(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor);
	void RenderDof(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor);
	void RenderSsao(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr, D3D12_VIEWPORT& viewPort, D3D12_RECT& scissor, const ICamera* pCamera);

private:
	// 表示デバッグ用のスプライト.
	CSprite m_gaussian;
	CSprite m_colorSprite;
	CSprite m_normalSprite;
	CSprite m_highBright;
	CSprite m_bloom;
	CSprite m_dof;
	CSprite m_ssao;

	CSprite m_postEffectBuffer;

	FFuncCallbackRenderTail m_appRenderTailCallback = nullptr;
};