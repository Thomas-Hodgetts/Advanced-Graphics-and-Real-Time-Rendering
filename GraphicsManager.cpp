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

DescriptorHeapHelper* GraphicsManager::CreateRenderTargetViews(D3D12_DESCRIPTOR_HEAP_DESC desc, IDXGISwapChain3* swapChain, std::wstring name)
{
	HRESULT hr;

	DescriptorHeapHelper* dHH =  new DescriptorHeapHelper(desc, m_Device, &hr);
	//m_RenderTargetHeaps[name] = dHH;

	for (int i = 0; i < desc.NumDescriptors; i++)
	{
		// first we get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargetMap[m_RenderTargetHeaps[name]->CPUCurrentAddress()]));
		if (FAILED(hr))
		{
			return nullptr;
		}

		// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		m_Device->CreateRenderTargetView(m_RenderTargetMap[m_RenderTargetHeaps[name]->CPUCurrentAddress()], nullptr, m_RenderTargetHeaps[name]->CPUCurrentAddress());

		// we increment the rtv handle by the rtv descriptor size we got above
		m_RenderTargetHeaps[name]->CPUOffset();
	}
	return dHH;
}
