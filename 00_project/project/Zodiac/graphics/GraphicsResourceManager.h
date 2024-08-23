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
	CRenderTarget& GetSpecular() { return m_specular; }
	CRenderTarget& GetNormal() { return m_normal; }
	CRenderTarget& GetWorldPos() { return m_worldPos; }
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
	IDepthStencil& GetDepthPrepass() { return m_depthPrepass; }

	uint32_t GetModelNum() const { return static_cast<uint32_t>(m_objectList.size()); }
	const IModel* GetModel(const uint32_t idx) const { return m_objectList[idx];  }

	uint32_t GetSpriteNum() const { return static_cast<uint32_t>(m_spriteList.size()); }
	const ISprite* GetSprite(const uint32_t idx) const { return m_spriteList[idx]; }

	uint32_t GetPlaneNum() const { return static_cast<uint32_t>(m_planeList.size()); }
	const IPlane* GetPlane(const uint32_t idx) const { return m_planeList[idx]; }
	

	uint32_t GetLightNum() const { return static_cast<uint32_t>(m_lightList.size()); }
	const SLightInfo* GetLight(const uint32_t idx) const { return m_lightList[idx]; }

	const ISprite* GetFrameBuffer() const { return m_pFrameBuffer; }

	const ICamera* GetMainCamera() const { return m_pMainCamera; }

	const IDecal* Get2DDecal() const { return m_p2DDecal; }

public:
	void AddModel(const IModel* pModel) { m_objectList.push_back(pModel); }
	void AddSprite(const ISprite* pObj) { m_spriteList.push_back(pObj); }
	void AddPlane(const IPlane* pObj) { m_planeList.push_back(pObj); }
	void AddLight(const SLightInfo* pObj) { m_lightList.push_back(pObj); }
	void SetFrameBuffer(const ISprite* pObj) { m_pFrameBuffer = pObj; }
	void SetMainCamera(const ICamera* pObj) { m_pMainCamera = pObj; }
	void Set2DDecal(const IDecal* pObj) { m_p2DDecal = pObj; }

public:
	void ClearAll() { ClearModel(); ClearSprite(); ClearLight(); ClearFrameBuffer();  ClearCamera(); ClearPlane(); }
	void ClearModel() { m_objectList.clear(); }
	void ClearSprite() { m_spriteList.clear(); }
	void ClearPlane() { m_planeList.clear(); }
	void ClearLight() { m_lightList.clear(); }
	void ClearFrameBuffer() { m_pFrameBuffer = nullptr; }
	void ClearCamera() { m_pMainCamera = nullptr; }

private:
	// TODO ��������e�X�g�p�ł�����.
	CRenderTarget m_testRT;
	// �K�E�V�A���u���[�p.
	CRenderTarget m_gaussian1RT;
	CRenderTarget m_gaussian2RT;
	// �f�B�t�@�[�h�p.
	CRenderTarget m_color;
	CRenderTarget m_specular;
	CRenderTarget m_normal;
	CRenderTarget m_worldPos;
	// �u���[���p.
	CRenderTarget m_highBrightness;
	CRenderTarget m_highBrightnessShrinkBuffer;
	CRenderTarget m_bloom;
	// DOF�p.
	CRenderTarget m_dofShrinkBuffer;
	CRenderTarget m_dof;
	// SSAO�p.
	CRenderTarget m_ssao;
	// �I�u�W�F�N�g���p.
	CRenderTarget m_objectInfo;

	// �[�x�֘A.
	CShadowMap m_shadowMap;
	CShadowMap m_depthPrepass;
	// DOF�p.
	CShadowMap m_dofDepth;
	// SSAO�p.
	CShadowMap m_ssaoDepth;

	// �`��ɕK�v�ȊO������̐ݒ���.
	std::vector<const IModel*> m_objectList = {};
	std::vector<const ISprite*> m_spriteList = {};
	std::vector<const IPlane*> m_planeList = {};
	std::vector<const SLightInfo*> m_lightList = {};
	const ISprite* m_pFrameBuffer = nullptr;
	const ICamera* m_pMainCamera = nullptr;
	const IDecal* m_p2DDecal = nullptr;
};