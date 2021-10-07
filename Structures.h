#pragma once
#include <windows.h>
#include <d3d12.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include "Vector3D.h"
#include "Vector2.h"
#include "Transform.h"
#include <random>
#include <vector>
#include <string>

using namespace DirectX;
struct Vertex
{
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	Vector3D pos;
	Vector2 texCoord;
};

enum class ObjectType
{
	GameObj,
	PhysicalObj,
	ImaginaryObj
};

struct CreateObjectStruct
{
	CreateObjectStruct(ID3D12Device* device, ID3D12GraphicsCommandList* cl, std::string oN , ObjectType type, std::vector<Vertex> verts, std::vector<DWORD> indies, int objNum, int alignment, Transform* trans) : dev(device), commandList(cl), objName(oN),objType(type), vertices(verts), indices(indies), bufferAlignment((objNum - 1) * alignment), transform(trans) {}
	ID3D12Device* dev = nullptr;
	ID3D12GraphicsCommandList* commandList;
	std::string objName;
	ObjectType objType;
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	int bufferAlignment;
	Transform* transform = nullptr;
};

struct DrawObjectsStruct
{
	DrawObjectsStruct(ID3D12GraphicsCommandList* cL, D3D12_VIEWPORT* vp, D3D12_RECT* rect, ID3D12Resource* cBUH, int CBO, int index) : commandList(cL), viewport(vp), sisRect(rect), constantBufferUploadHeaps(cBUH), frameIndex(index){ }
	ID3D12GraphicsCommandList* commandList;
	D3D12_VIEWPORT* viewport;
	D3D12_RECT* sisRect;
	ID3D12Resource* constantBufferUploadHeaps;
	int frameIndex;
};

struct Geometry
{
	ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data
	ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data
	int numberOfIndices;
	XMFLOAT3 Point;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
										   // the total size of the buffer, and the size of each element (vertex)
	D3D12_INDEX_BUFFER_VIEW indexBufferView; // a structure holding information about the index buffer
};

struct Material
{
	XMFLOAT4 diffuse;
	XMFLOAT4 ambient;
	XMFLOAT4 specular;
	float specularPower;
};

struct Colour
{
	Colour()
	{
		R = 0;
		G = 0;
		B = 0;
	}
	Colour(float r, float g, float b)
	{
		R = r;
		G = g;
		B = b;
	}
	Colour(int r, int g, int b)
	{
		R = r;
		G = g;
		B = b;
	}

	float R, G, B;
};

//Container For the MouseInput class. Makes it esier to move data
enum MouseButtonEvent 
{
	X_BUTTON1_CLICKED,
	X_BUTTON2_CLICKED,
	NONE_CLICKED
};

//Container For the MouseInput class. Makes it esier to move data
struct MouseData
{
	MouseData()
	{
		xPos = 0;
		yPos = 0;
		PrevXPos = 0;
		PrevYPos = 0;
		m_MouseEvent = NONE_CLICKED;
		Error = false;
	}
	~MouseData()
	{
		xPos = NULL;
		yPos = NULL;
		int PrevXPos = NULL;
		int PrevYPos = NULL;
	}
	int xPos;
	int yPos;
	int PrevXPos;
	int PrevYPos;
	MouseButtonEvent m_MouseEvent;
	bool Error;
};

//Container For the Camera class. Makes it esier to move data
struct CameraData
{
	XMFLOAT4 m_eye;
	XMFLOAT4 m_at;
	XMFLOAT4 m_up;

};


//Container For the Camera class. Makes it esier to move data
struct CameraBufferData
{
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_projection;
};

//Used to pass Blend data in one convient package
struct BlendFactor
{
	float R;
	float G;
	float B;
	float A;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;
	float gTime;
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;

};

enum Axis
{
	X,
	Y,
	Z
};

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

//Struct for storing object meshes
struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct ObjectStruct
{
	XMFLOAT3 Pos;
	MeshData MD;
	ID3D11ShaderResourceView* Tex;
};
