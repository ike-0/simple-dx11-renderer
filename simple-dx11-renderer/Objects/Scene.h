#pragma once

struct Scene
{
	DirectX::XMFLOAT3 suncolor;
	DirectX::XMFLOAT3 sunpos;
	DirectX::XMFLOAT3 iblcolor;
	float iblIntensity;
	bool moreShadowSamples;
	bool gaussianShadowBlur;

	bool fxaa = true;
	float bias = 0.688f;
	float biasMin = 0.021f;
	float spanMax = 8.0f;

};