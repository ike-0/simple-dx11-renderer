#pragma once
#include "Graphics/Objects/Texture.h"

struct MaterialTextures 
{
	Texture2D	diffuse;	// 1U << 0
	Texture2D	roughness;	// 1U << 1
	Texture2D	metallic;	// 1U << 2
	Texture2D	occlusion;	// 1U << 3
	Texture2D	emissive;	// 1U << 4
	Texture2D	normal;		// 1U << 5
};