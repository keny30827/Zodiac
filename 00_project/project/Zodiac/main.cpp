#include "app/Application.h"

LRESULT WinProc(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main()
{
	// DirectXTexライブラリを使う際に、COMを初期化しておかないと、エラーが出る.
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	SApplicationOption option;
	option.appClassName = u8"Zodiac";
	option.appTitleName = u8"Zodiac";
	option.proc = WinProc;
	option.x = 0;
	option.y = 0;
	option.w = 800;
	option.h = 800;
	option.windowStyle = WS_OVERLAPPEDWINDOW;
	option.windowExStyle = WS_EX_TOPMOST | WS_EX_WINDOWEDGE;

	CApplication app;
	if (app.Init(option)) {
		app.Update();
	}
	app.Term();
	return 0;
}