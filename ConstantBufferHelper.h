#pragma once
#include "Structures.h"


class ConstantBufferHelper
{
public:
	ConstantBufferHelper(std::wstring name);
	~ConstantBufferHelper();

	void Init(ID3D12Device* device, int blockCount, int classSize, int objectCount);

	ConstantBufferPerObject* GetBuffer() { return m_Buffer; };

	void FlushBuffer();

private:

	std::wstring m_Identifier;

	ID3D12Resource* m_UploadHeap;

	ConstantBufferPerObject* m_Buffer;

	UINT8* m_StartAddress;
	UINT8* m_CurrentAddress;

	int m_ConstantBufferOffset = 0;
	int m_Count = 0;

};

