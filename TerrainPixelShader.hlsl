#include "Header.hlsli"

SamplerState s1 : register(s0);
SamplerComparisonState gsamShadow : register(s1);

Texture2D GroundTexture1 : register(t0);
Texture2D GroundTexture2 : register(t1);
Texture2D GroundTexture3 : register(t2);
Texture2D RockTexture1 : register(t3);
Texture2D RockTexture2 : register(t4);
Texture2D RockTexture3 : register(t5);
Texture2D SnowTexture1 : register(t6);
Texture2D SnowTexture2 : register(t7);
Texture2D SnowTexture3 : register(t8);

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
float2 ParallaxMapping(float2 texCoords, float3 viewDir, float height)
{
	if (height > 5)
	{
		float height_scale = 0.04f;
		float height = -GroundTexture3.Sample(s1, texCoords).x;
		float2 p = viewDir.xy / viewDir.z * (height * height_scale);
		return texCoords - p;
	}
	if (height > 10)
	{
		float height_scale = 0.04f;
		float height = -RockTexture3.Sample(s1, texCoords).x;
		float2 p = viewDir.xy / viewDir.z * (height * height_scale);
		return texCoords - p;
	}
	if (height > 15)
	{
		float height_scale = 0.4f;
		float height = -SnowTexture3.Sample(s1, texCoords).x;
		float2 p = viewDir.xy / viewDir.z * (height * height_scale);
		return texCoords - p;
	}
	return texCoords;
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

LightingResult ComputeSimpleLighting(float3 toEye, float3 Norm, float3 LightVecW, SurfaceInfo surfInfo, Light light)
{
	LightingResult lr;

	float3 r = reflect(-LightVecW, Norm);

	float diffuseAmount = max(dot(LightVecW, Norm), 0.0f);
	lr.Diffuse.rgb = diffuseAmount * (surfInfo.DiffuseMtrl * light.DiffuseLight);
	lr.Diffuse.a = diffuseAmount * (surfInfo.DiffuseMtrl * light.DiffuseLight).a;

	float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);
	lr.Specular.rgb = specularAmount * (surfInfo.SpecularMtrl * light.SpecularLight);
	lr.Specular.a = specularAmount * (surfInfo.SpecularMtrl * light.SpecularLight).a;

	return lr;
}


float4 main(DS_OUTPUT input) : SV_TARGET
{

float3 lightLecNorm;
	float3 r;
	float specularAmount;
	float diffuseAmount;


	float2 texCoords = input.texCoord;
	float4 bumpMap;
	float shadowFactor = 1;
	LightingResult lr;

	if (input.distance < 150.0f)
	{
		texCoords = ParallaxMapping(input.texCoord, input.eyeVectorTS, input.posW.y);
		shadowFactor = parallaxSoftShadowMultiplier(input.lightVectorTS, texCoords, GroundTexture3.Sample(s1, texCoords).x);

		if (input.posW.y > 5)
		{
			bumpMap = ProcessBumpMap(GroundTexture2.Sample(s1, texCoords), input.TBN, 0);
		}
		if (input.posW.y > 10)
		{
			bumpMap = ProcessBumpMap(RockTexture2.Sample(s1, texCoords), input.TBN, 0);
		}
		if (input.posW.y > 15)
		{
			bumpMap = ProcessBumpMap(SnowTexture2.Sample(s1, texCoords), input.TBN, 0);
		}

		lr = ComputeSimpleLighting(input.eyeVectorTS, bumpMap, input.lightVectorTS, Si, light);
		if (texCoords.x > 1 || texCoords.x < 0 || texCoords.y > 1 || texCoords.y < 0)
		{
			discard;
		}
	}
	else if (input.distance < 320.0f)
	{
		if (input.posW.y < 5)
		{
			bumpMap = ProcessBumpMap(GroundTexture2.Sample(s1, texCoords), input.TBN, 0);
			lr = ComputeSimpleLighting(input.eyeVectorTS, bumpMap, input.lightVectorTS, Si, light);
		}
		else if (input.posW.y < 10)
		{
			bumpMap = ProcessBumpMap(RockTexture2.Sample(s1, texCoords), input.TBN, 0);
			lr = ComputeSimpleLighting(input.eyeVectorTS, bumpMap, input.lightVectorTS, Si, light);
		}
		else
		{
			bumpMap = ProcessBumpMap(SnowTexture2.Sample(s1, texCoords), input.TBN, 0);
			lr = ComputeSimpleLighting(input.eyeVectorTS, bumpMap, input.lightVectorTS, Si, light);
		}
	}
	else
	{
		lr = ComputeSimpleLighting(normalize(EyePosW - input.posW.xyz), normalize(float3(0,1,0)), normalize(light.LightVecW - input.posW.xyz), Si, light);
	}


	float3 ambient = (0, 0, 0);


	ambient += (light.AmbientLight * light.DiffuseLight).rgb;

	float4 finalCol = float4(0.0f, 0.0f, 0.0f, 0.0f);


	if (input.posW.y < 5)
	{
		finalCol.rgb = ((ambient + (lr.Diffuse) + lr.Specular) * shadowFactor) * GroundTexture1.Sample(s1, texCoords).rgb;
		finalCol.a = Si.DiffuseMtrl.a;
	}
	else if (input.posW.y < 10)
	{
		finalCol.rgb =  ((ambient + lr.Diffuse) + lr.Specular) * RockTexture1.Sample(s1, texCoords).rgb;
		finalCol.a = Si.DiffuseMtrl.a;
	}
	else
	{
		finalCol.rgb = ((ambient + lr.Diffuse) + lr.Specular) * SnowTexture1.Sample(s1, texCoords).rgb;
		finalCol.a = Si.DiffuseMtrl.a;
	}

	return finalCol;
}