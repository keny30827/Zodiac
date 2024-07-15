#include "GraphicsController.h"
#include "../object/render_target/RenderTarget.h"

//---------------------------------------------------
namespace g_util {

bool IsValidAdapter(const ADAPTER_TYPE e)
{
	return ((0 <= e) && (e < eADAPTER_TYPE_NUM));
}

IDXGIAdapter* GetAdapter(IDXGIFactory6* pFactory, const ADAPTER_TYPE type)
{
	if (!pFactory) {
		return nullptr;
	}
	if (!IsValidAdapter(type)) {
		return nullptr;
	}

	const wchar_t* pAdapterName[] =
	{
		L"NVIDIA",
		L"AMD",
		L"",
	};
	C_ASSERT(COUNTOF(pAdapterName) == eADAPTER_TYPE_NUM);

	std::vector<IDXGIAdapter*> adapterList;
	IDXGIAdapter* tmp = nullptr;
	for (int n = 0; pFactory->EnumAdapters(n, &tmp) != DXGI_ERROR_NOT_FOUND; n++) {
		adapterList.push_back(tmp);
	}
	
	for (auto it : adapterList) {
		DXGI_ADAPTER_DESC desc;
		HRESULT ret = it->GetDesc(&desc);
		if (ret != S_OK) {
			continue;
		}
		std::wstring str = desc.Description;
		if (str.find(pAdapterName[type]) != std::string::npos) {
			return it;
		}
	}

	return nullptr;
}

uint32_t AdjustRowPitch(const uint32_t origin, const uint32_t alignment)
{
	return (origin + (alignment - (origin % alignment)));
}

};

//---------------------------------------------------
bool CHeapWrapper::Init(ID3D12Device* m_pDevice)
{
	// ディスクリプタハンドルのサイズ.
	for (int n = 0; n < D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; n++) {
		const D3D12_DESCRIPTOR_HEAP_TYPE type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(n);
		m_descriptorHandleIncrementSize[type] = static_cast<SIZE_T>(m_pDevice->GetDescriptorHandleIncrementSize(type));
	}

	// ディスクリプタヒープを必要な分作る.
	// 固定数のディスクリプタを入れられる器を用意する.
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_SWAP_CHAIN);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_SWAP_CHAIN]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_SHADER_INFO);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_SHADER_INFO]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_MATERIAL);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_MATERIAL]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_DEPTH_STENCIL);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_DEPTH_STENCIL]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_DEPTH_STENCIL_SHADER_VIEW);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_DEPTH_STENCIL_SHADER_VIEW]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_RENDER_TARGET);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_RENDER_TARGET]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_RENDER_TARGET_SHADER_VIEW);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_RENDER_TARGET_SHADER_VIEW]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_DEBUG);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_DEBUG]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_TEXTURE);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_TEXTURE]));
		VRETURN_RET(ret == S_OK, false);
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NodeMask = 0;
		desc.NumDescriptors = GetHeapSize(HEAP_CATEGORY_HUGE);
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT ret = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapList[HEAP_CATEGORY_HUGE]));
		VRETURN_RET(ret == S_OK, false);
	}

	memset(m_descriptorHeapListNextIndex, 0, sizeof(m_descriptorHeapListNextIndex));
	// 先頭の2048個分をとりあえずデバッグ用に.
	m_descriptorHeapListNextIndex[HEAP_CATEGORY_HUGE] = 2048;

	return true;
}

