#include "SystemManager.h"

SystemManager::SystemManager()
{
	m_Camera = nullptr;
}

SystemManager::~SystemManager()
{
	if (m_Camera == nullptr)
	{
		delete m_Camera;
	}
}

HRESULT SystemManager::BuildObject(CreateObjectStruct COS)
{
	HRESULT hr;
	if (COS.objType == ObjectType::GameObj)
	{
		int vBufferSize = sizeof(COS.vertices);

		// create default heap
		// default heap is memory on the GPU. Only the GPU has access to this memory
		// To get data into this heap, we will have to upload the data using
		// an upload heap GO->m_Apperance->ReturnGeo().vertexBuffer
		
		ID3D12Resource* VB;
		hr = COS.dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
											// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&VB));
		if (FAILED(hr))
		{
			return (FAILED(hr));
		}

		// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
		VB->SetName(L"Vertex Buffer Resource Heap");

		// create upload heap
		// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
		// We will upload the vertex buffer using this heap to the default heap
		ID3D12Resource* vBufferUploadHeap;
		hr = COS.dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&vBufferUploadHeap));
		if (FAILED(hr))
		{
			return (FAILED(hr));
		}
		vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(COS.vertices.data()); // pointer to our vertex array
		vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
		vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

		// we are now creating a command with the command list to copy the data from
		// the upload heap to the default heap
		UpdateSubresources(COS.commandList, VB, vBufferUploadHeap, 0, 0, 1, &vertexData);

		// transition the vertex buffer data from copy destination state to vertex buffer state
		COS.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));



		int iBufferSize = sizeof(COS.indices);
		int numIndices = sizeof(COS.indices) / sizeof(DWORD);

		ID3D12Resource* IB;
		// create default heap to hold index buffer
		hr = COS.dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&IB));
		if (FAILED(hr))
		{
			return (FAILED(hr));
		}

		// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
		IB->SetName(L"Index Buffer Resource Heap");

		// create upload heap to upload index buffer
		ID3D12Resource* iBufferUploadHeap;
		hr = COS.dev->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&iBufferUploadHeap));
		if (FAILED(hr))
		{
			return (FAILED(hr));
		}
		iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(COS.indices.data()); // pointer to our index array
		indexData.RowPitch = iBufferSize; // size of all our index buffer
		indexData.SlicePitch = iBufferSize; // also the size of our index buffer

		// we are now creating a command with the command list to copy the data from
		// the upload heap to the default heap
		UpdateSubresources(COS.commandList, IB, iBufferUploadHeap, 0, 0, 1, &indexData);

		// transition the vertex buffer data from copy destination state to vertex buffer state
		COS.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		Geometry geo;
		geo.indexBuffer = IB;
		geo.vertexBuffer = VB;
		geo.numberOfIndices = numIndices;
		geo.Point = XMFLOAT3(0, 0, 0);
		// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
		geo.vertexBufferView.BufferLocation = geo.vertexBuffer->GetGPUVirtualAddress();
		geo.vertexBufferView.StrideInBytes = sizeof(Vertex);
		geo.vertexBufferView.SizeInBytes = vBufferSize;
		// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
		geo.indexBufferView.BufferLocation = geo.indexBuffer->GetGPUVirtualAddress();
		geo.indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
		geo.indexBufferView.SizeInBytes = iBufferSize;

		Apperance* objAppearence = new Apperance(geo, Material(), nullptr);
		objAppearence->SetBufferOffset(COS.bufferAlignment);
		GameObject* GO = new GameObject(COS.objName, objAppearence, COS.transform);
		m_Objects.AddObject(GO);
	}
	else
	{
		
	}
}

void SystemManager::SetUpCamera(XMFLOAT4 position, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_Camera = new Camera(position, at, up, windowWidth, windowHeight, nearDepth, farDepth);
}

void SystemManager::Update()
{
	m_Objects.Update();
}

void SystemManager::Draw(DrawObjectsStruct DOS)
{
	PreparePipeline(DOS);
}

void SystemManager::PreparePipeline(DrawObjectsStruct DOS)
{

	HRESULT hr;

	// reset the command list. by resetting the command list we are putting it into
	// a recording state so we can start recording commands into the command allocator.
	// the command allocator that we reference here may have multiple command lists
	// associated with it, but only one can be recording at any time. Make sure
	// that any other command lists associated to this command allocator are in
	// the closed state (not recording).
	// Here you will pass an initial pipeline state object as the second parameter,
	// but in this tutorial we are only clearing the rtv, and do not actually need
	// anything but an initial default pipeline, which is what we get by setting
	// the second parameter to NULL
	hr = DOS.commandList->Reset(DOS.commandAllocator, DOS.pipelineStateObject);
	if (FAILED(hr))
	{
		Debug::OutputString("Command List Failed to Reset");
		return;
	}

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	DOS.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DOS.renderTargets, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DOS.renderTargetResourceHeap->GetCPUDescriptorHandleForHeapStart(), DOS.frameIndex, DOS.renderTargetDescriptorSize);

	// get a handle to the depth/stencil buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(DOS.depthAndStencilResourceHeap->GetCPUDescriptorHandleForHeapStart());

	// set the render target for the output merger stage (the output of the pipeline)
	DOS.commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	DOS.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	DOS.commandList->ClearDepthStencilView(DOS.depthAndStencilResourceHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	DOS.commandList->SetGraphicsRootSignature(DOS.root); // set the root signature

	// set the descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { DOS.mainDescriptorHeap };
	DOS.commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	DOS.commandList->SetGraphicsRootDescriptorTable(1, DOS.mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//m_Objects.Draw(DOS);

	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
	// warning if present is called on the render target when it's not in the present state
	DOS.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DOS.renderTargets, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = DOS.commandList->Close();
	if (FAILED(hr))
	{
		Debug::OutputString("Command List Failed to Close");
		return;
	}
}

