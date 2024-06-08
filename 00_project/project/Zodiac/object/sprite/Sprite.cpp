#include "Sprite.h"
#include "../../graphics/GraphicsController.h"
#include "../../graphics/GraphicsResourceManager.h"

// ------------------------------------------------------.
void CQuad::Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h)
{
	// 頂点.
	{
		SVertex vertex[] =
		{
			{ {x, y + h, 0.0f},		{0.0f, 1.0f} },
			{ {x, y, 0.0f},			{0.0f, 0.0f} },
			{ {x + w, y + h, 0.0f},	{1.0f, 1.0f} },
			{ {x + w, y, 0.0f},		{1.0f, 0.0f} },
		};
		for (uint32_t n = 0; n < _countof(vertex); n++) {
			AddVertex(vertex[n]);
		}
	}

	// 頂点インデックス.
	{
		uint16_t index[] =
		{
			0, 1, 2,
			2, 1, 3,
		};
		for (uint32_t n = 0; n < _countof(index); n++) {
			AddIndex(index[n]);
		}
	}

	VRETURN(BuildVertexBuffer(rGraphicsController));
	VRETURN(BuildIndexBuffer(rGraphicsController));
}

void CQuad::Term()
{
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

bool CQuad::BuildVertexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_vertexList.size() > 0, false);
	VRETURN_RET(!m_pVertexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_vertexList[0].GetDataSize() * m_vertexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_vertexBufferViewInfo.BufferLocation = m_pVertexBufferResource->GetGPUVirtualAddress();
		m_vertexBufferViewInfo.SizeInBytes = m_vertexList[0].GetDataSize() * static_cast<int>(m_vertexList.size());
		m_vertexBufferViewInfo.StrideInBytes = m_vertexList[0].GetDataSize();
	}

	return true;
}

bool CQuad::BuildIndexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_indexList.size() > 0, false);
	VRETURN_RET(!m_pIndexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = sizeof(m_indexList[0]) * m_indexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pIndexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_indexBufferViewInfo.BufferLocation = m_pIndexBufferResource->GetGPUVirtualAddress();
		m_indexBufferViewInfo.SizeInBytes = sizeof(m_indexList[0]) * static_cast<int>(m_indexList.size());
		m_indexBufferViewInfo.Format = DXGI_FORMAT_R16_UINT;
	}

	return true;
}

void CQuad::MapVertexData()
{
	RETURN(m_pVertexBufferResource);

	uint8_t* pBuffer = nullptr;
	HRESULT ret = m_pVertexBufferResource->Map(0, nullptr, (void**)(&pBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_vertexList.size(); n++) {
		const auto& vertex = m_vertexList[n];

		auto pos = vertex.pos;
		memcpy(pBuffer, &pos, sizeof(pos));
		pBuffer += sizeof(pos);

		auto uv = vertex.uv;
		memcpy(pBuffer, &uv, sizeof(uv));
		pBuffer += sizeof(uv);
	}

	m_pVertexBufferResource->Unmap(0, nullptr);
}

void CQuad::MapIndexData()
{
	RETURN(m_pIndexBufferResource);

	uint16_t* pIndexBuffer = nullptr;
	HRESULT ret = m_pIndexBufferResource->Map(0, nullptr, (void**)(&pIndexBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_indexList.size(); n++) {
		pIndexBuffer[n] = m_indexList[n];
	}

	m_pIndexBufferResource->Unmap(0, nullptr);
}

// ------------------------------------------------------.
void CSpriteNew::Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h)
{
	m_pos.x = x;
	m_pos.y = y;
	m_size.x = w;
	m_size.y = h;
	m_quad.Init(rGraphicsController, x, y, w, h);
	m_pShader = nullptr;
}

void CSpriteNew::Term()
{
	m_quad.Term();
}

void CSpriteNew::Update()
{
}

void CSpriteNew::Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// シェーダーが設定されていない場合は描画できないのでダメ.
	VRETURN(m_pShader);

	// 描画時の環境情報.
	{
		// ビューポートとシザリングを設定.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}
	}

	// メッシュ情報.
	{
		// 処理するプリミティブタイプを設定.
		commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点バッファビューを設定.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = m_quad.GetVertexBufferViewInfo();
		D3D12_INDEX_BUFFER_VIEW indexBufferInfo = m_quad.GetIndexBufferViewInfo();
		commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
		commandWrapper.SetIndexBuffer(&indexBufferInfo);
	}

	// シェーダー関連の情報は任せる.
	{
		m_pShader->RenderSetup(commandWrapper, heapWrapper);
	}

	// 描画.
	commandWrapper.DrawIndexedInstanced(m_quad.GetIndexNum(), 1, 0, 0, 0);
}
// ------------------------------------------------------.

