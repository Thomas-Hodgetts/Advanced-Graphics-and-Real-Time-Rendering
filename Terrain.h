#pragma once
#include <vector>
#include "Structures.h"
#include <fstream>


class Terrain 
{
public:
	Terrain();
	Terrain(int x, int z, std::string MapLocale, float scale, std::wstring name);
	~Terrain();

	std::vector<Vertex> GetVertexStorage() { return m_VertexStore;};
	std::vector<DWORD> GetIndexStorage() { return m_IndexStore;};
	Geometry* GetGeometry() { return m_Geometry;};

private:


	Geometry* m_Geometry;
	std::wstring m_Name;
	std::vector<Vertex> m_VertexStore;
	std::vector<float> m_HeightMap;
	std::vector<DWORD> m_IndexStore;
	float m_HeightMapScale;
};

