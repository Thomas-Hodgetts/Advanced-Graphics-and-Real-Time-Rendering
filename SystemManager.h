#pragma once
#include "Flotilla.h"
#include "Camera.h"
#include "Structures.h"
#include "GameObject.h"

class SystemManager
{
public:
	SystemManager();
	~SystemManager();

	//Create Functions
	HRESULT BuildObject(CreateObjectStruct COS);


	void SetUpCamera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	Camera* ReturnCamera() { return m_Camera; };
	Object* GetStoredObject(int Pos) { return m_Objects.ReturnObject(Pos); }

	
	void Update();
	void Draw(DrawObjectsStruct DOS);


private:
	
	Camera* m_Camera;
	Flotilla m_Objects;
	Flotilla m_DrawQueue;
};

