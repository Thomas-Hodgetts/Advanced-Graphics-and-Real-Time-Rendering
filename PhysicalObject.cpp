#include "PhysicalObject.h"

PhysicalObject::PhysicalObject()
{
	m_Transform = nullptr;
}

PhysicalObject::~PhysicalObject()
{
}

XMFLOAT4X4 PhysicalObject::ReturnWorld()
{
	return XMFLOAT4X4();
}

MeshData PhysicalObject::ReturnMeshData()
{
	return m_Mesh;
}

bool PhysicalObject::CreateObject(MeshData MD, ID3D11ShaderResourceView* TD, XMFLOAT4X4 WD)
{
	m_Mesh = MD;
	//m_Texture = TD;
	//m_ObjectTransform = WD;
	return true;
}



void PhysicalObject::Draw(ID3D11DeviceContext* IC, ID3D11Buffer* _pCB, ConstantBuffer cb)
{
	/*
	//Sets the Texture to the local one
	if (m_Transparency)
	{
		IC->OMSetBlendState(m_Transparency, m_BlendFactor, 0xffffffff);
	}
	else
	{
		IC->OMSetBlendState(0, 0, 0xffffffff);
	}

	//Renders the Frame
	IC->PSSetShader(m_PixelShader, nullptr, 0);
	IC->VSSetShader(m_VertexShader, nullptr, 0);
	IC->PSSetShaderResources(0, 1, &m_Texture);
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&m_ObjectTransform));
	IC->UpdateSubresource(_pCB, 0, nullptr, &cb, 0, 0);
	IC->IASetVertexBuffers(0, 1, &m_Mesh.VertexBuffer, &m_Mesh.VBStride, &m_Mesh.VBOffset);
	IC->IASetIndexBuffer(m_Mesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	IC->DrawIndexed(m_Mesh.IndexCount, 0, 0);
	*/
}

void PhysicalObject::Update()
{

}