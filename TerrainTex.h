#pragma once
#include <d3d11.h>
class TerrainTex
{
	TerrainTex(int AH, ID3D11ShaderResourceView* tex);
	~TerrainTex();


private:

	ID3D11ShaderResourceView* m_Texture;
	int m_HieghtOfActivation;
};