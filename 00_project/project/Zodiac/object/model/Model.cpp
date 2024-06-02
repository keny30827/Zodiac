#include "Model.h"
#include "../../graphics/GraphicsController.h"

CModel::~CModel()
{
	SAEF_RELEASE(m_pPipelineState);
	SAEF_RELEASE(m_pRootSignature);
	SAEF_RELEASE(m_pPixelShader);
	SAEF_RELEASE(m_pVertexShader);
	SAEF_RELEASE(m_pShaderInfoResource);
	SAEF_RELEASE(m_pIndexBufferResource);
	SAEF_RELEASE(m_pVertexBufferResource);
}

bool CModel::Load(const char* foulderName, const char* fileName, CGraphicsController& graphicsController)
{
	VRETURN_RET(foulderName, false);
	VRETURN_RET(fileName, false);

	std::string foulder = foulderName;
	std::string modelName = fileName;
	FILE* fp = nullptr;
	fopen_s(&fp, (foulder + "\\" + modelName).c_str(), "rb");
	VRETURN_RET(fp, false);

	char signature[3] = {};
	fread(signature, sizeof(signature), 1, fp);

	SModelHeader& header = GetModelHeader();
	fread(&header, sizeof(header), 1, fp);

	// 頂点.
	{
		uint32_t vertexNum = 0;
		fread(&vertexNum, sizeof(vertexNum), 1, fp);
		for (uint32_t n = 0; n < vertexNum; n++) {
			SMMDVertex readData = {};
			readData.Serialize(fp);
			CVertex vertex = {};
			if (vertex.Load(readData)) {
				AddVertex(vertex);
			}
		}
	}

	// 頂点インデックス.
	{
		uint32_t indexNum = 0;
		fread(&indexNum, sizeof(indexNum), 1, fp);
		std::vector<uint16_t> indexBuffer(indexNum, 0);
		fread(indexBuffer.data(), sizeof(uint16_t), indexNum, fp);
		for (int n = 0; n < indexBuffer.size(); n++) {
			AddIndex(indexBuffer[n]);
		}
	}

	// マテリアル.
	{
		uint32_t materialNum = 0;
		fread(&materialNum, sizeof(materialNum), 1, fp);
		for (uint32_t n = 0; n < materialNum; n++) {
			SMMDMatarial data = {};
			data.Serialize(fp);
			CMatarial material = {};
			if (material.Load(data, foulderName, graphicsController)) {
				AddMaterial(material);
			}
		}
	}

	// 骨.
	{
		uint16_t boneNum = 0;
		fread(&boneNum, sizeof(boneNum), 1, fp);
		for (uint32_t n = 0; n < boneNum; n++) {
			SMMDBone data = {};
			data.Serialize(fp);
			CBone bone = {};
			if (bone.Load(data)) {
				AddBone(bone);
			}
		}
	}

	uint16_t ikNum = 0;
	fread(&ikNum, sizeof(ikNum), 1, fp);
	for (int n = 0; n < static_cast<int>(ikNum); n++) {
		SIkInfo ikInfo = {};
		ikInfo.Serialize(fp);
		AddIkInfo(ikInfo);
	}

	fclose(fp);

	// ボーン間の関連付け.
	for (uint32_t n = 0; n < GetBoneListNum(); n++) {
		auto& bone = GetRowBone(n);
		if (bone.GetParentNo() >= GetBoneListNum()) { continue; }
		auto& parentBone = GetRowBone(bone.GetParentNo());
		parentBone.GetChildren().push_back(n);
	}

	// モデル単位で保持すグラフィックスリソース関連のデータの準備.
	VRETURN_RET(BuildVertexBuffer(graphicsController), false);
	VRETURN_RET(BuildIndexBuffer(graphicsController), false);
	VRETURN_RET(BuildShader(graphicsController), false);
	VRETURN_RET(BuildShaderInfoBuffer(graphicsController), false);
	VRETURN_RET(BuildRootSignature(graphicsController), false);
	VRETURN_RET(BuildPipelineState(graphicsController), false);

	VRETURN_RET(BuildShadowShader(graphicsController), false);
	VRETURN_RET(BuildShadowRootSignature(graphicsController), false);
	VRETURN_RET(BuildShadowPipelineState(graphicsController), false);

	// 頂点などの固定情報をＣＰＵ->ＧＰＵにマッピング.
	MapVertexData();
	MapIndexData();

	return true;
}

