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
	Vertex() {};
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	Vector3D pos;
	Vector2 texCoord;
	Vector3D Normal;
	Vector3D tangent;
	Vector3D biTangent;

	bool operator<(const Vertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(Vertex)) > 0;
	};
};

struct WindowData
{
	int m_Height = 0;
	int m_Width = 0;
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
	DrawObjectsStruct() {};
	DrawObjectsStruct(ID3D12GraphicsCommandList* cL, ID3D12CommandAllocator* cA, ID3D12Resource* rT, ID3D12Resource* dSB, ID3D12DescriptorHeap* rTRH, ID3D12DescriptorHeap* dSRH, ID3D12DescriptorHeap* mDH, ID3D12PipelineState* pSO ,D3D12_VIEWPORT* vp, D3D12_RECT* rect, ID3D12Resource* cBUH, int CBO, ID3D12RootSignature* r, int index) : commandList(cL), commandAllocator(cA), renderTargets(rT), depthStencilBuffer(dSB),renderTargetResourceHeap(rTRH), depthAndStencilResourceHeap(dSRH), mainDescriptorHeap(mDH), pipelineStateObject(pSO),viewport(vp), sisRect(rect), constantBufferUploadHeaps(cBUH), root(r),frameIndex(index){ }
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12Resource* renderTargets = nullptr;
	ID3D12Resource* depthStencilBuffer = nullptr;
	ID3D12DescriptorHeap* renderTargetResourceHeap = nullptr;
	ID3D12DescriptorHeap* depthAndStencilResourceHeap = nullptr;
	ID3D12DescriptorHeap* mainDescriptorHeap = nullptr;
	ID3D12PipelineState* pipelineStateObject = nullptr;
	D3D12_VIEWPORT* viewport = nullptr;
	D3D12_RECT* sisRect = nullptr;
	ID3D12Resource* constantBufferUploadHeaps = nullptr;
	ID3D12RootSignature* root = nullptr;
	int frameIndex = 0;
	int renderTargetDescriptorSize = 0;
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

//48 bytes
struct Material
{
	//------ 16 bytes
	XMFLOAT4 AmbientMtrl;
	//------ 16 bytes
	XMFLOAT4 DiffuseMtrl;
	//------ 16 bytes
	XMFLOAT4 SpecularMtrl;
};

//64 bytes
struct Light
{
	//------ 16 bytes
	XMFLOAT4 AmbientLight;
	//------ 16 bytes
	XMFLOAT4 DiffuseLight;
	//------ 16 bytes
	XMFLOAT4 SpecularLight;
	//------ 16 bytes
	float SpecularPower;
	XMFLOAT3 LightVecW;
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

	//------ 16 bytes
	XMFLOAT4 DiffuseMtrl;
	//------ 16 bytes
	XMFLOAT4 DiffuseLight;
	//------ 16 bytes
	XMFLOAT3 LightVecW;
	float gTime;
	//------ 16 bytes
	XMFLOAT4 AmbientMtrl;
	//------ 16 bytes
	XMFLOAT4 AmbientLight;
	//------ 16 bytes
	XMFLOAT4 SpecularMtrl;
	//------ 16 bytes
	XMFLOAT4 SpecularLight;
	//------ 16 bytes
	float SpecularPower;
	XMFLOAT3 EyePosW;
};

enum Axis
{
	X,
	Y,
	Z
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

// this is the structure of our constant buffer.
struct ConstantBufferPerObject
{
	XMFLOAT4X4 wvpMat;
	XMFLOAT4X4 worldPos;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 shadowTransform;
	Material Mat;
	Light point;
	XMFLOAT3 EyePosW;
	int mode = 0;
};

struct ConstantBufferLighting
{
	Material Mat;
	Light point;
	XMFLOAT3 EyePosW;
};