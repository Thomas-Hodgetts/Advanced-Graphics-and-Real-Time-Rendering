#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <directxcollision.h>
#include <wincodec.h>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <directxcollision.h>
#include <string>
#include <wincodec.h>


#include <unordered_map>

#include "Debug.h"
#include "DescriptorHeapHelper.h"
#include "NormalCalculations.h"
#include "ConstantBufferHelper.h"
#include "GameObject.h"
#include "SystemManager.h"

enum class PIPELINE_SHADER_ENUM_TYPE
{
	PIPELINE_CREATION_ENUM_TYPE_VERTEX_SHADER = 0,
	PIPELINE_CREATION_ENUM_TYPE_PIXEL_SHADER = 1,
	PIPELINE_CREATION_ENUM_TYPE_GEOMERTY_SHADER = 2,
	PIPELINE_CREATION_ENUM_TYPE_HULL_SHADER = 3,
	PIPELINE_CREATION_ENUM_TYPE_MESH_SHADER_SHADER = 4,
	PIPELINE_CREATION_ENUM_TYPE_DOMIAN_SHADER = 5
};

struct PIPELINE_SHADER_CREATION_STRUCT
{
	std::wstring m_ObjectName;
	PIPELINE_SHADER_ENUM_TYPE m_PipelineVariableType;
};

class CPU_DESC_CONTAINER
{
public:
	CPU_DESC_CONTAINER(D3D12_CPU_DESCRIPTOR_HANDLE handle) : m_Handle(handle) {};

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_Handle;
};


#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }


class GraphicsManager
{
public:
	GraphicsManager();
	GraphicsManager(int width, int height);
	~GraphicsManager();

	void ExecuteCommands();

	void ForceCloseCommandList(std::wstring identifier);

	void Draw(ID3D12Resource* currentFrame, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, int frameIndex, std::wstring pipelineIdentifier, std::wstring dsvIdentifier, std::wstring srvIdentifer, std::wstring constantBufferIdentifier, SystemManager* sysManager, bool renderTerrain, bool renderObject, bool closeCommandList);

	void Render(int frameIndex, std::wstring identifier);

	void ResetAllocator(int frameIndex, std::wstring identifier);

	void ReopenAllocator(int frameIndex, std::wstring identifier, std::wstring pipelineIdentifier);

	void IncrementFenceValue(int currentFrame, std::wstring identifier) { m_FenceValueMap[identifier][currentFrame]++; };

	//Pipeline functions

	HRESULT CreatePipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeDesc, std::wstring name);

	HRESULT CreatePipeline(std::wstring name);

	bool CreateRootSignature(D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags, D3D12_STATIC_SAMPLER_DESC* samplers, int samplerCount, int rootParameterCount, int constantBufferCount, int textureCount, std::wstring name);

	bool CompileVertexShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompilePixelShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompileGeomertyShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompileHullShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompileDomainShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	void AddFrameSampleDescription(DXGI_SAMPLE_DESC desc, std::wstring identifier) { m_SampleDescMap[identifier] = desc; };

	void AddFrameInputLayout(D3D12_INPUT_ELEMENT_DESC* desc, std::wstring identifier, int size);

	bool CreateStencilDepthView(std::wstring name, int size, D3D12_DEPTH_STENCIL_VIEW_DESC* depthStencilDesc, D3D12_CLEAR_VALUE* depthOptimizedClearValue);

	bool WaitOnFrame(int frameIndex);

	//RenderTargets

	DescriptorHeapHelper* CreateRenderTargetViews(D3D12_DESCRIPTOR_HEAP_DESC desc, IDXGISwapChain3* swapChain, std::wstring name);

	//Game Objects

	bool CreateGeomerty(const char* fileLoation, std::wstring geomertyIdentifier);

	void CreateConstantBuffer(std::wstring name, int frameCount, int blockCount, int classSize, int objectCount);

	bool CreateGeomerty(Vertex* vertexList, int vertexCount, DWORD* iList, int indexCount, std::wstring geomertyIdentifier);

	bool CreateGeomerty(std::vector<Vertex> vertex, std::vector<DWORD> index, std::wstring geomertyIdentifier, Geometry* geo);

	bool CreateTextureHeap(LPCWSTR* textureLocations, int texCount, std::wstring name);

	bool CreateCustomTexture(D3D12_RESOURCE_FLAGS flags);

	//Getter functions

	ID3D12Device* GetDevice() { return m_Device; };

	ID3D12CommandQueue* GetCommandQueue() { return m_CommadQueue; };

	IDXGIFactory4* GetFactory() { return m_Factory; };

	D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(std::wstring identifier) { return m_VertexViewMap[identifier]; };

	D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView(std::wstring identifier) { return m_IndexViewMap[identifier]; };

	std::vector<ID3D12Resource*> GetRenderTargets(std::wstring identifier) { return m_RenderTargetGroups[identifier]; };

	//Helpers

	bool UpdateObjectConstantBuffer(ConstantBufferPerObject cBPO, std::wstring identifier, int frameIndex, int pos);

	bool FlushCommandList(std::wstring Idenifier, int frameIndex);

	void Wireframe(bool mode) { m_Wireframe = mode; };

