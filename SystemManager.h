#pragma once
#include "Flotilla.h"
#include "Camera.h"
#include "Debug.h"

class SystemManager
{
public:
	SystemManager();
	~SystemManager();

	void SetUpCamera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	Camera* ReturnCamera() { return m_Camera; };


private:
	
	Camera* m_Camera;
	Flotilla m_Objects;
	Flotilla m_DrawQueue;
};

