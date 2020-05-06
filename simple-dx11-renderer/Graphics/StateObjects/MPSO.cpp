#include "MPSO.h"


MPSO::MPSO(LogicalDevice* device) : _device(device)
{
	InitializeShaders();
	CreateInputLayout();
	CreateBuffers();
	CreateTextures();
	CreateSamplers();
	CreateComputeResources();
}

MPSO::~MPSO()
{

}

void MPSO::Prepare(CameraBuffer* cameraBuffer, SceneBuffer* sceneBuffer)
{
	_target->ClearView();
	_shadowtarget->ClearView();

	_device->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw cubemap first
	DrawCubemap(cameraBuffer);

	// Change rasterizer and depth stencil for shadow map
	_device->SetRasterizer(RasterizerType::SolidFrontCull);
	_device->SetDepthStencil(DepthStencilType::Less);

	// Map 2 buffers for shadow shader
	{
		D3D11_MAPPED_SUBRESOURCE mat = _camerabuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
		memcpy(mat.pData, static_cast<const void*>(cameraBuffer), sizeof(CameraBuffer));
		_camerabuffer.Unmap();
	}

	{
		D3D11_MAPPED_SUBRESOURCE mat = _scenebuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
		memcpy(mat.pData, static_cast<const void*>(sceneBuffer), sizeof(SceneBuffer));
		_scenebuffer.Unmap();
	}

	DX::States::Context->IASetInputLayout(_meshInputLayout.Get());

	// Prepare for shadows
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, SHADOWMAP_RES, SHADOWMAP_RES);

	DX::States::Context->OMSetRenderTargets(0, nullptr, _shadowmapDsv.Get());
	DX::States::Context->RSSetViewports(1, &viewport);

	DX::States::Context->VSSetShader(_shadowvs.Get(), nullptr, 0);
	DX::States::Context->PSSetShader(_shadowps.Get(), nullptr, 0);


	float near_plane = 5.0f, far_plane = 30.0f;
	DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicRH(32.0f, 32.0f, near_plane, far_plane);

	DirectX::XMMATRIX lookdir = DirectX::XMMatrixLookAtRH(
		DirectX::XMLoadFloat3(&sceneBuffer->sunpos),
		DirectX::XMVECTOR{ 0.0f,0.0f,0.0f,0.0f },
		DirectX::XMVECTOR{ 0.0f,1.0f,0.0f,0.0f }
	);

	DirectX::XMMATRIX lightspaceMatrix = DirectX::XMMatrixMultiplyTranspose(lookdir, lightProjection);

	D3D11_MAPPED_SUBRESOURCE mat = _lightspacebuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
	memcpy(mat.pData, static_cast<const void*>(&lightspaceMatrix), sizeof(LightSpaceBuffer));
	_lightspacebuffer.Unmap();

	DX::States::Context->VSSetConstantBuffers(0, 1, _lightspacebuffer.GetAddressOf());

	// All set ready to pool and draw meshes for shadows
}

void MPSO::DrawCubemap(CameraBuffer* cameraBuffer)
{
	// Set Cubmap shader
	DX::States::Context->VSSetShader(_cubemapvs.Get(), nullptr, NULL);
	DX::States::Context->PSSetShader(_cubemapps.Get(), nullptr, NULL);

	_device->SetRasterizer(RasterizerType::SolidNoCull);
	_device->SetDepthStencil(DepthStencilType::LessEqual);
	_device->SetRenderTarget(_target.get());

	DirectX::XMMATRIX m = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(5.0f, 5.0f, 5.0f);


	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(cameraBuffer->position.x, cameraBuffer->position.y, cameraBuffer->position.z);

	m = DirectX::XMMatrixMultiply(scale, translation);

	DirectX::XMMATRIX mvp = cameraBuffer->vp * DirectX::XMMatrixTranspose(m);

	DX::States::Context->VSSetConstantBuffers(0, 1, _mvpbuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE mat = _mvpbuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
	memcpy(mat.pData, static_cast<const void*>(&mvp), sizeof(MVPBuffer));
	_mvpbuffer.Unmap();

	DX::States::Context->PSSetSamplers(0, 1, _cubemapSampler.GetAddressOf());
	DX::States::Context->PSSetShaderResources(0, 1, _skybox->material->textures.diffuse.GetAddressOf());


	UINT stride = _skybox->bvertex.Stride();
	const UINT offset = 0;

	DX::States::Context->IASetInputLayout(_cubemapInputLayout.Get());

	DX::States::Context->IASetIndexBuffer(_skybox->bindex.Get(), DXGI_FORMAT_R16_UINT, 0);
	DX::States::Context->IASetVertexBuffers(0, 1, _skybox->bvertex.GetAddressOf(), &stride, &offset);

	DX::States::Context->DrawIndexed(_skybox->bindex.Size(), 0, 0);
}

void MPSO::SetModelConstants(const ModelBuffer* modelbuffer)
{
	DX::States::Context->VSSetConstantBuffers(1, 1, _modelbuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE mat = _modelbuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
	memcpy(mat.pData, static_cast<const void*>(modelbuffer), sizeof(ModelBuffer));
	_modelbuffer.Unmap();
}




void MPSO::DrawMeshShadows(const Buffer<Index>& indices, const Buffer<Vertex>& vertices)
{
	// Draw shadows
	UINT stride = vertices.Stride();
	const UINT offset = 0;

	DX::States::Context->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), &stride, &offset);
	DX::States::Context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	DX::States::Context->DrawIndexed(indices.Size(), 0, 0);
}

