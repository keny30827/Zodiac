#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "DirectXTex.h"

#include <vector>
#include <map>
#include <string>

// ====================================================.
// �e�X�g�p��`.
// ====================================================.
// #define ENABLE_GBUFFER_TEST

// ====================================================.
// �֗��}�N��.
// ====================================================.

#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))

#define VRETURN(a)			{ do { if (!(a)) { assert(false); return; } } while(false); }
#define VRETURN_RET(a, ret) { do { if (!(a)) { assert(false); return ret; } } while(false); }

#define RETURN(a)			{ do { if (!(a)) { return; } } while(false); }
#define RETURN_RET(a, ret)	{ do { if (!(a)) { return ret; } } while(false); }

#define SAEF_RELEASE(p)		{ if (p != nullptr) { p->Release(); p = nullptr; } }
#define SAEF_DELETE(p)		{ if (p != nullptr) { delete p; p = nullptr; } }

// ====================================================.
// �֗��֐�.������ǂ�����ł��A�N�Z�X�ł��郆�[�e�B���e�B�N���X�Ɉڂ�.
// ====================================================.

namespace util {

	static std::wstring C2W(const std::string& path)
	{
		std::wstring ret;
		const int wCharSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, path.c_str(), static_cast<int>(path.size()), nullptr, 0);
		if (wCharSize <= 0) { return ret; }
		ret.resize(wCharSize);
		MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, path.c_str(), static_cast<int>(path.size()), &ret[0], wCharSize);
		return ret;
	}

	static bool Split(const std::string& path, const char split, std::vector<std::string>& splitStr)
	{
		const size_t idx = path.rfind(split);
		if (idx == std::string::npos) {
			splitStr.push_back(path);
			return true;
		}
		splitStr.push_back(path.substr(0, idx));
		splitStr.push_back(path.substr(idx + 1, path.length() - idx - 1));
		return true;
	}

	static std::string GetExt(const std::string& path)
	{
		const size_t idx = path.rfind('.');
		if (idx == std::string::npos) { return std::string(); }
		return path.substr(idx + 1, path.length() - idx - 1);
	}

	static uint32_t AdjustRowPitch(const uint32_t origin, const uint32_t alignment)
	{
		return (origin + (alignment - (origin % alignment)));
	}
}

// ====================================================.
// �󂯓n���p�̋��ʃC���^�[�t�F�[�X.
// ====================================================.

// TODO NEXT �q�[�v��ނ̓}�e���A�����ɗp�ӂ��āA�r���[�P�ʂȂǂŗp�ӂ���̂͂�߂�.
//			 �q�[�v���̃f�[�^�̕��тƃ��[�g�p�����[�^�ł̃f�B�X�N���v�^�͈͎w�����v�����邽��.
// �f�B�X�N���v�^�q�[�v�̎��.
enum HEAP_CATEGORY {
	HEAP_CATEGORY_SWAP_CHAIN = 0,
	HEAP_CATEGORY_SHADER_INFO,
	HEAP_CATEGORY_MATERIAL,
	HEAP_CATEGORY_DEPTH_STENCIL,
	HEAP_CATEGORY_DEPTH_STENCIL_SHADER_VIEW,
	HEAP_CATEGORY_RENDER_TARGET,
	HEAP_CATEGORY_RENDER_TARGET_SHADER_VIEW,
	HEAP_CATEGORY_GAUSSIAN_TABLE,
	HEAP_CATEGORY_TEXTURE,

	HEAP_CATEGORY_HUGE,

	HEAP_CATEGORY_NUM,
	HEAP_CATEGORY_INVALID = -1,
};
static bool IsValidHeapCategory(const HEAP_CATEGORY e) { return (0 <= e) && (e < HEAP_CATEGORY_NUM); };

