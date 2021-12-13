#pragma once
#include "d3dx12.h"
#include <DirectXMath.h>
#include <d3d11_1.h>

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* device, UINT width, UINT height);
	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap() = default;

	UINT Width()const;
	UINT Height()const;

	ID3D12Resource* Resource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv();
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv();

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT Rect()const;

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

	void Update();

private:
	void BuildDescriptors();
	void BuildResource();

	ID3D12Device* m_Device = nullptr;


	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Rect;


	UINT m_Width;
	UINT m_Height;


	DXGI_FORMAT m_Format = DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_HGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HCpuDsv;

	ID3D12Resource* m_ShadowMap;
};

