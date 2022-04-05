#pragma once
#pragma comment(lib, "d3d11.lib")

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <directxcollision.h>
#include <string>
#include <wincodec.h>
#include <Random>
#include <fstream>

#include "SystemManager.h"
#include "Structures.h"
#include "NormalCalculations.h"
#include "ShadowMaps.h"
#include "GraphicsManager.h"
#include "OutputManager.h"

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

using namespace DirectX; // we will be using the directxmath library

// Handle to the window
HWND hwnd = NULL;

// name of the window (not the title)
LPCTSTR WindowName = L"Advanced Graphics and Real-time Rendering";

// title of the window
LPCTSTR WindowTitle = L"Advanced Graphics and Real-time Rendering";

// width and height of the window
int Width = 1920;
int Height = 1080;

//// is window full screen?
bool FullScreen = false;

// we will exit the program when this becomes false
bool Running = true;

// create a window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	bool fullscreen);

// main application loop
void mainloop();

// callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

// direct3d stuff
const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

IDXGISwapChain3* swapChain; // swapchain used to switch between render 
ID3D12Fence* fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
										 //as we have allocators (more if we want to know when the gpu is finished with an asset)

HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

int frameIndex; // current rtv we are on

int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
					   // function declarations
int dsvDescriptorSize;
int srvDescriptorSize;

bool InitD3D(); // initializes direct3d 12

void Update(); // update the game logic

void UpdatePipeline(); // update the direct3d pipeline (update command lists)

void Render(); // execute the command list

void Cleanup(); // release com ojects and clean up memory

void WaitForPreviousFrame(); // wait until gpu is finished with command list


struct Screen_Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
};


// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
// being copied.
int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu 
										// (which will be placed in the resource we created above)


ConstantBufferPerObject cbShadow;

ConstantBufferLighting cbLighting;

ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed
ID3D12Resource* m_BillboardConstantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

UINT8* cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps
UINT8* m_BillboardGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
XMFLOAT4X4 cameraViewMat; // this will store our view matrix

XMFLOAT4 cameraPosition; // this is our cameras position vector
XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
XMFLOAT4 cameraUp; // the worlds up vector

XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
XMFLOAT4 cube1Position; // our first cubes position in space

XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

int numCubeIndices; // the number of indices to draw the cube
int numCubeIndices2; // the number of indices to draw the cube

ID3D12Resource* textureBuffer[5]; // the resource heap containing our texture

Screen_Vertex svQuad[4];

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap[5];

ShadowMap m_SM;

enum RootParameterIndex
{
	Texture1SRV,
	Texture1Sampler,
	Texture2SRV,
	Texture2Sampler,
	ConstantBuffer,
	RootParameterCount
};


SystemManager* m_Manager = nullptr;
DrawObjectsStruct m_DrawObjectStructs = DrawObjectsStruct();
Material shinyMaterial;
Light basicLight;
int m_BillboardCount = 100;
int ImageOffset;
int buffOffset;
int m_Time = 0;
int m_TimePause = 0;
bool m_RenderToTexture = false;
bool m_TextureSetUp = false;
bool m_ShadowMapping = false;
bool m_NormalMapping = false;
bool m_Rotate = false;

XMFLOAT4X4 m_ShadowTransform;
XMFLOAT4X4 m_LightProj;
XMFLOAT4X4 m_LightView;
XMFLOAT4X4 m_LightMatrix;

ID3D12Debug* m_DebugLayer;

GraphicsManager m_GameManager;
OutputManager m_OutputManager;

BoundingSphere m_BS;