void MPSO::FinishShadows()
{

	// Finish up shadows by rebinding last depth stencil as texture for actual drawing stage
	// this is so we know what's in shadows what is not

	_device->SetRenderTargets(_target.get(), _shadowtarget.get());

	_device->SetRasterizer(RasterizerType::Solid);

	DX::States::Context->PSSetShader(_ps.Get(), NULL, 0);
	DX::States::Context->VSSetShader(_vs.Get(), NULL, 0);


	ID3D11Buffer* psbuffers[]{
		_camerabuffer.Get(),
		_scenebuffer.Get()
	};

	ID3D11Buffer* vsbuffers[]{
		_camerabuffer.Get(),
		nullptr,
		_lightspacebuffer.Get()
	};

	DX::States::Context->VSSetConstantBuffers(0, 3, vsbuffers);
	DX::States::Context->PSSetConstantBuffers(0, 2, psbuffers);


	ID3D11SamplerState* samplers[4]{ _anisotropicSampler.Get(), _linearSampler.Get(), _clampSampler.Get(), _iblSampler.Get() };
	DX::States::Context->PSSetSamplers(0, 4, samplers);

	DX::States::Context->PSSetShaderResources(6, 1, _skybox->material->textures.diffuse.GetAddressOf());
	DX::States::Context->PSSetShaderResources(7, 1, _shadowmap.GetAddressOf()); // Bind shadowmap
}


void MPSO::DrawMesh(const Buffer<Index>& indices, const Buffer<Vertex>& vertices, const std::shared_ptr<Material>& material)
{
	// Draw actual mesh
	// During this drawing shadows are split in to separate render target, but that is uncessary as it could be blended
	// at the same time
	// I wanted to see if bluring shadows and combining later would give a nicer effect

	DX::States::Context->PSSetConstantBuffers(2, 1, _materialbuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE mat = _materialbuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0);
	memcpy(mat.pData, static_cast<const void*>(&material->properties), sizeof(MaterialProperties));
	_materialbuffer.Unmap();

	UINT stride = vertices.Stride();
	const UINT offset = 0;

	ID3D11ShaderResourceView* views[] = {
		material->textures.diffuse.Get(),
		material->textures.metallic.Get(),
		material->textures.roughness.Get(),
		material->textures.occlusion.Get(),
		material->textures.emissive.Get(),
		material->textures.normal.Get(),
	};

	DX::States::Context->PSSetShaderResources(0, ARRAYSIZE(views), views);

	DX::States::Context->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), &stride, &offset);
	DX::States::Context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	DX::States::Context->DrawIndexed(indices.Size(), 0, 0);
}

