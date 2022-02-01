#pragma once
#include "Structures.h"
#include <dxgi1_4.h>
#include "DescriptorHeapHelper.h"
#include "GraphicsManager.h"
#include <vector>

enum class BUFFER_MODE
{
	NO_BUFFER = 0,
	DOUBLE_BUFFER = 1,
	TRIPLE_BUFFER = 2
};

class OutputManager
{
public:
	OutputManager(GraphicsManager* pGManager, DXGI_SWAP_CHAIN_DESC swapChaindesc, D3D12_DESCRIPTOR_HEAP_DESC descriptorHeap);
	~OutputManager();


	BUFFER_MODE GetBufferMode() { return m_BufferMode; }

private:

	ID3D12Device* m_Device = nullptr;
	ID3D12CommandQueue* m_CommadQueue = nullptr;
	IDXGISwapChain3* m_SwapChain;
	BUFFER_MODE m_BufferMode;
	DescriptorHeapHelper* m_RenderTargetHeap;

};

