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
	float4 ShadowPosH : POSITION0;
	float2 texCoord: TEXCOORD;
	float4 projTex: TEXCOORD1;
	float3 EyePosW : TANGENT2;
	Light l : LIGHTDATA;
	SurfaceInfo s : SURFACEINFO;
	float3 LightVecW : TANGENT1;
	float3 norm : NORMAL;
	float3 biNorm : TANGENT3;
	float3 tangent : TANGENT4;
	float3 irradiance : IRADIANCE;
	float4x4 worldMat : WORLDMAT;
	int mode : MODE;
};

struct VS_SHADOW
{
	float4 posH : SV_POSITION;
	float2 texCoord: TEXCOORD;
	float4 DiffuseMtrl : MATERIAL;
};

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

/***********************************************
MARKING SCHEME: Global Illumination (NON FUNCTIONING)
DESCRIPTION:
***********************************************/
void accumLightSH(float3 sharm[9], float3 colour, float3 n, float solAngle)
{
	float xy = n.x * n.y;
	float yz = n.y * n.z;
	float xz = n.x * n.z;

	float x2 = n.x * n.x;
	float y2 = n.y * n.y;
	float z2 = n.z * n.z;

	float3 Y00 = 0.282095;
	float3 Y1_1 = 0.488603 * n.y;
	float3 Y10 = 0.488603 * n.z;
	float3 Y11 = 0.488603 * n.x;
	float3 Y2_2 = 1.092548 * xy;
	float3 Y2_1 = 1.092548 * yz;
	float3 Y20 = 0.315392 * (3 * z2 - 1);
	float3 Y21 = 1.092548 * xz;
	float3 Y22 = 0.546274 * (x2 - y2);

	sharm[0] += colour * Y00 * solAngle;
	sharm[1] += colour * Y1_1 * solAngle;
	sharm[2] += colour * Y10 * solAngle;
	sharm[3] += colour * Y11 * solAngle;
	sharm[4] += colour * Y2_2 * solAngle;
	sharm[5] += colour * Y2_1 * solAngle;
	sharm[6] += colour * Y20 * solAngle;
	sharm[7] += colour * Y21 * solAngle;
	sharm[8] += colour * Y22 * solAngle;
}


float3 ISHTirradiance(float3 sharm[9], float3 n)
{
	float3 L00 = sharm[0];
	float3 L1_1 = sharm[1];
	float3 L10 = sharm[2];
	float3 L11 = sharm[3];
	float3 L2_2 = sharm[4];
	float3 L2_1 = sharm[5];
	float3 L20 = sharm[6];
	float3 L21 = sharm[7];
	float3 L22 = sharm[8];

	float c1 = 0.429043;
	float c2 = 0.511664;
	float c3 = 0.743125;
	float c4 = 0.886227;
	float c5 = 0.247708;

	float xy = n.x * n.y;
	float yz = n.y * n.z;
	float xz = n.x * n.z;

	float x2 = n.x * n.x;
	float y2 = n.y * n.y;
	float z2 = n.z * n.z;

	float3 irradiance = c1 * L22 * (x2 - y2) + c3 * L20 * z2 + c4 * L00 - c5 * L20 + 2 * c1 * (L2_2 * xy + L21 * xz + L2_1 * yz) + 2 * c2 * (L11 * n.x + L1_1 * n.y + L10 * n.z);

	return irradiance;
}


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(input.pos, wvpMat);
	output.posW = mul(input.pos, WorldPos);
	output.texCoord = input.texCoord;
	output.ShadowPosH = mul(output.posW, gShadowTransform);
	//output.projTex = mul(float4(light.LightVecW, 1.0f), gLightWorldViewProjTexture);
	output.norm = normalize(mul(input.normalL, WorldPos));
	output.biNorm = normalize(mul(input.biNorm, WorldPos));
	output.tangent = normalize(mul(input.tan, WorldPos));
	output.l.AmbientLight = light.AmbientLight;
	output.l.DiffuseLight = light.DiffuseLight;
	output.l.SpecularLight = light.SpecularLight;
	output.l.SpecularPower = light.SpecularPower;
	output.s.AmbientMtrl = Si.AmbientMtrl;
	output.s.DiffuseMtrl = Si.DiffuseMtrl;
	output.s.SpecularMtrl = Si.SpecularMtrl;
	output.LightVecW = normalize(light.LightVecW - output.posW.xyz);
	output.EyePosW = normalize(EyePosW - output.posW.xyz);
	output.worldMat = wvpMat;
	output.mode = mode;

	//float3 Harmonics[9];
	//accumLightSH(Harmonics, (255, 255, 255), output.norm, );
	//output.irradiance = ISHTirradiance(Harmonics, bumpMap);
	return output;
} 

VS_OUTPUT renderToTexMain(VS_INPUT2 input)
{
	VS_OUTPUT output;
	output.pos = mul(input.pos, wvpMat);
	output.texCoord = input.texCoord;
	return output;
}


VS_SHADOW shadowMain(VS_INPUT input)
{
	VS_SHADOW output;
	output.posH = mul(input.pos, WorldPos);
	output.posH = mul(output.posH, wvpMat);
	output.texCoord = input.texCoord;
	output.DiffuseMtrl = Si.DiffuseMtrl;
	return output;
}