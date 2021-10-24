struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord: TEXCOORD;
	float2 normalL: NORMAL;
	float3 tan: TANGENT;
	float3 biNorm: BINORMAL;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD;
	float3 NormalW : NORMAL;
	float3 PosL : POSITION;
	float3 Tan: TANGENT;
	float3 BiNorm: BINORMAL;
	float4 AmbientLight : POSITION1;
	float3 EyePosW : TANGENT2;
	float4 DiffuseLight: POSITION2;
	float4 SpecularLight: POSITION3;
	float4 AmbientMtrl: POSITION4;
	float4 DiffuseMtrl: POSITION5;
	float4 SpecularMtrl: POSITION6;
	float SpecularPower : PSIZE;
	float3 LightVecW : TANGENT1;
	float3 eyeVectorTS : TANGENT4;
	float3 lightVectorTS : TANGENT3;
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

cbuffer ConstantBuffer : register(b0)
{
	float4x4 wvpMat;
	float4x4 View;
	float4x4 Projection;

	SurfaceInfo Si;
	Light light;
	float3 EyePosW;
};

float3 VectorToTangentSpace(float3 vectorV,float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(input.pos, wvpMat);
	float4 WorldPos = output.pos;
	output.PosL = mul(input.pos, View);
	output.PosL = mul(input.pos, Projection);

	float3 T = normalize(mul(input.tan, wvpMat));
	float3 B = normalize(mul(input.biNorm, wvpMat));
	float3 N = normalize(mul(input.normalL, wvpMat));
	float3x3 TBN = float3x3(T, B, N);
	float3x3 TBN_inv = transpose(TBN);

	output.texCoord = input.texCoord;
	output.Tan = input.tan;
	output.BiNorm = input.biNorm;
	output.NormalW = mul(input.normalL, (float4x4)wvpMat);
	output.AmbientLight = light.AmbientLight;
	output.DiffuseLight = light.DiffuseLight;
	output.SpecularLight = light.SpecularLight;
	output.AmbientMtrl = Si.AmbientMtrl;
	output.DiffuseMtrl = Si.DiffuseMtrl;
	output.SpecularMtrl = Si.SpecularMtrl;
	output.SpecularPower = light.SpecularPower;
	output.LightVecW = light.LightVecW - WorldPos;
	output.EyePosW = EyePosW - WorldPos;
	output.eyeVectorTS = VectorToTangentSpace(output.EyePosW.xyz, TBN_inv);
	output.lightVectorTS = VectorToTangentSpace(output.LightVecW.xyz, TBN_inv);
	return output;
}