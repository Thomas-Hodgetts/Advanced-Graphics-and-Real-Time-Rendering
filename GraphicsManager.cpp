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

	CD3DX12_RASTERIZER_DESC rastDesc(D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_NONE, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
		0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	m_RasterDescription[L"Default"] = rastDesc;

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

	std::vector<ID3D12CommandAllocator*> commandAllcoators(1);

	hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllcoators[0]));
	if (FAILED(hr))
	{
		GraphicsManager::~GraphicsManager();
		return;
	}

	m_CommandAllocatorMap[L"Default"] = commandAllcoators;

	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocatorMap[L"Default"][0], NULL, IID_PPV_ARGS(&m_GraphicsCommandListMap[L"Default"]));
	if (FAILED(hr))
	{
		GraphicsManager::~GraphicsManager();
		return;
	}

}

GraphicsManager::~GraphicsManager()
{
}

void GraphicsManager::ExecuteCommands()
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
	HRESULT hr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso

	// fill out an input layout description structure
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	if (m_InputLayoutMap[name].size() != 0)
	{
		// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
		inputLayoutDesc.NumElements = m_InputLayoutMap[name].size();
		inputLayoutDesc.pInputElementDescs = m_InputLayoutMap[name].data();
	}
	else
	{
		return 1;
	}

	if (m_RootSignatureMap[name] != nullptr)
	{
		psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
		psoDesc.pRootSignature = m_RootSignatureMap[name]; // the root signature that describes the input data this pso needs
	}
	else
	{
		return 1;
	}

	if (m_VertexBlobMap[name] != nullptr)
	{
		D3D12_SHADER_BYTECODE vertexShaderByteCode;
		vertexShaderByteCode.BytecodeLength = m_VertexBlobMap[name]->GetBufferSize();
		vertexShaderByteCode.pShaderBytecode = m_VertexBlobMap[name]->GetBufferPointer();
		psoDesc.VS = vertexShaderByteCode;
	}
	if (m_PixelBlobMap[name] != nullptr)
	{
		D3D12_SHADER_BYTECODE pixelShaderBytecode;
		pixelShaderBytecode.BytecodeLength = m_PixelBlobMap[name]->GetBufferSize();
		pixelShaderBytecode.pShaderBytecode = m_PixelBlobMap[name]->GetBufferPointer();
		psoDesc.PS = pixelShaderBytecode;
	}
	if (m_GeoBlobMap[name] != nullptr)
	{
		D3D12_SHADER_BYTECODE geoShaderByteCode;
		geoShaderByteCode.BytecodeLength = m_GeoBlobMap[name]->GetBufferSize();
		geoShaderByteCode.pShaderBytecode = m_GeoBlobMap[name]->GetBufferPointer();
		psoDesc.GS = geoShaderByteCode;
	}

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = m_SampleDescMap[L"OutputManager"]; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = m_RasterDescription[L"Default"]; // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	hr = m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineMap[name]));
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
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

	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocatorMap[name][0], NULL, IID_PPV_ARGS(&m_GraphicsCommandListMap[name]));
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

bool GraphicsManager::CreateGeomerty(const char* fileLoation, std::wstring geomertyIdentifier)
{
	return false;
}

