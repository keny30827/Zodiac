#include "SSR.h"
#include "../../GraphicsController.h"

void CSSR::Init(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN(pDevice);

	// コンスタントバッファーの準備.
	{
		HRESULT ret = S_FALSE;

		// シーン情報.
		{
			// サイズ計算用の情報.
			const int dataSize = util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderPlaneInfo)), 256);

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
					IID_PPV_ARGS(&m_cbvResource));
				VRETURN(ret == S_OK);
			}

			// ヒープ位置を割り当てる.
			m_cbvHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_HUGE);
			VRETURN(m_cbvHeapPosition >= 0);

			// GPU側から見れるようにビューを用意.
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_cbvHeapPosition);
				cbvDesc.BufferLocation = m_cbvResource->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = dataSize;
				pDevice->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		// TODO 高速化のために最初からMapするようにしちゃう.
	}

	// シェーダービルド.
	{
		HRESULT ret = S_FALSE;
		ID3DBlob* pErrorBlob = nullptr;
		// 頂点シェーダー.
		{
			ret = D3DCompileFromFile(
				L"shader\\ssr\\SSRVS.hlsl",
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"vs_5_0",
				D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
				0,
				&m_pVertexShader,
				&pErrorBlob);
			VRETURN(ret == S_OK);
		}
		// ピクセルシェーダー.
		{
			ret = D3DCompileFromFile(
				L"shader\\ssr\\SSRPS.hlsl",
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"ps_5_0",
				D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
				0,
				&m_pPixelShader,
				&pErrorBlob);
			VRETURN(ret == S_OK);
		}
	}

	// ルートシグネチャでヒープ情報との紐づけを作成.
	{
		HRESULT ret = S_FALSE;

		D3D12_DESCRIPTOR_RANGE descRange[6] = {};
		{
			// 座標変換行列.
			descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descRange[0].BaseShaderRegister = 0;
			descRange[0].NumDescriptors = 1;
			descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// 深度.
			descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange[1].BaseShaderRegister = 0;
			descRange[1].NumDescriptors = 1;
			descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// カラー.
			descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange[2].BaseShaderRegister = 1;
			descRange[2].NumDescriptors = 1;
			descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// 法線.
			descRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange[3].BaseShaderRegister = 2;
			descRange[3].NumDescriptors = 1;
			descRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// オブジェクト情報.
			descRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange[4].BaseShaderRegister = 3;
			descRange[4].NumDescriptors = 1;
			descRange[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// スペキュラ.
			descRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange[5].BaseShaderRegister = 4;
			descRange[5].NumDescriptors = 1;
			descRange[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

		// ルートパラメータ自体は、ヒープ分用意する.
		D3D12_ROOT_PARAMETER rootParam[6] = {};
		{
			for (int n = 0; n < COUNTOF(rootParam); n++) {
				rootParam[n].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				rootParam[n].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				rootParam[n].DescriptorTable.pDescriptorRanges = &descRange[n];
				rootParam[n].DescriptorTable.NumDescriptorRanges = 1;
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
		VRETURN(ret == S_OK);

		ret = pDevice->CreateRootSignature(0, pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
		if (ret != S_OK) {
			pByteCode->Release();
		}
	}

	// パイプラインステートとして描画情報を生成.
	{
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
				"NORMAL",
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

		desc.NumRenderTargets = 6;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RTVFormats[4] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RTVFormats[5] = DXGI_FORMAT_R8G8B8A8_UNORM;

		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		desc.DepthStencilState.DepthEnable = true;
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
		VRETURN(ret == S_OK);
	}
}

void CSSR::Term()
{
	SAEF_RELEASE(m_pPipelineState);
	SAEF_RELEASE(m_pRootSignature);
	SAEF_RELEASE(m_pPixelShader);
	SAEF_RELEASE(m_pVertexShader);
	SAEF_RELEASE(m_cbvResource);
}

void CSSR::Update()
{
}

void CSSR::RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper)
{
	// 情報が足らない.
	VRETURN(m_inputDepthRT);
	VRETURN(m_inputBaseRT);
	VRETURN(m_inputNormalRT);
	VRETURN(m_inputObjectInfoRT);
	VRETURN(m_inputSpecularRT);

	// コマンドリストへの設定.
	{
		// パイプライン設定.
		commandWrapper.SetPipelineState(GetPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetRootSignature());
		// 必要なディスクリプタヒープを設定する.
		// TODO コマンドリスト全体で単一ヒープならどこかで1回設定すればいい.ドロー単位で設定するなら、毎回設定する.ただ、ストールする可能性を忘れないこと.
		ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_HUGE) };
		commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
		// 今回の描画におけるルートパラメータとの紐づけ.
		{
			// シーン情報.
			commandWrapper.SetGraphicsRootDescriptorTable(0, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_cbvHeapPosition));
			// 深度.
			commandWrapper.SetGraphicsRootDescriptorTable(1, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_inputDepthRT->GetSrvHeapPosition()));
			// カラー.
			commandWrapper.SetGraphicsRootDescriptorTable(2, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_inputBaseRT->GetSrvHeapPosition()));
			// 法線.
			commandWrapper.SetGraphicsRootDescriptorTable(3, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_inputNormalRT->GetSrvHeapPosition()));
			// オブジェクト情報.
			commandWrapper.SetGraphicsRootDescriptorTable(4, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_inputObjectInfoRT->GetSrvHeapPosition()));
			// スペキュラ.
			commandWrapper.SetGraphicsRootDescriptorTable(5, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_inputSpecularRT->GetSrvHeapPosition()));
		}
	}
}