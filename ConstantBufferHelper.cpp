#include "ConstantBufferHelper.h"

ConstantBufferHelper::ConstantBufferHelper(std::wstring name)
{
	m_Identifier = name;
}

ConstantBufferHelper::~ConstantBufferHelper()
{
	delete m_Buffer;
	delete[] m_Address;
}

void ConstantBufferHelper::Init(ID3D12Device* device, int blockCount, int classSize, int objectCount, int frameCount)
{
	HRESULT hr;

	UINT64 bC = blockCount;

	m_Count = objectCount;

	m_UploadHeap = std::vector<ID3D12Resource*>(frameCount);

	m_Buffer = new ConstantBufferPerObject();

	m_ConstantBufferOffset = (classSize + 255) & ~255;

	m_Address = new UINT8 * [objectCount];

	for (size_t i = 0; i < frameCount; ++i)
	{
		// create resource for cube 1
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(bC * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_UploadHeap[i]));
		if (FAILED(hr))
		{

		}
		std::wstring heapName = m_Identifier;

		heapName.append(L" Upload Resource Heap");

		heapName.append(std::to_wstring(i));

		m_UploadHeap[i]->SetName(heapName.c_str());

		CD3DX12_RANGE readRange(0, 0);

		hr = m_UploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_Address[i]));

		ZeroMemory(m_Buffer, sizeof(m_Buffer));

		for (size_t j = 0; j < objectCount; ++j)
		{
			memcpy(m_Address[i] + (m_ConstantBufferOffset * j), m_Buffer, sizeof(m_Buffer)); // cube2's constant buffer data
		}
	}
}

void ConstantBufferHelper::FlushBuffer(int frameIndex, int objCount)
{
	ConstantBufferPerObject* buff = m_Buffer;
	memcpy(m_Address[frameIndex] + (m_ConstantBufferOffset * objCount), buff, sizeof(*buff));
}
