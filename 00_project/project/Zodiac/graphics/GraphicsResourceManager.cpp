#include "GraphicsResourceManager.h"
#include "GraphicsController.h"

bool CScene::Init(CGraphicsController& rGraphicsController, uint32_t width, uint32_t height)
{
	m_testRT.Init(rGraphicsController, width, height);
	m_gaussian1RT.Init(rGraphicsController, width, height);
	m_gaussian2RT.Init(rGraphicsController, width, height);
	m_shadowMap.Init(rGraphicsController, width, height);
	m_color.Init(rGraphicsController, width, height);
	m_specular.Init(rGraphicsController, width, height);
	m_normal.Init(rGraphicsController, width, height, ACCESS_ROOT_PARAM_G_BUF_NORMAL);
	m_worldPos.Init(rGraphicsController, width, height);
	m_highBrightness.Init(rGraphicsController, width, height);
	m_highBrightnessShrinkBuffer.Init(rGraphicsController, width, height);
	m_bloom.Init(rGraphicsController, width, height);
	m_dofShrinkBuffer.Init(rGraphicsController, width, height);
	m_dof.Init(rGraphicsController, width, height);
	m_dofDepth.Init(rGraphicsController, width, height);
	m_ssao.Init(rGraphicsController, width, height);
	m_ssaoDepth.Init(rGraphicsController, width, height);
	m_objectInfo.Init(rGraphicsController, width, height);
	return true;
}

void CScene::Term()
{
	m_objectInfo.Term();
	m_ssaoDepth.Term();
	m_ssao.Term();
	m_dofDepth.Term();
	m_dofShrinkBuffer.Term();
	m_dof.Term();
	m_bloom.Term();
	m_highBrightnessShrinkBuffer.Term();
	m_highBrightness.Term();
	m_worldPos.Term();
	m_normal.Term();
	m_specular.Term();
	m_color.Term();
	m_shadowMap.Term();
	m_gaussian2RT.Term();
	m_gaussian1RT.Term();
	m_testRT.Term();
	ClearAll();
}
