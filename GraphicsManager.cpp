#include "GraphicsManager.h"

GraphicsManager::GraphicsManager()
{
}

GraphicsManager::GraphicsManager(int width, int height) : m_Height(height), m_Width(width)
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
	// Fill out the Viewport
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (float)m_Width;
	m_Viewport.Height = (float)m_Height;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	m_Scissor.left = 0;
	m_Scissor.top = 0;
	m_Scissor.right = m_Width;
	m_Scissor.bottom = m_Height;
}

GraphicsManager::~GraphicsManager()
{
}

void GraphicsManager::ExecuteCommands()
{
}

void GraphicsManager::Draw(void* gameObjectData, int objectCount,OutputManager* output,std::wstring pipelineIdentifier, std::wstring dsvIdentifier, std::wstring srvIdentifer)
{
	HRESULT hr;
	int frameIndex = output->GetCurrentFrameIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = output->GetHeap()->CPUCurrentAddress();

	hr = m_GraphicsCommandListMap[L"Default"]->Reset(m_CommandAllocatorMap[L"Default"].at(frameIndex), m_PipelineMap[pipelineIdentifier]);
	if (FAILED(hr))
	{
		Debug::OutputString("Drawing failed: Failed to reset the command list");
	}

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(output->GetCurrentFrame(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// set the render target for the output merger stage (the output of the pipeline)
	m_GraphicsCommandListMap[L"Default"]->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_DepthStencilHeapDescription[dsvIdentifier]->CPUCurrentAddress());

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_GraphicsCommandListMap[L"Default"]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	m_GraphicsCommandListMap[L"Default"]->ClearDepthStencilView(m_DepthStencilHeapDescription[dsvIdentifier]->CPUCurrentAddress(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	m_GraphicsCommandListMap[L"Default"]->SetGraphicsRootSignature(m_RootSignatureMap[pipelineIdentifier]); // set the root signature

	// set the descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_TextureHeapMap[srvIdentifer]->GetHeap() };
	m_GraphicsCommandListMap[L"Default"]->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	m_GraphicsCommandListMap[L"Default"]->SetGraphicsRootDescriptorTable(1, m_TextureHeapMap[srvIdentifer]->GPUStartAddress());

	m_GraphicsCommandListMap[L"Default"]->RSSetViewports(1, &m_Viewport); // set the viewports
	m_GraphicsCommandListMap[L"Default"]->RSSetScissorRects(1, &m_Scissor); // set the scissor rects
	m_GraphicsCommandListMap[L"Default"]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
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

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing

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
	if (m_HullBlobMap[name] != nullptr)
	{
		D3D12_SHADER_BYTECODE hullShaderBytecode;
		hullShaderBytecode.BytecodeLength = m_HullBlobMap[name]->GetBufferSize();
		hullShaderBytecode.pShaderBytecode = m_HullBlobMap[name]->GetBufferPointer();
		psoDesc.HS = hullShaderBytecode;
	}
	if (m_DomBlobMap[name] != nullptr)
	{
		D3D12_SHADER_BYTECODE domShaderByteCode;
		domShaderByteCode.BytecodeLength = m_DomBlobMap[name]->GetBufferSize();
		domShaderByteCode.pShaderBytecode = m_DomBlobMap[name]->GetBufferPointer();
		psoDesc.DS = domShaderByteCode;
	}

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
	std::vector<ID3D12Resource*> renderTargets;


	for (int i = 0; i < desc.NumDescriptors; i++)
	{
		CPU_DESC_CONTAINER* container = new CPU_DESC_CONTAINER(m_RenderTargetHeaps[name]->CPUCurrentAddress());

		renderTargets.push_back(nullptr);

		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[renderTargets.size() - 1]));
		if (FAILED(hr))
		{
			return nullptr;
		}

		m_Device->CreateRenderTargetView(renderTargets[renderTargets.size() - 1], nullptr, m_RenderTargetHeaps[name]->CPUCurrentAddress());

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

	m_RenderTargetGroups[name] = renderTargets;

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

bool GraphicsManager::CreateConstantBuffer(std::wstring name, int blockCount, int classSize, int objectCount)
{
	m_ConstantBufferMap[name] = new ConstantBufferHelper(name);
	m_ConstantBufferMap[name]->Init(m_Device, blockCount, classSize, objectCount);

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

	int vSize = (sizeof(Vertex) * vertexCount);
	int iSize = (sizeof(DWORD) * indexCount);

	NormalCalculations::CalculateModelVectors(vertexList, vertexCount);

	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vSize),
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
		&CD3DX12_RESOURCE_DESC::Buffer(vSize),
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
	vertexData.RowPitch = vSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vSize; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(m_GraphicsCommandListMap[L"Default"], m_VertexMap[geomertyIdentifier], vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_VertexMap[geomertyIdentifier], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// create default heap to hold index buffer
	hr = m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iSize), // resource description for a buffer
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
		&CD3DX12_RESOURCE_DESC::Buffer(iSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	if (FAILED(hr))
	{
		return false;
	}
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList); // pointer to our index array
	indexData.RowPitch = iSize; // size of all our index buffer
	indexData.SlicePitch = iSize; // also the size of our index buffer

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(m_GraphicsCommandListMap[L"Default"], m_IndexMap[geomertyIdentifier], iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexMap[geomertyIdentifier], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method

	m_VertexViewMap[geomertyIdentifier] = new D3D12_VERTEX_BUFFER_VIEW();
	m_VertexViewMap[geomertyIdentifier]->BufferLocation = m_VertexMap[geomertyIdentifier]->GetGPUVirtualAddress();
	m_VertexViewMap[geomertyIdentifier]->StrideInBytes = sizeof(Vertex);
	m_VertexViewMap[geomertyIdentifier]->SizeInBytes = vSize;

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	m_IndexViewMap[geomertyIdentifier] = new D3D12_INDEX_BUFFER_VIEW();
	m_IndexViewMap[geomertyIdentifier]->BufferLocation = m_IndexMap[geomertyIdentifier]->GetGPUVirtualAddress();
	m_IndexViewMap[geomertyIdentifier]->Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	m_IndexViewMap[geomertyIdentifier]->SizeInBytes = iSize;
}

bool GraphicsManager::CreateTextureHeap(LPCWSTR* textureLocations, int texCount, std::wstring name)
{
	HRESULT hr;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 5;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	DescriptorHeapHelper* dHH = new DescriptorHeapHelper(heapDesc, m_Device, &hr);
	m_TextureHeapMap[name] = dHH;

	ID3D12DescriptorHeap* mainDescriptorHeap = m_TextureHeapMap[name]->GetHeap();
	hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}

	// Load the image from file
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;

	BYTE* imageData;

	for (size_t i = 0; i < texCount; i++)
	{

		std::wstring texName = name;

		texName.append(std::to_wstring(i));

		int imageSize = LoadImageDataFromFile(&imageData, textureDesc, textureLocations[i], imageBytesPerRow);
		// make sure we have data
		if (imageSize <= 0)
		{
			return false;
		}

		// create a default heap where the upload heap will copy its contents into (contents being the texture)
		hr = m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&textureDesc, // the description of our texture
			D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
			nullptr, // used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&m_TextureMap[texName]));
		if (FAILED(hr))
		{
			return false;
		}
		m_TextureMap[texName]->SetName(L"Texture Buffer Resource Heap");

		UINT64 textureUploadBufferSize;
		m_Device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

		// now we create an upload heap to upload our texture to the GPU
		hr = m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
			D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
			nullptr,
			IID_PPV_ARGS(&m_TextureUploadHeapMap[texName]));
		if (FAILED(hr))
		{
			return false;
		}
		m_TextureUploadHeapMap[texName]->SetName(L"Texture Buffer Upload Resource Heap");

		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = &imageData[0]; // pointer to our image data
		textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
		textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data
		// Now we copy the upload buffer contents to the default heap
		//UpdateSubresources(m_CommandListMap[L"Default"], m_TextureMap[texName], m_TextureUploadHeapMap[texName], 0, 0, 1, &textureData);

		UpdateSubresources(m_GraphicsCommandListMap[L"Default"], m_TextureMap[texName], m_TextureUploadHeapMap[texName], 0, 0, 1, &textureData);
		// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
		m_GraphicsCommandListMap[L"Default"]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_TextureMap[texName], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		//BOOK

		// now we create a shader resource view (descriptor that points to the texture and describes it)
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		m_Device->CreateShaderResourceView(m_TextureMap[texName], &srvDesc, m_TextureHeapMap[name]->CPUCurrentAddress());
		m_TextureHeapMap[name]->CPUOffset();
		return true;
	}
}

bool GraphicsManager::CreateCustomTexture(D3D12_RESOURCE_FLAGS flags)
{
	D3D12_RESOURCE_DESC textureDesc;
	textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Height = m_Height;
	textureDesc.Width = m_Width;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Alignment = 0;
	textureDesc.Flags = flags;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	return true;
}

bool GraphicsManager::UpdateObjectConstantBuffer(ConstantBufferPerObject cBPO, std::wstring identifier, int pos)
{
	ConstantBufferPerObject * constBuffObject = m_ConstantBufferMap[identifier]->GetBuffer();
	*constBuffObject = cBPO;
	m_ConstantBufferMap[identifier]->FlushBuffer(pos);
	return true;
}

bool GraphicsManager::FlushCommandList(std::wstring idenifier)
{
	m_GraphicsCommandListMap[idenifier]->Close();
	ID3D12CommandList* ppCommandLists[] = { m_GraphicsCommandListMap[idenifier] };
	m_CommadQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	return true;
}

int GraphicsManager::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;

	return 0; // error
}