void CModel::Update()
{
}

void CModel::Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// コマンドリストにコマンドを積みましょう.
	{
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
			MapShaderInfo(rCamera);
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppList[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppList, COUNTOF(ppList));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// シャドー.
		{
			auto pDepth = GetDepthStencil();
			D3D12_GPU_DESCRIPTOR_HANDLE dHandle = heapWrapper.GetGPUDescriptorHandle(pDepth->GetSrvHeapCategory(), pDepth->GetSrvHeapPosition());
			ID3D12DescriptorHeap* ppList[] = { heapWrapper.GetDescriptorHeap(pDepth->GetSrvHeapCategory()) };
			commandWrapper.SetDescriptorHeaps(ppList, COUNTOF(ppList));
			commandWrapper.SetGraphicsRootDescriptorTable(pDepth->GetAccessRootParam(), dHandle);
		}

		// マテリアル基準で描画処理を行う.
		uint32_t drawIndexNum = 0;
		{
			ID3D12DescriptorHeap* ppList[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_MATERIAL) };
			commandWrapper.SetDescriptorHeaps(ppList, COUNTOF(ppList));
		}
		for (uint32_t n = 0; n < GetMaterialListNum(); n++) {
			auto* pMaterial = GetAtMaterial(n);
			const int position = pMaterial->GetLightHeapPosition();
			if (position < 0) { continue; }
			// テクスチャをGPUにコピーする.
			const ITexture* pList[] =
			{
				pMaterial->GetTexture(),
				pMaterial->GetSphereTexture(),
				pMaterial->GetSphereAddTexture(),
				pMaterial->GetToonTexture(),
			};
			for (int m = 0; m < _countof(pList); m++) {
				if (pList[m]->GetHeapPosition() < 0) { assert(false);  continue; }
				commandWrapper.CopyTexture(&pList[m]->GetCopyDst(), 0, 0, 0, &pList[m]->GetCopySrc(), nullptr);
				commandWrapper.ResourceBarrier(&pList[m]->GetCopyResourceBarrier(), 1);
			}
			// マテリアル毎に描画.
			D3D12_GPU_DESCRIPTOR_HANDLE handle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_MATERIAL, position);
			commandWrapper.SetGraphicsRootDescriptorTable(pMaterial->GetAccessRootParam(), handle);
			// 一旦地面影書くのやめる.
			// commandWrapper.DrawIndexedInstanced(pMaterial->GetIndicesNum(), 2, drawIndexNum, 0, 0);
			commandWrapper.DrawIndexedInstanced(pMaterial->GetIndicesNum(), 1, drawIndexNum, 0, 0);
			drawIndexNum += pMaterial->GetIndicesNum();
			// 使い終わったテクスチャの状態だけリセットしておく.
			for (int m = 0; m < _countof(pList); m++) {
				if (pList[m]->GetHeapPosition() < 0) { assert(false);  continue; }
				commandWrapper.ResourceBarrier(&pList[m]->GetResetResourceBarrier(), 1);
			}
		}
	}
}

void CModel::RenderShadow(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor)
{
	// コマンドリストにコマンドを積みましょう.
	{
		// パイプライン設定.
		commandWrapper.SetPipelineState(GetShadowPipelineState());
		// ルートシグネチャ設定.
		commandWrapper.SetGraphicsRootSignature(GetShadowRootSignature());

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
			MapShaderInfo(rCamera);
			D3D12_GPU_DESCRIPTOR_HANDLE shaderInfoHandle = heapWrapper.GetGPUDescriptorHandle(HEAP_CATEGORY_SHADER_INFO, GetShaderInfoHeapPosition());
			ID3D12DescriptorHeap* ppLists[] = { heapWrapper.GetDescriptorHeap(HEAP_CATEGORY_SHADER_INFO) };
			commandWrapper.SetDescriptorHeaps(ppLists, COUNTOF(ppLists));
			commandWrapper.SetGraphicsRootDescriptorTable(GetAccessRootParam(), shaderInfoHandle);
		}

		// 描画.
		uint32_t drawIndexNum = 0;
		for (uint32_t n = 0; n < GetMaterialListNum(); n++) {
			auto* pMaterial = GetAtMaterial(n);
			commandWrapper.DrawIndexedInstanced(pMaterial->GetIndicesNum(), 1, drawIndexNum, 0, 0);
			drawIndexNum += pMaterial->GetIndicesNum();
		}
	}
}

