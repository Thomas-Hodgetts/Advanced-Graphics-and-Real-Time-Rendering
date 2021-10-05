#pragma once
#include "Object.h"
class PhysicalObject : public Object
{
public:

	PhysicalObject();
	~PhysicalObject();


	//Returns the physical objects 4x4 matrix
	XMFLOAT4X4 ReturnWorld();
	//Returns the physical objects mesh data
	MeshData ReturnMeshData();
	//Returns the physical objects ID3D11ShaderResourceView*
	ID3D11ShaderResourceView* ReturnTextureData();
	//This Object handles the updating of the object in the frame;
	void Update();
	//This Function Handles the Drawing of the object in the frame
	void Draw(ID3D11DeviceContext* IC, ID3D11Buffer* _pCB, ConstantBuffer cb);
	//Updates m_ObjectTransform position
	void UpdateWorld(XMFLOAT4X4 NewWorld);
	//Sets the blend state of the object
	void SetTransparency(ID3D11BlendState* BS, BlendFactor* BF) { m_Transparency = BS; m_BlendFactor[0] = BF->R; m_BlendFactor[1] = BF->G; m_BlendFactor[2] = BF->B; m_BlendFactor[3] = BF->A; };
	//Sets The objects pixel Shader
	void SetPixelShader(ID3D11PixelShader* PS) { m_PixelShader = PS; };
	//Sets the objects vertex shader
	void SetVertexShader(ID3D11VertexShader* VS) { m_VertexShader = VS; };


protected:

	//Creates a Physical Object and assigns the basic values needed
	bool CreateObject(MeshData MD, ID3D11ShaderResourceView* TD, XMFLOAT4X4 WD);
	//TBD
	void ReturnObjectType();
	//Sets m_ObjectTransfrom to the identity matrix
	void InitWorld();
	//Controls the objects Position in the world
	ObjectController c_ObjectController;

private:

	//Mesh Data for the object
	MeshData m_Mesh;
	//Texture Resource for the object
	ID3D11ShaderResourceView* m_Texture = nullptr;
	//Location of the object in the world.
	XMFLOAT4X4	m_ObjectTransform;
	//Transparency of the object (Achieved using blend state)
	ID3D11BlendState* m_Transparency = nullptr;
	//Blend factor 
	float m_BlendFactor[4];
	//For Later Implementation
	ID3D11VertexShader* m_VertexShader;
	//For Later Implementation
	ID3D11PixelShader* m_PixelShader;

};

