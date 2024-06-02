#pragma once

#include "../../graphics/GraphicsDefs.h"

class CGraphicsController;
class CShadowMap : public IDepthStencil {
public:
	CShadowMap() {}
	~CShadowMap() {}

public:
	void Init(CGraphicsController& rGraphicsController, uint32_t width, uint32_t height);
	void Term();

public:
	virtual ID3D12Resource* GetResource() override { return m_pResource; }
	virtual D3D12_RESOURCE_BARRIER& GetResourceBattier() override { return m_resourceBarrier; }
	virtual HEAP_CATEGORY GetDsvHeapCategory() override { return m_dsvHeapCategory; }
	virtual int GetDsvHeapPosition() override { return m_dsvHeapPosition; }
	virtual HEAP_CATEGORY GetSrvHeapCategory() override { return m_srvHeapCategory; }
	virtual int GetSrvHeapPosition() override { return m_srvHeapPosition; }
	virtual ACCESS_ROOT_PARAM GetAccessRootParam() override { return ACCESS_ROOT_PARAM_DEPTH_STENCIL_SHADER_VIEW; }

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	HEAP_CATEGORY m_dsvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_INVALID;
	int m_dsvHeapPosition = -1;
	HEAP_CATEGORY m_srvHeapCategory = HEAP_CATEGORY::HEAP_CATEGORY_INVALID;
	int m_srvHeapPosition = -1;
	ID3D12Resource* m_pResource = nullptr;
	D3D12_RESOURCE_BARRIER m_resourceBarrier = {};
};