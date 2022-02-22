#pragma once
#include "Vector3D.h"
#include "Loader.h"
#include "TerrainTex.h"
#include "NormalCalculations.h"
#include "Structures.h"
#include "GraphicsManager.h"
class Terrain 
{
public:
	Terrain();
	Terrain(int x, int z, std::string MapLocale, float scale, GraphicsManager* gm, std::string TexLocale);
	~Terrain();



private:


	Loader m_Loader;
	Geometry* m_Geometry;
	std::vector<TerrainTex*> m_TextureStore;
	std::vector<Vertex> m_VertexStore;
	std::vector<float> m_HeightMap;
	std::vector<DWORD> m_IndexStore;
	float m_HeightMapScale;
};

