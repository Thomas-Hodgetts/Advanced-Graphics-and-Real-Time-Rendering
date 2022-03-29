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
	//----------------------------------- (16 byte boundary)
	int TessDistanceNear;
	int TessDistanceFar;
	int mode3;
	int mode4;
};

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<VS_OUTPUT, 3> ip, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float3 centerL = 0.25f * (ip[0].posL + ip[1].posL + ip[2].posL);
	float3 centerW = mul(float4(centerL, 1.0f), WorldPos).xyz;

	float d = distance(centerW, EyePosW);

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.

	const float d0 = TessDistanceNear;
	const float d1 = TessDistanceFar;

	//mess aroound with the 64 num
	float tess = 8.0f * saturate((d1 - d) / (d1 - d0)) + 1;

	Output.distance = d;

	// Uniformly tessellate the patch.
	Output.EdgeTessFactor[0] = tess;
	Output.EdgeTessFactor[1] = tess;
	Output.EdgeTessFactor[2] = tess;

	Output.InsideTessFactor = tess;

	return Output;
}

// Called once per control point // indicates a triangle patch (3 verts) [partitioning("fractional_odd")] // fractional avoids popping
// vertex ordering for the output triangles
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")][outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(7.0)] //hint to the driver – the lower the better
// Pass in the input patch and an index for the control point
HS_CONTROL_POINT_OUTPUT main(InputPatch<VS_OUTPUT, 3> inputPatch, uint uCPID : SV_OutputControlPointID)
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.pos = inputPatch[uCPID].pos;
	Output.posL = inputPatch[uCPID].posL;
	Output.posW = inputPatch[uCPID].posW;
	Output.texCoord = inputPatch[uCPID].texCoord;
	Output.normalL = inputPatch[uCPID].norm;
	Output.tan = inputPatch[uCPID].tangent;
	Output.biNorm = inputPatch[uCPID].biNorm;
	return Output;
}
