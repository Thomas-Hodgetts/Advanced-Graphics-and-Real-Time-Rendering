#include "Apperance.h"

Apperance::Apperance(Geometry geometry, Material material, ID3D11ShaderResourceView* tex) : m_Geometry(geometry), m_Material(material), m_TextureRV(tex)
{

}

Apperance::~Apperance()
{
	delete m_TextureRV;
	m_TextureRV = nullptr;
}

void Apperance::Draw(ID3D11DeviceContext* pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers

	//pImmediateContext->IASetVertexBuffers(0, 1, &m_Geometry.vertexBuffer, &m_Geometry.vertexBufferStride, &m_Geometry.vertexBufferOffset);
	//pImmediateContext->IASetIndexBuffer(m_Geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//pImmediateContext->DrawIndexed(m_Geometry.numberOfIndices, 0, 0);
}