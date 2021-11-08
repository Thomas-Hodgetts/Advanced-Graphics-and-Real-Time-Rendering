struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord: TEXCOORD;
	float3 normalL: NORMAL;
	float3 tan: TANGENT;
	float3 biNorm: BINORMAL;
};

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;

};

struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD;
	float3 PosL : POSITION;
	float3 EyePosW : TANGENT2;
	Light l : LIGHTDATA;
	SurfaceInfo s : SURFACEINFO;
	float3 LightVecW : TANGENT1;
	float3 biNorm : TANGENT3;
	float3 tangent : TANGENT4;
	float4x4 worldMat : WORLDMAT;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 wvpMat;
	float4x4 View;
	float4x4 Projection;

	SurfaceInfo Si;
	Light light;
	float3 EyePosW;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(input.pos, wvpMat);
	output.PosL = mul(input.pos, View);
	output.PosL = mul(input.pos, Projection);
	output.texCoord = input.texCoord;
	output.biNorm = normalize(mul(input.biNorm, wvpMat));
	output.tangent = normalize(mul(input.tan, wvpMat));
	output.l.AmbientLight = light.AmbientLight;
	output.l.DiffuseLight = light.DiffuseLight;
	output.l.SpecularLight = light.SpecularLight;
	output.s.AmbientMtrl = Si.AmbientMtrl;
	output.s.DiffuseMtrl = Si.DiffuseMtrl;
	output.s.SpecularMtrl = Si.SpecularMtrl;
	output.l.SpecularPower = light.SpecularPower;
	output.LightVecW = light.LightVecW - output.pos;
	output.EyePosW = EyePosW - output.pos;
	output.worldMat = wvpMat;
	return output;
} 