void MPSO::Finish(BOOL blurShadows, FxaaBuffer* fxaabuffer)
{
	// Finish up shadows
	ID3D11ShaderResourceView* srvs[]{ nullptr };
	DX::States::Context->PSSetShaderResources(7, 1, srvs); // Unbind shadowmap
	DX::States::Context->ClearDepthStencilView(_shadowmapDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_device->SetRenderTargetNothing(); // Set to nothing to unbind render target

	// Combine shadows and rendered mesh
	Blend(blurShadows);

	// Do simplistic fxaa
	if (fxaabuffer != nullptr)
	{
		FXAA(fxaabuffer);
		DX::States::Context->CopyResource(_device->swapchain->target->GetTexture(), _fxaaOutput.Get());
	}
	else
	{
		DX::States::Context->CopyResource(_device->swapchain->target->GetTexture(), _blendOutput.Get());
	}



}

void MPSO::CreateInputLayout()
{

	{
		// Mesh
		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",   1, DXGI_FORMAT_R32G32B32_FLOAT,	0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,     0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		CHECK(DX::States::Device->CreateInputLayout(
			&layout[0],
			ARRAYSIZE(layout),
			_vs.GetBufferPointer(),
			_vs.GetBufferSize(),
			&_meshInputLayout));

		DX::SetDebugName(_meshInputLayout.Get(), "Mesh IA layout");
	}

	{
		// Cubemap
		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,  0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,     0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		CHECK(DX::States::Device->CreateInputLayout(
			&layout[0],
			ARRAYSIZE(layout),
			_cubemapvs.GetBufferPointer(),
			_cubemapvs.GetBufferSize(),
			&_cubemapInputLayout));

		DX::SetDebugName(_cubemapInputLayout.Get(), "Cubemap IA layout");
	}

}

void MPSO::InitializeShaders()
{
	_vs = DataSystem::Instance->VertexShaders["mesh"];
	_ps = DataSystem::Instance->PixelShaders["mesh"];

	if (_vs.Get() == nullptr || _ps.Get() == nullptr)
	{
		ERROR("Unable to load mesh shaders in MPSO");
	}

	_cubemapvs = DataSystem::Instance->VertexShaders["cubemap"];
	_cubemapps = DataSystem::Instance->PixelShaders["cubemap"];

	if (_cubemapvs.Get() == nullptr || _cubemapps.Get() == nullptr)
	{
		ERROR("Unable to load cubemap shaders in MPSO");
	}

	_shadowvs = DataSystem::Instance->VertexShaders["shadow"];
	_shadowps = DataSystem::Instance->PixelShaders["shadow"];

	if (_shadowvs.Get() == nullptr || _shadowps.Get() == nullptr)
	{
		ERROR("Unable to load shadow shaders in MPSO");
	}

	_shadowblurcsX = DataSystem::Instance->ComputeShaders["shadowblur-x"];
	_shadowblurcsY = DataSystem::Instance->ComputeShaders["shadowblur-y"];
	_blendcs = DataSystem::Instance->ComputeShaders["blend-gs"];
	_fxaacs = DataSystem::Instance->ComputeShaders["fxaa"];

	if (_shadowblurcsX.Get() == nullptr || _shadowblurcsY.Get() == nullptr || _blendcs.Get() == nullptr || _fxaacs.Get() == nullptr)
	{
		ERROR("Unable to load compute shaders in MPSO");
	}
}

void MPSO::CreateBuffers()
{
	// Skybox
	_mvpbuffer = Buffer<MVPBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	// Updated multiple times per frame
	_modelbuffer = Buffer<ModelBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	// Updated multiple times per frame
	_materialbuffer = Buffer<MaterialProperties>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	// Updated single time per frame
	_camerabuffer = Buffer<CameraBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

	_fxaabuffer = Buffer<FxaaBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);


	_lightspacebuffer = Buffer<LightSpaceBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
		);

	// Updated irregulary, could be non-dynamic resource
	_scenebuffer = Buffer<SceneBuffer>(
		D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);
}

void MPSO::CreateSamplers()
{
	_anisotropicSampler = Sampler(
		D3D11_FILTER::D3D11_FILTER_ANISOTROPIC
	);
	_linearSampler = Sampler(
		D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR
	);
	DirectX::XMFLOAT4 clampcolor{ 1.0f, 1.0f, 1.0f, 1.0f };
	_clampSampler = Sampler(
		D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_BORDER,
		0,
		&clampcolor
	);
	_cubemapSampler = Sampler(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP
	);
	_iblSampler = Sampler(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		1
	);
}

