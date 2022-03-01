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

[domain("quad")]
VS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation,
	const OutputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	VS_OUTPUT Output;


	float3 v1 = lerp(patch[0].pos, patch[1].pos, uv.x);
	float3 v2 = lerp(patch[2].pos, patch[3].pos, uv.x);
	float3 p = lerp(v1, v2, uv.y);

	float3 n1 = lerp(patch[0].norm, patch[1].norm, uv.x);
	float3 n2 = lerp(patch[2].norm, patch[3].norm, uv.x);
	float3 norm = lerp(n1, n2, uv.y);

	float3 t1 = lerp(patch[0].tangent, patch[1].tangent, uv.x);
	float3 t2 = lerp(patch[2].tangent, patch[3].tangent, uv.x);
	float3 tan = lerp(t1, t2, uv.y);

	float3 b1 = lerp(patch[0].biNorm, patch[1].biNorm, uv.x);
	float3 b2 = lerp(patch[2].biNorm, patch[3].biNorm, uv.x);
	float3 binorm = lerp(b1, b2, uv.y);

	float3 tex1 = lerp(patch[0].biNorm, patch[1].biNorm, uv.x);
	float3 tex2 = lerp(patch[2].biNorm, patch[3].biNorm, uv.x);
	float3 tex = lerp(tex1, tex2, uv.y);

	Output.posW = mul(float4(p, 1.0f), WorldPos);
	Output.pos = mul(Output.posW, wvpMat);

	Output.norm = normalize(mul(norm, WorldPos));
	Output.biNorm = normalize(mul(binorm, WorldPos));
	Output.tangent = normalize(mul(tan, WorldPos));
	Output.texCoord = tex;
	Output.ShadowPosH = float4(0.f, 0.f, 0.f, 0.f);
	Output.projTex = float4(0.f, 0.f, 0.f, 0.f);

	return Output;
}
