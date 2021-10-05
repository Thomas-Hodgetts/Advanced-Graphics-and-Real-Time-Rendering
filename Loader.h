#pragma once
#include "Structures.h"
#include "OBJLoader.h"
#include "DDSTextureLoader.h"
#include <fstream>
#include <string>
#include <filesystem>


class Loader
{
public:
	Loader();
	~Loader();

	MeshData LoadMesh(char* filename, ID3D11Device* _pd3dDevice, bool invertTexCoords);
	ID3D11ShaderResourceView* LoadTex(ID3D11Device* d3dDevice, const wchar_t* fileName);
	ObjectStruct LoadFromFile(std::string FileLocation, ID3D11Device* d3dDevice);

};

