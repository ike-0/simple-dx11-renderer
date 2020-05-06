#ifndef PBR
#define PBR


#include "common.hlsli"


float3 FresnelReflectance(in float3 f0, in float3 f90, in float VdotH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0f - VdotH, 0.0f, 1.0f), 5.0f);
}

float GeometricOcclusion(float NdotL, float NdotV, float alphaRoughness)
{
	const float attenuationL = 2.0 * NdotL / (NdotL + sqrt(alphaRoughness * alphaRoughness + (1.0 - alphaRoughness * alphaRoughness) * (NdotL * NdotL)));
    const float attenuationV = 2.0 * NdotV / (NdotV + sqrt(alphaRoughness * alphaRoughness + (1.0 - alphaRoughness * alphaRoughness) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

float MicrofacetBRDF(float NdotH, float alphaRoughness)
{
    const float roughnessSq = alphaRoughness * alphaRoughness;
    const float f = (NdotH * roughnessSq - NdotH) * NdotH + 1.0;
    return roughnessSq / (PI * f * f);
}


static const float3 F0 = float3(0.04f, 0.04f, 0.04f);
struct PBRParameters
{
	float3 SpecEnvR0;
	float3 SpecEnvR90;
	float3 DiffuseColor;
	float AlphaRoughness;
};
struct DotProducts
{
    float NdotL; 				// cos angle between normal and light direction
    float NdotV; 				// cos angle between normal and view direction
    float NdotH; 				// cos angle between normal and half vector
    float LdotH; 				// cos angle between light direction and half vector
    float VdotH; 				// cos angle between view direction and half vector
};

// Raytraces through 3D voxelized lightmap 

float3 RaytraceLightmap(float3 raydir)
{
	return float3(0.0f, 0.0f, 0.0f);
}

// Computes common PBR paramters for all light sources
PBRParameters ComputePBRParameters(float3 diffuse, float metallic, float roughness)
{
	PBRParameters params;

	params.DiffuseColor = (diffuse * (float3(1.0f,1.0f,1.0f) - F0)) * (1.0 - metallic);

	params.SpecEnvR0 = lerp(F0, diffuse, metallic);

	// Calculate reflectance
	float reflectance = max(max(params.SpecEnvR0.r, params.SpecEnvR0.g), params.SpecEnvR0.b);
	float reflectance90 = clamp(reflectance * 25.0, 0, 1.0);
	params.SpecEnvR90 = float3(1.0f,1.0f,1.0f) * reflectance90;
	params.AlphaRoughness = roughness * roughness;
	return params;
}

// Computes PBR Color for generalized light source (Point, Directional, ...)
// See Ref:
// https://github.com/microsoft/glTF-DXViewer/blob/master/ModelViewer/Assets/Shaders/pbrpixel.hlsl
// https://github.com/microsoft/MixedRealityToolkit/blob/master/SpatialInput/Libs/Pbr/Shaders/PbrPixelShader.hlsl

float3 ComputePBRColor(PBRParameters params, float3 V, float3 N, float3 L, float3 lightColor)
{
	const float3 H = normalize(L + V); // Half vector between both L and V

	// Reflection term
	const float3 reflection = -normalize(reflect(V,N));

	// Dot products
	const float NdotL = clamp(dot(N,L), 0.001f, 1.0f);
	const float NdotV = abs(dot(N,V)) + 0.001;
	const float NdotH = clamp(dot(N,H), 0.0f, 1.0f);
	const float LdotH = clamp(dot(N,H), 0.0f, 1.0f);
	const float VdotH = clamp(dot(N,H), 0.0f, 1.0f);

	// Calculate the shading terms for the microfacet specular shading model
    float3 F = FresnelReflectance(params.SpecEnvR0, params.SpecEnvR90, VdotH);
    float G = GeometricOcclusion(NdotL, NdotV, params.AlphaRoughness);
    float D = MicrofacetBRDF(NdotH, params.AlphaRoughness);

	// Lighting contribution
	float3 diffuseContrib = (1.0 - F) * (params.DiffuseColor / PI);
    float3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    float3 color = NdotL * lightColor * (diffuseContrib + specContrib);

	return color;
}



#endif // PBR