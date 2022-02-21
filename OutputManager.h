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
	OutputManager() {};
	OutputManager(GraphicsManager* pGManager, DXGI_SWAP_CHAIN_DESC swapChaindesc, D3D12_DESCRIPTOR_HEAP_DESC descriptorHeap);
	~OutputManager();

	bool Present();

	BUFFER_MODE GetBufferMode() { return m_BufferMode; }

	int GetBufferSize() { return (int)m_BufferMode + 1 ; };

	WindowData GetWindowData() { return m_WindowData; };

	ID3D12Resource* GetCurrentFrame() { return m_RenderTargets[m_CurrentFrame]; }

	int GetCurrentFrameIndex() { return m_CurrentFrame; }

	DescriptorHeapHelper* GetHeap() { return m_RenderTargetHeap; };

	void UpdateFrameIndex();

	void OutputManager::operator++();

private:

	int m_CurrentFrame = 0;
	ID3D12Device* m_Device = nullptr;
	ID3D12CommandQueue* m_CommadQueue = nullptr;
	IDXGISwapChain3* m_SwapChain;
	BUFFER_MODE m_BufferMode;
	DescriptorHeapHelper* m_RenderTargetHeap;
	std::vector<ID3D12Resource*> m_RenderTargets;
	std::wstring m_Name = L"OutputManager";
	WindowData m_WindowData;
};

