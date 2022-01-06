#pragma once
#include "Structures.h"

class NormalCalculations
{
public:
	NormalCalculations();
	~NormalCalculations();

	static void CalculateObjectNormals(Vertex* vertices, DWORD* indices, int triangles)
	{
		for (size_t i = 0; i < triangles; i++)
		{
			UINT i0 = indices[i * 3 + 0];
			UINT i1 = indices[i * 3 + 1];
			UINT i2 = indices[i * 3 + 2];

			Vertex v0 = vertices[i0];
			Vertex v1 = vertices[i1];
			Vertex v2 = vertices[i2];

			Vector3D faceNormal = CalculateNormal(v0.pos.ConvertToXMvector3(), v1.pos.ConvertToXMvector3(), v2.pos.ConvertToXMvector3());

			vertices[i0].Normal += faceNormal;
			vertices[i1].Normal += faceNormal;
			vertices[i2].Normal += faceNormal;

		}

	}

	static XMVECTOR CalculateNormal(FXMVECTOR p0, FXMVECTOR p1, FXMVECTOR p2)
	{
		XMVECTOR u = p1 - p0;
		XMVECTOR v = p2 - p0;
		return XMVector3Normalize(XMVector3Cross(u, v));
	}

	static void CalculateTangentBinormal2(Vertex v0, Vertex v1, Vertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal)
	{
		// http://softimage.wiki.softimage.com/xsidocs/tex_tangents_binormals_AboutTangentsandBinormals.html
		
		XMFLOAT3 edge1(Vector3D(v1.pos - v0.pos).ConvertToXMfloat3());
		XMFLOAT3 edge2(Vector3D(v2.pos - v0.pos).ConvertToXMfloat3());

		XMFLOAT2 deltaUV1(v1.texCoord.ReturnU() - v0.texCoord.ReturnU(), v1.texCoord.ReturnV() - v0.texCoord.ReturnV());
		XMFLOAT2 deltaUV2(v2.texCoord.ReturnU() - v0.texCoord.ReturnU(), v2.texCoord.ReturnV() - v0.texCoord.ReturnV());

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		XMVECTOR tn = XMLoadFloat3(&tangent);
		tn = XMVector3Normalize(tn);
		XMStoreFloat3(&tangent, tn);

		binormal.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		binormal.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		binormal.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		tn = XMLoadFloat3(&binormal);
		tn = XMVector3Normalize(tn);
		XMStoreFloat3(&binormal, tn);


		XMVECTOR vv0 = XMLoadFloat3(&v0.pos.ConvertToXMfloat3());
		XMVECTOR vv1 = XMLoadFloat3(&v1.pos.ConvertToXMfloat3());
		XMVECTOR vv2 = XMLoadFloat3(&v2.pos.ConvertToXMfloat3());

		XMVECTOR e0 = vv1 - vv0;
		XMVECTOR e1 = vv2 - vv0;

		XMVECTOR e01cross = XMVector3Cross(e0, e1);
		e01cross = XMVector3Normalize(e01cross);
		XMFLOAT3 normalOut;
		XMStoreFloat3(&normalOut, e01cross);
		normal = normalOut;
		return;
	}


	// IMPORTANT NOTE!!
	// NOTE!! - this assumes each face is using its own vertices (no shared vertices)
	// so the index file would look like 0,1,2,3,4 and so on
	// it won't work with shared vertices as the tangent / binormal for a vertex is related to a specific face
	// REFERENCE this has largely been modified from "Mathematics for 3D Game Programmming and Computer Graphics" by Eric Lengyel
	static void CalculateModelVectors(Vertex* vertices, int vertexCount)
	{
		int faceCount, i, index;
		Vertex vertex1, vertex2, vertex3;
		XMFLOAT3 tangent, binormal, normal;


		// Calculate the number of faces in the model.
		faceCount = vertexCount / 3;

		// Initialize the index to the model data.
		index = 0;

		// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
		for (i = 0; i < faceCount; i++)
		{
			vertex1 = vertices[index];
			index++;
			vertex2 = vertices[index];
			index++;
			vertex3 = vertices[index];
			index++;

			// Calculate the tangent and binormal of that face.
			CalculateTangentBinormal2(vertex1, vertex2, vertex3, normal, tangent, binormal);

			// Store the normal, tangent, and binormal for this face back in the model structure.
			vertices[index - 1].Normal.SetX(normal.x);
			vertices[index - 1].Normal.SetY(normal.y);
			vertices[index - 1].Normal.SetZ(normal.z);
			vertices[index - 1].tangent.SetX(tangent.x);
			vertices[index - 1].tangent.SetY(tangent.y);
			vertices[index - 1].tangent.SetZ(tangent.z);
			vertices[index - 1].biTangent.SetX(binormal.x);
			vertices[index - 1].biTangent.SetY(binormal.y);
			vertices[index - 1].biTangent.SetZ(binormal.z);

			vertices[index - 2].Normal.SetX(normal.x);
			vertices[index - 2].Normal.SetY(normal.y);
			vertices[index - 2].Normal.SetZ(normal.z);
			vertices[index - 2].tangent.SetX(tangent.x);
			vertices[index - 2].tangent.SetY(tangent.y);
			vertices[index - 2].tangent.SetZ(tangent.z);
			vertices[index - 2].biTangent.SetX(binormal.x);
			vertices[index - 2].biTangent.SetY(binormal.y);
			vertices[index - 2].biTangent.SetZ(binormal.z);

			vertices[index - 3].Normal.SetX(normal.x);
			vertices[index - 3].Normal.SetY(normal.y);
			vertices[index - 3].Normal.SetZ(normal.z);
			vertices[index - 3].tangent.SetX(tangent.x);
			vertices[index - 3].tangent.SetY(tangent.y);
			vertices[index - 3].tangent.SetZ(tangent.z);
			vertices[index - 3].biTangent.SetX(binormal.x);
			vertices[index - 3].biTangent.SetY(binormal.y);
			vertices[index - 3].biTangent.SetZ(binormal.z);
		}

	}
private:

};


