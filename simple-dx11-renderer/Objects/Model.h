#pragma once
#include "Graphics/Objects/Mesh.h"

class Model
{
public:
	inline Model() : position{ 0.0f, 0.0f, 0.0f, 0.0f }, rotation{ DirectX::XMQuaternionIdentity() }, scale(1.0f)
	{ };
	inline ~Model() { };

	std::string				name;
	std::vector<Mesh>		meshes;

	DirectX::XMVECTOR		position;
	DirectX::XMVECTOR		rotation;
	float					scale;

	inline DirectX::XMMATRIX GetMatrix() const
	{

		DirectX::XMMATRIX trans = DirectX::XMMatrixScalingFromVector(DirectX::XMVECTOR{ scale, scale, scale, 0.0f });
		trans *= DirectX::XMMatrixTranslationFromVector(position);
		trans *= DirectX::XMMatrixRotationQuaternion(rotation);
		return trans;
	}
};

class AnimModel
{
public:
	std::string				name;
	std::vector<AnimMesh>	meshes;

	inline DirectX::XMMATRIX GetMatrix()
	{
		return DirectX::XMMatrixIdentity();
	}
};