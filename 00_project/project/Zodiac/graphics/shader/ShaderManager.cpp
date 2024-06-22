#include "ShaderManager.h"

bool CShaderManager::Init(CGraphicsController& graphicsController)
{
	m_gaussianBlur.Init(graphicsController);
	m_bloom.Init(graphicsController);
	m_basicSprite.Init(graphicsController);
	m_dof.Init(graphicsController);
	m_ssao.Init(graphicsController);
	m_deferredRender.Init(graphicsController);
	return true;
}

void CShaderManager::Term()
{
	m_deferredRender.Term();
	m_ssao.Term();
	m_dof.Term();
	m_basicSprite.Term();
	m_bloom.Term();
	m_gaussianBlur.Term();
}