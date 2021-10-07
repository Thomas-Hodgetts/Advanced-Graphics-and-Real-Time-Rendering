#include "Apperance.h"

Apperance::Apperance(Geometry geometry, Material material, ID3D11ShaderResourceView* tex) : m_Geometry(geometry), m_Material(material), m_TextureRV(tex)
{

}

Apperance::~Apperance()
{
	delete m_TextureRV;
	m_TextureRV = nullptr;
}

void Apperance::Draw(DrawObjectsStruct DOS)
{
	DOS.commandList->RSSetViewports(1, DOS.viewport); // set the viewports
	DOS.commandList->RSSetScissorRects(1, DOS.sisRect); // set the scissor rects
	DOS.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	DOS.commandList->IASetVertexBuffers(0, 1, &m_Geometry.vertexBufferView); // set the vertex buffer (using the vertex buffer view)
	DOS.commandList->IASetIndexBuffer(&m_Geometry.indexBufferView);

	//constant buffer
	DOS.commandList->SetGraphicsRootConstantBufferView(0, DOS.constantBufferUploadHeaps[DOS.frameIndex].GetGPUVirtualAddress() + constantBufferOffset);
	// draw
	DOS.commandList->DrawIndexedInstanced(m_Geometry.numberOfIndices, 1, 0, 0, 0);
}

void Apperance::SetSubResources()
{
}
