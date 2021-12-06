#include "ShadowMaps.h"

ShadowMap::ShadowMap(ID3D12Device* device, UINT width, UINT height)
{
	m_Width = width;
	m_Height = height;
	m_Device = device;

	m_Viewport = { 0.0f,0.0f, (float)width , (float)height, 0.0f, 1.0f };
	m_Rect = {0,0, (int)width, (int)height};

	BuildResource();
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0.f;

	HRESULT hr;
	hr = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear, IID_PPV_ARGS(&m_ShadowMap));
	if (FAILED(hr))
	{
		
	}
}

ID3D12Resource* ShadowMap::Resource()
{
	return m_ShadowMap;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMap::Srv()
{
	return m_HGpuSrv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv()
{
	return m_HCpuDsv;
}

D3D12_VIEWPORT ShadowMap::Viewport() const
{
	return m_Viewport;
}

D3D12_RECT ShadowMap::Rect() const
{
	return m_Rect;
}
