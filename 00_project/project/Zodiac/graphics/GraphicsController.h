#pragma once

#include "GraphicsDefs.h"
#include <stdint.h>
#include "../object/model/Model.h"
#include "../object/camera/Camera.h"
#include "GraphicsResourceManager.h"

namespace g_util {
	
	enum ADAPTER_TYPE {
		eADAPTER_TYPE_NVIDIA = 0,
		eADAPTER_TYPE_AMD,
		eADAPTER_TYPE_OTHER,
		eADAPTER_TYPE_NUM,
		eADAPTER_TYPE_INVALID = -1,
	};
	extern bool IsValidAdapter(const ADAPTER_TYPE e);

	extern IDXGIAdapter* GetAdapter(IDXGIFactory6* pFactory, const ADAPTER_TYPE type = eADAPTER_TYPE_INVALID);

	extern uint32_t AdjustRowPitch(const uint32_t origin, const uint32_t alignment);
}

// ディスクリプタヒープ管理.
// 割り当てなどを提供する.
class CHeapWrapper
{
public:
	CHeapWrapper() {};
	~CHeapWrapper() {};

public:
	bool Init(ID3D12Device* m_pDevice);
	void Term();

public:
	// TODO 途中破棄などがあった場合に無駄が出るので、極力ヒープ分を使いまわせるようなアルゴリズムに変えるべき.例えばフリーリストとか.
	int AllocateHeapPosition(HEAP_CATEGORY e)
	{
		VRETURN_RET(IsValidHeapCategory(e), -1);
		VRETURN_RET(m_descriptorHeapListNextIndex[e] < GetHeapSize(e), -1);
		return m_descriptorHeapListNextIndex[e]++;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const HEAP_CATEGORY e, const int offset);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const HEAP_CATEGORY e, const int offset);
	ID3D12DescriptorHeap* GetDescriptorHeap(const HEAP_CATEGORY e) const { return m_pDescriptorHeapList[e]; }

private:
	int GetHeapSize(const HEAP_CATEGORY e) const
	{
		VRETURN_RET(IsValidHeapCategory(e), 0);
		const int num[] =
		{
			2,
			500,
			500 * 10,
			10,
			10,
			50,
			50,

			4098,
			
			100,

			16392,
		};
		C_ASSERT(_countof(num) == HEAP_CATEGORY_NUM);
		return num[e];
	}

private:
	ID3D12DescriptorHeap* m_pDescriptorHeapList[HEAP_CATEGORY_NUM] = {};
	int m_descriptorHeapListNextIndex[HEAP_CATEGORY_NUM] = {};
	SIZE_T m_descriptorHandleIncrementSize[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};
};

// 描画コマンド管理のラッパー.
// 低レベルの仕組みを隠ぺいして使いやすい形にする.
class CCommandWrapper {
public:
	CCommandWrapper() {};
	~CCommandWrapper() {};

public:
	void Init(
		ID3D12CommandAllocator* _pCommandAllocator,
		ID3D12GraphicsCommandList* _pCommandList)
	{
		m_pCommandAllocator = _pCommandAllocator;
		m_pCommandList = _pCommandList;
		m_isUseCommand = false;
	}
	void Term() { m_pCommandAllocator = nullptr;  m_pCommandList = nullptr; }

public:
	void Begin()
	{
		// コマンドリスト内の情報を空に戻す.
		if (m_isUseCommand) {
			HRESULT ret = S_FALSE;
			ret = m_pCommandAllocator->Reset();
			VRETURN(ret == S_OK);
			ret = m_pCommandList->Reset(m_pCommandAllocator, nullptr);
			VRETURN(ret == S_OK);
		}
	};

