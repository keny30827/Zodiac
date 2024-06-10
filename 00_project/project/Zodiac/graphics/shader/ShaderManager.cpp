#include "ShaderManager.h"

bool CShaderManager::Init(CGraphicsController& graphicsController)
{
	m_gaussianBlur.Init(graphicsController);
	m_bloom.Init(graphicsController);
	m_basicSprite.Init(graphicsController);
	return true;
}

void CShaderManager::Term()
{
	m_basicSprite.Term();
	m_bloom.Term();
	m_gaussianBlur.Term();
}