void MPSO::CreateTextures()
{

	{
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
		CD3D11_TEXTURE2D_DESC desc(
			format,
			SHADOWMAP_RES,
			SHADOWMAP_RES,
			1,
			0,
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
		);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		CHECK(DX::States::Device->CreateTexture2D(&desc, nullptr, texture.ReleaseAndGetAddressOf()));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc = {};
		viewdesc.Texture2D.MostDetailedMip = 0;
		viewdesc.Texture2D.MipLevels = 1;
		viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewdesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		CHECK(DX::States::Device->CreateShaderResourceView(texture.Get(), &viewdesc, srv.ReleaseAndGetAddressOf()));
		_shadowmap = Texture2D(texture, srv);
		_shadowmap.SetName("Shadow Map");


		CD3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT);

		CHECK(DX::States::Device->CreateDepthStencilView(texture.Get(), &dsvdesc, _shadowmapDsv.ReleaseAndGetAddressOf()));

	}

	{
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT;
		CD3D11_TEXTURE2D_DESC desc(
			format,
			SHADOWMAP_RES,
			SHADOWMAP_RES,
			1,
			1,
			D3D11_BIND_SHADER_RESOURCE
		);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		CHECK(DX::States::Device->CreateTexture2D(&desc, nullptr, texture.ReleaseAndGetAddressOf()));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc = {};
		viewdesc.Texture2D.MostDetailedMip = 0;
		viewdesc.Texture2D.MipLevels = 1;
		viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewdesc.Format = format;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		CHECK(DX::States::Device->CreateShaderResourceView(texture.Get(), &viewdesc, srv.ReleaseAndGetAddressOf()));
		_blurredShadows = Texture2D(texture, srv);
		_blurredShadows.SetName("Blurred Shadows");

	}

	// Render Target Texture
	{
		_target = std::make_unique<RenderTarget>(
			DX::Formats::Backbuffer,
			_device->swapchain->width,
			_device->swapchain->height
			);

		_target->color = DirectX::XMVECTORF32{ 0.05f, 0.05f, 0.05f, 1.0f };
	}

	// Render Target ShadowMap
	{
		_shadowtarget = std::make_unique<RenderTarget>(
			DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT,
			_device->swapchain->width,
			_device->swapchain->height
			);

		_shadowtarget->color = DirectX::XMVECTORF32{ 1.0f, 1.0f, 1.0f, 1.0f };
	}

}

