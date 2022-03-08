#include "Header.hlsli"


cbuffer ConstantBuffer : register(b0)
{
	float4x4 wvpMat;
	//----------------------------------- (16 byte boundary)
	float4x4 WorldPos;
	//----------------------------------- (16 byte boundary)
	float4x4 Projection;
	//----------------------------------- (16 byte boundary)
	float4x4 gShadowTransform;
	//----------------------------------- (16 byte boundary)
	SurfaceInfo Si;
	//----------------------------------- (16 byte boundary)
	Light light;
	//----------------------------------- (16 byte boundary)
	float3 EyePosW;
	int mode;
};

//[domain("quad")]
//VS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
//{
//	VS_OUTPUT Output;
//
//	// Bilinear interpolation.
//	float3 v1 = lerp(patch[0].posL, patch[1].posL, uv.x);
//	float3 v2 = lerp(patch[2].posL, patch[3].posL, uv.x);
//	float3 p = lerp(v1, v2, uv.y);
//
//	// Displacement mapping
//	p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));
//
//	Output.posW = mul(float4(p, 1.0f), WorldPos);
//	Output.pos = mul(Output.posW, wvpMat);
//
//	float3 n1 = lerp(patch[0].normalL, patch[1].normalL, uv.x);
//	float3 n2 = lerp(patch[2].normalL, patch[3].normalL, uv.x);
//	float3 norm = lerp(n1, n2, uv.y);
//
//	float3 t1 = lerp(patch[0].tan, patch[1].tan, uv.x);
//	float3 t2 = lerp(patch[2].tan, patch[3].tan, uv.x);
//	float3 tan = lerp(t1, t2, uv.y);
//
//	float3 b1 = lerp(patch[0].biNorm, patch[1].biNorm, uv.x);
//	float3 b2 = lerp(patch[2].biNorm, patch[3].biNorm, uv.x);
//	float3 binorm = lerp(b1, b2, uv.y);
//
//	float2 tex1 = lerp(patch[0].texCoord, patch[1].texCoord, uv.x);
//	float2 tex2 = lerp(patch[2].texCoord, patch[3].texCoord, uv.x);
//	float2 tex = lerp(tex1, tex2, uv.y);
//
//	Output.norm = norm;
//	Output.biNorm = binorm;
//	Output.tangent = tan;
//	Output.texCoord = tex;
//	Output.ShadowPosH = float4(0.f, 0.f, 0.f, 0.f);
//	Output.projTex = float4(0.f, 0.f, 0.f, 0.f);
//
//	return Output;
//}

// Called once per tessellated vertex
[domain("tri")] // indicates that triangle patches were used
//The original patch is passed in, along with the vertex position //in barycentric coordinates, and the patch constant phase hull //shader output (tessellation factors)
DS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> TrianglePatch)
{
	DS_OUTPUT Output = (DS_OUTPUT)0;

	// Interpolate world space position with barycentric coordinates
	float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].posL + BarycentricCoordinates.y * TrianglePatch[1].posL + BarycentricCoordinates.z * TrianglePatch[2].posL;

	// Interpolate texture coordinates with barycentric coordinates
	Output.texCoord = BarycentricCoordinates.x * TrianglePatch[0].texCoord + BarycentricCoordinates.y * TrianglePatch[1].texCoord;

		// Interpolate normal with barycentric coordinates
	Output.normalW = BarycentricCoordinates.x * TrianglePatch[0].normalL + BarycentricCoordinates.y * TrianglePatch[1].normalL + BarycentricCoordinates.z * TrianglePatch[2].normalL;

	Output.biNorm = BarycentricCoordinates.x * TrianglePatch[0].biNorm + BarycentricCoordinates.y * TrianglePatch[1].biNorm + BarycentricCoordinates.z * TrianglePatch[2].biNorm;

	Output.tan = BarycentricCoordinates.x * TrianglePatch[0].tan + BarycentricCoordinates.y * TrianglePatch[1].tan + BarycentricCoordinates.z * TrianglePatch[2].tan;

	////	// Interpolate light vector with barycentric coordinates
	////	// sample the displacement map for the magnitude of displacement
	//float fDisplacement = g_DisplacementMap.SampleLevel(g_sampleLinear, Out.vTexCoord.xy, 0).r; 
	//fDisplacement *= g_Scale; fDisplacement += g_Bias;
	//float3 vDirection = -vNormal; // direction is opposite normal

	////// translate the position
	//vWorldPos += vDirection * fDisplacement;

	//// transform to clip space
	//Out.vPosCS = mul(float4(vWorldPos.xyz, 1.0), g_mWorldViewProjection);

// Displacement mapping
	//vWorldPos.y = 0.3f * (vWorldPos.z * sin(vWorldPos.x) + vWorldPos.x * cos(vWorldPos.z));


	Output.normalW = normalize(mul(float4(Output.normalW, 0), WorldPos));
	Output.biNorm = normalize(mul(float4(Output.biNorm, 0), WorldPos));
	Output.tan = normalize(mul(float4(Output.tan, 0), WorldPos));

	Output.TBN = float3x3(Output.tan, Output.biNorm, Output.normalW);
	Output.TBN_inv = transpose(Output.TBN);

	float3 EyePosWorld = normalize(EyePosW - Output.posW.xyz);
	float3 LightVectorWorld = normalize(light.LightVecW - Output.posW.xyz);

	Output.eyeVectorTS = VectorToTangentSpace(EyePosWorld, Output.TBN_inv);
	Output.lightVectorTS = VectorToTangentSpace(LightVectorWorld, Output.TBN_inv);


	Output.posW = mul(vWorldPos, WorldPos);
	Output.pos = mul(Output.posW, wvpMat);

	Output.ShadowPosH = float4(0.f, 0.f, 0.f, 0.f);
	Output.projTex = float4(0.f, 0.f, 0.f, 0.f);

	return Output;
}
