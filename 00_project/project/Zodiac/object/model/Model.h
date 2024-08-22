#pragma once

#include "../../graphics/GraphicsDefs.h"

#include "../vertex/Vertex.h"
#include "../material/Material.h"
#include "../bone/Bone.h"

struct SModelHeader {
	float version = 0.0f;
	char modelName[20] = {};
	char comment[256] = {};
};

struct SIkInfo {
	uint16_t boneIdx = 0;
	uint16_t targetIdx = 0;
	uint8_t chainBoneNum = 0;
	uint16_t iterationNum = 0;
	float rotateLimit = 0.0f;
	std::vector<uint16_t> chainBone = {};
	void Serialize(FILE* fp)
	{
		if (!fp) { return; }
		fread(&boneIdx, sizeof(boneIdx), 1, fp);
		fread(&targetIdx, sizeof(targetIdx), 1, fp);
		fread(&chainBoneNum, sizeof(chainBoneNum), 1, fp);
		fread(&iterationNum, sizeof(iterationNum), 1, fp);
		fread(&rotateLimit, sizeof(rotateLimit), 1, fp);
		chainBone.resize(chainBoneNum);
		fread(chainBone.data(), sizeof(chainBone[0]), chainBoneNum, fp);
	}
};

class CModel : public IModel {
public:
	CModel() = default;
	virtual ~CModel();

public:
	bool Load(const char* foulderName, const char* fileName, CGraphicsController& graphicsController);
	void Update();
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderShadow(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderDepthPrepass(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;
	virtual void RenderOutline(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) override;

private:
	void MapVertexData();
	void MapIndexData();
	void MapShaderInfo(const ICamera& rCamera);

public:
	SModelHeader& GetModelHeader() { return m_modelHeader; }

public:
	virtual const IVertex* GetVertexList() const override
	{
		return m_vertexList.data();
	}
	virtual const IVertex* GetAtVertexList(const uint32_t idx) const override
	{
		return &m_vertexList[idx];
	}
	virtual const uint32_t GetVertexListNum() const override
	{
		return static_cast<uint32_t>(m_vertexList.size());
	}
	virtual const uint16_t* GetVertexIndexList() const override
	{
		return m_indexList.data();
	}
	virtual const uint16_t* GetAtVertexIndex(const uint32_t idx) const override
	{
		return &m_indexList[idx];
	}
	virtual const uint32_t GetVertexIndexListNum() const override
	{
		return static_cast<uint32_t>(m_indexList.size());
	}
	virtual const IMaterial* GetMaterialList() const override
	{
		return m_materialList.data();
	}
	virtual const IMaterial* GetAtMaterial(const uint32_t idx) const override
	{
		return &m_materialList[idx];
	}
	virtual const uint32_t GetMaterialListNum() const override
	{
		return static_cast<uint32_t>(m_materialList.size());
	}
	virtual const IBone* GetBoneList() const override
	{
		return m_boneList.data();
	}
	virtual const IBone* GetAtBone(const uint32_t idx) const override
	{
		return &m_boneList[idx];
	}
	virtual const uint32_t GetBoneListNum() const override
	{
		return static_cast<uint32_t>(m_boneList.size());
	}
	virtual const DirectX::XMMATRIX GetAffineMatrix() const override
	{
		return m_pos * m_rot;
	}
	virtual ID3D12Resource* GetVertexBufferResource() override
	{
		return m_pVertexBufferResource;
	}
	virtual const D3D12_VERTEX_BUFFER_VIEW GetVertexBufferViewInfo() const override
	{
		return m_vertexBufferViewInfo;
	}
	virtual ID3D12Resource* GetIndexBufferResource() override
	{
		return m_pIndexBufferResource;
	}
	virtual const D3D12_INDEX_BUFFER_VIEW GetIndexBufferViewInfo() const override
	{
		return m_indexBufferViewInfo;
	}
	virtual ID3D12Resource* GetShaderInfoResource() override
	{
		return m_pShaderInfoResource;
	}
	virtual const int GetShaderInfoHeapPosition() const override
	{
		return m_shaderInfoHeapPosition;
	}
	virtual ID3D12PipelineState* GetPipelineState() override
	{
		return m_pPipelineState;
	}
	virtual ID3D12RootSignature* GetRootSignature() override
	{
		return m_pRootSignature;
	}
	virtual ID3DBlob* GetVertexShader() override
	{
		return m_pVertexShader;
	}
	virtual ID3DBlob* GetPixelShader() override
	{
		return m_pPixelShader;
	}
	virtual ID3D12PipelineState* GetShadowPipelineState() override
	{
		return m_pShadowPipelineState;
	}
	virtual ID3D12RootSignature* GetShadowRootSignature() override
	{
		return m_pShadowRootSignature;
	}
	virtual void SetDepthStencil(IDepthStencil* pDS) override
	{
		m_pDepthStencil = pDS;
	}
	virtual IDepthStencil* GetDepthStencil() override
	{
		return m_pDepthStencil;
	}
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() override
	{
		return ACCESS_ROOT_PARAM_SHADER_INFO;
	}

public:
	const CBone& GetBone(const int index) const { return m_boneList.at(index); }
	CBone& GetRowBone(const int index) { return m_boneList.at(index); }
	const CBone& GetBone(const std::string name) const
	{
		static const CBone error = {};
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			return GetBone(ret->second);
		}
		return error;
	}
	CBone& GetRowBone(const std::string name)
	{
		static CBone error = {};
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			return GetRowBone(ret->second);
		}
		return error;
	}

