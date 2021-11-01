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
	float3 normalW: NORMAL;
	float3 EyePosW : TANGENT2;
	Light l : LIGHTDATA;
	SurfaceInfo s : SURFACEINFO;
	float3 LightVecW : TANGENT1;
	float3x3 TBN : TBN;
	float3 eyeVectorTS : TANGENT4;
	float3 lightVectorTS : TANGENT3;
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

float3 VectorToTangentSpace(float3 vectorV,float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(input.pos, wvpMat);
	output.PosL = mul(input.pos, View);
	output.PosL = mul(input.pos, Projection);

	float4 WorldPos = output.pos;
	float4 outputs = normalize(mul(float4(input.normalL, 0.0f), wvpMat));
	output.normalW = outputs.xyz;

	float3 T = normalize(mul(input.tan, wvpMat));
	float3 B = normalize(mul(input.biNorm, wvpMat));
	float3 N = normalize(mul(input.normalL, wvpMat));
	float3x3 TBN = float3x3(T, B, N);
	float3x3 TBN_inv = transpose(TBN);
	output.TBN = TBN;
	output.texCoord = input.texCoord;
	output.l.AmbientLight = light.AmbientLight;
	output.l.DiffuseLight = light.DiffuseLight;
	output.l.SpecularLight = light.SpecularLight;
	output.s.AmbientMtrl = Si.AmbientMtrl;
	output.s.DiffuseMtrl = Si.DiffuseMtrl;
	output.s.SpecularMtrl = Si.SpecularMtrl;
	output.l.SpecularPower = light.SpecularPower;
	output.LightVecW = light.LightVecW - WorldPos;
	output.EyePosW = EyePosW - WorldPos;
	output.eyeVectorTS = VectorToTangentSpace(output.EyePosW.xyz, TBN_inv);
	output.lightVectorTS = VectorToTangentSpace(output.LightVecW.xyz, TBN_inv);
	output.worldMat = wvpMat;
	return output;
}