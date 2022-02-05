#include "GraphicsManager.h"

GraphicsManager::GraphicsManager()
{
	HRESULT hr;
	IDXGIFactory4* dxgiFactorydxgiFactory;


#ifdef _DEBUG
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory));

	D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugLayer));
	m_DebugLayer->EnableDebugLayer();
#endif // _DEBUG
#ifdef _RELEASE
	hr = CreateDXGIFactory(IID_PPV_ARGS(&m_Factory));
#endif // _RELEASE




	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found

							   // find first hardware gpu that supports d3d 12
	while (m_Factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		GraphicsManager::~GraphicsManager();
		return;
	}

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Device)
	);
	if (FAILED(hr))
	{
		GraphicsManager::~GraphicsManager();
		return;
	}

	// -- Create a direct command queue -- //

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = m_Device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_CommadQueue)); // create the command queue
	if (FAILED(hr))
	{
		GraphicsManager::~GraphicsManager();
		return;
	}

}

GraphicsManager::~GraphicsManager()
{
}

HRESULT GraphicsManager::CreatePipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeDesc, std::wstring name)
{
	HRESULT hr;
#ifdef _DEBUG
	try
	{
		hr = m_Device->CreateGraphicsPipelineState(&pipeDesc, IID_PPV_ARGS(&m_PipelineMap[name]));
		if (FAILED(hr))
		{
			return hr;
		}
	}
	catch (const std::exception&)
	{
			
	}


#endif // _DEBUG

#ifdef _RELEASE

	

#endif // _RELEASE


	return hr;
}

HRESULT GraphicsManager::CreatePipeline(std::wstring name)
{

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs


	if (m_VertexBlobMap[name] == nullptr)
	{
		D3D12_SHADER_BYTECODE vertexShaderByteCode;
		vertexShaderByteCode.BytecodeLength = m_VertexBlobMap[name]->GetBufferSize();
		vertexShaderByteCode.pShaderBytecode = m_VertexBlobMap[name]->GetBufferPointer();
		psoDesc.VS = vertexShaderByteCode;
	}
	if (m_PixelBlobMap[name] == nullptr)
	{
		D3D12_SHADER_BYTECODE pixelShaderBytecode;
		pixelShaderBytecode.BytecodeLength = m_PixelBlobMap[name]->GetBufferSize();
		pixelShaderBytecode.pShaderBytecode = m_PixelBlobMap[name]->GetBufferPointer();
		psoDesc.PS = pixelShaderBytecode;
	}
	if (m_GeoBlobMap[name] == nullptr)
	{
		D3D12_SHADER_BYTECODE geoShaderByteCode;
		geoShaderByteCode.BytecodeLength = m_GeoBlobMap[name]->GetBufferSize();
		geoShaderByteCode.pShaderBytecode = m_GeoBlobMap[name]->GetBufferPointer();
		psoDesc.GS = geoShaderByteCode;
	}

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = rastDesc; // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	return E_NOTIMPL;
}

DescriptorHeapHelper* GraphicsManager::CreateRenderTargetViews(D3D12_DESCRIPTOR_HEAP_DESC desc, IDXGISwapChain3* swapChain, std::wstring name)
{
	HRESULT hr;

	DescriptorHeapHelper* dHH =  new DescriptorHeapHelper(desc, m_Device, &hr);
	m_RenderTargetHeaps[name] = dHH;

	std::vector<ID3D12CommandAllocator*> commandAllcoators(desc.NumDescriptors);
	std::vector<ID3D12Fence*> fence(desc.NumDescriptors);
	std::vector<UINT64> fenceVal(desc.NumDescriptors);

	for (int i = 0; i < desc.NumDescriptors; i++)
	{
		CPU_DESC_CONTAINER* container = new CPU_DESC_CONTAINER(m_RenderTargetHeaps[name]->CPUCurrentAddress());

		m_RenderTargets.push_back(nullptr);

		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[m_RenderTargets.size() - 1]));
		if (FAILED(hr))
		{
			return nullptr;
		}

		m_Device->CreateRenderTargetView(m_RenderTargets[m_RenderTargets.size() - 1], nullptr, m_RenderTargetHeaps[name]->CPUCurrentAddress());

		m_RenderTargetHeaps[name]->CPUOffset();


		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllcoators[i]));
		if (FAILED(hr))
		{
			return nullptr;
		}


		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			return nullptr;
		}
		fenceVal[i] = 0;
	}

	m_CommandAllocatorMap[name] = commandAllcoators;
	m_FenceMap[name] = fence;
	m_FenceValueMap[name] = fenceVal;

	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocatorMap[name][0], NULL, IID_PPV_ARGS(&m_CommandListMap[name]));
	if (FAILED(hr))
	{
		return nullptr;
	}

	// create a handle to a fence event
	m_FenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEventHandle == nullptr)
	{
		return nullptr;
	}

	m_RenderTargetHeaps[name]->CPUOffset();

	return dHH;
}