	void ClearBoneAll()
	{
		for (auto& it : m_boneList) {
			it.ClearMatrix();
		}
	}

	void ClearAffineBoneRecursive(const std::string name)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			ClearAffineBoneRecursive(ret->second);
		}
	}
	void ClearAffineBoneRecursive(const uint32_t idx)
	{
		CBone& bone = GetRowBone(idx);
		bone.GetRowAffine() = DirectX::XMMatrixIdentity();
		for (auto it : bone.GetChildren()) {
			ClearAffineBoneRecursive(it);
		}
	}

	void ApplyRotationBone(const std::string name, DirectX::XMMATRIX rot, const bool isClear = false)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			ApplyRotationBone(ret->second, rot, isClear);
		}
	}

	void ApplyRotationBone(const uint32_t idx, DirectX::XMMATRIX rot, const bool isClear = false)
	{
		CBone& bone = GetRowBone(idx);
		auto& boneRot = bone.GetRowRot();
		const auto& bonePos = bone.GetOriginPos();
		if (isClear) { boneRot = DirectX::XMMatrixIdentity(); }
		boneRot *=
			DirectX::XMMatrixTranslation(-bonePos.x, -bonePos.y, -bonePos.z) *
			rot *
			DirectX::XMMatrixTranslation(bonePos.x, bonePos.y, bonePos.z);
	}

	void ApplyTransBone(const std::string name, DirectX::XMMATRIX trans)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			CBone& bone = GetRowBone(ret->second);
			bone.GetRowTrans() *= trans;
		}
	}

	void CalcAffineBone(const std::string name)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			CBone& bone = GetRowBone(ret->second);
			bone.GetRowAffine() = bone.GetRowRot() * bone.GetRowTrans();
		}
	}
	void CalcAffineBoneRecursive(const std::string name)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			CalcAffineBoneRecursive(ret->second);
		}
	}
	void CalcAffineBoneRecursive(const uint32_t boneIndex)
	{
		CBone& bone = GetRowBone(boneIndex);
		bone.GetRowAffine() = bone.GetRowRot() * bone.GetRowTrans();
		for (auto it : bone.GetChildren()) {
			CalcAffineBoneRecursive(it);
		}
	}

	void ApplyAffineBoneRecursive(const std::string name)
	{
		auto ret = m_boneName2Index.find(name);
		if (ret != m_boneName2Index.end()) {
			CBone& bone = GetRowBone(ret->second);
			if (GetBoneListNum() <= bone.GetParentNo()) {
				// 親なしは自分スタート.
				for (auto it : bone.GetChildren()) {
					ApplyAffineBoneRecursive(ret->second, it, bone.GetRowAffine());
				}
			}
			else {
				// 自分を含む更新から開始.
				CBone& parentBone = GetRowBone(bone.GetParentNo());
				ApplyAffineBoneRecursive(bone.GetParentNo(), ret->second, parentBone.GetRowAffine());
			}
		}
	}
	void ApplyAffineBoneRecursive(const uint32_t parentBoneIndex, const uint32_t boneIndex, DirectX::XMMATRIX affine)
	{
		if (GetBoneListNum() <= boneIndex) { return; }
		CBone& bone = GetRowBone(boneIndex);
		bone.GetRowAffine() *= affine;
		for (auto it : bone.GetChildren()) {
			ApplyAffineBoneRecursive(boneIndex, it, bone.GetRowAffine());
		}
	}

