#pragma once

class Texture
{
public:
	inline Texture() { };
	virtual ~Texture() = default;
};


class Texture2D : public Texture
{
public:
	// Empty
	inline Texture2D() : width(0), height(0), format(DXGI_FORMAT_UNKNOWN) { };

	// Internaly asks for description
	inline Texture2D(
		const Microsoft::WRL::ComPtr<ID3D11Texture2D>& res,
		const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv
	)
		: _resource(res), _srv(srv) {
		D3D11_TEXTURE2D_DESC desc;
		res->GetDesc(&desc);
		format = desc.Format;
		width = desc.Width;
		height = desc.Height;
	};

	// Internaly asks for description
	inline Texture2D(
		const std::string& name,
		const Microsoft::WRL::ComPtr<ID3D11Texture2D>& res,
		const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
		: _resource(res), _srv(srv) {
		D3D11_TEXTURE2D_DESC desc;
		res->GetDesc(&desc);
		format = desc.Format;
		width = desc.Width;
		height = desc.Height;
		SetName(name);
	};

	//bool operator==(const Texture2D& other)
	//{

	//}

	//bool operator==(nullptr)
	//{

	//}

	// Faster way if providing description from constructor is possible
	inline Texture2D(const Microsoft::WRL::ComPtr<ID3D11Texture2D>& res, const D3D11_TEXTURE2D_DESC& desc)
		: _resource(res), format(desc.Format), width(desc.Width), height(desc.Height) {
	};
	inline ~Texture2D() { };

	ID3D11Texture2D* GetTexture() const
	{
		return _resource.Get();
	}
	ID3D11Texture2D* const* GetTextureAddressOf() const
	{
		return _resource.GetAddressOf();
	}
	ID3D11ShaderResourceView* Get() const
	{
		return _srv.Get();
	}
	ID3D11ShaderResourceView* const* GetAddressOf() const
	{
		return _srv.GetAddressOf();
	}

	inline void Reset()
	{
		_resource.Reset();
	}
	inline void SetName(const std::string& _name)
	{
		name = _name;
		DX::SetDebugName(_resource.Get(), _name.c_str());
	}

	// Clears and resizes non static resources
	inline void Resize(UINT _width, UINT _height)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		_resource->GetDesc(&desc);
		desc.Height = _height;
		desc.Width = _width;

		width = _width;
		height = _height;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
		_srv->GetDesc(&srvdesc);

		DX::States::Device->CreateTexture2D(&desc, nullptr, _resource.ReleaseAndGetAddressOf());
		DX::States::Device->CreateShaderResourceView(_resource.Get(), &srvdesc, _srv.ReleaseAndGetAddressOf());
	}

	UINT width;
	UINT height;
	DXGI_FORMAT format;
	std::string name;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				_resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	_srv;
};
