#include "ConstantBufferHelper.h"

ConstantBufferHelper::ConstantBufferHelper(std::wstring name)
{
	m_Identifier = name;
}

ConstantBufferHelper::~ConstantBufferHelper()
{
	delete[] m_Buffer;
}

void ConstantBufferHelper::Init(ID3D12Device* device, int blockCount, int classSize, int objectCount)
{
	HRESULT hr;

	UINT64 bC = blockCount;

	m_Count = objectCount;

	// create resource for cube 1
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(bC * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
		nullptr, // we do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&m_UploadHeap));
	if (FAILED(hr))
	{

	}
	std::wstring heapName = m_Identifier;

	heapName.append(L" Upload Resource Heap");

	m_UploadHeap->SetName(heapName.c_str());

	CD3DX12_RANGE readRange(0, 0);

	// map the resource heap to get a gpu virtual address to the beginning of the heap
	hr = m_UploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&m_StartAddress));

	m_CurrentAddress = m_StartAddress;

	m_ConstantBufferOffset = (classSize + 255) & ~255;

	m_Buffer = new ConstantBufferPerObject[objectCount];

	for (size_t i = 0; i < objectCount; i++)
	{
		ZeroMemory(&m_Buffer[i], sizeof(m_Buffer[i]));
	}

	ConstantBufferPerObject* buff = m_Buffer;

	for (size_t i = 0; i < m_Count; i++)
	{
		memcpy(m_CurrentAddress, buff, sizeof(*buff));
		m_CurrentAddress + m_ConstantBufferOffset;
		++buff;
	}

}

void ConstantBufferHelper::FlushBuffer()
{
	ConstantBufferPerObject* buff = m_Buffer;

	for (size_t i = 0; i < m_Count; i++)
	{
		memcpy(m_CurrentAddress, buff, sizeof(*buff));
		m_CurrentAddress + m_ConstantBufferOffset;
		++buff;
	}
	m_CurrentAddress = m_StartAddress;
}

void ConstantBufferHelper::FlushBuffer(int pos)
{
	UINT8* baseAddr = m_StartAddress + (m_ConstantBufferOffset);
	memcpy(baseAddr, m_Buffer, sizeof(*m_Buffer));
}