DXGI_FORMAT GraphicsManager::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)

{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

WICPixelFormatGUID GraphicsManager::GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

int GraphicsManager::LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow)
{
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	static IWICImagingFactory* wicFactory;

	// reset decoder, frame, and converter, since these will be different for each image we load
	IWICBitmapDecoder* wicDecoder = NULL;
	IWICBitmapFrameDecode* wicFrame = NULL;
	IWICFormatConverter* wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		CoInitialize(NULL);

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;

		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;
	}

	if (wicConverter == NULL)
	{
		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;
	}

	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromFilename(
		filename,                        // Image we want to load in
		NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
		GENERIC_READ,                    // We want to read from this file
		WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
		&wicDecoder                      // the wic decoder to be created
	);
	if (FAILED(hr)) return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

	// allocate enough memory for the raw image data, and set imageData to point to that memory
	*imageData = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}

	// now describe the texture with the information we have obtained from the image
	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	resourceDescription.Width = textureWidth; // width of the texture
	resourceDescription.Height = textureHeight; // height of the texture
	resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
	resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
	return imageSize;
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
	hr = D3DCompileFromFile(shaderFileLocation.c_str(),
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
	hr = D3DCompileFromFile(shaderFileLocation.c_str(),
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

bool GraphicsManager::CompileHullShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName)
{
	HRESULT hr;
	hr = D3DCompileFromFile(shaderFileLocation.c_str(),
		nullptr,
		nullptr,
		functionName,
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_HullBlobMap[shaderName],
		&m_ErrorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)m_ErrorBuff->GetBufferPointer());
		return false;
	}
}

