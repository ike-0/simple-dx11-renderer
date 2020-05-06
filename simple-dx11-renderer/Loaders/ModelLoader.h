#pragma once
#include "Graphics/Objects/Mesh.h"
#include "Objects/Material.h"
#include "Objects/Model.h"

#include "Loaders/TextureLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h> 
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>

class ModelLoader
{
public:

	// Takes model path, and returns mesh and/or animation
	static void	LoadFromFile(
			const std::filesystem::path&	path,
			const std::filesystem::path&	dir,
			std::shared_ptr<Model>*			model,
			std::shared_ptr<AnimModel>*		animmodel
		);

private:

	struct MatOverrides
	{

		inline MatOverrides() {};
		inline MatOverrides(const std::string& _name, float _roughness, float _metalness) : name(_name), roughness(_roughness), metalness(_metalness), enabled(1) {}
		BOOL enabled = 0;
		std::string name;
		float roughness;
		float metalness;
	};

	static std::shared_ptr<Model> LoadModel(
			const std::string&				name,
			const aiScene*					scene,
			const std::filesystem::path&	path,
			const std::filesystem::path&	dir
		);

	static std::shared_ptr<AnimModel> LoadAnimatedModel(
			const std::string&				name,
			const aiScene*					scene,
			const std::filesystem::path&	path,
			const std::filesystem::path&	dir
		);

	static void	LoadMeshes(
			const aiScene*					scene,
			const std::filesystem::path&	dir,
			std::vector<Mesh>*				meshes
		);

	static std::shared_ptr<Material> CreateMaterial(
			const std::filesystem::path&	dir,
			aiMesh*							aimesh,
			const aiScene*					scene
		);

};