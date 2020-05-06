#pragma once
#include "Windowing/RenderWindow.h"
#include "Graphics/Objects/Swapchain.h"
#include "Graphics/Objects/RenderTarget.h"

enum class RasterizerType
{
	Solid = 0,
	SolidNoCull = 1,
	SolidFrontCull = 2,
	Wireframe = 3,
	Conservative = 4
};

enum class DepthStencilType
{
	Less = 0,
	LessEqual = 1
};

class LogicalDevice
{
public:

	LogicalDevice(RenderWindow* window);
	~LogicalDevice();

	std::unique_ptr<Swapchain> swapchain;
	//inline void SetViewport(const D3D11_VIEWPORT viewport) { _activeViewport = viewport; };
	inline void SetRenderTarget(const RenderTarget* target)
	{
		DX::States::Context->OMSetRenderTargets(1, target->GetRTVAddress(), target->GetDSV());
		DX::States::Context->RSSetViewports(1, &_window->viewport);
	}
	// Uses main target depth buffer
	inline void SetRenderTargets(const RenderTarget* maintarget, const RenderTarget* second)
	{
		ID3D11RenderTargetView* views[]{ maintarget->GetRTV(), second->GetRTV() };
		DX::States::Context->OMSetRenderTargets(2, views, maintarget->GetDSV());
		DX::States::Context->RSSetViewports(1, &_window->viewport);
	}
	inline void SetRenderTargetBackbuffer()
	{
		ID3D11RenderTargetView* views[]{ swapchain->target->GetRTV(), nullptr };

		DX::States::Context->OMSetRenderTargets(2, views, swapchain->target->GetDSV());
		DX::States::Context->RSSetViewports(1, &_window->viewport);
	}
	inline void SetRenderTargetNothing()
	{
		ID3D11RenderTargetView* views[]{ nullptr, nullptr };

		DX::States::Context->OMSetRenderTargets(2, views, nullptr);
		DX::States::Context->RSSetViewports(1, &_window->viewport);
	}
	void SetDepthStencil(DepthStencilType type)
	{
		switch (type)
		{
		case DepthStencilType::Less:
			DX::States::Context->OMSetDepthStencilState(_depthStateLess.Get(), 0);
			return;
		case DepthStencilType::LessEqual:
			DX::States::Context->OMSetDepthStencilState(_depthStateLessEqual.Get(), 0);
			return;
		default:
			break;
		}
	}
	void SetRasterizer(RasterizerType type)
	{
		switch (type)
		{
		case RasterizerType::Solid:
			DX::States::Context->RSSetState(_rasterizerStateSolid.Get());
			return;
		case RasterizerType::SolidNoCull:
			DX::States::Context->RSSetState(_rasterizerStateSolidNoCull.Get());
			return;
		case RasterizerType::SolidFrontCull:
			DX::States::Context->RSSetState(_rasterizerStateSolidFrontCull.Get());
			return;
		case RasterizerType::Wireframe:
			DX::States::Context->RSSetState(_rasterizerStateWireframe.Get());
			return;
		case RasterizerType::Conservative:
			DX::States::Context->RSSetState(_rasterizerStateConservative.Get());
			return;
		default:
			break;
		}
	}

	inline void DrawIndexed(const UINT idxCount, const UINT idxStart = 0, const UINT baseVertexLocation = 0) const
	{
		DX::States::Context->DrawIndexed(idxCount, idxStart, baseVertexLocation);
	}
	//inline void SetViewport(const D3D11_VIEWPORT& viewport) { DX::States::Context->RSSetViewports(1, &viewport); }
	inline void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) { DX::States::Context->IASetPrimitiveTopology(topology); };
	inline void ClearBackbuffer() {
		swapchain->ClearTextures();
	}
	inline void Present()
	{
		swapchain->SwapBuffers();
	}
private:
	void CreateDepthState();
	void CreateRasterizerStates();
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		_depthStateLess;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		_depthStateLessEqual;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>		_rasterizerStateSolid;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>		_rasterizerStateSolidNoCull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>		_rasterizerStateSolidFrontCull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>		_rasterizerStateWireframe;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>		_rasterizerStateConservative;
	RenderWindow* _window;

	//D3D11_VIEWPORT _activeViewport;
};