CSprite::~CSprite()
{
	SAEF_RELEASE(m_pPipelineState);
	SAEF_RELEASE(m_pRootSignature);
	SAEF_RELEASE(m_pPixelShader);
	SAEF_RELEASE(m_pVertexShader);
	SAEF_RELEASE(m_pShaderInfoResource);
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

bool CSprite::Init(CGraphicsController& rGraphicsController, float x, float y, float w, float h)
{
	m_pos.x = x;
	m_pos.y = y;
	m_size.x = w;
	m_size.y = h;

	// 頂点.
	{
		SVertex vertex[] =
		{
			{ {x, y + h, 0.0f},		{0.0f, 1.0f} },
			{ {x, y, 0.0f},			{0.0f, 0.0f} },
			{ {x + w, y + h, 0.0f},	{1.0f, 1.0f} },
			{ {x + w, y, 0.0f},		{1.0f, 0.0f} },
		};
		for (uint32_t n = 0; n < _countof(vertex); n++) {
			AddVertex(vertex[n]);
		}
	}

	// 頂点インデックス.
	{
		uint16_t index[] =
		{
			0, 1, 2,
			2, 1, 3,
		};
		for (uint32_t n = 0; n < _countof(index); n++) {
			AddIndex(index[n]);
		}
	}

	// フィルター用のテクスチャ.
	{
		if (!m_normalFilter.Load(u8".\\bin\\texture\\normal\\test_normal.png", rGraphicsController)) {
			assert(false);
		}
	}

	// モデル単位で保持すグラフィックスリソース関連のデータの準備.
	VRETURN_RET(BuildVertexBuffer(rGraphicsController), false);
	VRETURN_RET(BuildIndexBuffer(rGraphicsController), false);
	VRETURN_RET(BuildShader(rGraphicsController), false);
	VRETURN_RET(BuildShaderInfoBuffer(rGraphicsController), false);
	VRETURN_RET(BuildGaussianBuffer(rGraphicsController), false);
	VRETURN_RET(BuildRootSignature(rGraphicsController), false);
	VRETURN_RET(BuildPipelineState(rGraphicsController), false);

	VRETURN_RET(BuildBloomShader(rGraphicsController), false);
	VRETURN_RET(BuildBloomPipelineState(rGraphicsController), false);

	VRETURN_RET(BuildSSAOShader(rGraphicsController), false);
	VRETURN_RET(BuildSSAOPipelineState(rGraphicsController), false);

	// 頂点などの固定情報をＣＰＵ->ＧＰＵにマッピング.
	MapVertexData();
	MapIndexData();
	MapGaussianData();

	return true;
}

void CSprite::Update()
{
	// TODO もしキー入力などで位置やサイズを変更するなら、頂点の位置を計算し直してマップする.
}

void CSprite::Render(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// RT設定用のシェーダー情報.
	MapShaderInfo(graphicsController);

	// コマンドリストにコマンドを積みましょう.
	{
		CCommandWrapper& commandWrapper = graphicsController.GetCommandWrapper();
		CHeapWrapper& heapWrapper = graphicsController.GetHeapWrapper();

		// パイプライン設定.
		commandWrapper.SetPipelineState(GetPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetRootSignature());

		// 処理するプリミティブタイプを設定.
		commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点バッファビューを設定.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
		D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
		commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
		commandWrapper.SetIndexBuffer(&indexBufferInfo);

		// ビューポートとシザリングを設定.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}

		// 頂点描画時などに使うシェーダー情報.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// レンダーターゲットが設定されている場合はそれを使う.
		if (GetRenderTarget()) {
			auto* pRT = GetRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pRT->GetAccessRootParam(), handle);
		}
		else {
			// 今のところ不正.
			assert(false);
		}

		// GBuffer法線.
		if (GetNormalRenderTarget()) {
			auto* pRT = GetNormalRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pRT->GetAccessRootParam(), handle);
		}

		// DOF.
		if (GetDofRenderTarget()) {
			auto* pRT = GetDofRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_DOF, handle);
		}
		if (GetDofShrinkRenderTarget()) {
			auto* pRT = GetDofShrinkRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_DOF_SHRINK, handle);
		}

		// ガウシアン情報設定.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_GAUSSIAN_TABLE, GetGaussianHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_GAUSSIAN_TABLE) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_GAUSSIAN_TABLE, dHandle);
		}

		ITexture& iTex = GetNormalTexture();
		// 法線テクスチャをGPU側にコピー.
		{
			commandWrapper.CopyTexture(&iTex.GetCopyDst(), 0, 0, 0, &iTex.GetCopySrc(), nullptr);
			commandWrapper.ResourceBarrier(&iTex.GetCopyResourceBarrier(), 1);
		}

		// 法線.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(iTex.GetHeapCategory(), iTex.GetHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(iTex.GetHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(iTex.GetAccessRootParam(), dHandle);
		}

		// 深度.
		auto* pLightDs = GetDepthStencil();
		if (pLightDs) {
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(pLightDs->GetSrvHeapCategory(), pLightDs->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pLightDs->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pLightDs->GetAccessRootParam(), dHandle);
		}

		// SSAO.
		auto* pSSAO = GetSSAORenderTarget();
		if (pSSAO) {
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(pSSAO->GetSrvHeapCategory(), pSSAO->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pSSAO->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_SSAO, dHandle);
		}

		commandWrapper.DrawIndexedInstanced(GetIndexNum(), 1, 0, 0, 0);

		// 次回更新用のリソースの用途を戻しておく.
		{
			commandWrapper.ResourceBarrier(&iTex.GetResetResourceBarrier(), 1);
		}
	}
}

