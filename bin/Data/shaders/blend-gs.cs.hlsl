
// Blends 2 textures together
// One must be grayscale

Texture2D<float4> SourceA : register(t0);
Texture2D<float> SourceB : register(t1);

RWTexture2D<float4> Output : register(u0);

[numthreads(16, 16, 1)]
void main(int3 DispatchID : SV_DispatchThreadID)
{
	// float4 color = SourceA[DispatchID.xy] * max(pow(SourceB[DispatchID.xy],1.0f/2.2f), 0.02f);
	// float4 color = SourceA[DispatchID.xy] * max(pow(SourceB[DispatchID.xy],2.2f), 0.02f);
	float4 color = SourceA[DispatchID.xy] * SourceB[DispatchID.xy];
	// float4 color = SourceA[DispatchID.xy];


	// Manualy gamma correct

	Output[DispatchID.xy] = pow(color, 1.0f/2.2f);
}