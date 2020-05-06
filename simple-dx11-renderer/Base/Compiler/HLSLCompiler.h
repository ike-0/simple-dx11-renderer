#pragma once

class IncludeHandler : public ID3DInclude
{
public:
	inline IncludeHandler(const std::filesystem::path& _dir) : dir(_dir) {};
	inline ~IncludeHandler() {};

	inline HRESULT Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) final override
	{
		std::filesystem::path filepath;
		if (IncludeType == D3D_INCLUDE_LOCAL)
			filepath = dir / std::filesystem::path(pFileName);
		else if (IncludeType == D3D_INCLUDE_SYSTEM)
			filepath = Path::Relative("Shaders\\" + std::string(pFileName));
		else
			return E_FAIL;

		if (std::filesystem::exists(filepath))
		{
			std::ifstream i(filepath, std::ios::binary | std::ios::ate);

			auto size = i.tellg();
			char* pdata = static_cast<char*>(std::malloc(size));


			i.seekg(0, std::ios::beg);
			i.read(pdata, size);
			i.close();

			*pBytes = size;
			*ppData = pdata;

			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}

	inline HRESULT Close(THIS_ LPCVOID pData) final override
	{
		std::free(const_cast<void*>(pData));
		return S_OK;
	}

	std::filesystem::path dir;

};

class HLSLCompiler
{
public:
	static Microsoft::WRL::ComPtr<ID3DBlob> LoadFromFile(const std::filesystem::path& filepath);
private:
	static BOOL CheckResult(HRESULT hr, const Microsoft::WRL::ComPtr<ID3DBlob>& shader, const Microsoft::WRL::ComPtr<ID3DBlob>& errorBlob);
	static BOOL CheckExtension(const std::string& ext)
	{
		return ext == "vs" || ext == "gs" || ext == "ps" || ext == "hs" || ext == "cs";
	}
};