void CSprite::RenderShrinkBuffer(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// 縮小バッファを作るための大本の画像がないならダメ.
	VRETURN(GetRenderTarget());

	// RT設定用のシェーダー情報.
	MapShaderInfo(graphicsController);

	// コマンドリストにコマンドを積みましょう.
	{
		CCommandWrapper& commandWrapper = graphicsController.GetCommandWrapper();
		CHeapWrapper& heapWrapper = graphicsController.GetHeapWrapper();

		// パイプライン設定.
		commandWrapper.SetPipelineState(GetPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetRootSignature());

		// 処理するプリミティブタイプを設定.
		commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点バッファビューを設定.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
		D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
		commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
		commandWrapper.SetIndexBuffer(&indexBufferInfo);

		// 頂点描画時などに使うシェーダー情報.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// レンダーターゲットが設定されている場合はそれを使う.
		{
			auto* pRT = GetRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pRT->GetAccessRootParam(), handle);
		}

		// ビューポートとシザリングを切り替えながら縮小バッファを作成する.
		D3D12_VIEWPORT tmpViewPort = *pViewPort;
		D3D12_RECT tmpScissor = *pScissor;
		for (int n = 0; n < 8; n++) {
			// サイズを半分に.
			tmpViewPort.Width *= 0.5f;
			tmpViewPort.Height *= 0.5f;
			const int scissorOffsetY = (tmpScissor.bottom - tmpScissor.top) / 2;
			tmpScissor.right = tmpScissor.left + ((tmpScissor.right - tmpScissor.left) / 2);
			tmpScissor.bottom = tmpScissor.top + scissorOffsetY;
			// そのサイズで描画.
			commandWrapper.SetViewports(&tmpViewPort, 1);
			commandWrapper.SetScissorRects(&tmpScissor, 1);
			commandWrapper.DrawIndexedInstanced(GetIndexNum(), 1, 0, 0, 0);
			// 位置ずらす.
			tmpViewPort.TopLeftY += tmpViewPort.Height;
			tmpScissor.top += scissorOffsetY;
			tmpScissor.bottom += scissorOffsetY;
		}
	}
}

