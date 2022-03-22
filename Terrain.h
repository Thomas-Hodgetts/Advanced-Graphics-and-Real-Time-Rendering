#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <iomanip> 
#include "Structures.h"
#include "Transform.h"
#include "NormalCalculations.h"
#include "NumberGenerator.h"
#include <fstream>

enum class RANDOM_MODE
{
	RANDOM_MODE_DIAMOND = 0,
	RANDOM_MODE_SQUARE,

};

class Terrain 
{
public:
	Terrain();
	Terrain(int x, int z, std::string MapLocale, float scale, std::wstring name);
	~Terrain();

	void RandomInit(std::wstring name, float scale, RANDOM_MODE mode);

	std::vector<Vertex> GetVertexStorage() { return m_VertexStore;};
	std::vector<DWORD> GetIndexStorage() { return m_IndexStore;};
	Geometry* GetGeometry() { return m_Geometry;};
	std::wstring GetIdentifier() { return m_Name; };
	void CreateAnchor(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 sca) { m_Anchor = new Transform(pos, rot, sca); }
	void CreateAnchor(Vector3D pos, Vector3D rot, Vector3D sca) { m_Anchor = new Transform(pos, rot, sca); }

	XMFLOAT4X4 ReturnAnchor();

	void Fractal();

	void Clamp(float* val, float min, float max);

	void Clamp_map();

	void print_map();

	void ClearCache() { m_VertexStore.clear(); m_IndexStore.clear(); m_HeightMap.clear(); };

private:

	void Diamond(int sideLength);

	void Square(int sideLength);

	void Average(int x, int y, int sideLength);

	int rnd(int min = 0, int max = 255)
	{
		//return rnd2(min, max);
		return min + ( rand() % static_cast<int>(max - min + 1));
	}
	int rnd2(int min = 0, int max = 255)
	{
		int result = NumberGenerator::GenerateRandomInteger(min, max);
		return min + (result % static_cast<int>(max - min + 1));
	}

	Geometry* m_Geometry;
	std::wstring m_Name;
	std::vector<Vertex> m_VertexStore;
	std::vector<float> m_HeightMap;
	std::vector<DWORD> m_IndexStore;
	float m_HeightMapScale;
	Transform* m_Anchor;
	float** m_TerrainData;

	const int m_Size = 513;
	int m_Range = 196;

};