void CModel::MapVertexData()
{
	RETURN(m_pVertexBufferResource);

	uint8_t* pBuffer = nullptr;
	HRESULT ret = m_pVertexBufferResource->Map(0, nullptr, (void**)(&pBuffer));
	VRETURN(ret == S_OK);

	const IVertex* pList = GetVertexList();
	for (uint32_t n = 0; n < GetVertexListNum(); n++) {
		const IVertex* pVertex = GetAtVertexList(n);

		auto pos = pVertex->GetPosition();
		memcpy(pBuffer, &pos, sizeof(pos));
		pBuffer += sizeof(pos);

		auto normal = pVertex->GetNormal();
		memcpy(pBuffer, &normal, sizeof(normal));
		pBuffer += sizeof(normal);

		auto uv = pVertex->GetUV();
		memcpy(pBuffer, &uv, sizeof(uv));
		pBuffer += sizeof(uv);

		auto pBoneNoList = pVertex->GetBoneNumberList();
		for (uint32_t m = 0; m < pVertex->GetBoneNumberListNum(); m++) {
			memcpy(pBuffer, &pBoneNoList[m], sizeof(pBoneNoList[m]));
			pBuffer += sizeof(pBoneNoList[m]);
		}

		auto boneW = pVertex->GetBoneWeight();
		memcpy(pBuffer, &boneW, sizeof(boneW));
		pBuffer += sizeof(boneW);

		auto edgeOption = pVertex->GetEdgeOption();
		memcpy(pBuffer, &edgeOption, sizeof(edgeOption));
		pBuffer += sizeof(edgeOption);
	}

	m_pVertexBufferResource->Unmap(0, nullptr);
}

void CModel::MapIndexData()
{
	RETURN(m_pIndexBufferResource);

	uint16_t* pIndexBuffer = nullptr;
	HRESULT ret = m_pIndexBufferResource->Map(0, nullptr, (void**)(&pIndexBuffer));
	VRETURN(ret == S_OK);

	auto* pIndexList = GetVertexIndexList();
	for (uint32_t n = 0; n < GetVertexIndexListNum(); n++) {
		pIndexBuffer[n] = pIndexList[n];
	}

	m_pIndexBufferResource->Unmap(0, nullptr);
}

void CModel::MapShaderInfo(const ICamera& rCamera)
{
	auto* pShaderInfoBuffer = GetShaderInfoResource();
	VRETURN(pShaderInfoBuffer);
	assert(pShaderInfoBuffer->GetDesc().Width == util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderCoordinateInfo)), 256));
	SShaderCoordinateInfo* shaderInfo = new SShaderCoordinateInfo();

	// 必要な情報集める.
	shaderInfo->world *= GetAffineMatrix();
	shaderInfo->view *= rCamera.GetViewMatrix();
	shaderInfo->proj *= rCamera.GetPerspectiveMatrix();
	shaderInfo->eye = rCamera.GetEye();

	// TEST 影.
	{
		DirectX::XMVECTOR placeNormal = { 0.0f, 1.0f, 0.0f, 0.0f };
		// wは0が平行、1が点光源.
		DirectX::XMVECTOR light = { -1.0f, 1.0f, -1.0f, 0.0f };
		shaderInfo->shadow = DirectX::XMMatrixShadow(placeNormal, light);

		// 光の方向.
		DirectX::XMVECTOR inverse = { -1.0f, -1.0f, -1.0f, 0.0f };
		DirectX::XMVECTOR lightDir = { 1.0f, -1.0f, 1.0f, 0.0f };
		DirectX::XMVECTOR vTarget = DirectX::XMLoadFloat3(&rCamera.GetAt());
		DirectX::XMVECTOR vEye = DirectX::XMLoadFloat3(&rCamera.GetEye());
		DirectX::XMVECTOR vUp = DirectX::XMLoadFloat3(&rCamera.GetUp());
		DirectX::XMVECTOR vLength = DirectX::XMVector3Length(DirectX::XMVectorSubtract(vTarget, vEye));
		vLength.m128_f32[3] = 0.0f;
		DirectX::XMVECTOR lightPos = DirectX::XMVectorAdd(vTarget, DirectX::XMVectorMultiply(DirectX::XMVectorMultiply(lightDir, inverse), vLength));
		// カメラ座標への変換行列＆平行投影によるクリップ空間への変換.
		shaderInfo->lightView =
			DirectX::XMMatrixLookAtLH(lightPos, vTarget, vUp) *
			DirectX::XMMatrixOrthographicLH(20.0f, 20.0f, 1.0f, 100.0f);
	}

	for (uint32_t n = 0; n < GetBoneListNum(); n++) {
		const auto bone = GetAtBone(n);
		shaderInfo->bone[n] = bone->GetAffineMatrix();
	}
	// リソースにマッピング.
	SShaderCoordinateInfo* pBuff = nullptr;
	if (pShaderInfoBuffer->Map(0, nullptr, (void**)&pBuff) == S_OK) {
		(*pBuff) = (*shaderInfo);
		pShaderInfoBuffer->Unmap(0, nullptr);
	}

	delete shaderInfo;
}