void CSprite::RenderBloom(CGraphicsController& graphicsController, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// 今のところ、描画用のRTが設定されている前提.
	VRETURN(GetRenderTarget());
	// ブルーム用のぼかし画像ないと使えない.
	VRETURN(GetBloomRenderTarget());
	VRETURN(GetBloomShrinkRenderTarget());

	// RT設定用のシェーダー情報.
	MapShaderInfo(graphicsController);

	// コマンドリストにコマンドを積みましょう.
	{
		CCommandWrapper& commandWrapper = graphicsController.GetCommandWrapper();
		CHeapWrapper& heapWrapper = graphicsController.GetHeapWrapper();

		// パイプライン設定.
		commandWrapper.SetPipelineState(GetBloomPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetRootSignature());

		// 処理するプリミティブタイプを設定.
		commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点バッファビューを設定.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
		D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
		commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
		commandWrapper.SetIndexBuffer(&indexBufferInfo);

		// ビューポートとシザリングを設定.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}

		// 頂点描画時などに使うシェーダー情報.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// レンダーターゲットが設定されている場合はそれを使う.
		{
			auto* pRT = GetRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pRT->GetAccessRootParam(), handle);
		}

		// ブルームぼかし1.
		{
			auto* pRT = GetBloomRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_BLOOM, handle);
		}

		// ブルームぼかし2.
		{
			auto* pRT = GetBloomShrinkRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(ACCESS_ROOT_PARAM_BLOOM_SHRINK, handle);
		}

		commandWrapper.DrawIndexedInstanced(GetIndexNum(), 1, 0, 0, 0);
	}
}

