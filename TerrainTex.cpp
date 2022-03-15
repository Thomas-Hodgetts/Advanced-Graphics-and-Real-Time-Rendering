#include "TerrainTex.h"

TerrainTex::TerrainTex(int AH, ID3D11ShaderResourceView* tex)
{
	m_HieghtOfActivation = AH;
	m_Texture = tex;
}

TerrainTex::~TerrainTex()
{
	delete m_Texture;
}
