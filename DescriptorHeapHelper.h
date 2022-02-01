#pragma once
#include <d3d12.h>
#include "Structures.h"




class DescriptorHeapHelper
{
public:
	DescriptorHeapHelper(D3D12_DESCRIPTOR_HEAP_DESC heapDesc, ID3D12Device* device, HRESULT* hr);
	~DescriptorHeapHelper();

	void CPUOffset();

	void GPUOffset();

	D3D12_CPU_DESCRIPTOR_HANDLE CPUStartAddress();

	D3D12_CPU_DESCRIPTOR_HANDLE CPUCurrentAddress();

	D3D12_GPU_DESCRIPTOR_HANDLE GPUStartAddress();

	D3D12_GPU_DESCRIPTOR_HANDLE GPUCurrentAddress();

	int ReturnSize() { return m_Size; };

	D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHeapHelper::operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& vec);

	D3D12_GPU_DESCRIPTOR_HANDLE& DescriptorHeapHelper::operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& vec);
private:

	int m_DescriptorSize;
	ID3D12DescriptorHeap* m_DescriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
	D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorType;
	int m_Size, m_CPUActive = 0, m_GPUActive = 0;

};

