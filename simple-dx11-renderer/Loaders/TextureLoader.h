#pragma once
#include "Graphics/Objects/Texture.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

class TextureLoader
{
public:

	static Texture2D LoadFromFile(
			const std::filesystem::path& filepath
		);
	static Texture2D LoadCubemapFromFile(
		const std::filesystem::path& filepath
	);

private:

	static Texture2D LoadDDSFromFile(
			const std::filesystem::path& filepath
		);
	static Texture2D LoadPNGFromFile(
			const std::filesystem::path& filepath
		);
	static Texture2D LoadCubemapDDSFromFile(
		const std::filesystem::path& filepath
	);

	static Texture2D CreateTexture(
			const std::string&									name, 
			const Microsoft::WRL::ComPtr<ID3D11Resource>&		res, 
			const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>&	srv
		);

};