bool GraphicsManager::CreateRootSignature(D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags, D3D12_STATIC_SAMPLER_DESC* samplers, int samplerCount, int rootParameterCount, int constantBufferCount, int textureCount, std::wstring name)
{
	HRESULT hr;

	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(rootParameterCount);

	//for (size_t i = 0; i < textureCount; ++i)
	//{
	//	CD3DX12_DESCRIPTOR_RANGE textureRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
	//	rootParameters[i].InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
	//}

	UINT cbCount = 0;
	UINT texCount = 0;
	for (size_t i = 0; i < rootParameterCount; ++i)
	{
		if (cbCount < constantBufferCount)
		{
			rootParameters[i].InitAsConstantBufferView(0, cbCount, D3D12_SHADER_VISIBILITY_ALL);
			++cbCount;
		}
		else if(texCount < textureCount)
		{
			//CD3DX12_DESCRIPTOR_RANGE textureRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, texCount);
			//rootParameters[i].InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
			
			// Texture 1
			CD3DX12_DESCRIPTOR_RANGE texture1Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			rootParameters[1].InitAsDescriptorTable(1, &texture1Range, D3D12_SHADER_VISIBILITY_PIXEL);

			//// Texture 2
			CD3DX12_DESCRIPTOR_RANGE texture2Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
			rootParameters[2].InitAsDescriptorTable(1, &texture2Range, D3D12_SHADER_VISIBILITY_PIXEL);

			//// Texture 3
			CD3DX12_DESCRIPTOR_RANGE texture3Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
			rootParameters[3].InitAsDescriptorTable(1, &texture3Range, D3D12_SHADER_VISIBILITY_PIXEL);
			//// Texture 4
			CD3DX12_DESCRIPTOR_RANGE texture4Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
			rootParameters[4].InitAsDescriptorTable(1, &texture4Range, D3D12_SHADER_VISIBILITY_PIXEL);

			++texCount;
		}
	}

	enum RootParameterIndex
	{
		Texture1SRV,
		Texture2SRV,
		Texture3SRV,
		Texture4SRV,
		ConstantBuffer
	};

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	////// Texture 1
	//CD3DX12_DESCRIPTOR_RANGE texture1Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	//rootParameters[RootParameterIndex::Texture1SRV].InitAsDescriptorTable(1, &texture1Range, D3D12_SHADER_VISIBILITY_PIXEL);

	////// Texture 2
	//CD3DX12_DESCRIPTOR_RANGE texture2Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	//rootParameters[RootParameterIndex::Texture2SRV].InitAsDescriptorTable(1, &texture2Range, D3D12_SHADER_VISIBILITY_PIXEL);

	////// Texture 3
	//CD3DX12_DESCRIPTOR_RANGE texture3Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	//rootParameters[RootParameterIndex::Texture3SRV].InitAsDescriptorTable(1, &texture3Range, D3D12_SHADER_VISIBILITY_PIXEL);
	////// Texture 4
	//CD3DX12_DESCRIPTOR_RANGE texture4Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	//rootParameters[RootParameterIndex::Texture4SRV].InitAsDescriptorTable(1, &texture4Range, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(rootParameters.size(), rootParameters.data(), samplerCount, samplers, rootSigFlags);

	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &m_ErrorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)m_ErrorBuff->GetBufferPointer());
		return false;
	}

	hr = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignatureMap[name]));
	if (FAILED(hr))
	{
		return false;
	}
}

bool GraphicsManager::CompileVertexShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName)
{
	HRESULT hr;

	// compile vertex shader
	hr = D3DCompileFromFile(shaderFileLocation.c_str(),
		nullptr,
		nullptr,
		functionName,
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_VertexBlobMap[shaderName],
		&m_ErrorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)m_ErrorBuff->GetBufferPointer());
		return false;
	}

}

bool GraphicsManager::CompilePixelShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName)
{
	HRESULT hr;
	hr = D3DCompileFromFile(L"PixelShader.hlsl",
		nullptr,
		nullptr,
		functionName,
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_PixelBlobMap[shaderName],
		&m_ErrorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)m_ErrorBuff->GetBufferPointer());
		return false;
	}
}

bool GraphicsManager::CompileGeomertyShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName)
{
	HRESULT hr;
	hr = D3DCompileFromFile(L"GeometryShader.hlsl",
		nullptr,
		nullptr,
		functionName,
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_GeoBlobMap[shaderName],
		&m_ErrorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)m_ErrorBuff->GetBufferPointer());
		return false;
	}
}
