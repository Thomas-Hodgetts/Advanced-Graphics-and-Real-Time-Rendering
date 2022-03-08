Texture2D t1 : register(t0);
Texture2D heightMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D gShadowMap : register(t3);
SamplerState s1 : register(s0);
SamplerComparisonState gsamShadow : register(s1);

#include "Header.hlsli"

/***********************************************
MARKING SCHEME: Normal Mapping
DESCRIPTION: Transposing Vectors into Tangent Space
***********************************************/
float3 VectorToTangentSpace(float3 vectorV, float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}

/***********************************************
MARKING SCHEME: Shadow Mapping
DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space
***********************************************/
float CalcShadowFactor(float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
	float width = 0;
	float height = 0;
	float numMips = 0;
	gShadowMap.GetDimensions(0, width, height, numMips);
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;


	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx), float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f), float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit / 9.0f;
}



/***********************************************
MARKING SCHEME: Normal Mapping
DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space
***********************************************/
float4 ProcessBumpMap(float4 Sample, float3x3 tbn, int flip)
{
	if (0)
	{
		float4 bumpMap = Sample;
		bumpMap = (bumpMap * 2.0f) - 1.0f;
		bumpMap = float4(normalize(bumpMap.xyz), 1);
		bumpMap.rgb = normalize(mul(bumpMap, tbn));
		return bumpMap;
	}
	else
	{
		float4 bumpMap = Sample;
		bumpMap = -bumpMap;
		bumpMap = (bumpMap * 2.0f) - 1.0f;
		bumpMap = float4(normalize(bumpMap.xyz), 1);
		bumpMap.rgb = normalize(mul(bumpMap, tbn));
		return bumpMap;
	}
}

/***********************************************
MARKING SCHEME: Parallax Mapping
DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space
***********************************************/
float2 ParallaxMapping(float2 texCoords, float3 viewDir)
{
	float height_scale = 0.04f;
	float height = -heightMap.Sample(s1, texCoords).x;
	float2 p = viewDir.xy / viewDir.z * (height * height_scale);
	return texCoords - p;
}


/***********************************************
MARKING SCHEME: Parallax Mapping
DESCRIPTION: Self Shadowing
***********************************************/

float parallaxSoftShadowMultiplier(float3 L, float2 initTex, float initHieght)
{
	float shadowMultiplier = 1;
	const float minLayers = 15;
	const float maxLayers = 30;

	if (dot(float3(0, 0, 1), L) > 0)
	{
		float numSamplesUnderSurface = 0;
		shadowMultiplier = 0;
		float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0, 0, 1), L)));
		float layerHeight = initHieght / numLayers;
		float2 texStep = 0.1 * L.xy / L.z / numLayers;

		float currentLayerHeight = initHieght - layerHeight;
		float2 currentTextureCoords = initTex + texStep;

		int stepIndex = 1;

		while (currentLayerHeight > 0)
		{
			float heightFromTexture = currentLayerHeight - initTex.y;
			if (heightFromTexture < currentLayerHeight)
			{
				numSamplesUnderSurface += 1;
				float newShadowMultiplier = (currentLayerHeight - heightFromTexture) * (1.0 - stepIndex / numLayers);
				shadowMultiplier = max(shadowMultiplier, newShadowMultiplier);
			}
			stepIndex += 1;
			currentLayerHeight -= layerHeight;
			currentTextureCoords += texStep;
		}

		if (numSamplesUnderSurface < 1)
		{
			shadowMultiplier = 1;
		}
		else
		{
			shadowMultiplier = 1.0 - shadowMultiplier;
		}
	}
	return shadowMultiplier;
}

LightingResult ComputeSimpleLighting(float3 toEye, float3 Norm, float3 LightVecW, SurfaceInfo surfInfo, Light light, float3 ShadowFactor)
{
	LightingResult lr;

	float3 r = reflect(-LightVecW, Norm);

	float diffuseAmount = max(dot(LightVecW, Norm), 0.0f);
	lr.Diffuse.rgb = ShadowFactor * diffuseAmount * (surfInfo.DiffuseMtrl * light.DiffuseLight);
	lr.Diffuse.a = ShadowFactor * diffuseAmount * (surfInfo.DiffuseMtrl * light.DiffuseLight).a;

	float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);
	lr.Specular.rgb = ShadowFactor * specularAmount * (surfInfo.SpecularMtrl * light.SpecularLight);
	lr.Specular.a = ShadowFactor * specularAmount * (surfInfo.SpecularMtrl * light.SpecularLight).a;

	return lr;
}