bool GraphicsManager::CompileDomainShader(std::wstring shaderName, std::wstring shaderFileLocation, const char* functionName)
{
	HRESULT hr;
	hr = D3DCompileFromFile(shaderFileLocation.c_str(),
		nullptr,
		nullptr,
		functionName,
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&m_DomBlobMap[shaderName],
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

bool GraphicsManager::CreateStencilDepthView(std::wstring name, int size, D3D12_DEPTH_STENCIL_VIEW_DESC* depthStencilDesc, D3D12_CLEAR_VALUE* depthOptimizedClearValue)
{
	HRESULT hr;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = size;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	DescriptorHeapHelper* dHH = new DescriptorHeapHelper(dsvHeapDesc, m_Device, &hr);
	m_DepthStencilHeapDescription[name] = dHH;

	if (depthStencilDesc == nullptr || depthOptimizedClearValue == nullptr)
	{
		return true;
	}

	m_DepthBufferMap[name] = std::vector<ID3D12Resource*>(size);

	for (size_t i = 0; i < size; ++i)
	{

		hr = m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Width, m_Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			depthOptimizedClearValue,
			IID_PPV_ARGS(&m_DepthBufferMap[name][i])
		);
		if (FAILED(hr))
		{
			return false;
		}


		std::wstring heapName = name;
		heapName.append(L" Heap");
		m_DepthStencilHeapDescription[name]->SetName(heapName);

		m_Device->CreateDepthStencilView(m_DepthBufferMap[name][i], depthStencilDesc, dHH->CPUCurrentAddress());
		dHH->CPUOffset();
		++depthStencilDesc;
		++depthOptimizedClearValue;
	}
	return true;
}
