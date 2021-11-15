Texture2D t1 : register(t0);
Texture2D heightMap : register(t1);
Texture2D normalMap : register(t2);
SamplerState s1 : register(s0);

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
	int mode : MODE;
};

float3 VectorToTangentSpace(float3 vectorV, float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}

/***********************************************
MARKING SCHEME: Parallax Mapping
DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space
***********************************************/
float2 ParallaxMapping(float2 texCoords, float3 viewDir)
{
	float heightScale = 0.1;
	float height = heightMap.Sample(s1, texCoords).x;
	float2 p = viewDir.xy / viewDir.z * (height * heightScale);
	return texCoords - p;
}

/***********************************************
MARKING SCHEME: Normal Mapping
DESCRIPTION: Map sampling, normal value decompression, transformation to tangent space
***********************************************/
float4 ProcessBumpMap(float4 Sample, float3x3 tbn, float2 texCoords, int flip)
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

//reflDiffLight

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float3 toEye;
	float3 lightLecNorm;
	float3 r;
	float specularAmount;
	float diffuseAmount;


	float4 normalSample = normalMap.Sample(s1, input.texCoord);
	float3 N = normalize(mul(normalSample, input.worldMat)).rgb;
	float3x3 TBN = float3x3(input.tangent, input.biNorm, N);
	float3x3 TBN_inv = transpose(TBN);

	float3 eyeVectorTS = VectorToTangentSpace(input.EyePosW.xyz, TBN_inv);
	float3 lightVectorTS = VectorToTangentSpace(input.LightVecW.xyz, TBN_inv);
	float3 normalTS = VectorToTangentSpace(N, TBN_inv);
	float3 posTS = VectorToTangentSpace(input.pos.xyz, TBN_inv);

	float3 viewDir = normalize(eyeVectorTS - posTS);
	float2 texCoords = input.texCoord;
	float4 bumpMap;

	if (input.mode == 0)
	{
		float2 texCoords = ParallaxMapping(input.texCoord, viewDir);

		bumpMap = ProcessBumpMap(heightMap.Sample(s1, texCoords),TBN, texCoords, 1);
		toEye = normalize(eyeVectorTS - posTS);
		lightLecNorm = lightVectorTS;
		r = reflect(-lightLecNorm, bumpMap);
		specularAmount = pow(max(dot(r, toEye), 0.0f), input.l.SpecularPower);
		diffuseAmount = max(dot(lightLecNorm, bumpMap), 0.0f);

		if (diffuseAmount <= 0.0f)
		{
			specularAmount = 0.0f;
		}
	}
	if (input.mode == 1)
	{
		bumpMap = ProcessBumpMap(normalMap.Sample(s1, texCoords), TBN, texCoords, 1);

		toEye = normalize(eyeVectorTS - posTS);

		lightLecNorm = lightVectorTS;
		// Compute Colour

		// Compute the reflection vector.
		r = reflect(-lightLecNorm, bumpMap);

		// Determine how much specular light makes it into the eye.
		specularAmount = pow(max(dot(r, toEye), 0.0f), input.l.SpecularPower);

		// Determine the diffuse light intensity that strikes the vertex.
		diffuseAmount = max(dot(lightLecNorm, bumpMap), 0.0f);

		// Only display specular when there is diffuse
		if (diffuseAmount <= 0.0f)
		{
			specularAmount = 0.0f;
		}
	}
	if (input.mode == 2)
	{
		bumpMap = heightMap.Sample(s1, input.texCoord);
		bumpMap = (bumpMap * 2.0f) - 1.0f;
		bumpMap = float4(normalize(bumpMap.xyz), 1);
		bumpMap = normalize(mul(bumpMap, N));
		bumpMap = -bumpMap;

		float3 toEye = normalize(input.EyePosW.xyz - input.pos.xyz);

		float3 lightLecNorm = normalize(input.LightVecW.xyz);

		// Compute Colour

		// Compute the reflection vector.
		r = reflect(-lightLecNorm, bumpMap);

		// Determine how much specular light makes it into the eye.
		specularAmount = pow(max(dot(r, toEye), 0.0f), input.l.SpecularPower);

		// Determine the diffuse light intensity that strikes the vertex.
		diffuseAmount = max(dot(lightLecNorm, bumpMap), 0.0f);

		// Only display specular when there is diffuse
		if (diffuseAmount <= 0.0f)
		{
			specularAmount = 0.0f;
		}
	}

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	specular += specularAmount * (input.s.SpecularMtrl * input.l.SpecularLight).rgb;
	diffuse += diffuseAmount * (input.s.DiffuseMtrl * input.l.DiffuseLight).rgb;
	ambient += (input.s.AmbientMtrl * input.l.AmbientLight).rgb;

	float4 finalCol;
	finalCol.rgb = (t1.Sample(s1, input.texCoord).rgb * (ambient + diffuse)) + specular;
	finalCol.a = input.s.DiffuseMtrl.a;
	return finalCol;

} 