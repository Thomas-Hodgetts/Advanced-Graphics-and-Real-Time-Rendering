

Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
SamplerState s1 : register(s0);


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
	float3x3 TBN : TBN;
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

float4 main(VS_OUTPUT input) : SV_TARGET
{

	input.NormalW = normalize(input.NormalW);
	

	float3 toEye = normalize(input.EyePosW - input.pos);

	float4 bumpMap;
	bumpMap = t1.Sample(s1, input.texCoord);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpMap = float4(normalize(bumpMap.xyz), 1);

	bumpMap.rgb = mul(bumpMap, input.TBN);

	float3 lightLecNorm = normalize(input.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
	float3 r = reflect(-lightLecNorm, (bumpMap));

	// Determine how much specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), input.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
	float diffuseAmount = max(dot(lightLecNorm, bumpMap), 0.0f);

	// Only display specular when there is diffuse
	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
	}

	float3 tx = t1.Sample(s1, input.texCoord).rgb;

	float4 finalCol;
	finalCol.rgb = (t1.Sample(s1, input.texCoord).rgb * (((input.AmbientMtrl * input.AmbientLight).rgb) + (diffuseAmount * (input.DiffuseMtrl * input.DiffuseLight).rgb))) + specularAmount * (input.SpecularMtrl * input.SpecularLight).rgb;
	//finalCol.rgb = tx * (ambient + diffuse) + specular;
	finalCol.a = input.DiffuseMtrl.a;
	return finalCol;

} 