void CSprite::RenderSSAO(CGraphicsController& graphicsController, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// 法線と深度から計算するので、これがないと話にならない.
	VRETURN(GetNormalRenderTarget());
	VRETURN(GetDepthStencil());

	// RT設定用のシェーダー情報.
	MapShaderInfo(graphicsController, &rCamera);

	// コマンドリストにコマンドを積みましょう.
	{
		CCommandWrapper& commandWrapper = graphicsController.GetCommandWrapper();
		CHeapWrapper& heapWrapper = graphicsController.GetHeapWrapper();

		// パイプライン設定.
		commandWrapper.SetPipelineState(GetSSAOPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetRootSignature());

		// 処理するプリミティブタイプを設定.
		commandWrapper.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点バッファビューを設定.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferInfo = GetVertexBufferViewInfo();
		D3D12_INDEX_BUFFER_VIEW indexBufferInfo = GetIndexBufferViewInfo();
		commandWrapper.SetVertexBuffers(&vertexBufferInfo, 1);
		commandWrapper.SetIndexBuffer(&indexBufferInfo);

		// ビューポートとシザリングを設定.
		if (pViewPort) {
			commandWrapper.SetViewports(pViewPort, 1);
		}
		if (pScissor) {
			commandWrapper.SetScissorRects(pScissor, 1);
		}

		// 頂点描画時などに使うシェーダー情報.
		{
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// 深度.
		{
			auto* pDs = GetDepthStencil();
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(pDs->GetSrvHeapCategory(), pDs->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pDs->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pDs->GetAccessRootParam(), dHandle);
		}

		// 法線.
		{
			auto* pRT = GetNormalRenderTarget();
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(pRT->GetSrvHeapCategory(), pRT->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(pRT->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(pRT->GetAccessRootParam(), handle);
		}

		commandWrapper.DrawIndexedInstanced(GetIndexNum(), 1, 0, 0, 0);
	}
}

void CSprite::MapVertexData()
{
	RETURN(m_pVertexBufferResource);

	uint8_t* pBuffer = nullptr;
	HRESULT ret = m_pVertexBufferResource->Map(0, nullptr, (void**)(&pBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_vertexList.size(); n++) {
		const auto& vertex = m_vertexList[n];

		auto pos = vertex.pos;
		memcpy(pBuffer, &pos, sizeof(pos));
		pBuffer += sizeof(pos);

		auto uv = vertex.uv;
		memcpy(pBuffer, &uv, sizeof(uv));
		pBuffer += sizeof(uv);
	}

	m_pVertexBufferResource->Unmap(0, nullptr);
}

void CSprite::MapIndexData()
{
	RETURN(m_pIndexBufferResource);

	uint16_t* pIndexBuffer = nullptr;
	HRESULT ret = m_pIndexBufferResource->Map(0, nullptr, (void**)(&pIndexBuffer));
	VRETURN(ret == S_OK);

	for (uint32_t n = 0; n < m_indexList.size(); n++) {
		pIndexBuffer[n] = m_indexList[n];
	}

	m_pIndexBufferResource->Unmap(0, nullptr);
}

void CSprite::MapGaussianData()
{
	RETURN(m_pGaussianResource);

	SShaderGaussianInfo* pBuffer = nullptr;
	HRESULT ret = m_pGaussianResource->Map(0, nullptr, (void**)(&pBuffer));
	VRETURN(ret == S_OK);

	// ガウス関数（a * exp(-(x - b)^2 / 2c^2)）を使って釣鐘型のウェイトを作る.
	// 合計で１にしたいので、最終的にトータルのウェイトで割るようにする.
	// シェーダー側で使う際に、n = 0を中心として上下、もしくは左右に適用させるため、1~7に関しては2倍換算にする.
	const float base = 0.0f;
	const float smoothness = 3.0f;
	float total = 0.0f;
	for (int n = 0; n < _countof(pBuffer->weight); n++) {
		pBuffer->weight[n] = expf(-powf(static_cast<float>(n) - base, 2.0f) / (2 * powf(smoothness, 2.0f)));
		total += (n == 0) ? pBuffer->weight[n] : pBuffer->weight[n] * 2.0f;
	}
	for (int n = 0; n < _countof(pBuffer->weight); n++) {
		pBuffer->weight[n] /= total;
	}

	// 有効度に関しての情報も渡しておく.
	pBuffer->isEnableX = (m_isEnableGaussianX) ? 1.0f : 0.0f;
	pBuffer->isEnableY = (m_isEnableGaussianY) ? 1.0f : 0.0f;

	m_pGaussianResource->Unmap(0, nullptr);
}

void CSprite::MapShaderInfo(CGraphicsController& graphicsController, const ICamera* pCamera)
{
	VRETURN(m_pShaderInfoResource);
	assert(m_pShaderInfoResource->GetDesc().Width == util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderSpriteInfo)), 256));
	SShaderSpriteInfo shaderInfo = {};
	{
		// 必要な情報集める.
		shaderInfo.toScreen.r[0].m128_f32[0] = 2.0f / graphicsController.GetWindowWidth();
		shaderInfo.toScreen.r[1].m128_f32[1] = -2.0f / graphicsController.GetWindowHeight();
		shaderInfo.toScreen.r[3].m128_f32[0] = -1.0f;
		shaderInfo.toScreen.r[3].m128_f32[1] = 1.0f;
		shaderInfo.isEnableDof = (IsEnableDof()) ? 1.0f : 0.0f;
		shaderInfo.isEnableSsao = (IsEnableSsao()) ? 1.0f : 0.0f;
		if (pCamera) {
			// 位置3D位置復元用に.
			shaderInfo.proj = pCamera->GetPerspectiveMatrix();
			DirectX::XMVECTOR date = {};
			shaderInfo.projInv = DirectX::XMMatrixInverse(&date, pCamera->GetPerspectiveMatrix());
		}
		// リソースにマッピング.
		SShaderSpriteInfo* pBuff = nullptr;
		if (m_pShaderInfoResource->Map(0, nullptr, (void**)&pBuff) == S_OK) {
			(*pBuff) = shaderInfo;
			m_pShaderInfoResource->Unmap(0, nullptr);
		}
	}
}

bool CSprite::BuildVertexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_vertexList.size() > 0, false);
	VRETURN_RET(!m_pVertexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_vertexList[0].GetDataSize() * m_vertexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_vertexBufferViewInfo.BufferLocation = m_pVertexBufferResource->GetGPUVirtualAddress();
		m_vertexBufferViewInfo.SizeInBytes = m_vertexList[0].GetDataSize() * static_cast<int>(m_vertexList.size());
		m_vertexBufferViewInfo.StrideInBytes = m_vertexList[0].GetDataSize();
	}

	return true;
}