struct VS_OUTPUT_3
{
	float4 pos: SV_POSITION;
	float3 posW: POSW;
	float4 ShadowPosH : POSITION0;
	float2 texCoord: TEXCOORD;
	float4 projTex: TEXCOORD1;
	float3 EyePosW : TANGENT2;
	float3 LightVecW : TANGENT1;
	float3 norm : NORMAL;
	float3 biNorm : TANGENT3;
	float3 tangent : TANGENT4;
	float3 irradiance : IRADIANCE;
	float4x4 worldMat : WORLDMAT;
	int mode : MODE;
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

float4 main(VS_OUTPUT_3 input) : SV_TARGET
{

	float3 lightLecNorm;
	float3 r;
	float specularAmount;
	float diffuseAmount;

	input.posW = mul(input.posW, WorldPos);
	input.pos = mul(input.posW, wvpMat);
	input.norm = normalize(mul(input.norm, WorldPos));
	input.biNorm = normalize(mul(input.biNorm, WorldPos));
	input.tangent = normalize(mul(input.tangent, WorldPos));

	float3x3 TBN = float3x3(input.tangent, input.biNorm, input.norm);
	float3x3 TBN_inv = transpose(TBN);

	float3 eyeVectorTS = VectorToTangentSpace(input.EyePosW.xyz, TBN_inv);
	float3 lightVectorTS = VectorToTangentSpace(input.LightVecW.xyz, TBN_inv);
	float3 normalTS = VectorToTangentSpace(input.norm, TBN_inv);

	float3 toEye = input.EyePosW;
	float3 toEyeTS = VectorToTangentSpace(toEye, TBN_inv);
	float2 texCoords = input.texCoord;
	float4 bumpMap;
	float shadowFactor = 1;
	float3 shadowFactor2 = float3(1.f,1.f,1.f);

	LightingResult lr;

	if (input.mode == 0)
	{
		texCoords = ParallaxMapping(input.texCoord, toEyeTS);

		bumpMap = ProcessBumpMap(normalMap.Sample(s1, input.texCoord),TBN, 1);

		//shadowFactor = parallaxSoftShadowMultiplier(lightVectorTS, texCoords, heightMap.Sample(s1, texCoords).x);
		
		lr = ComputeSimpleLighting(eyeVectorTS, bumpMap, lightVectorTS, Si, light, shadowFactor);

		if (texCoords.x > 1 || texCoords.x < 0 || texCoords.y > 1 || texCoords.y < 0)
		{
			discard;
		}

	}
	if (input.mode == 1)
	{
		bumpMap = ProcessBumpMap(normalMap.Sample(s1, texCoords), TBN, 0);

		lr = ComputeSimpleLighting(toEyeTS, bumpMap, lightVectorTS, Si, light, shadowFactor2);
	}
	if (input.mode == 2)
	{ 
		lr = ComputeSimpleLighting(toEye, input.norm, input.LightVecW, Si, light, shadowFactor2);
	}
	if (input.mode == 3)
	{

		input.ShadowPosH.xyz /= input.ShadowPosH.w;

		shadowFactor2 = CalcShadowFactor(input.ShadowPosH);

		lr = ComputeSimpleLighting(toEye, input.norm, input.LightVecW, Si, light, shadowFactor2);
	}


	float3 ambient = (0, 0, 0);


	ambient += (light.AmbientLight * light.DiffuseLight).rgb;

	float4 finalCol = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (input.mode == 0)
	{
		finalCol.rgb = ((ambient + lr.Diffuse) + lr.Specular * shadowFactor) * t1.Sample(s1, texCoords).rgb;
		finalCol.a = Si.DiffuseMtrl.a;
	}
	else
	{
		finalCol.rgb = ((ambient + lr.Diffuse) + lr.Specular) * t1.Sample(s1, input.texCoord).rgb;
		finalCol.a = Si.DiffuseMtrl.a;
	}
	return finalCol;

}


/***********************************************
MARKING SCHEME: Render to texture
DESCRIPTION: Calculating the colour value of each pixel for the texture. Postprocessing would go here
***********************************************/
float4 mainRenderToTex(VS_OUTPUT2 input) : SV_TARGET
{
	float4 finalCol = float4(0.0f, 0.0f, 0.0f, 0.0f);
	finalCol.rgb = t1.Sample(s1, input.texCoord).rgb;
	return finalCol;
}


/***********************************************
MARKING SCHEME: Shadow Mapping
DESCRIPTION: Clipping any element under a certain alpha value out of the depth result
***********************************************/
void shadowOuput(VS_SHADOW input)
{
	float4 diffuseAlbedo = input.DiffuseMtrl;
	diffuseAlbedo* t1.Sample(s1, input.texCoord);
#ifdef APLHA_CLIP
	clip(diffuseAlbedo.a - 0.1f);
#endif
}