cbuffer CameraBuffer                     : register(b0)
{
	matrix	            VP;
	float3	            CameraPosition;
	float	            Exposure;
	float3	            ViewDir;
	float 				_placeholder4;
};

cbuffer GridBuffer						: register(b1)
{
	matrix	            M;
};

struct Vertex
{
	float3	position        : POSITION;
	float3	normal          : NORMAL;
	float2	texcoord        : TEXCOORD0;
	float3  instpos			: INSTANCEPOS;
	uint	instance		: SV_InstanceID;
};

struct Out
{
    float3 vPixelWorldPos   : POSITION;
	float3 vNormal			: NORMAL;
    float2 vTexcoord        : TEXCOORD0;
    float4 vPosition        : SV_POSITION;
};

Out main(Vertex input)
{
	Out output;

	output.vNormal = mul(float4(input.normal,1.0f), M).xyz;
	output.vPixelWorldPos = mul(float4(input.position + input.instpos, 1.0f), M).xyz;
	output.vPosition = mul(float4(output.vPixelWorldPos, 1.0f), VP);
	output.vTexcoord = input.texcoord;
	return output;
}