bool CModel::BuildVertexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(GetVertexListNum() > 0, false);
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
		resourceDesc.Width = GetVertexList()[0].GetDataSize() * GetVertexListNum();
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
		m_vertexBufferViewInfo.SizeInBytes = GetVertexList()[0].GetDataSize() * GetVertexListNum();
		m_vertexBufferViewInfo.StrideInBytes = GetVertexList()[0].GetDataSize();
	}

	return true;
}

bool CModel::BuildIndexBuffer(CGraphicsController& graphicsController)
{
	VRETURN_RET(GetVertexIndexListNum() > 0, false);
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
		resourceDesc.Width = sizeof(GetVertexIndexList()[0]) * GetVertexIndexListNum();
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
		m_indexBufferViewInfo.SizeInBytes = sizeof(GetVertexIndexList()[0]) * GetVertexIndexListNum();
		m_indexBufferViewInfo.Format = DXGI_FORMAT_R16_UINT;
	}

	return true;
}

bool CModel::BuildShaderInfoBuffer(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	// サイズ計算用の情報.
	const int dataSize = util::AdjustRowPitch(static_cast<uint32_t>(sizeof(SShaderCoordinateInfo)), 256);

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

bool CModel::BuildShader(CGraphicsController& graphicsController)
{
	(void)graphicsController;
	HRESULT ret = S_FALSE;
	ID3DBlob* pErrorBlob = nullptr;

	ret = D3DCompileFromFile(
		L"shader\\BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pVertexShader,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	ret = D3DCompileFromFile(
		L"shader\\BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pPixelShader,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	return true;
}

bool CModel::BuildRootSignature(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pRootSignature, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	D3D12_DESCRIPTOR_RANGE descRange[4] = {};
	{
		// 座標変換行列用.
		descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descRange[0].BaseShaderRegister = 0;
		descRange[0].NumDescriptors = 1;
		descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// マテリアル情報用.
		descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descRange[1].BaseShaderRegister = 1;
		descRange[1].NumDescriptors = CMatarial::GetLightHeapSize();
		descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// マテリアルテクスチャ用.
		descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[2].BaseShaderRegister = 0;
		descRange[2].NumDescriptors = CMatarial::GetTexHeapSize();
		descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// シャドウ用.
		descRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange[3].BaseShaderRegister = CMatarial::GetTexHeapSize();
		descRange[3].NumDescriptors = 1;
		descRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	// ルートパラメータ自体は、ヒープ分用意する.
	D3D12_ROOT_PARAMETER rootParam[ACCESS_ROOT_PARAM_NUM] = {};
	{
		ACCESS_ROOT_PARAM shaderInfo = ACCESS_ROOT_PARAM_SHADER_INFO;
		rootParam[shaderInfo].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[shaderInfo].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[shaderInfo].DescriptorTable.pDescriptorRanges = &descRange[0];
		rootParam[shaderInfo].DescriptorTable.NumDescriptorRanges = 1;

		ACCESS_ROOT_PARAM material = ACCESS_ROOT_PARAM_MATERIAL;
		rootParam[material].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[material].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[material].DescriptorTable.pDescriptorRanges = &descRange[1];
		rootParam[material].DescriptorTable.NumDescriptorRanges = 2;

		ACCESS_ROOT_PARAM dssv = ACCESS_ROOT_PARAM_DEPTH_STENCIL_SHADER_VIEW;
		rootParam[dssv].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[dssv].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[dssv].DescriptorTable.pDescriptorRanges = &descRange[3];
		rootParam[dssv].DescriptorTable.NumDescriptorRanges = 1;
	}

	D3D12_STATIC_SAMPLER_DESC samplerDesc[3] = {};
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

		samplerDesc[1] = samplerDesc[0];
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[1].ShaderRegister = 1;

		samplerDesc[2] = samplerDesc[0];
		samplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc[2].ShaderRegister = 2;
		samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc[2].MaxAnisotropy = 1;
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

bool CModel::BuildPipelineState(CGraphicsController& graphicsController)
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
		{
			"BONE_NO",
			0,
			DXGI_FORMAT_R16G16_UINT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"BONE_W",
			0,
			DXGI_FORMAT_R16_UINT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"EDGE",
			0,
			DXGI_FORMAT_R16_UINT,
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

	desc.NumRenderTargets = 4;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.DepthStencilState.DepthEnable = true;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pPipelineState));
	return (ret == S_OK);
}

bool CModel::BuildShadowShader(CGraphicsController& graphicsController)
{
	(void)graphicsController;
	HRESULT ret = S_FALSE;
	ID3DBlob* pErrorBlob = nullptr;

	ret = D3DCompileFromFile(
		L"shader\\ShadowVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_pShadowVertexShader,
		&pErrorBlob);
	VRETURN_RET(ret == S_OK, false);

	return true;
}

bool CModel::BuildShadowRootSignature(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pShadowRootSignature, false);

	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	D3D12_DESCRIPTOR_RANGE descRange[1] = {};
	{
		// 座標変換行列用.
		descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descRange[0].BaseShaderRegister = 0;
		descRange[0].NumDescriptors = 1;
		descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	// ルートパラメータ自体は、ヒープ分用意する.
	D3D12_ROOT_PARAMETER rootParam[ACCESS_ROOT_PARAM_NUM] = {};
	{
		ACCESS_ROOT_PARAM shaderInfo = ACCESS_ROOT_PARAM_SHADER_INFO;
		rootParam[shaderInfo].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[shaderInfo].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParam[shaderInfo].DescriptorTable.pDescriptorRanges = &descRange[0];
		rootParam[shaderInfo].DescriptorTable.NumDescriptorRanges = 1;
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

	ret = pDevice->CreateRootSignature(0, pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), IID_PPV_ARGS(&m_pShadowRootSignature));
	if (ret != S_OK) {
		pByteCode->Release();
		return false;
	}

	return true;
}

bool CModel::BuildShadowPipelineState(CGraphicsController& graphicsController)
{
	VRETURN_RET(!m_pShadowPipelineState, false);

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
		{
			"BONE_NO",
			0,
			DXGI_FORMAT_R16G16_UINT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"BONE_W",
			0,
			DXGI_FORMAT_R16_UINT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
		{
			"EDGE",
			0,
			DXGI_FORMAT_R16_UINT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

	desc.pRootSignature = m_pShadowRootSignature;

	// 頂点シェーダー.
	desc.VS.pShaderBytecode = m_pShadowVertexShader->GetBufferPointer();
	desc.VS.BytecodeLength = m_pShadowVertexShader->GetBufferSize();

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

	// RTへの書き込みは不要なので、設定しない.
	desc.NumRenderTargets = 0;
	desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	desc.DepthStencilState.DepthEnable = true;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	HRESULT ret = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pShadowPipelineState));
	return (ret == S_OK);
}