void CHeapWrapper::Term()
{
	for (int n = 0; n < HEAP_CATEGORY_NUM; n++) {
		const HEAP_CATEGORY e = static_cast<HEAP_CATEGORY>(n);
		if (m_pDescriptorHeapList[e]) {
			m_pDescriptorHeapList[e]->Release();
			m_pDescriptorHeapList[e] = nullptr;
		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE CHeapWrapper::GetCPUDescriptorHandle(const HEAP_CATEGORY e, const int offset)
{
	D3D12_CPU_DESCRIPTOR_HANDLE error = { 0 };
	VRETURN_RET(IsValidHeapCategory(e), error);
	VRETURN_RET(m_pDescriptorHeapList[e], error);
	auto desc = m_pDescriptorHeapList[e]->GetDesc();
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pDescriptorHeapList[e]->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_descriptorHandleIncrementSize[desc.Type] * offset;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CHeapWrapper::GetGPUDescriptorHandle(const HEAP_CATEGORY e, const int offset)
{
	D3D12_GPU_DESCRIPTOR_HANDLE error = { 0 };
	VRETURN_RET(IsValidHeapCategory(e), error);
	VRETURN_RET(m_pDescriptorHeapList[e], error);
	auto desc = m_pDescriptorHeapList[e]->GetDesc();
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_pDescriptorHeapList[e]->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += m_descriptorHandleIncrementSize[desc.Type] * offset;
	return handle;
}

//---------------------------------------------------
bool CGraphicsController::Init(HWND hWnd, long wndWidth, long wndHeight)
{
	HRESULT ret = S_FALSE;

	m_windowWidth = wndWidth;
	m_windowHeight = wndHeight;

#if defined(_DEBUG)
	EnableDebugLayer();
#endif

#if defined(_DEBUG)
	ret = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_pDXGIFactory));
#else
	ret = CreateDXGIFactory1(IID_PPV_ARGS(&m_pDXGIFactory));
#endif
	if (ret != S_OK) {
		return false;
	}

	ret = D3D12CreateDevice(g_util::GetAdapter(m_pDXGIFactory, g_util::eADAPTER_TYPE_NVIDIA), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_pDevice));
	if (ret != S_OK) {
		return false;
	}

	if (!InitCommand()) {
		return false;
	}

	if (!InitSwapChain(hWnd, wndWidth, wndHeight)) {
		return false;
	}

	if (!InitDesc()) {
		return false;
	}

	if (!InitResourceBarrier()) {
		return false;
	}

	ret = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (ret != S_OK) {
		return false;
	}

	return true;
}

bool CGraphicsController::InitCommand()
{
	HRESULT ret = S_FALSE;
	const D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ret = m_pDevice->CreateCommandAllocator(cmdListType, IID_PPV_ARGS(&m_pCommandAllocator));
	if (ret != S_OK) {
		return false;
	}

	ret = m_pDevice->CreateCommandList(
		0,
		cmdListType,
		m_pCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&m_pCommandList));
	if (ret != S_OK) {
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Type = cmdListType;
	ret = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
	if (ret != S_OK) {
		return false;
	}

	m_commandWrapper.Init(m_pCommandAllocator, m_pCommandList);

	return true;
}

bool CGraphicsController::InitSwapChain(HWND hWnd, long wndWidth, long wndHeight)
{
	HRESULT ret = S_FALSE;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = wndWidth;
	swapChainDesc.Height = wndHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ret = m_pDXGIFactory->CreateSwapChainForHwnd(
		m_pCommandQueue,
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&m_pSwapchain);
	if (ret != S_OK) {
		return false;
	}

	return true;
}

bool CGraphicsController::InitDesc()
{
	HRESULT ret = S_FALSE;

	// ディスクリプタヒープをまとめたラッパを生成.
	m_heapWrapper.Init(m_pDevice);

	// リソースとヒープ内のアドレスを紐づける.
	{
		// スワップチェインの情報を参照しながら設定するので、最初に取得しておく.
		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		ret = m_pSwapchain->GetDesc1(&scDesc);
		if (ret != S_OK) {
			return false;
		}
		for (uint32_t n = 0; n < scDesc.BufferCount; n++) {
			ID3D12Resource* pResource = nullptr;
			ret = m_pSwapchain->GetBuffer(n, IID_PPV_ARGS(&pResource));
			if (ret != S_OK) { continue; }
			const int heapIndex = m_heapWrapper.AllocateHeapPosition(HEAP_CATEGORY_SWAP_CHAIN);
			const D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_SWAP_CHAIN, heapIndex);
			m_pDevice->CreateRenderTargetView(pResource, nullptr, handle);
		}
	}
	{
		// 深度バッファーはゲーム中１つでよいので、最初に作っておく.
		m_depthStencilBuffer = nullptr;
		{
			D3D12_HEAP_PROPERTIES heapDesc = {};
			heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = m_windowWidth;
			resourceDesc.Height = m_windowHeight;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			ret = m_pDevice->CreateCommittedResource(
				&heapDesc,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				nullptr,
				IID_PPV_ARGS(&m_depthStencilBuffer));
			VRETURN_RET(ret == S_OK, false);
		}
		// ビューを作成しておく.
		{
			const int heapIndex = m_heapWrapper.AllocateHeapPosition(HEAP_CATEGORY_DEPTH_STENCIL);
			const D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_DEPTH_STENCIL, heapIndex);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			m_pDevice->CreateDepthStencilView(m_depthStencilBuffer, &dsvDesc, handle);
		}
	}

	return true;
}

