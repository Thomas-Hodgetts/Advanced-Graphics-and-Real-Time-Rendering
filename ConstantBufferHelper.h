#pragma once
#include "Structures.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class ConstantBufferHelper
{
public:
	ConstantBufferHelper(std::wstring name);
	~ConstantBufferHelper();

	void Init(ID3D12Device* device, int blockCount, int classSize, int objectCount, int frameCount);

	ConstantBufferPerObject* GetBuffer() { return m_Buffer; };

	ID3D12Resource* GetHeapPointer(int frameIndex) { return m_UploadHeap[frameIndex]; };

	void FlushBuffer(int frameIndex, int objCount);

private:

	std::wstring m_Identifier;

	std::vector<ID3D12Resource*> m_UploadHeap;

	ConstantBufferPerObject* m_Buffer;

	UINT8** m_Address;

	int m_ConstantBufferOffset = 0;
	int m_Count = 0;

};

