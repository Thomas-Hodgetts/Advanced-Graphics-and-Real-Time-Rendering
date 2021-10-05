#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>
#include <random>
#include <vector>


using namespace DirectX;


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

struct SystemData
{
	HINSTANCE hInst;
	HWND hWnd;
	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* ImmediateContext;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11VertexShader* VertexShader;
	ID3D11PixelShader* PixelShader;
	ID3D11InputLayout* VertexLayout;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	ID3D11Buffer* ConstantBuffer;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11RasterizerState* wireFrame;
	ID3D11RasterizerState* solidFrame;
	ID3D11SamplerState* SamplerLinear;
	ID3D11BlendState* Transparency;
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
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
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

//Used to Calculate normals in a convient container. A vestage from week 4
struct NormalsCalculations
{


	XMFLOAT3 Original;
	XMFLOAT3 NextPoint;
	XMFLOAT3 NextPoint2;
	XMFLOAT3 FinalCrossProduct;
	XMFLOAT3 Average;

	//Calculates the normals for each point using Cross Products and normalisation. Doesn't calculate averages
	XMFLOAT3 CrossProductCalculation()
	{

		XMVECTOR U = XMVectorSubtract(XMLoadFloat3(&NextPoint), XMLoadFloat3(&Original));
		XMVECTOR V = XMVectorSubtract(XMLoadFloat3(&NextPoint2), XMLoadFloat3(&Original));
		XMVECTOR Cross = XMVector3Cross(U, V);
		XMStoreFloat3(&FinalCrossProduct, XMVector3Normalize(Cross));
		return FinalCrossProduct;
	}

	//Calculates Cross Product Averages
	XMFLOAT3 CalculateAverage(XMFLOAT3 CrossList[])
	{
		XMFLOAT3 Average = { 0,0,0 };
		int dd = *(&CrossList + 1) - CrossList;
		for (int i = 0; i < *(&CrossList + 1) - CrossList; i++)
		{
			Average.x += CrossList[i].x;
			Average.y += CrossList[i].y;
			Average.z += CrossList[i].z;
		}

		Average.x /= *(&CrossList + 1) - CrossList;
		Average.y /= *(&CrossList + 1) - CrossList;
		Average.z /= *(&CrossList + 1) - CrossList;
		return Average;
	}

	//Calculates Cross Product Averages
	XMFLOAT3 CalculateAverageWithVector(std::vector<XMFLOAT3> CrossList)
	{
		XMFLOAT3 Average = { 0,0,0 };
		for (int i = 0; i < CrossList.size(); i++)
		{
			Average.x += CrossList[i].x;
			Average.y += CrossList[i].y;
			Average.z += CrossList[i].z;
		}

		Average.x /= CrossList.size();
		Average.y /= CrossList.size();
		Average.z /= CrossList.size();
		return Average;
	}


};
