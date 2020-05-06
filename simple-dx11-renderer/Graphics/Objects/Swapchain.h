#pragma once
#include "Graphics/Objects/RenderTarget.h"

class Swapchain
{
public:
	Swapchain(UINT _width, UINT _height, BOOL vsync, HWND hwnd);
	~Swapchain();

	void SwapBuffers();
	void ClearTextures();
	void SetClearColor(DirectX::XMVECTORF32 color);
	void SetClearColor(DirectX::XMFLOAT4 color);
	void Resize(int _width, int _height);

	UINT width;
	UINT height;

	std::unique_ptr<RenderTarget>			target;

private:
	void Create();
	void HandleResult(HRESULT hr);
	void CreateTextures(int _width, int _height);
	void DestroyTextures();

	BOOL _vsync;
	HWND _hwnd;
	UINT _swapflag;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain;
};