#pragma once
#include "../graphics/GraphicsDefs.h"
#include "../object/render_target/RenderTarget.h"
#include "../object/depth_stencil/ShadowMap.h"

class CGraphicsController;
class CScene {
public:
	CScene() = default;
	~CScene() = default;

public:
	bool Init(CGraphicsController& rGraphicsController, uint32_t width, uint32_t height);
	void Term();

public:
	CRenderTarget& GetTestRT() { return m_testRT; }
	CRenderTarget& GetGaussian1RT() { return m_gaussian1RT; }
	CRenderTarget& GetGaussian2RT() { return m_gaussian2RT; }
	CRenderTarget& GetColor() { return m_color; }
	CRenderTarget& GetNormal() { return m_normal; }
	CRenderTarget& GetHighBrightness() { return m_highBrightness; }
	CRenderTarget& GetHighBrightnessShrinkBuffer() { return m_highBrightnessShrinkBuffer; }
	CRenderTarget& GetBloom() { return m_bloom; }
	CRenderTarget& GetDofShrinkBuffer() { return m_dofShrinkBuffer; }
	CRenderTarget& GetDof() { return m_dof; }
	CRenderTarget& GetSsao() { return m_ssao; }
	CRenderTarget& GetObjectInfo() { return m_objectInfo; }

	IDepthStencil& GetShadowMap() { return m_shadowMap; }
	IDepthStencil& GetDofDepth() { return m_dofDepth; }
	IDepthStencil& GetSsaoDepth() { return m_ssaoDepth; }
	

	uint32_t GetModelNum() const { return static_cast<uint32_t>(m_objectList.size()); }
	const IModel* GetModel(const uint32_t idx) const { return m_objectList[idx];  }

	uint32_t GetSpriteNum() const { return static_cast<uint32_t>(m_spriteList.size()); }
	const ISprite* GetSprite(const uint32_t idx) const { return m_spriteList[idx]; }

	const ISprite* GetFrameBuffer() const { return m_pFrameBuffer; }

	const ICamera* GetMainCamera() const { return m_pMainCamera; }

public:
	void AddModel(const IModel* pModel) { m_objectList.push_back(pModel); }
	void AddSprite(const ISprite* pObj) { m_spriteList.push_back(pObj); }
	void SetFrameBuffer(const ISprite* pObj) { m_pFrameBuffer = pObj; }
	void SetMainCamera(const ICamera* pObj) { m_pMainCamera = pObj; }

public:
	void ClearAll() { ClearModel(); ClearSprite(); ClearFrameBuffer();  ClearCamera(); }
	void ClearModel() { m_objectList.clear(); }
	void ClearSprite() { m_spriteList.clear(); }
	void ClearFrameBuffer() { m_pFrameBuffer = nullptr; }
	void ClearCamera() { m_pMainCamera = nullptr; }

private:
	// TODO いったんテスト用でここに.
	CRenderTarget m_testRT;
	// ガウシアンブラー用.
	CRenderTarget m_gaussian1RT;
	CRenderTarget m_gaussian2RT;
	// ディファード用.
	CRenderTarget m_color;
	CRenderTarget m_normal;
	// ブルーム用.
	CRenderTarget m_highBrightness;
	CRenderTarget m_highBrightnessShrinkBuffer;
	CRenderTarget m_bloom;
	// DOF用.
	CRenderTarget m_dofShrinkBuffer;
	CRenderTarget m_dof;
	// SSAO用.
	CRenderTarget m_ssao;
	// オブジェクト情報用.
	CRenderTarget m_objectInfo;

	// 深度関連.
	CShadowMap m_shadowMap;
	// DOF用.
	CShadowMap m_dofDepth;
	// SSAO用.
	CShadowMap m_ssaoDepth;

	// 描画に必要な外部からの設定情報.
	std::vector<const IModel*> m_objectList = {};
	std::vector<const ISprite*> m_spriteList = {};
	const ISprite* m_pFrameBuffer = nullptr;
	const ICamera* m_pMainCamera = nullptr;
};