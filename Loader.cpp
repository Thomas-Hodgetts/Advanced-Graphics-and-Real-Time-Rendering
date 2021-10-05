#include "Loader.h"

Loader::Loader()
{

}
Loader::~Loader()
{

}


ObjectStruct Loader::LoadFromFile(std::string FileLocation, ID3D11Device* d3dDevice)
{
	FileLocation = std::filesystem::current_path().string() + FileLocation;
	ObjectStruct ObjectFromFile;
	std::string Deliminator = "*";
	std::ifstream inFile;
	inFile.open(FileLocation, std::ios::in);
	if (inFile)
	{
		std::string FileDataIn;
		inFile >> FileDataIn;
		int pos = FileDataIn.find(Deliminator);
		ObjectFromFile.Pos.x = std::stoi(FileDataIn);
		FileDataIn.erase(0, pos+1);

		pos = FileDataIn.find(Deliminator);
		ObjectFromFile.Pos.y = std::stoi(FileDataIn);
		FileDataIn.erase(0, pos+1);

		pos = FileDataIn.find(Deliminator);
		ObjectFromFile.Pos.z = std::stoi(FileDataIn);
		FileDataIn.erase(0, pos+1);

		pos = FileDataIn.find(Deliminator);
		std::string FileLoc = FileDataIn.substr(0, pos);
		std::wstring* wstring = new std::wstring(FileLoc.begin(), FileLoc.end());
		//ObjectFromFile.Tex = LoadTex(d3dDevice, (L"" + *wstring + L"").c_str());
		FileDataIn.erase(0, pos+1);

		pos = FileDataIn.find(Deliminator);
		FileLoc = FileDataIn.substr(0, pos);
		char* File = const_cast<char*>(FileLoc.c_str());
		ObjectFromFile.MD = LoadMesh(File, d3dDevice, false);


		delete wstring;
		return ObjectFromFile;

	}
	else 
	{
	}
}

ID3D11ShaderResourceView* Loader::LoadTex(ID3D11Device* d3dDevice, const wchar_t* fileName)
{
	ID3D11ShaderResourceView* texture;
	HRESULT hr;
	hr = CreateDDSTextureFromFile(d3dDevice, fileName, nullptr, &texture);
	if (FAILED(hr))
		return nullptr;
	else
	{
		return texture;
	}
}

MeshData Loader::LoadMesh(char* filename, ID3D11Device* _pd3dDevice, bool From3dsMax)
{
	if (From3dsMax)
	{
		return OBJLoader::Load(filename, _pd3dDevice);
	}
	else if (!From3dsMax)
	{
		return OBJLoader::Load(filename, _pd3dDevice, false);
	}
}