bool CGraphicsController::InitResourceBarrier()
{
	m_resourceBarrierForSwcTrans.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_resourceBarrierForSwcTrans.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	return true;
}

void CGraphicsController::Term()
{
	m_commandWrapper.Term();
	m_heapWrapper.Term();
	if (m_depthStencilBuffer) {
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}
	if (m_pFence) {
		m_pFence->Release();
		m_pFence = nullptr;
	}
	if (m_pCommandQueue) {
		m_pCommandQueue->Release();
		m_pCommandQueue = nullptr;
	}
	if (m_pCommandList) {
		m_pCommandList->Release();
		m_pCommandList = nullptr;
	}
	if (m_pCommandAllocator) {
		m_pCommandAllocator->Release();
		m_pCommandAllocator = nullptr;
	}
	if (m_pSwapchain) {
		m_pSwapchain->Release();
		m_pSwapchain = nullptr;
	}
	if (m_pDXGIFactory) {
		m_pDXGIFactory->Release();
		m_pDXGIFactory = nullptr;
	}
	if (m_pDevice) {
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
}

bool CGraphicsController::IsValid() const
{
	if (!m_pDevice) {
		return false;
	}
	if (!m_pDXGIFactory) {
		return false;
	}
	if (!m_pSwapchain) {
		return false;
	}
	if (!m_pCommandAllocator) {
		return false;
	}
	if (!m_pCommandList) {
		return false;
	}
	if (!m_pCommandQueue) {
		return false;
	}
	if (!m_pFence) {
		return false;
	}
	return true;
}

bool CGraphicsController::BeginScene(const bool isClear, IDepthStencil* pDS)
{
	RETURN_RET(IsValid(), false);

	HRESULT ret = S_FALSE;

	// 描画コマンド開始.
	m_commandWrapper.Begin();

	// バックバッファのハンドルを取得.
	const uint8_t backBufferIdx = m_pSwapchain->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_SWAP_CHAIN, 0);
	handle.ptr += backBufferIdx * static_cast<SIZE_T>(m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	// まずリソースバリアでリソースが使える状態になるまで待つ.
	{
		ID3D12Resource* pResource = nullptr;
		ret = m_pSwapchain->GetBuffer(backBufferIdx, IID_PPV_ARGS(&pResource));
		VRETURN_RET(ret == S_OK, false);
		m_resourceBarrierForSwcTrans.Transition.pResource = pResource;
		m_resourceBarrierForSwcTrans.Transition.Subresource = 0;
		m_resourceBarrierForSwcTrans.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		m_resourceBarrierForSwcTrans.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_commandWrapper.ResourceBarrier(&m_resourceBarrierForSwcTrans, 1);

		if (pDS) {
			m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}

	// 画面のバックバッファをレンダーターゲットに設定.
	auto dsvHandle =
		(pDS) ?
		GetCPUDescriptorHandle(pDS->GetDsvHeapCategory(), pDS->GetDsvHeapPosition()) :
		m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_DEPTH_STENCIL, 0);
	{
		m_commandWrapper.SetRenderTargets(&handle, 1, &dsvHandle, true);
	}

	// 画面をクリア.
	if (isClear) {
		m_commandWrapper.ClearDepthStencil(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
		m_commandWrapper.ClearRenderTarget(handle, color);
	}

	return true;
}

void CGraphicsController::EndScene(IDepthStencil* pDS)
{
	if (!IsValid()) {
		return;
	}

	if (pDS) {
		m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// リソースバリアで描画できる状態になるまで待つ.
	{
		m_resourceBarrierForSwcTrans.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_resourceBarrierForSwcTrans.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_commandWrapper.ResourceBarrier(&m_resourceBarrierForSwcTrans, 1);
	}

	// 投げるコマンドリストはここまで.
	m_commandWrapper.End();

	// 投下.
	ID3D12CommandList* ppList[] = { m_commandWrapper.GetCommandList() };
	m_pCommandQueue->ExecuteCommandLists(COUNTOF(ppList), ppList);

	// 終了待ち.
	m_fenceValue = (m_fenceValue + 1) % UINT32_MAX;
	m_pCommandQueue->Signal(m_pFence, m_fenceValue);
	while (m_pFence->GetCompletedValue() != m_fenceValue) {}

	// 画面更新.
	m_pSwapchain->Present(1, 0);
}

bool CGraphicsController::BeginScene(IRenderTarget* pRT, IDepthStencil* pDS)
{
	RETURN_RET(pRT, false);
	RETURN_RET(IsValid(), false);

	// 描画コマンド開始.
	m_commandWrapper.Begin();

	// リソースの状態を変える.
	m_commandWrapper.ChangeBarrierState(pRT->GetResourceBattier(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 深度も同様.
	if (pDS) {
		m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	// 描画先として設定.
	auto rtvHandle = GetCPUDescriptorHandle(pRT->GetRtvHeapCategory(), pRT->GetRtvHeapPosition());
	auto dsvHandle =
		(pDS) ?
		GetCPUDescriptorHandle(pDS->GetDsvHeapCategory(), pDS->GetDsvHeapPosition()) :
		m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_DEPTH_STENCIL, 0);
	{
		m_commandWrapper.SetRenderTargets(&rtvHandle, 1, &dsvHandle, true);
	}

	// 描画先と深度をクリア.
	{
		m_commandWrapper.ClearDepthStencil(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

		float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_commandWrapper.ClearRenderTarget(rtvHandle, color);
	}

	return true;
}

bool CGraphicsController::EndScene(IRenderTarget* pRT, IDepthStencil* pDS)
{
	RETURN_RET(pRT, false);
	RETURN_RET(IsValid(), false);

	// 描画後にレンダーターゲットとして使える状態にしておく.
	m_commandWrapper.ChangeBarrierState(pRT->GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// 深度も同様.
	if (pDS) {
		m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// 投げるコマンドリストはここまで.
	m_commandWrapper.End();

	// 投下.
	ID3D12CommandList* ppList[] = { m_commandWrapper.GetCommandList() };
	m_pCommandQueue->ExecuteCommandLists(COUNTOF(ppList), ppList);

	// 終了待ち.
	m_fenceValue = (m_fenceValue + 1) % UINT32_MAX;
	m_pCommandQueue->Signal(m_pFence, m_fenceValue);
	while (m_pFence->GetCompletedValue() != m_fenceValue) {}

	return true;
}

bool CGraphicsController::BeginScene(IRenderTarget** pRTList, const int nListNum, GAME_COLOR* pColor, IDepthStencil* pDS)
{
	RETURN_RET(pRTList, false);
	RETURN_RET(nListNum > 0, false);
	RETURN_RET(pColor, false);
	RETURN_RET(IsValid(), false);

	m_commandWrapper.Begin();

	// 深度リソースの状態変更諸々.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
	{
		// 外部から指定されたものは、強制的に使える状態に.
		if (pDS) {
			m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
		// 使うポインタを決定.
		dsvHandle =
			(pDS) ?
			GetCPUDescriptorHandle(pDS->GetDsvHeapCategory(), pDS->GetDsvHeapPosition()) :
			m_heapWrapper.GetCPUDescriptorHandle(HEAP_CATEGORY_DEPTH_STENCIL, 0);
		// 内容はクリアしておく.
		m_commandWrapper.ClearDepthStencil(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);
	}
	
	// レンダーターゲットの状態変更クリア諸々.
	{
		// 描画先として設定しておく.
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles = {};
		for (int n = 0; n < nListNum; n++) {
			handles.push_back(GetCPUDescriptorHandle(pRTList[n]->GetRtvHeapCategory(), pRTList[n]->GetRtvHeapPosition()));
		}
		m_commandWrapper.SetRenderTargets(handles.data(), static_cast<uint32_t>(handles.size()), &dsvHandle, false);

		float white[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int n = 0; n < nListNum; n++) {
			// 使える状態に.
			m_commandWrapper.ChangeBarrierState(pRTList[n]->GetResourceBattier(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			// 中身をクリアしておく.
			if (IsValidGameColor(pColor[n])) {
				float* pTmpColor = (pColor[n] == GAME_COLOR::GAME_COLOR_BLACK) ? black : white;
				m_commandWrapper.ClearRenderTarget(handles[n], pTmpColor);
			}
		}
	}

	return true;
}

bool CGraphicsController::EndScene(IRenderTarget** pRTList, const int nListNum, GAME_COLOR* pColor, IDepthStencil* pDS)
{
	RETURN_RET(pRTList, false);
	RETURN_RET(nListNum > 0, false);
	RETURN_RET(pColor, false);
	RETURN_RET(IsValid(), false);

	// 描画後にレンダーターゲットとして使える状態にしておく.
	for (int n = 0; n < nListNum; n++) {
		m_commandWrapper.ChangeBarrierState(pRTList[n]->GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// 深度も同様.
	if (pDS) {
		m_commandWrapper.ChangeBarrierState(pDS->GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// 投げるコマンドリストはここまで.
	m_commandWrapper.End();

	// 投下.
	ID3D12CommandList* ppList[] = { m_commandWrapper.GetCommandList() };
	m_pCommandQueue->ExecuteCommandLists(COUNTOF(ppList), ppList);

	// 終了待ち.
	m_fenceValue = (m_fenceValue + 1) % UINT32_MAX;
	m_pCommandQueue->Signal(m_pFence, m_fenceValue);
	while (m_pFence->GetCompletedValue() != m_fenceValue) {}

	return true;
}

void CGraphicsController::CopyFromRenderTargetToBackBuffer(IRenderTarget& rt)
{
	VRETURN(m_pSwapchain);
	VRETURN(m_pDevice);
	const uint8_t backBufferIdx = m_pSwapchain->GetCurrentBackBufferIndex();
	ID3D12Resource* pResource = nullptr;
	HRESULT ret = m_pSwapchain->GetBuffer(backBufferIdx, IID_PPV_ARGS(&pResource));
	VRETURN(ret == S_OK);
	{
		m_commandWrapper.ChangeBarrierState(rt.GetResourceBattier(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	}
	D3D12_RESOURCE_BARRIER tmpBarrier = {};
	{
		tmpBarrier.Transition.pResource = pResource;
		tmpBarrier.Transition.Subresource = 0;
		tmpBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		tmpBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		m_commandWrapper.ResourceBarrier(&tmpBarrier, 1);
	}
	{
		m_commandWrapper.CopyResource(rt.GetResource(), pResource);
	}
	{
		tmpBarrier.Transition.pResource = pResource;
		tmpBarrier.Transition.Subresource = 0;
		tmpBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		tmpBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_commandWrapper.ResourceBarrier(&tmpBarrier, 1);
	}
	{
		m_commandWrapper.ChangeBarrierState(rt.GetResourceBattier(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

#if defined(_DEBUG)
void CGraphicsController::EnableDebugLayer()
{
	ID3D12Debug* pDebug = nullptr;
	HRESULT ret = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
	if (ret == S_OK) {
		pDebug->EnableDebugLayer();
		pDebug->Release();
	}
}
#endif