	void End()
	{
		m_isUseCommand = (m_pCommandList->Close() == S_OK);
	};

public: // 描画命令.
	void DrawIndexedInstanced(
		UINT IndexCountPerInstance,
		UINT InstanceCount,
		UINT StartIndexLocation,
		INT BaseVertexLocation,
		UINT StartInstanceLocation)
	{
		m_pCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}
	void Dispatch(
		UINT ThreadGroupCountX,
		UINT ThreadGroupCountY,
		UINT ThreadGroupCountZ)
	{
		m_pCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}

	void ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTHandle, const FLOAT ColorRGBA[4], const D3D12_RECT* pRects = nullptr, const UINT NumRects = 0)
	{
		m_pCommandList->ClearRenderTargetView(RTHandle, ColorRGBA, NumRects, pRects);
	}
	void ClearDepthStencil(D3D12_CPU_DESCRIPTOR_HANDLE DSHandle, D3D12_CLEAR_FLAGS ClearFlags, FLOAT depth, UINT8 stencil, const D3D12_RECT* pRects = nullptr, const UINT NumRects = 0)
	{
		m_pCommandList->ClearDepthStencilView(DSHandle, ClearFlags, depth, stencil, NumRects, pRects);
	}

public: // コピー命令.
	void CopyTexture(
		const D3D12_TEXTURE_COPY_LOCATION* pDst,
		const UINT DstX,
		const UINT DstY,
		const UINT DstZ,
		const D3D12_TEXTURE_COPY_LOCATION* pSrc,
		const D3D12_BOX* pSrcBox
	)
	{
		m_pCommandList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
	}
	void CopyResource(ID3D12Resource* pSrc, ID3D12Resource* pDst)
	{
		m_pCommandList->CopyResource(pDst, pSrc);
	}

public: // バリア命令.用途指定などで必要に応じて差し込む必要がある.
	void ResourceBarrier(const D3D12_RESOURCE_BARRIER* pBarriers, const UINT NumBarriers) { m_pCommandList->ResourceBarrier(NumBarriers, pBarriers); }
	void ChangeBarrierState(D3D12_RESOURCE_BARRIER& rBarrier, D3D12_RESOURCE_STATES state)
	{
		assert(rBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);
		RETURN(rBarrier.Transition.StateAfter != state);
		rBarrier.Transition.StateBefore = rBarrier.Transition.StateAfter;
		rBarrier.Transition.StateAfter = state;
		ResourceBarrier(&rBarrier, 1);
	}

public: // 描画のためのオプション設定.
	void SetPipelineState(ID3D12PipelineState* p) { m_pCommandList->SetPipelineState(p); }
	void SetGraphicsRootSignature(ID3D12RootSignature* p) { m_pCommandList->SetGraphicsRootSignature(p); }
	void SetComputeRootSignature(ID3D12RootSignature* p) { m_pCommandList->SetComputeRootSignature(p); }
	void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY e) { m_pCommandList->IASetPrimitiveTopology(e); }
	void SetVertexBuffers(const D3D12_VERTEX_BUFFER_VIEW* pViews, const UINT viewNum) { m_pCommandList->IASetVertexBuffers(0, viewNum, pViews); }
	void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews) { m_pCommandList->IASetIndexBuffer(pViews); }
	void SetViewports(const D3D12_VIEWPORT* pViewports, const UINT NumViewports) { m_pCommandList->RSSetViewports(NumViewports, pViewports); }
	void SetScissorRects(const D3D12_RECT* pRects, const UINT NumRects) { m_pCommandList->RSSetScissorRects(NumRects, pRects); }
	// RootParameterで指定しているシェーダーレジスタに紐づけるリソースが入ってるヒープを指定.
	void SetDescriptorHeaps(ID3D12DescriptorHeap* const* ppHeaps, const UINT NumHeaps) { m_pCommandList->SetDescriptorHeaps(NumHeaps, ppHeaps); }
	// RootParameterで指定している各レジスタに紐づけるリソースの先頭アドレスを指定.
	void SetGraphicsRootDescriptorTable(const UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) { m_pCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor); }
	void SetComputeRootDescriptorTable(const UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) { m_pCommandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor); }
	void SetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE* pRTHandles, const UINT NumRT, const D3D12_CPU_DESCRIPTOR_HANDLE* pDSHandles, const BOOL RTsSingleHandleToDescriptorRange)
	{
		m_pCommandList->OMSetRenderTargets(NumRT, pRTHandles, RTsSingleHandleToDescriptorRange, pDSHandles);
	}

