#include "LightCulling.h"
#include "../../GraphicsController.h"

void CLightCulling::Init(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN(pDevice);

	// ���o�̓o�b�t�@�[�̏���.
	{
		HRESULT ret = S_FALSE;

		// ���C�g���.
		{
			// �T�C�Y�v�Z�p�̏��.
			const int dataSize = util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderLightInfo)), 256);

			// CPU������A�N�Z�X�ł���A�b�v���[�h�p�o�b�t�@��p��.
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

			// �q�[�v�ʒu�����蓖�Ă�.
			m_cbvHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_HUGE);
			VRETURN(m_cbvHeapPosition >= 0);

			// GPU�����猩���悤�Ƀr���[��p��.
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_cbvHeapPosition);
				cbvDesc.BufferLocation = m_cbvResource->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = dataSize;
				pDevice->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		// ���C�g�J�����O�̌��ʎ󂯎��.
		{
			// �T�C�Y�v�Z�p�̏��.
			const int dataSize = util::AdjustRowPitch(sizeof(uint32_t) * LIGHT_CULLING_INDEX_MAX, 256);

			// GPU�����珑���߂����郊�\�[�X��p��.
			{
				D3D12_HEAP_PROPERTIES heapDesc = {};
				heapDesc.Type = D3D12_HEAP_TYPE_CUSTOM;
				heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
				// L0���V�X�e���������AL1��VRAM�炵��.�V�X�e���������ɕ`���߂�����A�Ƃ����w��Ƃ����F��.
				heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

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
				resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

				ret = pDevice->CreateCommittedResource(
					&heapDesc,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(&m_uavResource));
				VRETURN(ret == S_OK);
			}

			// �q�[�v�ʒu�����蓖�Ă�.
			m_uavHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_HUGE);
			VRETURN(m_uavHeapPosition >= 0);

			// GPU�����猩���悤�Ƀr���[��p��.
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_uavHeapPosition);
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = LIGHT_CULLING_INDEX_MAX;
				uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
				uavDesc.Buffer.CounterOffsetInBytes = 0;
				pDevice->CreateUnorderedAccessView(m_uavResource , nullptr, &uavDesc, handle);
			}
		}

		// TODO �������̂��߂ɍŏ�����Map����悤�ɂ����Ⴄ.
	}

	// �V�F�[�_�[�r���h.
	{
		HRESULT ret = S_FALSE;
		ID3DBlob* pErrorBlob = nullptr;
		// �R���s���[�g�V�F�[�_�[.
		{
			ret = D3DCompileFromFile(
				L"shader\\light_culling\\LightCullingCS.hlsl",
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"cs_5_0",
				D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
				0,
				&m_pComputeShader,
				&pErrorBlob);
			VRETURN(ret == S_OK);
		}
	}

	// ���[�g�V�O�l�`���Ńq�[�v���Ƃ̕R�Â����쐬.
	{
		HRESULT ret = S_FALSE;

		D3D12_DESCRIPTOR_RANGE descRange[2] = {};
		{
			// ���C�g���.
			descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descRange[0].BaseShaderRegister = 0;
			descRange[0].NumDescriptors = 1;
			descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			// �����߂��p�o�b�t�@�[.
			descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descRange[1].BaseShaderRegister = 0;
			descRange[1].NumDescriptors = 1;
			descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

		// ���[�g�p�����[�^���̂́A�q�[�v���p�ӂ���.
		D3D12_ROOT_PARAMETER rootParam[2] = {};
		{
			rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[0].DescriptorTable.pDescriptorRanges = &descRange[0];
			rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

			rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam[1].DescriptorTable.pDescriptorRanges = &descRange[1];
			rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
		}

		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.pParameters = rootParam;
		desc.NumParameters = _countof(rootParam);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ID3DBlob* pByteCode = nullptr;
		ID3DBlob* pError = nullptr;
		ret = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pByteCode, &pError);
		VRETURN(ret == S_OK);

		ret = pDevice->CreateRootSignature(0, pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
		if (ret != S_OK) {
			pByteCode->Release();
		}
	}

	// �p�C�v���C���X�e�[�g�Ƃ��ĕ`����𐶐�.
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};

		desc.pRootSignature = m_pRootSignature;

		// �V�F�[�_�[.
		desc.CS.pShaderBytecode = m_pComputeShader->GetBufferPointer();
		desc.CS.BytecodeLength = m_pComputeShader->GetBufferSize();

		HRESULT ret = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
		VRETURN(ret == S_OK);
	}
}

void CLightCulling::Term()
{
	SAEF_RELEASE(m_pPipelineState);
	SAEF_RELEASE(m_pRootSignature);
	SAEF_RELEASE(m_pComputeShader);
	SAEF_RELEASE(m_uavResource);
}

void CLightCulling::Update()
{
}

void CLightCulling::RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper)
{
	// �R�}���h���X�g�ւ̐ݒ�.
	{
		// �p�C�v���C���ݒ�.
		commandWrapper.SetPipelineState(GetPipelineState());
		// ���[�g�V�O�l�`���ݒ�.
		commandWrapper.SetComputeRootSignature(GetRootSignature());
		// �K�v�ȃf�B�X�N���v�^�q�[�v��ݒ肷��.
		// TODO �R�}���h���X�g�S�̂ŒP��q�[�v�Ȃ�ǂ�����1��ݒ肷��΂���.�h���[�P�ʂŐݒ肷��Ȃ�A����ݒ肷��.�����A�X�g�[������\����Y��Ȃ�����.
		ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_HUGE) };
		commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
		// ����̕`��ɂ����郋�[�g�p�����[�^�Ƃ̕R�Â�.
		{
			// �V�[�����.
			commandWrapper.SetComputeRootDescriptorTable(0, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_uavHeapPosition));
			// �����߂����.
			commandWrapper.SetComputeRootDescriptorTable(1, heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_HUGE, m_cbvHeapPosition));
		}
	}
}