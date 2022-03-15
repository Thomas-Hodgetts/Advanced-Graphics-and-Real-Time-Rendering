#pragma once
#include "Flotilla.h"
#include "Camera.h"
#include "Structures.h"
#include "GameObject.h"
#include "Terrain.h"

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
	Flotilla GetObjectVector() { return m_Objects; };
	void CreateTerrain(int x, int z, std::string MapLocale, float scale, std::wstring name) { m_Terrain = new Terrain(x,z, MapLocale, scale, name); }
	void CreateTerrain(float scale, std::wstring name) { m_Terrain = new Terrain(); m_Terrain->RandomInit(name, scale, RANDOM_MODE::RANDOM_MODE_DIAMOND); };
	Terrain* GetTerrain() { return m_Terrain; };
	
	void Update();
	void Draw(DrawObjectsStruct DOS);


private:
	
	void PreparePipeline(DrawObjectsStruct DOS);
	
	Camera* m_Camera;
	Flotilla m_Objects;
	Terrain* m_Terrain;
};