public:
	ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList; }

private:
	// ここにいるのは本体ではなく、外で作成したものを設定しているだけ.
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	bool m_isUseCommand = false;
};

class CGraphicsController {
public:
	CGraphicsController() {};
	~CGraphicsController() = default;

public:
	bool Init(HWND hWnd, long wndWidth, long wndHeight);
private:
	bool InitCommand();
	bool InitSwapChain(HWND hWnd, long wndWidth, long wndHeight);
	bool InitDesc();
	bool InitResourceBarrier();

public:
	void Term();
	bool IsValid() const;

public:
	ID3D12Device* GetGraphicsDevice() { return m_pDevice; }
	IDXGISwapChain4* GetSwapChain() { return m_pSwapchain; }
	CCommandWrapper& GetCommandWrapper() { return m_commandWrapper; }
	CHeapWrapper& GetHeapWrapper() { return m_heapWrapper; }
	long GetWindowWidth() const { return m_windowWidth; }
	long GetWindowHeight() const { return m_windowHeight; }

public:
	bool BeginScene(const bool isClear = true, IDepthStencil* pDS = nullptr);
	void EndScene(IDepthStencil* pDS = nullptr);

	bool BeginScene(IRenderTarget* pRT, IDepthStencil* pDS = nullptr);
	bool EndScene(IRenderTarget* pRT, IDepthStencil* pDS = nullptr);

	bool BeginScene(IRenderTarget** pRTList, const int nListNum, GAME_COLOR* color, IDepthStencil* pDS = nullptr);
	bool EndScene(IRenderTarget** pRTList, const int nListNum, GAME_COLOR* color, IDepthStencil* pDS = nullptr);

public:
	void CopyFromRenderTargetToBackBuffer(IRenderTarget& rt);

public:
	// TODO 途中破棄などがあった場合に無駄が出るので、極力ヒープ分を使いまわせるようなアルゴリズムに変えるべき.例えばフリーリストとか.
	int AllocateHeapPosition(HEAP_CATEGORY e)
	{
		return m_heapWrapper.AllocateHeapPosition(e);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const HEAP_CATEGORY e, const int offset) { return m_heapWrapper.GetCPUDescriptorHandle(e, offset); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const HEAP_CATEGORY e, const int offset) { return m_heapWrapper.GetGPUDescriptorHandle(e, offset); }

#if defined(_DEBUG)
private:
	void EnableDebugLayer();
#endif

private:
	// パイプラインの大本となるデバイスと、ドライバなどへのアクセスに使うファクトリ.
	ID3D12Device* m_pDevice = nullptr;
	IDXGIFactory6* m_pDXGIFactory = nullptr;
	// 画面フリップ用.
	IDXGISwapChain4* m_pSwapchain = nullptr;
	// GPUへのコマンド送信用.入れ物と操作類.
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	CCommandWrapper m_commandWrapper;
	// GPUリソース操作用のディスクリプタ類.
	CHeapWrapper m_heapWrapper;
	// フェンス.
	ID3D12Fence* m_pFence = nullptr;
	uint32_t m_fenceValue = 0;
	// リソースバリア.スワップチェイン用.
	D3D12_RESOURCE_BARRIER m_resourceBarrierForSwcTrans = {};
	// 深度バッファ用のリソース.
	ID3D12Resource* m_depthStencilBuffer = nullptr;
	// スクリーンサイズ.
	long m_windowWidth = 0;
	long m_windowHeight = 0;
};