// TODO NEXT ���[�g�p�����[�^�̕��т́A�}�e���A���P�ʂŗp�ӂ���.
//			 �q�[�v���̃f�[�^�̕��тƃ��[�g�p�����[�^�ł̃f�B�X�N���v�^�͈͎w�����v�����邽��.
// ���[�g�p�����[�^�w����.
enum ACCESS_ROOT_PARAM {
	ACCESS_ROOT_PARAM_SHADER_INFO = 0,
	ACCESS_ROOT_PARAM_MATERIAL,
	ACCESS_ROOT_PARAM_DEPTH_STENCIL,
	ACCESS_ROOT_PARAM_DEPTH_STENCIL_SHADER_VIEW,
	ACCESS_ROOT_PARAM_RENDER_TARGET,
	ACCESS_ROOT_PARAM_RENDER_TARGET_SHADER_VIEW,
	ACCESS_ROOT_PARAM_GAUSSIAN_TABLE,
	ACCESS_ROOT_PARAM_TEXTURE,
	ACCESS_ROOT_PARAM_G_BUF_NORMAL,
	ACCESS_ROOT_PARAM_BLOOM,
	ACCESS_ROOT_PARAM_BLOOM_SHRINK,
	ACCESS_ROOT_PARAM_DOF,
	ACCESS_ROOT_PARAM_DOF_SHRINK,
	ACCESS_ROOT_PARAM_SSAO,

	ACCESS_ROOT_PARAM_NUM,
	ACCESS_ROOT_PARAM_INVALID = -1,
};
static bool IsValidHeapAccessRootParam(const ACCESS_ROOT_PARAM e) { return (0 <= e) && (e < ACCESS_ROOT_PARAM_NUM); };

// �F�w��.
enum GAME_COLOR {
	GAME_COLOR_WHITE = 0,
	GAME_COLOR_BLACK,
	GAME_COLOR_NUM,
	GAME_COLOR_INVALID = -1,
};
static bool IsValidGameColor(const GAME_COLOR e) { return (0 <= e) && (e < GAME_COLOR_NUM); };

// �����_�[�^�[�Q�b�g�p�̃C���^�[�t�F�[�X.
class IRenderTarget {
public:
	virtual ID3D12Resource* GetResource() = 0;
	virtual D3D12_RESOURCE_BARRIER& GetResourceBattier() = 0;
	virtual HEAP_CATEGORY GetRtvHeapCategory() = 0;
	virtual int GetRtvHeapPosition() = 0;
	virtual HEAP_CATEGORY GetSrvHeapCategory() = 0;
	virtual int GetSrvHeapPosition() = 0;
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() = 0;
};

// �f�v�X�X�e���V���p�̃C���^�[�t�F�[�X.
class IDepthStencil {
public:
	virtual ID3D12Resource* GetResource() = 0;
	virtual D3D12_RESOURCE_BARRIER& GetResourceBattier() = 0;
	virtual HEAP_CATEGORY GetDsvHeapCategory() = 0;
	virtual int GetDsvHeapPosition() = 0;
	virtual HEAP_CATEGORY GetSrvHeapCategory() = 0;
	virtual int GetSrvHeapPosition() = 0;
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() = 0;
};

// ���f���p�̃C���^�[�t�F�[�X.
class IVertex {
public:
	virtual const DirectX::XMFLOAT3& GetPosition() const = 0;
	virtual const DirectX::XMFLOAT3& GetNormal() const = 0;
	virtual const DirectX::XMFLOAT2& GetUV() const = 0;
	virtual const uint32_t GetBoneNumberListNum() const = 0;
	virtual const uint16_t* GetBoneNumberList() const = 0;
	virtual const uint8_t GetBoneWeight() const = 0;
	virtual const uint8_t GetEdgeOption() const = 0;
	virtual const uint32_t GetDataSize() const = 0;
};
class ITexture {
public:
	virtual const DirectX::TexMetadata& GetMetaData() const = 0;
	virtual const DirectX::ScratchImage& GetImage() const = 0;

	virtual const ID3D12Resource* GetCPUResource() const = 0;
	virtual const ID3D12Resource* GetGPUResource() const = 0;
	virtual const HEAP_CATEGORY GetHeapCategory() const = 0;
	virtual const int GetHeapPosition() const = 0;

	virtual const D3D12_TEXTURE_COPY_LOCATION& GetCopySrc() const = 0;
	virtual const D3D12_TEXTURE_COPY_LOCATION& GetCopyDst() const = 0;
	virtual const D3D12_RESOURCE_BARRIER& GetCopyResourceBarrier() const = 0;
	virtual const D3D12_RESOURCE_BARRIER& GetResetResourceBarrier() const = 0;

	virtual ACCESS_ROOT_PARAM GetAccessRootParam() = 0;
};
class IMaterial {
public:
	virtual const uint32_t GetIndicesNum() const = 0;