bool CSprite::BuildIndexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(m_indexList.size() > 0, false);
	VRETURN_RET(!m_pIndexBufferResource, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// GPUリソースを確保.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = sizeof(m_indexList[0]) * m_indexList.size();
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pIndexBufferResource));
		RETURN_RET(ret == S_OK, false);
	}

	// GPU側から見れるようにビューを用意.
	{
		m_indexBufferViewInfo.BufferLocation = m_pIndexBufferResource->GetGPUVirtualAddress();
		m_indexBufferViewInfo.SizeInBytes = sizeof(m_indexList[0]) * static_cast<int>(m_indexList.size());
		m_indexBufferViewInfo.Format = DXGI_FORMAT_R16_UINT;
	}

	return true;
}

bool CSprite::BuildShaderInfoBuffer(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// サイズ計算用の情報.
	const int dataSize = util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderSpriteInfo)), 256);

	// CPU側からアクセスできるアップロード用バッファを用意.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = dataSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pShaderInfoResource));
		VRETURN_RET(ret == S_OK, false);
	}

	// ヒープ位置を割り当てる.
	m_shaderInfoHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_SHADER_INFO);
	VRETURN_RET(m_shaderInfoHeapPosition >= 0, false);

	// GPU側から見れるようにビューを用意.
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, m_shaderInfoHeapPosition);
		cbvDesc.BufferLocation = m_pShaderInfoResource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = dataSize;
		pDevice->CreateConstantBufferView(&cbvDesc, handle);
	}

	return true;
}

bool CSprite::BuildGaussianBuffer(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// サイズ計算用の情報.
	const int dataSize = util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderGaussianInfo)), 256);

	// CPU側からアクセスできるアップロード用バッファを用意.
	{
		D3D12_HEAP_PROPERTIES heapDesc = {};
		heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = dataSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ret = pDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pGaussianResource));
		VRETURN_RET(ret == S_OK, false);
	}

	// ヒープ位置を割り当てる.
	m_gaussianHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_GAUSSIAN_TABLE);
	VRETURN_RET(m_gaussianHeapPosition >= 0, false);

	// GPU側から見れるようにビューを用意.
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_GAUSSIAN_TABLE, m_gaussianHeapPosition);
		cbvDesc.BufferLocation = m_pGaussianResource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = dataSize;
		pDevice->CreateConstantBufferView(&cbvDesc, handle);
	}

	return true;
}

