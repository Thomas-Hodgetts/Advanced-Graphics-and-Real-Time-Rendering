#include "DescriptorHeapHelper.h"

DescriptorHeapHelper::DescriptorHeapHelper(D3D12_DESCRIPTOR_HEAP_DESC heapDesc, ID3D12Device* device, HRESULT* hr)
{
	m_DescriptorType = heapDesc.Type;
	m_DescriptorSize = device->GetDescriptorHandleIncrementSize(m_DescriptorType);
	*hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
	m_CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	m_Size = heapDesc.NumDescriptors;
	m_CPUActive = 0;
	m_GPUActive = 0;
}

DescriptorHeapHelper::~DescriptorHeapHelper()
{
}

void DescriptorHeapHelper::CPUOffset()
{
	if (m_CPUActive <= m_Size)
	{
		m_CPUHandle.Offset(m_DescriptorSize);
	}
	else
	{
		m_CPUHandle.Offset(-m_DescriptorSize * m_Size);
	}
}

void DescriptorHeapHelper::GPUOffset()
{
	if (m_GPUActive <= m_Size)
	{
		m_GPUHandle.Offset(m_DescriptorSize);
	}
	else
	{
		m_GPUHandle.Offset(-m_DescriptorSize * m_Size);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapHelper::CPUCurrentAddress()
{
	return m_CPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapHelper::GPUStartAddress()
{
	return m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapHelper::GPUCurrentAddress()
{
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapHelper::CPUStartAddress()
{
	return m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHeapHelper::operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& vec)
{
	return m_CPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE& DescriptorHeapHelper::operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& vec)
{
	return m_GPUHandle;
}

