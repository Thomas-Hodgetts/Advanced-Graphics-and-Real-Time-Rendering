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

	GraphicsManager* gm = (GraphicsManager*)COS.graphicsManager;

	Geometry geo;
	geo.Point = XMFLOAT3(0, 0, 0);
	geo.vertexBufferView = gm->GetVertexBufferView(COS.identifier);
	geo.indexBufferView = gm->GetIndexBufferView(COS.identifier);
	geo.numberOfIndices = geo.indexBufferView->SizeInBytes / sizeof(DWORD);


	Apperance* objAppearence = new Apperance(geo, Material(), nullptr);
	GameObject* GO = new GameObject(COS.objName, objAppearence, COS.transform);
	m_Objects.AddObject(GO);

	return 0;
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


