#pragma once
#include "Object.h"
#include "Transform.h"

class PhysicalObject : public Object
{
public:

	PhysicalObject();
	~PhysicalObject();


	//Returns the physical objects 4x4 matrix
	XMFLOAT4X4 ReturnWorld();
	//Returns the physical objects mesh data
	MeshData ReturnMeshData();
	//This Object handles the updating of the object in the frame;
	void Update();
	//This Function Handles the Drawing of the object in the frame
	void Draw(ID3D11DeviceContext* IC, ID3D11Buffer* _pCB, ConstantBuffer cb);

protected:

	//Creates a Physical Object and assigns the basic values needed
	bool CreateObject(MeshData MD, ID3D11ShaderResourceView* TD, XMFLOAT4X4 WD);
	//Controls the objects Position in the world
	Transform* m_Transform = nullptr;

private:

	// the number of indices to draw the object
	int m_NumIndices; 
	//Mesh Data for the object
	MeshData m_Mesh;

	ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data
	ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView; // a structure containing a pointer to the vertex data in gpu memory										 // the total size of the buffer, and the size of each element (vertex)
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView; // a structure holding information about the index buffer
};

