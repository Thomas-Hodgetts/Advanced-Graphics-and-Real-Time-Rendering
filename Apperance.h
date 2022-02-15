#pragma once
#include "Structures.h"

using namespace DirectX;
using namespace std;

//Controls the apperance of a game object
class Apperance
{
public:
	Apperance(Geometry geometry, Material material, ID3D11ShaderResourceView* tex);
	~Apperance();

	void Draw(DrawObjectsStruct DOS);

	//Set

	//Set the Geometry variable of the game object
	void SetGeo(Geometry geo) { m_Geometry = geo; };
	//Set the Material variable of the game object
	void SetMat(Material mat) { m_Material = mat; };
	//Set the Texture variable of the game object
	void SetTex(ID3D11ShaderResourceView* tex) { m_TextureRV = tex; };
	//Sets subresources for the next draw
	void SetSubResources();


	//Return

	//Returns the geometry of the object 
	Geometry ReturnGeo() { return m_Geometry; };
	//Returns the Material of the object 
	Material ReturnMat() { return m_Material; };
	//Returns the ID3D11ShaderResourceView of the object 
	ID3D11ShaderResourceView* ReturnTex() { return m_TextureRV; };

private:

	Geometry m_Geometry;
	Material m_Material;
	ID3D11ShaderResourceView* m_TextureRV;



};

