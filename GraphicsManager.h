#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <directxcollision.h>

#include <unordered_map>

#include "Debug.h"
#include "DescriptorHeapHelper.h"

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


class GraphicsManager
{
public:
	GraphicsManager();
	~GraphicsManager();

	HRESULT CreatePipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeDesc, std::wstring name);

	HRESULT CreatePipeline(std::wstring name);

	DescriptorHeapHelper* CreateRenderTargetViews(D3D12_DESCRIPTOR_HEAP_DESC desc, IDXGISwapChain3* swapChain, std::wstring name);

	bool CreateRootSignature(D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags, D3D12_STATIC_SAMPLER_DESC* samplers, int samplerCount, int rootParameterCount, int constantBufferCount, int textureCount, std::wstring name);

	bool CompileVertexShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompilePixelShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);

	bool CompileGeomertyShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName);


	ID3D12Device* GetDevice() { return m_Device; };
	ID3D12CommandQueue* GetCommandQueue() { return m_CommadQueue; };
	IDXGIFactory4* GetFactory() { return m_Factory; };

private:


#ifdef _DEBUG
	ID3D12Debug* m_DebugLayer;
#endif // _DEBUG



	ID3D12Device* m_Device = nullptr;
	ID3D12CommandQueue* m_CommadQueue = nullptr;
	IDXGIFactory4* m_Factory = nullptr;
	HANDLE m_FenceEventHandle;

	ID3DBlob* m_ErrorBuff; // a buffer holding the error data if any
	ID3DBlob* m_Signature;

	std::vector<ID3D12Resource*> m_RenderTargets;

	//Pileline data
	std::unordered_map<std::wstring,ID3D12PipelineState*> m_PipelineMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_VertexBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_PixelBlobMap;
	std::unordered_map<std::wstring, ID3DBlob*> m_GeoBlobMap;
	std::unordered_map<std::wstring, ID3D12RootSignature*> m_RootSignatureMap;

	//Objects data
	std::unordered_map<std::wstring, D3D12_VERTEX_BUFFER_VIEW*> m_VertexViewMap;
	std::unordered_map<std::wstring, D3D12_INDEX_BUFFER_VIEW*> m_IndexViewMap;

	//Core Data
	std::unordered_map<std::wstring, DescriptorHeapHelper*> m_RenderTargetHeaps;
	std::unordered_map<std::wstring, std::vector<ID3D12CommandAllocator*>> m_CommandAllocatorMap;
	std::unordered_map<std::wstring, std::vector<ID3D12Fence*>> m_FenceMap;
	std::unordered_map<std::wstring, std::vector<UINT64>> m_FenceValueMap;
	std::unordered_map<std::wstring, ID3D12CommandList*> m_CommandListMap;
	std::unordered_map<std::wstring, ID3D12Resource*> m_DepthStencilBufferMap;

};

