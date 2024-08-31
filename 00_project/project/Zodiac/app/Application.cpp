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
	if (!m_shaderMgr.Init(m_graphicsController)) {
		return false;
	}
	if (!m_renderPassTest.Init()) {
		return false;
	}
	if (!m_renderPassSprite.Init(m_graphicsController)) {
		return false;
	}
	if (!m_player.Init(m_graphicsController, 0.0f, 0.0f, 5.0f)) {
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
	if (!m_2dDecal.Init(m_graphicsController, 0.0f, 5.0f, 0.0f, 5.0f, 5.0f)) {
		return false;
	}
	{
		DirectX::XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 rot(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT2 size(10.0f, 10.0f);
		if (!m_3dPlane.Init(m_graphicsController, pos, rot, size)) {
			return false;
		}
	}
#if defined(DEBUG)
	if (!m_debugManager.Init(m_graphicsController, m_hWnd)) {
		return false;
	}
#endif

	// 事前設定.
	{
		DirectX::XMFLOAT3 focusPos = m_player.GetPos();
		// カメラを合わせるのは下目.
		focusPos.y += 10.f;
		m_camera.Update(&focusPos);
		// ちょっと雑だがライト用意.
		{
			m_light[0].pos = DirectX::XMFLOAT4(-5.0f, 5.0f, -5.0f, 1.0f);
			m_light[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

			m_light[1].pos = DirectX::XMFLOAT4(-5.0f, 5.0f, 0.0f, 1.0f);
			m_light[1].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

			m_light[2].pos = DirectX::XMFLOAT4(-5.0f, 5.0f, 5.0f, 1.0f);
			m_light[2].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

			m_light[3].pos = DirectX::XMFLOAT4(5.0f, 5.0f, -5.0f, 1.0f);
			m_light[3].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

			m_light[4].pos = DirectX::XMFLOAT4(5.0f, 5.0f, 0.0f, 1.0f);
			m_light[4].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

			for (int n = 0; n < COUNTOF(m_light); n++) {
				m_light[n].attenuationDistance = 20.0f;
			}
		}
	}

	// コールバック仕込み場.
	{
		auto CallbackObj = [&](CGraphicsController* p) {
			OnRenderTail(p);
		};
		m_renderPassSprite.SetRenderTailCallback(CallbackObj);
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
#if defined(DEBUG)
	m_debugManager.Term();
#endif
	m_renderPassSprite.Term();
	m_renderPassTest.Term();
	m_shaderMgr.Term();
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
#if defined(DEBUG)
	m_debugManager.Begin();
#endif

	// ゲーム内でのオブジェクト更新.
	{
#if defined(DEBUG)
		m_debugManager.UpdateGUI(*this);
#endif
		m_player.Update();
		m_player2.Update();
		m_camera.Update();
		for (int n = 0; n < COUNTOF(m_light); n++) {
			const DirectX::XMVECTOR vec = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&m_light[n].pos), m_camera.GetViewMatrix());
			DirectX::XMStoreFloat4(&m_light[n].posInView, vec);
		}
	}

	// 描画登録.
	{
		m_scene.AddModel(&(m_player.GetModel()));
		m_scene.AddModel(&(m_player2.GetModel()));
		for (int n = 0; n < COUNTOF(m_light); n++) {
			m_scene.AddLight(&m_light[n]);
		}
		m_scene.AddPlane(&m_3dPlane);
		m_scene.SetMainCamera(&m_camera);
		m_scene.SetFrameBuffer(&m_sprite);
		m_scene.Set2DDecal(&m_2dDecal);
	}

	// 描画.
	{
		m_renderPassTest.Render(m_scene, m_graphicsController, m_shaderMgr);
		m_renderPassSprite.Render(m_scene, m_graphicsController, m_shaderMgr);
	}

	// 登録分は破棄する.
	m_scene.ClearAll();

#if defined(DEBUG)
	m_debugManager.End();
#endif
}

void CApplication::OnRenderTail(CGraphicsController* pController)
{
#if defined(DEBUG)
	m_debugManager.Render(*pController);
#endif
}