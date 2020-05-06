#include "Renderer.h"

Renderer::Renderer(RenderWindow* window) : _window(window)
{
	_device = std::make_unique<LogicalDevice>(window);
}

Renderer::~Renderer()
{
	_mpso.reset();
	_device.reset();
}

void Renderer::Initialize()
{
	_mpso = std::make_unique<MPSO>(_device.get());

	_window->ResizeEvent += [this](UINT w, UINT h) {
		_mpso->Resize(w, h);
	};

	Texture2D tex = TextureLoader::LoadCubemapFromFile(Path::Relative("textures\\cubemap.dds"));

	_mpso->CreateCubeMap(tex);

	_device->swapchain->SetClearColor( DirectX::XMFLOAT4{ 0.01f, 0.01f, 0.01f, 1.0f });
	
	
}

void Renderer::SetCamera(Camera* camera)
{
	_camera = camera;
}

void Renderer::StagePrepare()
{
#ifdef _DEBUG
	assert(_camera != nullptr);
#endif

	_device->ClearBackbuffer();

	DirectX::XMMATRIX proj = _camera->GetProjectionMatrix();
	DirectX::XMMATRIX view = _camera->GetViewMatrix();

	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	DirectX::XMStoreFloat3(&pos, _camera->GetPosition());

	CameraBuffer cbuff{
		DirectX::XMMatrixMultiplyTranspose(view, proj),
		pos,
		0,
		DirectX::XMFLOAT3{0.0,0.0f,0.0f},
		0
	};
	SceneBuffer sbuff{};

	sbuff.sunpos = _scene->sunpos;
	sbuff.suncolor = _scene->suncolor;
	sbuff.iblColor = _scene->iblcolor;
	sbuff.iblIntensity = _scene->iblIntensity;
	sbuff.ambientcolor = DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f };
	sbuff.preciseShadows = _scene->moreShadowSamples;

	_mpso->Prepare(&cbuff, &sbuff);
	_drawmodels.clear();
	_modelcount = 0;
}


void Renderer::StageSubmit()
{
	_device->Present();
}

void Renderer::StageImGui()
{
	if (_scene->fxaa)
	{
		FxaaBuffer fxaabuff = {};
		fxaabuff.textureSize = DirectX::XMINT2{ 
			static_cast<int>(_device->swapchain->width),
			static_cast<int>(_device->swapchain->height)
		};
		fxaabuff.bias = _scene->bias;
		fxaabuff.biasMin = _scene->biasMin;
		fxaabuff.spanMax = _scene->spanMax;

		_mpso->Finish(_scene->gaussianShadowBlur, &fxaabuff);
	}
	else
	{
		_mpso->Finish(_scene->gaussianShadowBlur, nullptr);
	}
	_device->SetRenderTargetBackbuffer();
}

