#pragma once

#define MAX_JOINTS 24

typedef UINT32 Index;

struct JointBuffer
{
	//joint
	DirectX::XMMATRIX	transforms[MAX_JOINTS];
};
struct _declspec(align(16)) CameraBuffer
{
	DirectX::XMMATRIX	vp;
	_declspec(align(16)) DirectX::XMFLOAT3	position;
	float				exposure;
	_declspec(align(16)) DirectX::XMFLOAT3	viewdir;
	float				_placeholder4 = 0;

};
struct _declspec(align(16)) SceneBuffer
{
	_declspec(align(16)) DirectX::XMFLOAT3	ambientcolor;
	float				iblIntensity = 0;
	_declspec(align(16)) DirectX::XMFLOAT3	sunpos;
	UINT				preciseShadows = 0;
	_declspec(align(16)) DirectX::XMFLOAT3	suncolor;
	float				_placeholder3 = 0;
	_declspec(align(16)) DirectX::XMFLOAT3	iblColor;
};

struct _declspec(align(16)) FxaaBuffer
{
	_declspec(align(16)) DirectX::XMINT2	textureSize;
	float									bias = 0.1f;
	float									biasMin = 0.05f;
	_declspec(align(16)) float				spanMax = 8.0f;
	float									_spacer1 = 0.0f;
	float									_spacer2 = 0.0f;
	float									_spacer3 = 0.0f;
};

#define DIFFUSE_BIT		1U << 0
#define METALNESS_BIT	1U << 1
#define ROUGHNESS_BIT	1U << 2
#define OCCLUSION_BIT	1U << 3
#define EMISSIVE_BIT	1U << 4
#define NORMAL_BIT		1U << 5 // Not implemented

struct _declspec(align(16)) MaterialProperties
{
	_declspec(align(16)) DirectX::XMFLOAT3	diffuse = { 1.0f, 1.0f, 1.0f };
	float				roughness = 0.9f;
	_declspec(align(16)) DirectX::XMFLOAT3	emissive = { 0.0f, 0.0f, 0.0f };
	float				metalness = 0.0f;
	_declspec(align(16)) UINT bitmask = 0;
};

struct ModelBuffer
{
	DirectX::XMMATRIX	modelmatrix;
};

struct MVPBuffer
{
	DirectX::XMMATRIX	MVP;
};

struct LightSpaceBuffer
{
	DirectX::XMMATRIX	lightSpaceMatrix;
};

struct GridBuffer
{
	DirectX::XMMATRIX	m;  // Model-World matrix
};

struct BlurParameters
{
	DirectX::XMINT2	textureSize;
	float	_spacer1 = 0.0f;
	float	_spacer2 = 0.0f;
};

struct Vertex
{
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT3	normal;
	DirectX::XMFLOAT3	tangent;
	DirectX::XMFLOAT3	bitangent;
	DirectX::XMFLOAT2	texcoord;
};

struct AnimVertex
{
	DirectX::XMFLOAT3	position;
	DirectX::XMFLOAT3	normal;
	DirectX::XMFLOAT2	texcoord;
	DirectX::XMINT3		jointid;
	DirectX::XMFLOAT3	weight;
};

