#include "Application.h"

CApplication::CApplication()
{
	memset(&m_option, 0, sizeof(m_option));
	memset(&m_windowClass, 0, sizeof(m_windowClass));
}

bool CApplication::Init(SApplicationOption option)
{
	if (!InitWindow(option)) {
		return false;
	}
	if (!m_graphicsController.Init(m_hWnd, option.w, option.h)) {
		return false;
	}
	if (!m_scene.Init(m_graphicsController, option.w, option.h)) {
		return false;
	}
	if (!m_renderPassTest.Init()) {
		return false;
	}
	if (!m_renderPassSprite.Init(m_graphicsController)) {
		return false;
	}
	if (!m_player.Init(m_graphicsController, 0.0f, 0.0f, 0.0f)) {
		return false;
	}
	if (!m_player2.Init(m_graphicsController, 15.0f, 0.0f, 10.0f)) {
		return false;
	}
	if (!m_sprite.Init(m_graphicsController, 150.0f, 150.0f, 400.0f, 400.0f)) {
		return false;
	}
	if (!m_camera.Init(option.w, option.h)) {
		return false;
	}
	return true;
}

bool CApplication::InitWindow(SApplicationOption option)
{
	m_option = option;

	m_windowClass.cbSize = sizeof(m_windowClass);
	m_windowClass.lpszClassName = m_option.appClassName.c_str();
	m_windowClass.lpfnWndProc = m_option.proc;
	m_windowClass.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&m_windowClass);

	RECT rect = { m_option.x, m_option.y, m_option.w, m_option.h };
	AdjustWindowRect(&rect, m_option.windowStyle, false);

	m_hWnd = CreateWindowEx(
		m_option.windowExStyle,
		m_option.appClassName.c_str(),
		m_option.appTitleName.c_str(),
		m_option.windowStyle,
		m_option.x,
		m_option.y,
		m_option.w,
		m_option.h,
		nullptr,
		nullptr,
		m_windowClass.hInstance,
		nullptr);

	ShowWindow(m_hWnd, SW_SHOW);
	return (m_hWnd != nullptr);
}

void CApplication::Term()
{
	m_renderPassSprite.Term();
	m_renderPassTest.Term();
	m_scene.Term();
	m_graphicsController.Term();
	TermWindow();
}

void CApplication::TermWindow()
{
	UnregisterClass(m_windowClass.lpszClassName, m_windowClass.hInstance);

	memset(&m_option, 0, sizeof(m_option));
	memset(&m_windowClass, 0, sizeof(m_windowClass));
	m_hWnd = nullptr;
}

bool CApplication::Update()
{
	MSG msg = {};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		UpdateGame();
	}
	return false;
}

void CApplication::UpdateGame()
{
	// ÉQÅ[ÉÄì‡Ç≈ÇÃÉIÉuÉWÉFÉNÉgçXêV.
	{
		m_player.Update();
		m_player2.Update();

		DirectX::XMFLOAT3 focusPos = m_player.GetPos();
		// ÉJÉÅÉâÇçáÇÌÇπÇÈÇÃÇÕâ∫ñ⁄.
		focusPos.y += 10.f;
		m_camera.Update(&focusPos);

		m_sprite.SetRenderTarget(&m_scene.GetTestRT());
		m_sprite.Update();
	}

	// ï`âÊìoò^.
	{
		m_scene.AddModel(&(m_player.GetModel()));
		m_scene.AddModel(&(m_player2.GetModel()));
		m_scene.SetMainCamera(&m_camera);
		m_scene.SetFrameBuffer(&m_sprite);
	}

	// ï`âÊ.
	{
		m_renderPassTest.Render(m_scene, m_graphicsController);
		m_renderPassSprite.Render(m_scene, m_graphicsController);
	}

	// ìoò^ï™ÇÕîjä¸Ç∑ÇÈ.
	m_scene.ClearAll();
}