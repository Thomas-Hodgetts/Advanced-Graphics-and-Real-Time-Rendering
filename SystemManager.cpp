#include "SystemManager.h"

SystemManager::SystemManager()
{
	m_Camera = nullptr;
}

SystemManager::~SystemManager()
{
	if (m_Camera == nullptr)
	{
		delete m_Camera;
	}
}

void SystemManager::SetUpCamera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_Camera = new Camera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth);
}
