#define NUM_CONTROL_POINTS 4

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 posL: POSL;
	float3 posW: POSW;
	float2 texCoord: TEXCOORD;
	float3 normalL: NORMAL;
	float3 tan: TANGENT;
	float3 biNorm: BINORMAL;
};

struct DS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 posL: POSL;
	float3 posW: POSW;
	float2 texCoord: TEXCOORD;
	float3 normalW: NORMAL;
	float3 tan: TANGENT;
	float3 biNorm: BINORMAL;
	float3x3 TBN : TBN_MATRIX;
	float3x3 TBN_inv : INV_TBN_MATRIX;
	float3 eyeVectorTS : TANGENT_SPACE_VAR;
	float3 lightVectorTS : TANGENT_SPACE_VAR1;	
	float4 ShadowPosH : POSITION;
	float4 projTex: TEXCOORD1;
	float distance : DISTANCE;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	float distance : DISTANCE;
};

struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord: TEXCOORD;
	float3 normalL: NORMAL;
	float3 tan: TANGENT;
	float3 biNorm: BINORMAL;
};

struct VS_INPUT2
{
	float4 pos : POSITION;
	float2 texCoord: TEXCOORD;
};

struct VS_OUTPUT2
{
	float4 pos : SV_POSITION;
	float2 texCoord: TEXCOORD;
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
	float4 posL: POSL;
	float3 posW: POSW;
	float4 ShadowPosH : POSITION;
	float2 texCoord: TEXCOORD;
	float4 projTex: TEXCOORD1;
	float3 norm : NORMAL;
	float3 biNorm : TANGENT3;
	float3 tangent : TANGENT4;
};

struct VS_SHADOW
{
	float4 posH : SV_POSITION;
	float2 texCoord: TEXCOORD;
	float4 DiffuseMtrl : POSITION0;
};

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

struct GSOutput
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 norm : NORMAL;
	float2 texCoord : TEXCOORD;
};

/***********************************************
MARKING SCHEME: Normal Mapping
DESCRIPTION: Transposing Vectors into Tangent Space
***********************************************/
float3 VectorToTangentSpace(float3 vectorV, float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}