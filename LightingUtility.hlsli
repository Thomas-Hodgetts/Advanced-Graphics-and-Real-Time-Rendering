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

float CalcAttenuation(float d, float falloffStart, float fallofEnd)
{
	return saturate((fallofEnd - d) / (fallofEnd - falloffStart));
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	float cosIncidentAngle = saturate(dot(normal, lightVec));
	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
	return reflectPercent;
}

float3 BlinPhong(float3 Strength, float3 normal, float3 lightVec, float3 toEye, SurfaceInfo mat)
{
	const float m = 0.25 * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);
	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = SchlickFresnel(0.1f, halfVec, lightVec);
	mat.SpecularMtrl = mat.SpecularMtrl / (mat.SpecularMtrl + 1.0f);
	return (mat.DiffuseMtrl.rgb + mat.SpecularMtrl) * Strength;
}

float3 ComputeDirectionalLight(Light L, SurfaceInfo mat, float3 normal, float3 toEye)
{
	float3 lightVec = -L.LightVecW;
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = 1.0f * ndotl;
	return BlinPhong(lightStrength, normal, lightVec, toEye, mat);
}