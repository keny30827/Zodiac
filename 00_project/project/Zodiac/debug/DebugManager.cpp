#include "DebugManager.h"

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
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_DEBUG), 
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_DEBUG)->GetCPUDescriptorHandleForHeapStart(),
		rGraphicsController.GetHeapWrapper().GetDescriptorHeap(HEAP_CATEGORY_DEBUG)->GetGPUDescriptorHandleForHeapStart());
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

void CDebugManager::UpdateGUI()
{
	ImGui::ShowDemoWindow();
}

void CDebugManager::End()
{
	ImGui::EndFrame();
}

void CDebugManager::Render(CGraphicsController& rGraphicsController)
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), rGraphicsController.GetCommandWrapper().GetCommandList());
}