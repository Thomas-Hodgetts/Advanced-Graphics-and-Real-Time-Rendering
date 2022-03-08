#pragma once
#include <vector>
#include "Structures.h"
#include "Transform.h"
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
	std::wstring GetIdentifier() { return m_Name; };
	void CreateAnchor(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 sca) { m_Anchor = new Transform(pos, rot, sca); }
	void CreateAnchor(Vector3D pos, Vector3D rot, Vector3D sca) { m_Anchor = new Transform(pos, rot, sca); }

	XMFLOAT4X4 ReturnAnchor();

	void ClearCache() { m_VertexStore.clear(); m_IndexStore.clear(); m_HeightMap.clear(); };

private:

	Geometry* m_Geometry;
	std::wstring m_Name;
	std::vector<Vertex> m_VertexStore;
	std::vector<float> m_HeightMap;
	std::vector<DWORD> m_IndexStore;
	float m_HeightMapScale;
	Transform* m_Anchor;

};

