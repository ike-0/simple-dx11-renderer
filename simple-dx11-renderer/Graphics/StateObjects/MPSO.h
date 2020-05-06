#pragma once
#include "Systems/DataSystem.h"
#include "Graphics/StateObjects/PSO.h"
#include "Graphics/Objects/LogicalDevice.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Objects/Mesh.h"
#include "Graphics/Objects/Sampler.h"
#include "Objects/Camera.h"
#include "Objects/Primitives.h"

// Change in shader too
#define SHADOWMAP_RES 4096

// Mesh based Pipeline State object
class MPSO : public PSO
{
public:
	MPSO(LogicalDevice* device);
	~MPSO();

	//void Bind();
	void Prepare(CameraBuffer* cameraBuffer, SceneBuffer* sceneBuffer);
	void DrawMesh(const Buffer<Index>& indices, const Buffer<Vertex>& vertices, const std::shared_ptr<Material>& material);
	void DrawMeshShadows(const Buffer<Index>& indices, const Buffer<Vertex>& vertices);

	void FinishShadows();
	
	void SetModelConstants(const ModelBuffer* modelbuffer);
	void Finish(BOOL blurShadows, FxaaBuffer* fxaabuffer);
	void CreateCubeMap(const Texture2D& texture);
	inline void Resize(UINT width, UINT height) {
		_target->Resize(width, height);
		_shadowtarget->Resize(width, height);
		_blurredShadows.Resize(width, height);
		CreateComputeResources();
	}

private:
	void DrawCubemap(CameraBuffer* cameraBuffer);
	void CreateInputLayout();
	void InitializeShaders();
	void CreateBuffers();
	void CreateSamplers();
	void CreateTextures();
	void CreateComputeResources();

	void Blend(BOOL blurShadows);
	void FXAA(FxaaBuffer* fxaabuffer);

private:
	LogicalDevice* _device;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> _meshInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> _cubemapInputLayout;


	VertexShader	_vs;
	PixelShader		_ps;

	VertexShader	_cubemapvs;
	PixelShader		_cubemapps;

	VertexShader	_shadowvs;
	PixelShader		_shadowps;

	ComputeShader	_shadowblurcsX;
	ComputeShader	_shadowblurcsY;
	ComputeShader	_blendcs;
	ComputeShader	_fxaacs;

	Buffer<MaterialProperties>	_materialbuffer;
	Buffer<ModelBuffer>		_modelbuffer;
	Buffer<LightSpaceBuffer>_lightspacebuffer;
	Buffer<CameraBuffer>	_camerabuffer;
	Buffer<SceneBuffer>		_scenebuffer;
	Buffer<FxaaBuffer>		_fxaabuffer;

	Buffer<MVPBuffer>		_mvpbuffer;

	Sampler			_anisotropicSampler;
	Sampler			_linearSampler;
	Sampler			_cubemapSampler;
	Sampler			_iblSampler;
	Sampler			_clampSampler;

	// Shadow Gaussian filter
	Microsoft::WRL::ComPtr<ID3D11Buffer>					_filterbuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>		_filterWeightsSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>					_filterOutputA;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>					_filterOutputB;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>		_filterOutputAUAV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>		_filterOutputBUAV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>		_filterOutputASRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>		_filterOutputBSRV;
	Buffer<BlurParameters>									_blurbuffer;


	// Blending 
	Microsoft::WRL::ComPtr<ID3D11Texture2D>					_blendOutput;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>		_blendOutputUAV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>		_blendOutputSRV;

	// FXAA
	Microsoft::WRL::ComPtr<ID3D11Texture2D>					_fxaaOutput;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>		_fxaaOutputUAV;

	Texture2D		_shadowmap;				// Rendered Shadowmap for blending
	Texture2D		_blurredShadows;
	std::unique_ptr<RenderTarget>	_target;
	std::unique_ptr<RenderTarget>	_shadowtarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	_shadowmapDsv;
	std::unique_ptr<Sphere>	_skybox;


};