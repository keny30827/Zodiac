#pragma once

#include "../../graphics/GraphicsDefs.h"

class CGraphicsController;
class CRenderTarget : public IRenderTarget {
public:
	CRenderTarget() {}
	~CRenderTarget() {}

public:
	void Init(
		CGraphicsController& rGraphicsController,
		uint32_t width,
		uint32_t height,
		ACCESS_ROOT_PARAM rootParam = ACCESS_ROOT_PARAM_RENDER_TARGET_SHADER_VIEW);
	void Term();

public:
	virtual ID3D12Resource* GetResource() override { return m_pResource; }
	virtual D3D12_RESOURCE_BARRIER& GetResourceBattier() override { return m_resourceBarrier; }
	virtual HEAP_CATEGORY GetRtvHeapCategory() override { return m_rtvHeapCategory; }
	virtual int GetRtvHeapPosition() override { return m_rtvHeapPosition; }
	virtual HEAP_CATEGORY GetSrvHeapCategory() override { return m_srvHeapCategory; }
	virtual int GetSrvHeapPosition() override { return m_srvHeapPosition; }
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() override { return m_accessRootParam; }

public:
	void SetAccessRootParam(ACCESS_ROOT_PARAM e) { m_accessRootParam = e; }

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	HEAP_CATEGORY m_rtvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_INVALID;
	int m_rtvHeapPosition = -1;
	HEAP_CATEGORY m_srvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_INVALID;
	int m_srvHeapPosition = -1;
	ID3D12Resource* m_pResource = nullptr;
	D3D12_RESOURCE_BARRIER m_resourceBarrier = {};
	ACCESS_ROOT_PARAM m_accessRootParam = ACCESS_ROOT_PARAM::ACCESS_ROOT_PARAM_INVALID;
};