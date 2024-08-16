#include "DebugManager.h"

#include "../app/Application.h"
#include "../graphics/GraphicsController.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

bool CDebugManager::Init(CGraphicsController& rGraphicsController, HWND hWnd)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX12_Init(
		rGraphicsController.GetGraphicsDevice(), 1, DXGI_FORMAT_R8G8B8A8_UNORM,
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_HUGE), 
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_HUGE)->GetCPUDescriptorHandleForHeapStart(),
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_HUGE)->GetGPUDescriptorHandleForHeapStart());
	return true;
}

void CDebugManager::Term()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void CDebugManager::Begin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CDebugManager::UpdateGUI(CApplication& app)
{
	if (ImGui::Begin(u8"debug", &m_isOpen)) {
		if (ImGui::CollapsingHeader(u8"camera")) {
			{
				bool isInput = false;
				DirectX::XMFLOAT3 eye = app.GetCamera().GetEye();
				isInput |= ImGui::InputFloat(u8"caemra eye x", &eye.x, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"caemra eye y", &eye.y, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"caemra eye z", &eye.z, 1.0f, 10.0f);
				if (isInput) {
					app.GetCamera().OnDebugInputEye(eye);
				}
			}
			{
				bool isInput = false;
				DirectX::XMFLOAT3 at = app.GetCamera().GetAt();
				isInput |= ImGui::InputFloat(u8"caemra at x", &at.x, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"caemra at y", &at.y, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"caemra at z", &at.z, 1.0f, 10.0f);
				if (isInput) {
					app.GetCamera().OnDebugInputAt(at);
				}
			}
		}
		if (ImGui::CollapsingHeader(u8"player")) {
			{
				bool isInput = false;
				DirectX::XMFLOAT3 pos = app.GetPlayer().GetPos();
				isInput |= ImGui::InputFloat(u8"player x", &pos.x, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"player y", &pos.y, 1.0f, 10.0f);
				isInput |= ImGui::InputFloat(u8"player z", &pos.z, 1.0f, 10.0f);
				if (isInput) {
					app.GetPlayer().OnDebugInputPos(pos);
				}
			}
		}
	}
	ImGui::End();
}

void CDebugManager::End()
{
}

void CDebugManager::Render(CGraphicsController& rGraphicsController)
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), rGraphicsController.GetCommandWrapper().GetCommandList());
}