#include "Material.h"
#include "../../graphics/GraphicsController.h"

CMatarial::~CMatarial()
{
	SAEF_RELEASE(m_pLightResource);
}

bool CMatarial::Load(const SMMDMatarial& readData, const std::string& foulderName, CGraphicsController& graphicsController)
{
	VRETURN_RET(!foulderName.empty(), false);

	m_indicesNum = readData.indicesNum;

	m_light.diffuse = readData.diffuse;
	m_light.diffuse_alpha = readData.diffuse_alpha;
	m_light.specular = readData.specular;
	m_light.specularity = readData.specularity;
	m_light.ambient = readData.ambient;

	m_option.toonIdx = readData.toonIdx;
	m_option.edgeOption = readData.edgeOption;

	// ��Ƀ��C�g�֘A�̏��p�̃V�F�[�_�[�p���\�[�X�쐬.
	BuildLight(graphicsController);

	// �e�N�X�`���p�X�������Ă�̂ŁA�e�N�X�`���쐬���s��.
	bool isExistToon = false;
	{
		std::string texFileName = readData.texPath;

		std::vector<std::string> baseTexlist = {};
		std::vector<std::string> spTexlist = {};
		std::vector<std::string> spaTexlist = {};
		std::vector<std::string> toonTexlist = {};
		std::vector<std::string> splitList = {};
		util::Split(texFileName, '*', splitList);
		for (auto& it : splitList) {
			if (it.size() <= 0) {
				continue;
			}
			if ((util::GetExt(it) == u8"sph")) {
				spTexlist.push_back(it);
			}
			else if ((util::GetExt(it) == u8"spa")) {
				spaTexlist.push_back(it);
			}
			else {
				baseTexlist.push_back(it);
			}
		}

		if (baseTexlist.size() <= 0) {
			baseTexlist.push_back(u8"white.png");
		}
		if (spTexlist.size() <= 0) {
			spTexlist.push_back(u8"white.png");
		}
		if (spaTexlist.size() <= 0) {
			spaTexlist.push_back(u8"black.png");
		}

		// �g�D�[���͐������ԍ��w�肪����Ă��邩�ǂ����Ŕ���.
		char toonFileName[256] = {};
		sprintf_s(toonFileName, u8"toon%02d.bmp", static_cast<uint8_t>(readData.toonIdx + 1));
		toonTexlist.push_back(toonFileName);

		if (!baseTexlist.empty()) {
			if (!m_texture.texture.Load(foulderName + "\\" + baseTexlist[0], graphicsController, HEAP_CATEGORY_MATERIAL)) {
				VRETURN_RET(false, false);
			}
		}
		if (!spTexlist.empty()) {
			if (!m_texture.sphereTexture.Load(foulderName + "\\" + spTexlist[0], graphicsController, HEAP_CATEGORY_MATERIAL)) {
				VRETURN_RET(false, false);
			}
		}
		if (!spaTexlist.empty()) {
			if (!m_texture.sphereAddTexture.Load(foulderName + "\\" + spaTexlist[0], graphicsController, HEAP_CATEGORY_MATERIAL)) {
				VRETURN_RET(false, false);
			}
		}
		if (!toonTexlist.empty()) {
			if (!m_texture.toonTexture.Load(foulderName + "\\" + toonTexlist[0], graphicsController, HEAP_CATEGORY_MATERIAL)) {
				VRETURN_RET(false, false);
			}
			isExistToon = true;
		}
	}

	// �g�D�[�����ݒ肳��Ă���Ȃ�A�V�F�[�_�[���Ŏg���悤��.
	m_light.isValidToon = (isExistToon) ? 1.0f : 0.0f;

	// ���\�[�X�ɌŒ�����}�b�s���O.
	MapLightData();

	return true;
}

void CMatarial::MapLightData()
{
	VRETURN(m_pLightResource);
	
	uint8_t* pBuff = nullptr;
	const auto ret = m_pLightResource->Map(0, nullptr, (void**)&pBuff);
	VRETURN(ret == S_OK);

	uint8_t* pTmpFront = pBuff;

	auto diffuse = GetDiffuse();
	memcpy(pTmpFront, &diffuse, sizeof(diffuse));
	pTmpFront += sizeof(diffuse);

	auto diffuseA = GetDiffuseAlpha();
	memcpy(pTmpFront, &diffuseA, sizeof(diffuseA));
	pTmpFront += sizeof(diffuseA);

	auto specular = GetSpecular();
	memcpy(pTmpFront, &specular, sizeof(specular));
	pTmpFront += sizeof(specular);

	auto specularity = GetSpecularity();
	memcpy(pTmpFront, &specularity, sizeof(specularity));
	pTmpFront += sizeof(specularity);

	auto ambient = GetAmbient();
	memcpy(pTmpFront, &ambient, sizeof(ambient));
	pTmpFront += sizeof(ambient);

	auto isValidToon = IsValidToon();
	memcpy(pTmpFront, &isValidToon, sizeof(isValidToon));
	pTmpFront += sizeof(isValidToon);

	m_pLightResource->Unmap(0, nullptr);
	return;
}

bool CMatarial::BuildLight(CGraphicsController& graphicsController)
{
	auto* pDevice = graphicsController.GetGraphicsDevice();
	VRETURN_RET(pDevice, false);

	HRESULT ret = S_FALSE;

	const uint32_t dataSize = util::AdjustRowPitch(GetShaderUseDataSize(), 256);

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
			IID_PPV_ARGS(&m_pLightResource));
		VRETURN_RET(ret == S_OK, false);
	}

	// �q�[�v�ʒu�����蓖�Ă�.
	m_lightHeapPosition = graphicsController.AllocateHeapPosition(HEAP_CATEGORY_MATERIAL);
	VRETURN_RET(m_lightHeapPosition >= 0, false);

	// GPU�����猩���悤�Ƀr���[��p��.
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE handle = graphicsController.GetCPUDescriptorHandle(HEAP_CATEGORY_MATERIAL, m_lightHeapPosition);
		cbvDesc.BufferLocation = m_pLightResource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = dataSize;
		pDevice->CreateConstantBufferView(&cbvDesc, handle);
	}

	return true;
}