public:
	int GetIkInfoNum() const { return static_cast<int>(m_ikInfo.size()); }
	const SIkInfo& GetIkInfo(const int index) const { return m_ikInfo.at(index); }

public:
	void SetRot(const DirectX::XMMATRIX& mat) { m_rot = mat; }
	void SetPos(const DirectX::XMMATRIX& mat) { m_pos = mat; }

	float GetOutlineScale() const { return m_outlineScale; }
#if defined(DEBUG)
	void OnDebugInputOutlineScale(float f) { m_outlineScale = f; }
#endif

private:
	void AddVertex(const CVertex& vertex) { m_vertexList.push_back(vertex); }
	void AddIndex(const uint16_t index) { m_indexList.push_back(index); }
	void AddMaterial(const CMatarial& data) { m_materialList.push_back(data); }
	void AddBone(const CBone data) {
		m_boneName2Index[data.GetName()] = static_cast<uint32_t>(m_boneList.size());
		m_boneList.push_back(data);
	}
	void AddIkInfo(const SIkInfo data) { m_ikInfo.push_back(data); }

private:
	CMatarial& GetRowMaterial(const int index) { return m_materialList.at(index); }

private:
	bool BuildVertexBuffer(CGraphicsController& graphicsController);
	bool BuildIndexBuffer(CGraphicsController& graphicsController);
	bool BuildShaderInfoBuffer(CGraphicsController& graphicsController);
	bool BuildShader(CGraphicsController& graphicsController);
	bool BuildRootSignature(CGraphicsController& graphicsController);
	bool BuildPipelineState(CGraphicsController& graphicsController);

	// シャドウマップ用.
	bool BuildShadowShader(CGraphicsController& graphicsController);
	bool BuildShadowRootSignature(CGraphicsController& graphicsController);
	bool BuildShadowPipelineState(CGraphicsController& graphicsController);

	// デプスプリパス用.
	bool BuildDepthPrepassShader(CGraphicsController& graphicsController);
	bool BuildDepthPrepassRootSignature(CGraphicsController& graphicsController);
	bool BuildDepthPrepassPipelineState(CGraphicsController& graphicsController);

	// アウトライン用.
	bool BuildOutlineShader(CGraphicsController& graphicsController);
	bool BuildOutlineRootSignature(CGraphicsController& graphicsController);
	bool BuildOutlinePipelineState(CGraphicsController& graphicsController);

private:
	// CPU側で使うモデル情報.
	std::vector<CVertex> m_vertexList = {};
	std::vector<uint16_t> m_indexList = {};
	std::vector<CMatarial> m_materialList = {};
	std::vector<CBone> m_boneList = {};
	std::vector<SIkInfo> m_ikInfo = {};
	std::map<std::string, uint32_t> m_boneName2Index = {};
	SModelHeader m_modelHeader = {};
	DirectX::XMMATRIX m_rot = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_pos = DirectX::XMMatrixIdentity();

	// GPUリソース関連.
	ID3D12Resource* m_pVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViewInfo = {};
	ID3D12Resource* m_pIndexBufferResource = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferViewInfo = {};
	ID3D12Resource* m_pShaderInfoResource = nullptr;
	int m_shaderInfoHeapPosition = -1;
	ID3D12PipelineState* m_pPipelineState = nullptr;
	ID3D12RootSignature* m_pRootSignature = nullptr;
	ID3DBlob* m_pVertexShader = nullptr;
	ID3DBlob* m_pPixelShader = nullptr;
	// シャドウマップ用.
	ID3D12PipelineState* m_pShadowPipelineState = nullptr;
	ID3D12RootSignature* m_pShadowRootSignature = nullptr;
	ID3DBlob* m_pShadowVertexShader = nullptr;
	IDepthStencil* m_pDepthStencil = nullptr;
	// シャドウマップ用.
	ID3D12PipelineState* m_pDepthPrepassPipelineState = nullptr;
	ID3D12RootSignature* m_pDepthPrepassRootSignature = nullptr;
	ID3DBlob* m_pDepthPrepassVertexShader = nullptr;
	// アウトライン用.
	ID3D12PipelineState* m_pOutlinePipelineState = nullptr;
	ID3D12RootSignature* m_pOutlineRootSignature = nullptr;
	ID3DBlob* m_pOutlineVertexShader = nullptr;
	ID3DBlob* m_pOutlinePixelShader = nullptr;

	float m_outlineScale = 0.001f;
};