	virtual const DirectX::XMFLOAT3& GetDiffuse() const = 0;
	virtual const float GetDiffuseAlpha() const = 0;
	virtual const DirectX::XMFLOAT3& GetSpecular() const = 0;
	virtual const float GetSpecularity() const = 0;
	virtual const DirectX::XMFLOAT3& GetAmbient() const = 0;
	virtual const float IsValidToon() const = 0;

	virtual const ITexture* GetTexture() const = 0;
	virtual const ITexture* GetSphereTexture() const = 0;
	virtual const ITexture* GetSphereAddTexture() const = 0;
	virtual const ITexture* GetToonTexture() const = 0;
	virtual const uint8_t GetToonIdx() const = 0;
	virtual const uint8_t GetEdgeOption() const = 0;

	virtual const uint32_t GetShaderUseDataSize() const = 0;

	virtual const ID3D12Resource* GetLightResource() const = 0;
	virtual const int GetLightHeapPosition() const = 0;
	virtual const uint32_t GetTotalHeapUseSize() const = 0;
	virtual const uint32_t GetLightHeapUseSize() const = 0;
	virtual const uint32_t GetTexHeapUseSize() const = 0;

	virtual ACCESS_ROOT_PARAM GetAccessRootParam() const = 0;
};
class IBone {
public:
	virtual const DirectX::XMMATRIX& GetAffineMatrix() const = 0;
};

class CCommandWrapper;
class CHeapWrapper;
class ICamera;
class IModel {
public:
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) = 0;
	virtual void RenderShadow(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const ICamera& rCamera, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) = 0;
public:
	virtual const IVertex* GetVertexList() const = 0;
	virtual const IVertex* GetAtVertexList(const uint32_t idx) const = 0;
	virtual const uint32_t GetVertexListNum() const = 0;

	virtual const uint16_t* GetVertexIndexList() const = 0;
	virtual const uint16_t* GetAtVertexIndex(const uint32_t idx) const = 0;
	virtual const uint32_t GetVertexIndexListNum() const = 0;

	virtual const IMaterial* GetMaterialList() const = 0;
	virtual const IMaterial* GetAtMaterial(const uint32_t idx) const = 0;
	virtual const uint32_t GetMaterialListNum() const = 0;

	virtual const IBone* GetBoneList() const = 0;
	virtual const IBone* GetAtBone(const uint32_t idx) const = 0;
	virtual const uint32_t GetBoneListNum() const = 0;

	virtual const DirectX::XMMATRIX GetAffineMatrix() const = 0;

	virtual ID3D12Resource* GetVertexBufferResource() = 0;
	virtual const D3D12_VERTEX_BUFFER_VIEW GetVertexBufferViewInfo() const = 0;

	virtual ID3D12Resource* GetIndexBufferResource() = 0;
	virtual const D3D12_INDEX_BUFFER_VIEW GetIndexBufferViewInfo() const = 0;

	virtual ID3D12Resource* GetShaderInfoResource() = 0;
	virtual const int GetShaderInfoHeapPosition() const = 0;

	virtual ID3D12PipelineState* GetPipelineState() = 0;
	virtual ID3D12RootSignature* GetRootSignature() = 0;

	virtual ID3DBlob* GetVertexShader() = 0;
	virtual ID3DBlob* GetPixelShader() = 0;

	virtual ID3D12PipelineState* GetShadowPipelineState() = 0;
	virtual ID3D12RootSignature* GetShadowRootSignature() = 0;

	virtual void SetDepthStencil(IDepthStencil* pDS) = 0;
	virtual IDepthStencil* GetDepthStencil() = 0;

	virtual ACCESS_ROOT_PARAM GetAccessRootParam() = 0;
};

// �X�v���C�g�p�̃C���^�[�t�F�[�X.
class CGraphicsController;
class ISprite {
public:
	virtual void Update() = 0;
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) = 0;
};

// �J�����p�̃C���^�[�t�F�[�X.
class ICamera {
public:
	virtual const DirectX::XMMATRIX& GetPerspectiveMatrix() const = 0;
	virtual const DirectX::XMMATRIX& GetViewMatrix() const = 0;
	virtual const DirectX::XMFLOAT3& GetEye() const = 0;
	virtual const DirectX::XMFLOAT3& GetAt() const = 0;
	virtual const DirectX::XMFLOAT3& GetUp() const = 0;
};

