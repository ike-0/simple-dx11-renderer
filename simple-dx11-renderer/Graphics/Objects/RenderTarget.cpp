#include "RenderTarget.h"

// Create render target for backbuffer or other kind of texture
RenderTarget::RenderTarget(Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture, DXGI_FORMAT _format, UINT _width, UINT _height)
	: _texture(texture), _srv(nullptr), _dsv(nullptr), _rtv(nullptr), color{ 0.05f, 0.05f, 0.05f, 1.0f },
	format(_format), _bsrv(0), width(_width), height(_height)
{
	CreateTargets();
}

// Create texture and render target
RenderTarget::RenderTarget(DXGI_FORMAT _format, UINT _width, UINT _height)
	: color{ 0.1f, 0.1f, 0.1f, 1.0f }, _srv(nullptr), _dsv(nullptr), _rtv(nullptr),
	format(_format), width(_width), height(_height), _bsrv(1)
{
	CreateTexture();
	CreateTargets();
	CreateShaderResourceView();
}

RenderTarget::~RenderTarget()
{
	DestroyTargets();
}

void RenderTarget::Resize(UINT _width, UINT _height)
{
    // Check if necessary
    if (width == _width && height == _height)
        return;

    width = _width;
    height = _height;

    DestroyTargets();
    if (_bsrv)
        CreateTexture();

    CreateTargets();

    if (_bsrv)
        CreateShaderResourceView();
}


void RenderTarget::CreateTexture()
{
    CD3D11_TEXTURE2D_DESC desc(
        format,
        width,
        height,
        1,
        1,
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE
    );

    CHECK(DX::States::Device->CreateTexture2D(
        &desc,
        nullptr,
        _texture.ReleaseAndGetAddressOf()
    ));
}

void RenderTarget::CreateTargets()
{
    CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
        D3D11_RTV_DIMENSION_TEXTURE2D,
        format
    );

    CHECK(DX::States::Device->CreateRenderTargetView(
        _texture.Get(),
        &renderTargetViewDesc,
        _rtv.ReleaseAndGetAddressOf()
    ));


    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DX::Formats::Depthbuffer,
        width,
        height,
        1, // This depth stencil view has only one texture.
        1, // Use a single mipmap level.
        D3D11_BIND_DEPTH_STENCIL
    );

    CHECK(DX::States::Device->CreateTexture2D(
        &depthStencilDesc,
        nullptr,
        _stencil.ReleaseAndGetAddressOf()
    ));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    CHECK(DX::States::Device->CreateDepthStencilView(
        _stencil.Get(),
        &depthStencilViewDesc,
        _dsv.ReleaseAndGetAddressOf()
    ));
}

void RenderTarget::CreateShaderResourceView()
{
    if (!_bsrv)
        return;

    CHECK(DX::States::Device->CreateShaderResourceView(_texture.Get(),
        nullptr, _srv.ReleaseAndGetAddressOf()));
}

void RenderTarget::DestroyTargets()
{
    _rtv.Reset();
    _dsv.Reset();
    _srv.Reset();

    _stencil.Reset();
    _texture.Reset();
}