void MPSO::CreateComputeResources()
{
	// Gaussian filter (BLUR_RADIUS = 3)
	// x = -3, -2, -1, 0, 1, 2, 3
	// Normalized gaussian weights, sum to 1
	const float weights[]{ 0.00598f, 0.060626f, 0.241843f, 0.383103f, 0.241843f, 0.060626f, 0.00598f };

	// Larger filer (BLUR_RADIUS = 5)
	//const float weights[]{ 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };

	// Create immutable buffer only once
	if (_filterWeightsSRV.Get() == nullptr)
	{
		// Filter Weights
		{
			D3D11_BUFFER_DESC filterBufferDesc = {};
			filterBufferDesc.ByteWidth = ARRAYSIZE(weights) * sizeof(float);
			filterBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		// Make it immutable as these are constants
			filterBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
			filterBufferDesc.CPUAccessFlags = 0;
			filterBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA filterSubresourceDesc = {};
			filterSubresourceDesc.pSysMem = weights;
			filterSubresourceDesc.SysMemPitch = NULL;
			filterSubresourceDesc.SysMemSlicePitch = NULL;

			CHECK(DX::States::Device->CreateBuffer(&filterBufferDesc, &filterSubresourceDesc, _filterbuffer.ReleaseAndGetAddressOf()));

			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
				D3D11_SRV_DIMENSION_BUFFER,
				DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,
				0,
				ARRAYSIZE(weights)
			);

			CHECK(DX::States::Device->CreateShaderResourceView(_filterbuffer.Get(), &srvDesc, _filterWeightsSRV.ReleaseAndGetAddressOf()));
		}
	}


	// Filter Compute shader output
	{

		D3D11_TEXTURE2D_DESC blendDesc = {};
		blendDesc.Width = _device->swapchain->width;
		blendDesc.Height = _device->swapchain->height;
		blendDesc.MipLevels = 1;
		blendDesc.ArraySize = 1;
		blendDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
		blendDesc.Usage = D3D11_USAGE_DEFAULT;
		blendDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		blendDesc.CPUAccessFlags = 0;
		blendDesc.MiscFlags = 0;
		blendDesc.SampleDesc.Count = 1;
		blendDesc.SampleDesc.Quality = 0;

		CHECK(DX::States::Device->CreateTexture2D(&blendDesc, nullptr, _filterOutputA.ReleaseAndGetAddressOf()));
		CHECK(DX::States::Device->CreateTexture2D(&blendDesc, nullptr, _filterOutputB.ReleaseAndGetAddressOf()));

		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(
			D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,
			0,
			1
		);

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D,
			DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT,
			0,
			1
		);

		CHECK(DX::States::Device->CreateUnorderedAccessView(_filterOutputA.Get(), &uavDesc, _filterOutputAUAV.ReleaseAndGetAddressOf()));
		CHECK(DX::States::Device->CreateUnorderedAccessView(_filterOutputB.Get(), &uavDesc, _filterOutputBUAV.ReleaseAndGetAddressOf()));
		CHECK(DX::States::Device->CreateShaderResourceView(_filterOutputA.Get(), &srvDesc, _filterOutputASRV.ReleaseAndGetAddressOf()));
		CHECK(DX::States::Device->CreateShaderResourceView(_filterOutputB.Get(), &srvDesc, _filterOutputBSRV.ReleaseAndGetAddressOf()));

	}

	// Blend Compute shader output
	{

		D3D11_TEXTURE2D_DESC blendDesc = {};
		blendDesc.Width = _device->swapchain->width;
		blendDesc.Height = _device->swapchain->height;
		blendDesc.MipLevels = 1;
		blendDesc.ArraySize = 1;
		blendDesc.Format = Common::NoSRGB(DX::Formats::Backbuffer);
		blendDesc.Usage = D3D11_USAGE_DEFAULT;
		blendDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		blendDesc.CPUAccessFlags = 0;
		blendDesc.MiscFlags = 0;
		blendDesc.SampleDesc.Count = 1;
		blendDesc.SampleDesc.Quality = 0;

		CHECK(DX::States::Device->CreateTexture2D(&blendDesc, nullptr, _blendOutput.ReleaseAndGetAddressOf()));

		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(
			D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D,
			Common::NoSRGB(DX::Formats::Backbuffer),
			0,
			1
		);

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D,
			Common::NoSRGB(DX::Formats::Backbuffer),
			0,
			1
		);

		CHECK(DX::States::Device->CreateUnorderedAccessView(_blendOutput.Get(), &uavDesc, _blendOutputUAV.ReleaseAndGetAddressOf()));
		CHECK(DX::States::Device->CreateShaderResourceView(_blendOutput.Get(), &srvDesc, _blendOutputSRV.ReleaseAndGetAddressOf()));
	}

	// FXAA output
	{

		D3D11_TEXTURE2D_DESC fxaaDesc = {};
		fxaaDesc.Width = _device->swapchain->width;
		fxaaDesc.Height = _device->swapchain->height;
		fxaaDesc.MipLevels = 1;
		fxaaDesc.ArraySize = 1;
		fxaaDesc.Format = Common::NoSRGB(DX::Formats::Backbuffer);
		fxaaDesc.Usage = D3D11_USAGE_DEFAULT;
		fxaaDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
		fxaaDesc.CPUAccessFlags = 0;
		fxaaDesc.MiscFlags = 0;
		fxaaDesc.SampleDesc.Count = 1;
		fxaaDesc.SampleDesc.Quality = 0;

		CHECK(DX::States::Device->CreateTexture2D(&fxaaDesc, nullptr, _fxaaOutput.ReleaseAndGetAddressOf()));

		CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(
			D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D,
			Common::NoSRGB(DX::Formats::Backbuffer),
			0,
			1
		);
		CHECK(DX::States::Device->CreateUnorderedAccessView(_fxaaOutput.Get(), &uavDesc, _fxaaOutputUAV.ReleaseAndGetAddressOf()));
	}


	// Craete if it's first time
	if (_blurbuffer.Get() == nullptr)
	{
		_blurbuffer = Buffer<BlurParameters>(
			D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER,
			D3D11_USAGE::D3D11_USAGE_DEFAULT,
			NULL
		);
	}

	BlurParameters params = {};
	params.textureSize = DirectX::XMINT2{ static_cast<int>(_device->swapchain->width), static_cast<int>(_device->swapchain->height) };

	DX::States::Context->UpdateSubresource(_blurbuffer.Get(), 0, NULL, &params, NULL, NULL);


}

