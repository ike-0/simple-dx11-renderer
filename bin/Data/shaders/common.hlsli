#ifndef COMMON
#define COMMON


static const float PI = 3.14159265f;
static const float EPSILON = 1e-6f;

float4 ToneMap(float4 colorin) {
	float4 mapped = colorin / (colorin + float4(1.0f, 1.0f, 1.0f, 1.0f));
	mapped.w = 1.0f;
	return mapped;
}

float ToneMap1D(float colorin) {
	float mapped = colorin / (colorin + float(1.0f));
	return mapped;
}

float GetPointLightAttenuation(float distance, float Kl, float Kq) {
    return 1.0f / (1 + Kl * distance + Kq * pow(distance,2));
}
float GetDirectionalLightAttenuation() {
    return 5.0f; // Overestimate energy from the sun as it is dominant source
}

#endif

