#pragma once
#include "../../graphics/GraphicsDefs.h"

class Player;
class CScene;
class CCamera : public ICamera {
public:
	CCamera();
	~CCamera() = default;

public:
	bool Init(uint32_t width, uint32_t height);
	void Update(const DirectX::XMFLOAT3* pFocusPosition = nullptr);

public:
	virtual const DirectX::XMMATRIX& GetPerspectiveMatrix() const override { return m_perspectiveMat; }
	virtual const DirectX::XMMATRIX& GetViewMatrix() const override { return m_view; }
	virtual const DirectX::XMFLOAT3& GetEye() const override { return m_eye; }
	virtual const DirectX::XMFLOAT3& GetAt() const override { return m_at; }
	virtual const DirectX::XMFLOAT3& GetUp() const override { return m_up; }

#if defined(DEBUG)
public:
	void OnDebugInputEye(DirectX::XMFLOAT3& e) { m_eye = e; }
	void OnDebugInputAt(DirectX::XMFLOAT3& e) { m_at = e; }
#endif

private:
	DirectX::XMMATRIX m_perspectiveMat = {};
	DirectX::XMMATRIX m_view = {};
	DirectX::XMFLOAT3 m_eye = {};
	DirectX::XMFLOAT3 m_at = {};
	DirectX::XMFLOAT3 m_up = {};
	float m_dist = 0.0f;
};