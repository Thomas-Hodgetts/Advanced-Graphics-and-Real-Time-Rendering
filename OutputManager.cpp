#include "OutputManager.h"

OutputManager::OutputManager(GraphicsManager* pGManager ,DXGI_SWAP_CHAIN_DESC swapChaindesc, D3D12_DESCRIPTOR_HEAP_DESC descriptorHeap)
{

	if (descriptorHeap.Type != D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		OutputManager::~OutputManager();
	}

	IDXGISwapChain* tempSwapChain;
	HRESULT hr;
	hr = pGManager->GetFactory()->CreateSwapChain(pGManager->GetCommandQueue(), &swapChaindesc, &tempSwapChain);
	m_SwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
	if (swapChaindesc.BufferCount == 1)
	{
		m_BufferMode = BUFFER_MODE::NO_BUFFER;
	}
	else if(swapChaindesc.BufferCount == 2)
	{
		m_BufferMode = BUFFER_MODE::DOUBLE_BUFFER;
	}
	else if (swapChaindesc.BufferCount == 3)
	{
		m_BufferMode = BUFFER_MODE::TRIPLE_BUFFER;
	}
	else
	{
		m_BufferMode = BUFFER_MODE::NO_BUFFER;
	}
	m_RenderTargetHeap = pGManager->CreateRenderTargetViews(descriptorHeap, m_SwapChain, L"OutputManagerSwapChain");
}

OutputManager::~OutputManager()
{
}