bool GraphicsManager::CreateGeomerty(Vertex* vertexList, int vertexCount, DWORD* iList, int indexCount, std::wstring geomertyIdentifier)
{
	HRESULT hr;
	for (size_t i = 0; i < indexCount / 3; i++)
	{
		UINT i0 = iList[i * 3 + 0];
		UINT i1 = iList[i * 3 + 1];
		UINT i2 = iList[i * 3 + 2];

		Vertex v0 = vertexList[i0];
		Vertex v1 = vertexList[i1];
		Vertex v2 = vertexList[i2];

		Vector3D faceNormal = NormalCalculations::CalculateNormal(v0.pos.ConvertToXMvector3(), v1.pos.ConvertToXMvector3(), v2.pos.ConvertToXMvector3());

		vertexList[i0].Normal += faceNormal;
		vertexList[i1].Normal += faceNormal;
		vertexList[i2].Normal += faceNormal;
	}

	NormalCalculations::CalculateModelVectors(vertexList, vertexCount);

	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexCount),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_VertexMap[geomertyIdentifier]));
	if (FAILED(hr))
	{
		return false;
	}

	m_VertexMap[geomertyIdentifier]->SetName(L"Vertex Buffer Resource Heap");

	ID3D12Resource* vBufferUploadHeap;
	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	if (FAILED(hr))
	{
		return false;
	}
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertexList); // pointer to our vertex array
	vertexData.RowPitch = (sizeof(Vertex) * vertexCount); // size of all our triangle vertex data
	vertexData.SlicePitch = (sizeof(Vertex) * vertexCount); // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(m_GraphicsCommandListMap[L"Default"], m_VertexMap[geomertyIdentifier], vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_VertexMap[geomertyIdentifier], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// create default heap to hold index buffer
	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(indexCount), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&m_IndexMap[geomertyIdentifier]));
	if (FAILED(hr))
	{
		return false;
	}

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	m_IndexMap[geomertyIdentifier]->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	ID3D12Resource* iBufferUploadHeap;
	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(indexCount), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	if (FAILED(hr))
	{
		return false;
	}
	vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList); // pointer to our index array
	indexData.RowPitch = (sizeof(DWORD) * indexCount); // size of all our index buffer
	indexData.SlicePitch = (sizeof(DWORD) * indexCount); // also the size of our index buffer

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(m_GraphicsCommandListMap[L"Default"], m_IndexMap[geomertyIdentifier], iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexMap[geomertyIdentifier], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
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

	std::vector<CD3DX12_DESCRIPTOR_RANGE>ranges(textureCount) ;

	for (size_t i = 0; i < rootParameterCount; ++i)
	{
		if (cbCount < constantBufferCount)
		{
			rootParameters[i].InitAsConstantBufferView(0, cbCount, D3D12_SHADER_VISIBILITY_ALL);
			++cbCount;
		}
		else if(texCount < textureCount)
		{
			ranges[texCount] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, texCount);
			rootParameters[i].InitAsDescriptorTable(1, &ranges[texCount], D3D12_SHADER_VISIBILITY_PIXEL);

			///// Texture 1
			//CD3DX12_DESCRIPTOR_RANGE texture1Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			//rootParameters[1].InitAsDescriptorTable(1, &texture1Range, D3D12_SHADER_VISIBILITY_PIXEL);

			////// Texture 2
			//CD3DX12_DESCRIPTOR_RANGE texture2Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
			//rootParameters[2].InitAsDescriptorTable(1, &texture2Range, D3D12_SHADER_VISIBILITY_PIXEL);

			////// Texture 3
			//CD3DX12_DESCRIPTOR_RANGE texture3Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
			//rootParameters[3].InitAsDescriptorTable(1, &texture3Range, D3D12_SHADER_VISIBILITY_PIXEL);
			////// Texture 4
			//CD3DX12_DESCRIPTOR_RANGE texture4Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
			//rootParameters[4].InitAsDescriptorTable(1, &texture4Range, D3D12_SHADER_VISIBILITY_PIXEL);

			++texCount;
		}
	}

	CD3DX12_ROOT_PARAMETER test = rootParameters[2];
	CD3DX12_ROOT_PARAMETER test2 = rootParameters[1];
	test.DescriptorTable.pDescriptorRanges->RegisterSpace;
	test2.DescriptorTable.pDescriptorRanges->RegisterSpace;

	if (test.DescriptorTable.pDescriptorRanges->RegisterSpace == test2.DescriptorTable.pDescriptorRanges->RegisterSpace)
	{
		int i = 0;
	}
	if (test.DescriptorTable.pDescriptorRanges->BaseShaderRegister == test2.DescriptorTable.pDescriptorRanges->BaseShaderRegister)
	{
		int i = 0;
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

void GraphicsManager::AddFrameInputLayout(D3D12_INPUT_ELEMENT_DESC* desc, std::wstring identifier, int size)
{
	for (size_t i = 0; i < size; ++i)
	{
		m_InputLayoutMap[identifier].push_back(*desc);
		++desc;
	}
}
