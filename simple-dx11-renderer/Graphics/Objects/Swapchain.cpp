#include "Swapchain.h"

Swapchain::Swapchain(UINT _width, UINT _height, BOOL vsync, HWND hwnd)
	: width(_width), height(_height), _vsync(vsync), _hwnd(hwnd), _swapflag(0)
{
	Create();
}

Swapchain::~Swapchain()
{
	_swapchain.Reset();
}

void Swapchain::SwapBuffers()
{
	HRESULT hr;
	if (_vsync)
	{
		// Blocks to screen refresh rate, Vsync On
		hr = _swapchain->Present(1, 0);
	}
	else
	{
		// Full on tearing, Vsync Off
		hr = _swapchain->Present(0, _swapflag);
	}
	
	HandleResult(hr);
}

void Swapchain::ClearTextures()
{
	target->ClearView();
}

void Swapchain::SetClearColor(DirectX::XMVECTORF32 color)
{
	target->color = color;
}
void Swapchain::SetClearColor(DirectX::XMFLOAT4 color)
{
	target->color = DirectX::XMVECTORF32{ color.x, color.y, color.z, color.w };
}

void Swapchain::Resize(int _width, int _height)
{
	width = _width;
	height = _height;

	DestroyTextures();

	// If the swap chain already exists, resize it.
	HRESULT hr = _swapchain->ResizeBuffers(
		MAX_BACKBUFFER_COUNT,
		width,
		height,
		Common::NoSRGB(DX::Formats::Backbuffer),
		(_vsync || !Options::Graphics::VariableRefreshRate) ? 0u : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
	);

	HandleResult(hr);

	CreateTextures(width, height);
}

void Swapchain::Create()
{
	// Create a descriptor for the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = Common::NoSRGB(DX::Formats::Backbuffer);
	swapChainDesc.Stereo = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = MAX_BACKBUFFER_COUNT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = (_vsync || !Options::Graphics::VariableRefreshRate)? 0u : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;

	if (Options::Graphics::VariableRefreshRate)
		_swapflag = DXGI_PRESENT_ALLOW_TEARING;
	else
		_swapflag = 0;


	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchainInt;

	// Create a SwapChain from a Win32 window.
	try
	{
		HRESULT res = DX::States::Factory->CreateSwapChainForHwnd(
			DX::States::Device.Get(),
			_hwnd,
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			swapchainInt.ReleaseAndGetAddressOf()
		);
	}
	catch (const std::exception& e)
	{
		ERROR(e.what());
	}

	CHECK(swapchainInt.As(&_swapchain));

	DX::SetDebugName(_swapchain.Get(), "Swapchain");

	// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	// Due to allow tearing for exclusive windowed mode
	CHECK(DX::States::Factory->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER));

	CreateTextures(width, height);
}

void Swapchain::HandleResult(HRESULT hr)
{
	// If the device was removed either by a disconnection or a driver upgrade, we
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		DX::HandleDeviceRemoved(hr);
	}
	else
	{
		CHECK(hr);

		DX::CheckCurrentFactory();
	}
}

void Swapchain::CreateTextures(int _width, int _height)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> swapchainTexture;

	CHECK(_swapchain->GetBuffer(0, IID_PPV_ARGS(swapchainTexture.ReleaseAndGetAddressOf())));

	width = _width;
	height = _height;

	// Manualy resolve to sRGB
	target = std::make_unique<RenderTarget>(swapchainTexture, DX::Formats::Backbuffer, width, height);

	DX::States::Context->OMSetRenderTargets(1, target->GetRTVAddress(), target->GetDSV());

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = _width;
	vp.TopLeftY = _height;
	DX::States::Context->RSSetViewports(1, &vp);

	DX::SetDebugName(target->GetRTV(), "Backbuffer Render Target View");
	DX::SetDebugName(target->GetDSV(), "Backbuffer Depth Stencil View");
	DX::SetDebugName(target->GetTexture(), "Backbuffer Render Target");
	DX::SetDebugName(target->GetStencil(), "Backbuffer Depth Stencil");
}

void Swapchain::DestroyTextures()
{
	DX::States::Context->OMSetRenderTargets(0, 0, 0);
	DX::States::Context->ClearState();
	target.reset();
	DX::States::Context->Flush();
}
