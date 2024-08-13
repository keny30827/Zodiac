#pragma once
#include <Windows.h>
#include <string>

#include "../graphics/GraphicsController.h"
#include "../graphics/GraphicsResourceManager.h"
#include "../graphics/shader/ShaderManager.h"

#include "../graphics/pass/RenderPassTest.h"
#include "../graphics/pass/RenderPassSprite.h"

#include "../object/player/Player.h"
#include "../object/camera/Camera.h"
#include "../object/sprite/Sprite.h"
#include "../object/decal/Decal.h"

#if defined(DEBUG)
#include "../debug/DebugManager.h"
#endif

struct SApplicationOption {
	std::string appClassName;
	std::string appTitleName;
	long x = 0, y = 0, w = 0, h = 0;
	WNDPROC proc = nullptr;
	long windowStyle = 0;
	long windowExStyle = 0;
};

class CApplication {
public:
	CApplication();
	~CApplication() = default;

public:
	bool Init(SApplicationOption option);
private:
	bool InitWindow(SApplicationOption option);

public:
	void Term();
private:
	void TermWindow();

public:
	bool Update();
private:
	void UpdateGame();

public:
	inline HWND GetWindowHandle() const { return m_hWnd; }

private:
	void OnRenderTail(CGraphicsController* pController);

#if defined(DEBUG)
public:
	CPlayer& GetPlayer() { return m_player; }
	CCamera& GetCamera() { return m_camera; }
#endif

private:
	// Window生成用の変数.
	SApplicationOption m_option;
	WNDCLASSEX m_windowClass;
	HWND m_hWnd = nullptr;
	// グラフィックス用.
	CGraphicsController m_graphicsController;
	CScene m_scene;
	CShaderManager m_shaderMgr;
	// レンダーパス.
	CRenderPassTest m_renderPassTest;
	CRenderPassSprite m_renderPassSprite;
	// TODO 本来はGameクラスとか作るべきだが、テスト的な環境なので….
	CPlayer m_player;
	CPlayer m_player2;
	CCamera m_camera;
	CSprite m_sprite;
	C2DDecal m_2dDecal;
	SLightInfo m_light[5];

#if defined(DEBUG)
	CDebugManager m_debugManager;
#endif
};