// �V�F�[�_�[�p�̃C���^�[�t�F�[�X.
class IShader {
public:
	virtual void Update() = 0;
	virtual void RenderSetup(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper) = 0;
	virtual ID3D12PipelineState* GetPipelineState() = 0;
	virtual ID3D12RootSignature* GetRootSignature() = 0;
};

// �f�J�[���p.
class IDecal {
public:
	virtual void Update() = 0;
	virtual void Render(CCommandWrapper& commandWrapper, CHeapWrapper& heapWrapper, const D3D12_VIEWPORT* pViewPort, const D3D12_RECT* pScissor) = 0;
};

// ====================================================.
// MMD�p�̃f�[�^�󂯓n���p�\����.
// MMD�̓}�e���A����񂪃t�@�C���P�ʂɕ�����Ă��Ȃ��̂�....
// ====================================================.

struct SMMDMatarial {
	DirectX::XMFLOAT3 diffuse = {};
	float diffuse_alpha = 0.0f;
	float specularity = 0.0f;
	DirectX::XMFLOAT3 specular = {};
	DirectX::XMFLOAT3 ambient = {};
	uint8_t toonIdx = 0;
	uint8_t edgeOption = 0;
	uint32_t indicesNum = 0;
	char texPath[20] = {};
	void Serialize(FILE* fp)
	{
		if (!fp) { return; }
		fread(&diffuse, sizeof(diffuse), 1, fp);
		fread(&diffuse_alpha, sizeof(diffuse_alpha), 1, fp);
		fread(&specularity, sizeof(specularity), 1, fp);
		fread(&specular, sizeof(specular), 1, fp);
		fread(&ambient, sizeof(ambient), 1, fp);
		fread(&toonIdx, sizeof(toonIdx), 1, fp);
		fread(&edgeOption, sizeof(edgeOption), 1, fp);
		fread(&indicesNum, sizeof(indicesNum), 1, fp);
		fread(&texPath, sizeof(texPath), 1, fp);
	}
};

struct SMMDBone {
	char name[20] = {};
	uint16_t parentNo = 0;
	uint16_t nextNo = 0;
	uint8_t type = 0;
	uint16_t ikBoneNo = 0;
	DirectX::XMFLOAT3 pos = {};
	void Serialize(FILE* fp)
	{
		if (!fp) { return; }
		fread(&name, sizeof(name), 1, fp);
		fread(&parentNo, sizeof(parentNo), 1, fp);
		fread(&nextNo, sizeof(nextNo), 1, fp);
		fread(&type, sizeof(type), 1, fp);
		fread(&ikBoneNo, sizeof(ikBoneNo), 1, fp);
		fread(&pos, sizeof(pos), 1, fp);
	}
};

struct SMMDVertex {
	DirectX::XMFLOAT3 position = {};
	DirectX::XMFLOAT3 normal = {};
	DirectX::XMFLOAT2 uv = {};
	uint16_t boneNo[2] = {};
	uint8_t boneWeight = 0;
	uint8_t edgeOption = 0;
	void Serialize(FILE* fp)
	{
		if (!fp) { return; }
		fread(&position, sizeof(position), 1, fp);
		fread(&normal, sizeof(normal), 1, fp);
		fread(&uv, sizeof(uv), 1, fp);
		fread(&boneNo, sizeof(boneNo), 1, fp);
		fread(&boneWeight, sizeof(boneWeight), 1, fp);
		fread(&edgeOption, sizeof(edgeOption), 1, fp);
	}
};

// ====================================================.
// �V�F�[�_�[���ɓn���\����.
// �V�F�[�_�[���ɂ������̒�`���K�v.
// ====================================================.

struct SShaderCoordinateInfo {
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX proj = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX shadow = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX lightView = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX bone[256] = { DirectX::XMMatrixIdentity() };
	DirectX::XMFLOAT3 eye = {};
};

struct SShaderDecalInfo {
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX proj = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX lightView = DirectX::XMMatrixIdentity();
};

struct SShaderSpriteInfo {
	DirectX::XMMATRIX toScreen = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX proj = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX projInv = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT3 eye = {};
};

struct SShaderGaussianInfo {
	float weight[8] = {};
	float isEnableX = 0.0f;
	float isEnableY = 0.0f;
};
