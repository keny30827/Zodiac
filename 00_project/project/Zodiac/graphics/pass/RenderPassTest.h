#pragma once

class CScene;
class CGraphicsController;
class CShaderManager;
class CRenderPassTest {
public:
	CRenderPassTest() = default;
	~CRenderPassTest() = default;

public:
	bool Init() { return true; }
	void Term() {}

public:
	void Render(CScene& scene, CGraphicsController& graphicsController, CShaderManager& shaderMgr);
};