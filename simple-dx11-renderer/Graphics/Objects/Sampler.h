#pragma once

class Sampler
{
public:
	inline Sampler() {};
	inline Sampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_MIRROR, float offset = 0, DirectX::XMFLOAT4* borderColor = nullptr)
	{
		Create(filter, addressMode, offset, borderColor);
	};
	inline ~Sampler() {};

	inline ID3D11SamplerState* Get()
	{
		return _sampler.Get();
	}
	inline ID3D11SamplerState* const* GetAddressOf()
	{
		return _sampler.GetAddressOf();
	}

private:
	void Create(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_MIRROR, float offset = 0, DirectX::XMFLOAT4* borderColor = nullptr) {
		D3D11_SAMPLER_DESC desc = {};

		desc.Filter = filter;
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		desc.MipLODBias = offset;
		desc.MaxAnisotropy = Options::Graphics::MaxAnisotropy;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		if (borderColor != nullptr)
		{
			desc.BorderColor[0] = borderColor->x;
			desc.BorderColor[1] = borderColor->y;
			desc.BorderColor[2] = borderColor->z;
			desc.BorderColor[3] = borderColor->w;
		}

		DX::States::Device->CreateSamplerState(&desc, _sampler.ReleaseAndGetAddressOf());
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler;

};