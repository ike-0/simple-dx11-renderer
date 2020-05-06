#include "HLSLCompiler.h"

Microsoft::WRL::ComPtr<ID3DBlob> HLSLCompiler::LoadFromFile(const std::filesystem::path& filepath)
{
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorblob;

	unsigned int compileFlags = 0;

#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif 
#ifdef NDEBUG
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

	std::filesystem::path filename = filepath.filename();
	std::string ext = filename.replace_extension().extension().string();
	filename.replace_extension();

	if (ext == "")
		return nullptr;

	ext.erase(0, 1);
	if (!CheckExtension(ext))
	{
		ERROR(std::string("Unrecognized extension \"") + ext + "\"");
	}
		

	// Posible options 

	// cs - Compute Shader
	// ds - Domain Shader
	// gs - Geometry Shader
	// hs - Hull Shader
	// ps - Pixel Shader
	// vs - Vertex Shade

	HRESULT hr = NULL;

	IncludeHandler handler(filepath.parent_path());
	// This is fairly limiting 5.1 is only supported on D3D12
	hr = D3DCompileFromFile(filepath.c_str(), NULL, &handler, "main", std::string(ext + "_5_0").c_str(), compileFlags, NULL, blob.ReleaseAndGetAddressOf(), errorblob.ReleaseAndGetAddressOf());
	
	if (!CheckResult(hr, blob, errorblob))
	{
		WARN("Failed compilation");
		return nullptr;
	}

	return blob;
}

BOOL HLSLCompiler::CheckResult(HRESULT hr, const Microsoft::WRL::ComPtr<ID3DBlob>& shader, const Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob)
{
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			ERROR((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shader)
			shader->Release();
		return 0;
	}
	else
	{
		if (errorBlob.Get() != nullptr)
		{
			WARN((char*)errorBlob->GetBufferPointer());
		}
	}
	return 1;
}
