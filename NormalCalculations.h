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

		// 1. CALCULATE THE NORMAL
		XMVECTOR vv0 = XMLoadFloat3(&v0.pos.ConvertToXMfloat3());
		XMVECTOR vv1 = XMLoadFloat3(&v1.pos.ConvertToXMfloat3());
		XMVECTOR vv2 = XMLoadFloat3(&v2.pos.ConvertToXMfloat3());

		XMVECTOR P = vv1 - vv0;
		XMVECTOR Q = vv2 - vv0;

		XMVECTOR e01cross = XMVector3Cross(P, Q);
		XMFLOAT3 normalOut;
		XMStoreFloat3(&normalOut, e01cross);
		normal = normalOut;

		// 2. CALCULATE THE TANGENT from texture space

		float s1 = v1.texCoord.ReturnU() - v0.texCoord.ReturnU();
		float t1 = v1.texCoord.ReturnV() - v0.texCoord.ReturnV();
		float s2 = v2.texCoord.ReturnU() - v0.texCoord.ReturnU();
		float t2 = v2.texCoord.ReturnV() - v0.texCoord.ReturnV();


		float tmp = 0.0f;
		if (fabsf(s1 * t2 - s2 * t1) <= 0.0001f)
		{
			tmp = 1.0f;
		}
		else
		{
			tmp = 1.0f / (s1 * t2 - s2 * t1);
		}

		XMFLOAT3 PF3, QF3;
		XMStoreFloat3(&PF3, P);
		XMStoreFloat3(&QF3, Q);

		tangent.x = (t2 * PF3.x - t1 * QF3.x);
		tangent.y = (t2 * PF3.y - t1 * QF3.y);
		tangent.z = (t2 * PF3.z - t1 * QF3.z);

		tangent.x = tangent.x * tmp;
		tangent.y = tangent.y * tmp;
		tangent.z = tangent.z * tmp;

		XMVECTOR vn = XMLoadFloat3(&normal);
		XMVECTOR vt = XMLoadFloat3(&tangent);

		// 3. CALCULATE THE BINORMAL
		// left hand system b = t cross n (rh would be b = n cross t)
		XMVECTOR vb = XMVector3Cross(vt, vn);

		vn = XMVector3Normalize(vn);
		vt = XMVector3Normalize(vt);
		vb = XMVector3Normalize(vb);

		XMStoreFloat3(&normal, vn);
		XMStoreFloat3(&tangent, vt);
		XMStoreFloat3(&binormal, vb);

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