void MPSO::Blend(BOOL blurShadows)
{
	const UINT offsets[]{ 0 };

	if (blurShadows)
	{

		// Blur in X
		{
			DX::States::Context->CSSetShader(_shadowblurcsX.Get(), nullptr, NULL);
			// Blur shadows
			ID3D11ShaderResourceView* views[]{ _shadowtarget->GetSRV(), _filterWeightsSRV.Get() };
			//ID3D11ShaderResourceView* blankviews[]{ nullptr };
			DX::States::Context->CSSetShaderResources(0, 2, views);

			ID3D11UnorderedAccessView* bluruavs[]{ _filterOutputAUAV.Get() };

			DX::States::Context->CSSetUnorderedAccessViews(0, 1, bluruavs, offsets);

			DX::States::Context->CSSetConstantBuffers(0, 1, _blurbuffer.GetAddressOf());

			// 256 threads per group
			DX::States::Context->Dispatch(
				std::ceilf(static_cast<float>(_device->swapchain->width) / 256.0f),
				_device->swapchain->height,
				1
			);
		}

		// Blur in Y
		{
			DX::States::Context->CSSetShader(_shadowblurcsY.Get(), nullptr, NULL);

			ID3D11UnorderedAccessView* bluruavs[]{ _filterOutputBUAV.Get() };

			DX::States::Context->CSSetUnorderedAccessViews(0, 1, bluruavs, offsets);

			ID3D11ShaderResourceView* views[]{ _filterOutputASRV.Get(), _filterWeightsSRV.Get() };
			//ID3D11ShaderResourceView* blankviews[]{ nullptr };
			DX::States::Context->CSSetShaderResources(0, 2, views);

			DX::States::Context->CSSetConstantBuffers(0, 1, _blurbuffer.GetAddressOf());

			// 256 threads per group
			DX::States::Context->Dispatch(
				_device->swapchain->width,
				std::ceilf(static_cast<float>(_device->swapchain->height) / 256.0f),
				1
			);
		}
	}


	// Blend in shadows
	DX::States::Context->CSSetShader(_blendcs.Get(), nullptr, NULL);

	ID3D11UnorderedAccessView* uavs[]{ _blendOutputUAV.Get() };
	//ID3D11UnorderedAccessView* blankuavs[]{ nullptr };

	DX::States::Context->CSSetUnorderedAccessViews(0, 1, uavs, offsets);

	if (blurShadows)
	{
		ID3D11ShaderResourceView* srvs[] = { _target->GetSRV(), _filterOutputBSRV.Get() };
		DX::States::Context->CSSetShaderResources(0, 2, srvs);
	}
	else
	{
		ID3D11ShaderResourceView* srvs[] = { _target->GetSRV(), _shadowtarget->GetSRV() };
		DX::States::Context->CSSetShaderResources(0, 2, srvs);
	}


	// 32 threads per group
	DX::States::Context->Dispatch(
		std::ceilf(_device->swapchain->width / 16.0f),
		std::ceilf(_device->swapchain->height / 16.0f),
		1
	);

	ID3D11ShaderResourceView* blanksrvs[]{ nullptr, nullptr };


	DX::States::Context->CSSetShaderResources(0, 2, blanksrvs);

	// Clear views
	// DX::States::Context->CSSetShaderResources(0, 1, blankviews);

}

void MPSO::FXAA(FxaaBuffer* fxaabuffer)
{
	const UINT offsets[]{ 0 };


	D3D11_MAPPED_SUBRESOURCE subressource = _fxaabuffer.Map(D3D11_MAP::D3D11_MAP_WRITE_DISCARD);
	memcpy(subressource.pData, static_cast<const void*>(fxaabuffer), sizeof(FxaaBuffer));
	_fxaabuffer.Unmap();

	DX::States::Context->CSSetShader(_fxaacs.Get(), nullptr, NULL);

	ID3D11UnorderedAccessView* uavs[]{ _fxaaOutputUAV.Get() };
	ID3D11UnorderedAccessView* blankuavs[]{ nullptr };
	DX::States::Context->CSSetUnorderedAccessViews(0, 1, uavs, offsets);

	// Blur shadows
	ID3D11ShaderResourceView* views[]{ _blendOutputSRV.Get() };
	ID3D11ShaderResourceView* blankviews[]{ nullptr };

	DX::States::Context->CSSetShaderResources(0, 1, views);

	DX::States::Context->CSSetConstantBuffers(0, 1, _fxaabuffer.GetAddressOf());

	// 256 threads per group
	DX::States::Context->Dispatch(
		std::ceilf(static_cast<float>(_device->swapchain->width) / 16.0f),
		std::ceilf(static_cast<float>(_device->swapchain->height) / 16.0f),
		1
	);

	DX::States::Context->CSSetUnorderedAccessViews(0, 1, blankuavs, offsets);
	DX::States::Context->CSSetShaderResources(0, 1, blankviews);
}



void MPSO::CreateCubeMap(const Texture2D& texture)
{
	_skybox = std::make_unique<Sphere>("Cubemap", 30);
	_skybox->material->textures.diffuse = texture;
}
