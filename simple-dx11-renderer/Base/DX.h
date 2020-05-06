#pragma once

namespace DX
{
	void Initialize(DXGI_FORMAT fbackbuffer, DXGI_FORMAT fdepthbuffer, D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1, BOOL debugLayer = 0);
	void Destroy();
	void CheckCurrentFactory();
	void HandleDeviceRemoved(HRESULT hr);
	void HandleGraphicsOptions();

	namespace States
	{
		extern Microsoft::WRL::ComPtr<ID3D11Device5>				Device;
		extern Microsoft::WRL::ComPtr<IDXGIAdapter3>				Adapter;
		extern Microsoft::WRL::ComPtr<IDXGIFactory4>				Factory;
		extern Microsoft::WRL::ComPtr<ID3D11DeviceContext4>			Context;
		extern Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>	Annotations;
	}
	namespace Formats
	{
		extern DXGI_FORMAT Backbuffer;
		extern DXGI_FORMAT Depthbuffer;
	}

#if defined(_DEBUG) || defined(STAGING)
	inline void SetDebugName(IDXGIObject* obj, const std::string& name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, name.length() - 1, name.c_str());
	}
	inline void SetDebugName(ID3D11Device* obj, const std::string& name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, name.length() - 1, name.c_str());
	}
	inline void SetDebugName(ID3D11DeviceChild* obj, const std::string& name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, name.length() - 1, name.c_str());
	}

	inline void SetDebugName(IDXGIObject* obj, const char* name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
	}
	inline void SetDebugName(ID3D11Device* obj, const char* name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
	}
	inline void SetDebugName(ID3D11DeviceChild* obj, const char* name)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
	}

	inline void BeginEvent(LPCWSTR name)
	{
		DX::States::Annotations->BeginEvent(name);
	}
	inline void EndEvent(LPCWSTR name)
	{
		DX::States::Annotations->EndEvent();
	}
	inline void SetMarker(LPCWSTR name)
	{
		DX::States::Annotations->SetMarker(name);
	}

#else
	inline void SetDebugName(const IDXGIObject* obj, const std::string name) { };
	inline void SetDebugName(const ID3D11Device* obj, const std::string name) { };
	inline void SetDebugName(const ID3D11DeviceChild* obj, const std::string name) { };
	inline void SetDebugName(const IDXGIObject* obj, const char* name) { };
	inline void SetDebugName(const ID3D11Device* obj, const char* name) { };
	inline void SetDebugName(const ID3D11DeviceChild* obj, const char* name) { };

	inline void BeginEvent(LPCWSTR name) { };
	inline void EndEvent(LPCWSTR name) { };
	inline void SetMarker(LPCWSTR name) { };
#endif

	namespace Internal
	{
		void CreateStates(D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer);

		Microsoft::WRL::ComPtr<IDXGIFactory4>			CreateFactory(D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer);
		Microsoft::WRL::ComPtr<IDXGIAdapter3>			CreateAdapter(Microsoft::WRL::ComPtr<IDXGIFactory4>& factory);
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> CreateAnnotations(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& context);
		std::tuple<Microsoft::WRL::ComPtr<ID3D11Device5>, Microsoft::WRL::ComPtr<ID3D11DeviceContext4>>	CreateDevice(IDXGIAdapter3* adapter, D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer);

		extern D3D_FEATURE_LEVEL	FeatureLevel;
		extern BOOL					DebugLayer;
	}
}
