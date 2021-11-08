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
};

float3 VectorToTangentSpace(float3 vectorV, float3x3 TBN_inv)
{
	float3 tangentSpaceNormal = normalize(mul(vectorV, TBN_inv));
	return tangentSpaceNormal;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{

	float4 normalSample = normalMap.Sample(s1, input.texCoord);
	float3 N = normalize(mul(normalSample, input.worldMat)).rgb;
	float3x3 TBN = float3x3(input.tangent, input.biNorm, N);
	float3x3 TBN_inv = transpose(TBN);

	float3 eyeVectorTS = VectorToTangentSpace(input.EyePosW.xyz, TBN_inv);
	float3 lightVectorTS = VectorToTangentSpace(input.LightVecW.xyz, TBN_inv);


	float3 toEye = normalize(input.EyePosW - input.pos.xyz);

	float4 bumpMap;
	bumpMap = heightMap.Sample(s1, input.texCoord);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpMap = float4(normalize(bumpMap.xyz), 1);
	bumpMap = normalize(mul(bumpMap, N));
	bumpMap = -bumpMap;

	float3 lightLecNorm = normalize(input.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
	float3 r = reflect(-lightLecNorm, bumpMap);

	// Determine how much specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), input.l.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
	float diffuseAmount = max(dot(lightLecNorm, bumpMap), 0.0f);

	// Only display specular when there is diffuse
	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
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