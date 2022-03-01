#define NUM_CONTROL_POINTS 4

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
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
	float3 posW: POSW;
	float4 ShadowPosH : POSITION;
	float2 texCoord: TEXCOORD;
	float4 projTex: TEXCOORD1;
	float3 norm : NORMAL;
	float3 biNorm : TANGENT3;
	float3 tangent : TANGENT4;
	float3 irradiance : IRADIANCE;
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