private:

	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

	int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);




#ifdef _DEBUG
	ID3D12Debug* m_DebugLayer;
#endif // _DEBUG


	int m_Height = 0, m_Width = 0;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Scissor;

	ID3D12Device* m_Device = nullptr;
	ID3D12CommandQueue* m_CommadQueue = nullptr;
	IDXGIFactory4* m_Factory = nullptr;
	HANDLE m_FenceEventHandle;

	ID3DBlob* m_ErrorBuff; // a buffer holding the error data if any
	ID3DBlob* m_Signature;

	HANDLE m_FenceEvent; // a handle to an event when our fence is unlocked by the gpu

	bool m_Wireframe = false;

	//Render Targets

	std::vector<ID3D12Resource*> m_RenderTargets;
	std::unordered_map<std::wstring, std::vector<ID3D12Resource*>> m_RenderTargetGroups;

	//Pileline data
	std::unordered_map<std::wstring,ID3D12PipelineState*> m_PipelineMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_VertexBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_PixelBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_GeoBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_HullBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_DomBlobMap;
	std::unordered_map<std::wstring, ID3D12RootSignature*> m_RootSignatureMap;
	std::unordered_map<std::wstring, std::vector<D3D12_INPUT_ELEMENT_DESC>> m_InputLayoutMap;
	std::unordered_map<std::wstring, DXGI_SAMPLE_DESC> m_SampleDescMap;
	std::unordered_map<std::wstring, D3D12_RASTERIZER_DESC> m_RasterDescription;

	//Objects data
	std::unordered_map<std::wstring, D3D12_VERTEX_BUFFER_VIEW*> m_VertexViewMap;
	std::unordered_map<std::wstring, D3D12_INDEX_BUFFER_VIEW*> m_IndexViewMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_VertexMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_TextureMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_TextureUploadHeapMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_IndexMap;
	std::unordered_map<std::wstring, ConstantBufferHelper*> m_ConstantBufferMap;
	std::unordered_map<std::wstring, DescriptorHeapHelper*> m_TextureHeapMap;


	//Core Data
	std::unordered_map<std::wstring, DescriptorHeapHelper*> m_RenderTargetHeaps;
	std::unordered_map<std::wstring, DescriptorHeapHelper*> m_DepthStencilHeapDescription;
	std::unordered_map<std::wstring, std::vector<ID3D12CommandAllocator*>> m_CommandAllocatorMap;
	std::unordered_map<std::wstring, std::vector<ID3D12Resource*>> m_DepthBufferMap;
	std::unordered_map<std::wstring, std::vector<ID3D12Fence*>> m_FenceMap;
	std::unordered_map<std::wstring, std::vector<UINT64>> m_FenceValueMap;
	std::unordered_map<std::wstring, ID3D12GraphicsCommandList*> m_GraphicsCommandListMap;
	std::unordered_map<std::wstring, ID3D12CommandList*> m_CommandListMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_DepthStencilBufferMap;
	std::unordered_map<std::wstring, bool> m_PipelineTess;

};

