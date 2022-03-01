#include "Header.hlsli"

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float4 vPosition : WORLDPOS;
	// TODO: change/add other stuff
};

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = 3;
	Output.EdgeTessFactor[1] = 3;
	Output.EdgeTessFactor[2] = 3;
	Output.EdgeTessFactor[3] = 3;
	Output.InsideTessFactor[0] = 3; // e.g. could calculate dynamic tessellation factors instead
	Output.InsideTessFactor[1] = 3; // e.g. could calculate dynamic tessellation factors instead

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.f)]
VS_OUTPUT main(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	VS_OUTPUT Output;
	//Output = ip[i];
	Output.pos = ip[i].pos;
	Output.posW = ip[i].posW;
	Output.norm = ip[i].norm;
	Output.biNorm = ip[i].biNorm;
	Output.tangent = ip[i].tangent;
	Output.texCoord = ip[i].texCoord;
	return Output;
}