bool CSprite::BuildShader(CGraphicsController& graphicsController)
{
	(void)graphicsController;
	HRESULT ret = S_FALSE;
	ID3DBlob* pErrorBlob = nullptr;

	ret = D3DCompileFromFile(
		L"shader\\SpriteVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pVertexShader,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);
	
	ret = D3DCompileFromFile(
#if defined(ENABLE_GBUFFER_TEST)
		L"shader\\DeferredSpritePixelShader.hlsl",
#else
		L"shader\\SpritePixelShader.hlsl",
#endif
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pPixelShader,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	return true;
}

bool CSprite::BuildRootSignature(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pRootSignature, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	D3D12_DESCRIPTOR_RANGE descRange[11] = {};
	{
		// 座標変換行列用.
		descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descRange[0].BaseShaderRegister = 0;
		descRange[0].NumDescriptors = 1;
		descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// レンダーターゲット設定されていた時に使う.
		descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[1].BaseShaderRegister = 0;
		descRange[1].NumDescriptors = 1;
		descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// ガウシアン用情報.
		descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descRange[2].BaseShaderRegister = 1;
		descRange[2].NumDescriptors = 1;
		descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// 法線用.
		descRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[3].BaseShaderRegister = 1;
		descRange[3].NumDescriptors = 1;
		descRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// 深度描画用.
		descRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[4].BaseShaderRegister = 2;
		descRange[4].NumDescriptors = 1;
		descRange[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// ディファード用のノーマルマップ.
		descRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[5].BaseShaderRegister = 3;
		descRange[5].NumDescriptors = 1;
		descRange[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// ブルーム用のぼかし1.
		descRange[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[6].BaseShaderRegister = 4;
		descRange[6].NumDescriptors = 1;
		descRange[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// ブルーム用のぼかし2(縮小バッファ).
		descRange[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[7].BaseShaderRegister = 5;
		descRange[7].NumDescriptors = 1;
		descRange[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// DOF用ぼかし.
		descRange[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[8].BaseShaderRegister = 6;
		descRange[8].NumDescriptors = 1;
		descRange[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// DOF用ぼかし(縮小バッファ).
		descRange[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[9].BaseShaderRegister = 7;
		descRange[9].NumDescriptors = 1;
		descRange[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// SSAO
		descRange[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[10].BaseShaderRegister = 8;
		descRange[10].NumDescriptors = 1;
		descRange[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	// ルートパラメータ自体は、ヒープ分用意する.
	D3D12_ROOT_PARAMETER rootParam[ACCESS_ROOT_PARAM_NUM] = {};
	{
		ACCESS_ROOT_PARAM shaderInfo = ACCESS_ROOT_PARAM_SHADER_INFO;
		rootParam[shaderInfo].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[shaderInfo].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[shaderInfo].DescriptorTable.pDescriptorRanges = &descRange[0];
		rootParam[shaderInfo].DescriptorTable.NumDescriptorRanges = 1;

		ACCESS_ROOT_PARAM rtShaderView = ACCESS_ROOT_PARAM_RENDER_TARGET_SHADER_VIEW;
		rootParam[rtShaderView].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[rtShaderView].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[rtShaderView].DescriptorTable.pDescriptorRanges = &descRange[1];
		rootParam[rtShaderView].DescriptorTable.NumDescriptorRanges = 1;

		ACCESS_ROOT_PARAM gaussian = ACCESS_ROOT_PARAM_GAUSSIAN_TABLE;
		rootParam[gaussian].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[gaussian].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[gaussian].DescriptorTable.pDescriptorRanges = &descRange[2];
		rootParam[gaussian].DescriptorTable.NumDescriptorRanges = 1;

		ACCESS_ROOT_PARAM texture = ACCESS_ROOT_PARAM_TEXTURE;
		rootParam[texture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[texture].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[texture].DescriptorTable.pDescriptorRanges = &descRange[3];
		rootParam[texture].DescriptorTable.NumDescriptorRanges = 1;

		ACCESS_ROOT_PARAM dssv = ACCESS_ROOT_PARAM_DEPTH_STENCIL_SHADER_VIEW;
		rootParam[dssv].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[dssv].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[dssv].DescriptorTable.pDescriptorRanges = &descRange[4];
		rootParam[dssv].DescriptorTable.NumDescriptorRanges = 1;
		
		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_G_BUF_NORMAL;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[5];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}

		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_BLOOM;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[6];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}

		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_BLOOM_SHRINK;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[7];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}

		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_DOF;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[8];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}

		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_DOF_SHRINK;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[9];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}

		{
			ACCESS_ROOT_PARAM param = ACCESS_ROOT_PARAM_SSAO;
			rootParam[param].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[param].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[param].DescriptorTable.pDescriptorRanges = &descRange[10];
			rootParam[param].DescriptorTable.NumDescriptorRanges = 1;
		}
	}

	D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
	{
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc[0].MinLOD = 0;
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc[0].ShaderRegister = 0;
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.pParameters = rootParam;
	desc.NumParameters = _countof(rootParam);
	desc.pStaticSamplers = samplerDesc;
	desc.NumStaticSamplers = _countof(samplerDesc);
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pByteCode = nullptr;
	ID3DBlob* pError = nullptr;
	ret = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pByteCode, &pError);
	VRETURN_RET(ret == S_OK, false);

	ret = pDevice->CreateRootSignature(0, pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
	if (ret != S_OK) {
		pByteCode->Release();
		return false;
	}

	return true;
}

bool CSprite::BuildPipelineState(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pPipelineState, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

	desc.pRootSignature = m_pRootSignature;

	// 頂点シェーダー.
	desc.VS.pShaderBytecode = m_pVertexShader->GetBufferPointer();
	desc.VS.BytecodeLength = m_pVertexShader->GetBufferSize();

	// ピクセルシェーダー.
	desc.PS.pShaderBytecode = m_pPixelShader->GetBufferPointer();
	desc.PS.BytecodeLength = m_pPixelShader->GetBufferSize();

	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	desc.RasterizerState.MultisampleEnable = false;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.DepthClipEnable = true;

	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 頂点レイアウト.
	desc.InputLayout.pInputElementDescs = vertexLayout;
	desc.InputLayout.NumElements = _countof(vertexLayout);

	desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.DepthStencilState.DepthEnable = true;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
	return (ret == S_OK);
}

bool CSprite::BuildBloomShader(CGraphicsController& graphicsController)
{
	(void)graphicsController;
	HRESULT ret = S_FALSE;
	ID3DBlob* pErrorBlob = nullptr;

	ret = D3DCompileFromFile(
		L"shader\\BloomSpritePixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pBloomPS,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	return true;
}

bool CSprite::BuildBloomPipelineState(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pBloomPipelineState, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

	desc.pRootSignature = m_pRootSignature;

	// 頂点シェーダー.
	desc.VS.pShaderBytecode = m_pVertexShader->GetBufferPointer();
	desc.VS.BytecodeLength = m_pVertexShader->GetBufferSize();

	// ピクセルシェーダー.
	desc.PS.pShaderBytecode = m_pBloomPS->GetBufferPointer();
	desc.PS.BytecodeLength = m_pBloomPS->GetBufferSize();

	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	desc.RasterizerState.MultisampleEnable = false;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.DepthClipEnable = true;

	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 頂点レイアウト.
	desc.InputLayout.pInputElementDescs = vertexLayout;
	desc.InputLayout.NumElements = _countof(vertexLayout);

	desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.DepthStencilState.DepthEnable = true;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pBloomPipelineState));
	return (ret == S_OK);
}

bool CSprite::BuildSSAOShader(CGraphicsController& graphicsController)
{
	(void)graphicsController;
	HRESULT ret = S_FALSE;
	ID3DBlob* pErrorBlob = nullptr;

	ret = D3DCompileFromFile(
		L"shader\\SSAOSpritePixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pSSAOPS,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	return true;
}

bool CSprite::BuildSSAOPipelineState(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pSSAOPipelineState, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

	desc.pRootSignature = m_pRootSignature;

	// 頂点シェーダー.
	desc.VS.pShaderBytecode = m_pVertexShader->GetBufferPointer();
	desc.VS.BytecodeLength = m_pVertexShader->GetBufferSize();

	// ピクセルシェーダー.
	desc.PS.pShaderBytecode = m_pSSAOPS->GetBufferPointer();
	desc.PS.BytecodeLength = m_pSSAOPS->GetBufferSize();

	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	desc.RasterizerState.MultisampleEnable = false;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.DepthClipEnable = true;

	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// 頂点レイアウト.
	desc.InputLayout.pInputElementDescs = vertexLayout;
	desc.InputLayout.NumElements = _countof(vertexLayout);

	desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.DepthStencilState.DepthEnable = true;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pSSAOPipelineState));
	return (ret == S_OK);
}
