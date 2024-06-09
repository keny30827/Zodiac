#include "ShaderManager.h"

bool CShaderManager::Init(CGraphicsController& graphicsController)
{
	m_gaussianBlur.Init(graphicsController);
	return true;
}

void CShaderManager::Term()
{
	m_gaussianBlur.Term();
}