#pragma once

#include "../graphics/GraphicsDefs.h"

class CGraphicsController;
class CDebugManager {
public:
	CDebugManager() = default;
	~CDebugManager() = default;

public:
	bool Init(CGraphicsController& rGraphicsController, HWND hWnd);
	void Term();

public:
	void Begin();
	void UpdateGUI();
	void End();

	void Render(CGraphicsController& rGraphicsController);

private:

};