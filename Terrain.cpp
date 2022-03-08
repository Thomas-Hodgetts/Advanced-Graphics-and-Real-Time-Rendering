#include "Terrain.h"


Terrain::Terrain()
{
}

Terrain::~Terrain()
{
	delete m_Anchor;
}

XMFLOAT4X4 Terrain::ReturnAnchor()
{
	XMFLOAT4X4 result;

	XMMATRIX scale = XMMatrixScaling(m_Anchor->ReturnSca().ReturnX(), m_Anchor->ReturnSca().ReturnY(), m_Anchor->ReturnSca().ReturnZ());
	XMMATRIX rotation = XMMatrixRotationX(m_Anchor->ReturnRot().ReturnX()) * XMMatrixRotationY(m_Anchor->ReturnRot().ReturnY()) * XMMatrixRotationZ(m_Anchor->ReturnRot().ReturnZ());
	XMMATRIX translation = XMMatrixTranslation(m_Anchor->ReturnPos().ReturnX(), m_Anchor->ReturnPos().ReturnY(), m_Anchor->ReturnPos().ReturnZ());

	XMStoreFloat4x4(&result, scale * rotation * translation);

	return result;
}

Terrain::Terrain(int x, int z, std::string MapLocale, float scale, std::wstring name)
{
	m_Name = name;
	m_HeightMapScale = scale;

	std::vector<unsigned char> in(x * z);
	std::vector<float> out(z * x, 0);

	std::ifstream inFile;
	inFile.open(MapLocale.c_str(), std::ios_base::binary);

	if (inFile)
	{
		inFile.read((char*)&in[0], (std::streamsize)in.size());
		inFile.close();
	}

	for (UINT i = 0; i < x * z; ++i)
	{
		out[i] = (in[i] / 255.0f) * m_HeightMapScale;
	}


	UINT vertexCount = x * z;
	UINT faceCount = (x - 1) * (z - 1) * 2;
	float halfWidth = 0.5 * x;
	float halfHeight = 0.5 * z;

	float dx = x / (x - 1);
	float dz = z / (z - 1);
	float du = 1.0f / (x - 1);
	float dv = 1.0f / (z - 1);

	m_VertexStore.resize(vertexCount);
	for (UINT i = 0; i < z; i++)
	{
		float nZ = -halfHeight + i * dz;
		for (UINT j = 0; j < x; j++)
		{
			float nX = -halfWidth + j * dx;
			Vertex s;
			s.texCoord = { float(i) / float(x), float(j) / float(z) };
			s.pos.SetX(nX);
			s.pos.SetY(out[i + j]);
			s.pos.SetZ(nZ);
			m_VertexStore[i * x + j] = s;
		}
	}

	m_IndexStore.resize(faceCount * 3);
	UINT k = 0;
	for (UINT i = 0; i < z - 1; i++)
	{
		for (UINT j = 0; j < x - 1; j++)
		{
			m_IndexStore[k] = i * x + j;
			m_IndexStore[k + 1] = i * x + j + 1;
			m_IndexStore[k + 2] = (i + 1) * x + j;
			m_IndexStore[k + 3] = (i + 1) * x + j;
			m_IndexStore[k + 4] = i * x + j + 1;
			m_IndexStore[k + 5] = (i + 1) * x + j + 1;
			k += 6;
		}
	}


	for (UINT i = 0; i < faceCount; i++)
	{
		UINT i0 = m_IndexStore[i * 3];
		UINT i1 = m_IndexStore[i * 3 + 1];
		UINT i2 = m_IndexStore[i * 3 + 2];
		m_VertexStore[i0].Normal = { 0,1,0 };
		m_VertexStore[i1].Normal = { 0,1,0 };
		m_VertexStore[i2].Normal = { 0,1,0 };
	}


	//LPCWSTR* TexLocale, int texCount,

	m_Geometry = new Geometry();
}

