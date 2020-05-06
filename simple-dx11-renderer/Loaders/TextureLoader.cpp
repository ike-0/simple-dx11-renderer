#include "TextureLoader.h"

Texture2D TextureLoader::LoadFromFile(const std::filesystem::path& filepath)
{
	if (filepath.extension() == ".dds")
	{
		return LoadDDSFromFile(filepath);
	}
	else if (filepath.extension() == ".png")
	{
		return LoadPNGFromFile(filepath);
	}
	else if (filepath.extension() == ".jpg")
	{
		return LoadPNGFromFile(filepath);
	}
	else 
	{
		WARN(std::string("Unrecognized extension \"") + filepath.extension().string() + "\" for Texture2D");
		return Texture2D();
	}
}

Texture2D TextureLoader::LoadCubemapFromFile(const std::filesystem::path& filepath)
{
	if (filepath.extension() == ".dds")
	{
		return LoadCubemapDDSFromFile(filepath);
	}
	else
	{
		WARN(std::string("Unrecognized Cubemap extension \"") + filepath.extension().string() + "\" for Texture2D");
		return Texture2D();
	}
}

// Expects that mips are already in dds, so thread safe
Texture2D TextureLoader::LoadDDSFromFile(const std::filesystem::path& filepath)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> res;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	CHECK(DirectX::CreateDDSTextureFromFileEx(
		DX::States::Device.Get(),
		filepath.wstring().c_str(),
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
		true,
		res.ReleaseAndGetAddressOf(),
		srv.ReleaseAndGetAddressOf(),
		nullptr
	));

	std::filesystem::path file = filepath.filename();
	std::string filename = file.replace_extension().string();

	return CreateTexture(filename, res, srv);
}

// Thread unsafe, generates mipmaps
Texture2D TextureLoader::LoadPNGFromFile(const std::filesystem::path& filepath)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> res;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	DirectX::CreateWICTextureFromFileEx(
		DX::States::Device.Get(),
		DX::States::Context.Get(),
		filepath.wstring().c_str(),
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
		true,	// WIC_LOADER_FORCE_SRGB No actual conversion takes, just renames to correct format
		res.ReleaseAndGetAddressOf(),
		srv.ReleaseAndGetAddressOf()
	);

	std::filesystem::path file = filepath.filename();
	std::string filename = file.replace_extension().string();

	return CreateTexture(filename, res, srv);
}

Texture2D TextureLoader::LoadCubemapDDSFromFile(const std::filesystem::path& filepath)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> res;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	CHECK(DirectX::CreateDDSTextureFromFileEx(
		DX::States::Device.Get(),
		filepath.wstring().c_str(),
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		D3D11_RESOURCE_MISC_TEXTURECUBE,
		false,
		res.ReleaseAndGetAddressOf(),
		srv.ReleaseAndGetAddressOf(),
		nullptr
	));

	std::filesystem::path file = filepath.filename();
	std::string filename = file.replace_extension().string();

	return CreateTexture(filename, res, srv);
}

Texture2D TextureLoader::CreateTexture(const std::string& name, const Microsoft::WRL::ComPtr<ID3D11Resource>& res, const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	D3D11_RESOURCE_DIMENSION dim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	res->GetType(&dim);

	switch (dim) {
	case(D3D11_RESOURCE_DIMENSION_TEXTURE2D):
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d;
		CHECK(res.As(&tex2d));
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		srv->GetDesc(&desc);

		return Texture2D(name, tex2d, srv);
	}
	default:
	{
		WARN("Unknown texuture dimension, expected 2D \"" + name + "\"");
	}
	}

	// Empty
	return Texture2D();
}
