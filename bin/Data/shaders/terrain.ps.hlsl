#include "common.hlsli"

cbuffer CameraBuffer                     : register(b0)
{
	matrix	            VP;
	float3	            CameraPosition;
	float	            Exposure;
	float3	            ViewDir;
	float 				_placeholder4;
};

cbuffer SceneBuffer                     : register(b1)          
{
	float3	            AmbientColor;
	float	            _placeholder;
    float3	            SunPos;
	float	            _placeholder2;
	float3	            SunColor;
	float	            _placeholder3;
};

struct In
{
    float3 vPixelWorldPos   : POSITION;
	float3 vNormal			: NORMAL;
    float2 vTexcoord        : TEXCOORD0;
    float4 vPosition        : SV_POSITION;
};

float4 main(In input) : SV_TARGET
{
	float3 diffuse = float3(1.0f, 1.0f, 1.0f);


	float3 V = normalize(CameraPosition - input.vPixelWorldPos);	// Camera dir
	float3 P = input.vPixelWorldPos;								// Pixel pos
	float3 L = normalize(SunPos);									// Light dir
	float3 N = normalize(input.vNormal);


	float dotL = max(mul(L, N), 0.0f);
	float3 diffuseColor = diffuse * dotL * SunColor;

	return ToneMap(float4(diffuseColor,1.0f));
}