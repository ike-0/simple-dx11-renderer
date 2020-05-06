#pragma once
#include "Base/Compiler/HLSLCompiler.h"
#include "Graphics/Shaders/Shader.h"
#include "Loaders/ModelLoader.h"
#include "Loaders/TextureLoader.h"

// Main system for storing runtime data
class DataSystem
{
public:
	static DataSystem* Instance;
public:
	DataSystem();
	~DataSystem();

	void LoadShaders();
	void LoadModels();

	//void ReloadShaders();

	//Event<> OnShadersReloadedEvent;
	//Event<> ShadersReloadedEvent;

	std::unordered_map<std::string, VertexShader>	VertexShaders;
	std::unordered_map<std::string, HullShader>		HullShaders;
	std::unordered_map<std::string, DomainShader>	DomainShaders;
	std::unordered_map<std::string, GeometryShader>	GeometryShaders;
	std::unordered_map<std::string, PixelShader>	PixelShaders;
	std::unordered_map<std::string, ComputeShader>	ComputeShaders;

	std::unordered_map<std::string, std::shared_ptr<Model>>		Models;
	std::unordered_map<std::string, std::shared_ptr<AnimModel>>	AnimatedModels;

private:

	void AddShaderFromPath(const std::filesystem::path& filepath);
	void AddModel(const std::filesystem::path& filepath, const std::filesystem::path& dir);
	void AddShader(const std::string& name, const std::string& ext, const Microsoft::WRL::ComPtr<ID3DBlob>& blob);
	void RemoveShaders();

};