#include "Terrain.h"


Terrain::Terrain()
{
}

Terrain::~Terrain()
{
	delete m_Anchor;
}

void Terrain::RandomInit(std::wstring name, float scale, RANDOM_MODE mode)
{
	m_TerrainData - new std::map<int, int>(m_Size, m_Size);
	m_TerrainData[0][0] = rnd();
	m_TerrainData[0][m_Size - 1] = rnd();
	m_TerrainData[m_Size - 1][0] = rnd();
	m_TerrainData[m_Size - 1][m_Size - 1] = rnd();

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


void Terrain::Diamond(int sideLength, int xLength, int zLength, float scale)
{
	int halfSide = sideLength / 2;

	for (int y = 0; y < m_Size / (sideLength - 1); y++)
	{
		for (int x = 0; x < m_Size / (sideLength - 1); x++)
		{
			int center_x = x * (sideLength - 1) + halfSide;
			int center_y = y * (sideLength - 1) + halfSide;

			int avg = (m_TerrainData[x * (sideLength - 1)][y * (sideLength - 1)] +
				m_TerrainData[x * (sideLength - 1)][(y + 1) * (sideLength - 1)] +
				m_TerrainData[(x + 1) * (sideLength - 1)][y * (sideLength - 1)] +
				m_TerrainData[(x + 1) * (sideLength - 1)][(y + 1) * (sideLength - 1)]) / 4.0f;

			m_TerrainData[center_x][center_y] = avg + rnd(-m_Range, m_Range);
		}
	}

}

void Terrain::Average(int x, int y, int sideLength, float scale)
{
	float counter = 0;
	float accumulator = 0;

	int halfSide = sideLength / 2;

	if (x != 0)
	{
		counter += 1.0f;
		accumulator += m_TerrainData[y][x - halfSide];
	}
	if (y != 0)
	{
		counter += 1.0f;
		accumulator += m_TerrainData[y - halfSide][x];
	}
	if (x != m_Size - 1)
	{
		counter += 1.0f;
		accumulator += m_TerrainData[y][x + halfSide];
	}
	if (y != m_Size - 1)
	{
		counter += 1.0f;
		accumulator += m_TerrainData[y + halfSide][x];
	}

	m_TerrainData[y][x] = (accumulator / counter) + rnd(-scale, scale);

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
		XMFLOAT3 tangent, binormal, normal;
		NormalCalculations::CalculateTangentBinormal2(m_VertexStore[i0], m_VertexStore[i1], m_VertexStore[i2], normal, tangent, binormal);

		m_VertexStore[i0].Normal = normal;
		m_VertexStore[i1].Normal = normal;
		m_VertexStore[i2].Normal = normal;
		m_VertexStore[i0].tangent = tangent;
		m_VertexStore[i1].tangent = tangent;
		m_VertexStore[i2].tangent = tangent;
		m_VertexStore[i0].biTangent = binormal;
		m_VertexStore[i1].biTangent = binormal;
		m_VertexStore[i2].biTangent = binormal;
	}
	

	m_Geometry = new Geometry();
}

