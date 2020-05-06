#include "DX.h"

Microsoft::WRL::ComPtr<ID3D11Device5>				DX::States::Device = nullptr;
Microsoft::WRL::ComPtr<IDXGIFactory4>				DX::States::Factory = nullptr;
Microsoft::WRL::ComPtr<ID3D11DeviceContext4>		DX::States::Context = nullptr;
Microsoft::WRL::ComPtr<IDXGIAdapter3>				DX::States::Adapter = nullptr;
Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>	DX::States::Annotations = nullptr;

DXGI_FORMAT DX::Formats::Backbuffer = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
DXGI_FORMAT DX::Formats::Depthbuffer = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

D3D_FEATURE_LEVEL DX::Internal::FeatureLevel = D3D_FEATURE_LEVEL_11_1;
BOOL DX::Internal::DebugLayer = FALSE;


void DX::Initialize(DXGI_FORMAT fbackbuffer, DXGI_FORMAT fdepthbuffer, D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer)
{
	DX::Formats::Backbuffer = fbackbuffer;
	DX::Formats::Depthbuffer = fdepthbuffer;
	DX::Internal::CreateStates(featureLevel, debugLayer);
}

void DX::Destroy()
{
	DX::States::Context->Flush();
	DX::States::Annotations.Reset();
	DX::States::Context.Reset();
	DX::States::Device.Reset();
	DX::States::Adapter.Reset();
	DX::States::Factory.Reset();
}

void DX::CheckCurrentFactory()
{
	if (!DX::States::Factory->IsCurrent())
	{
		WARN("DX11 Factory is out of date");
		DX::States::Factory = DX::Internal::CreateFactory(
			DX::Internal::FeatureLevel,
			DX::Internal::DebugLayer
		);
	}
}

void DX::HandleDeviceRemoved(HRESULT hr)
{
#ifdef _DEBUG
	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		WARN("Device Lost on Present: Reason code - " + std::to_string(DX::States::Device->GetDeviceRemovedReason()));
	}
	else
	{
		WARN("Device Lost on Present: Reason code - " + std::to_string(hr));
	}
#endif
	//D3D11System::InvokeRecreation();

	// This is the point where we serialize all states or recreate all devices
	ERROR("Rendering Device Lost");
}

void DX::HandleGraphicsOptions()
{
	D3D11_FEATURE_DATA_D3D11_OPTIONS2 features;
	DX::States::Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &features, sizeof(features));

	if (features.ConservativeRasterizationTier == D3D11_CONSERVATIVE_RASTERIZATION_NOT_SUPPORTED)
	{
		WARN("Conservative Rasterization is not supported");
		Options::Graphics::ConservativeRasterization = 0;
	}
	//if (features.ROVsSupported == FALSE)
	//{
	//	WARN("Rasterizer Order Views is not supported");
	//	Options::Graphics::RasterizerOrderViews = 0;
	//}
	//if (features.TiledResourcesTier != D3D11_TILED_RESOURCES_TIER_3)
	//{
	//	WARN("Tiled 3D Resources are not supported");
	//	Options::Graphics::Tiled3DResources = 0;
	//}
	//if (features.TiledResourcesTier == D3D11_TILED_RESOURCES_NOT_SUPPORTED)
	//{
	//	WARN("Tiled 2D Resources are not supported");
	//	Options::Graphics::Tiled2DResources = 0;
	//}

}

void DX::Internal::CreateStates(D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer)
{
	DX::Internal::DebugLayer = debugLayer;
	DX::Internal::FeatureLevel = featureLevel;

	DX::States::Factory = CreateFactory(featureLevel, debugLayer);
	DX::States::Adapter = CreateAdapter(DX::States::Factory);
	auto res = CreateDevice(DX::States::Adapter.Get(), featureLevel, debugLayer);
	DX::States::Device = std::get<0>(res);
	DX::States::Context = std::get<1>(res);
	DX::States::Annotations = CreateAnnotations(DX::States::Context);
}

Microsoft::WRL::ComPtr<IDXGIFactory4> DX::Internal::CreateFactory(D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer)
{
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	UINT flags = 0;
#ifdef _DEBUG
	if (debugLayer)
	{
		// Enable the debug interface

		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			ERROR("Could not create debug interface");
		}

		flags |= DXGI_CREATE_FACTORY_DEBUG;

		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

		DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
		{
			80 /* The swapchain's adapter does not control the output on which the swapchain's window resides. */,
		};
		DXGI_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
}
#endif
	CHECK(CreateDXGIFactory2(flags, IID_PPV_ARGS(factory.ReleaseAndGetAddressOf())));
	return factory;
}

Microsoft::WRL::ComPtr<IDXGIAdapter3> DX::Internal::CreateAdapter(Microsoft::WRL::ComPtr<IDXGIFactory4>& factory)
{
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)

	Microsoft::WRL::ComPtr<IDXGIFactory7> factory7;

	HRESULT hr = factory.As(&factory7);
	if (SUCCEEDED(hr))
	{

		for (UINT adapterIndex = 0;
			SUCCEEDED(factory7->EnumAdapterByGpuPreference(
				adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
			adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			CHECK(adapter->GetDesc1(&desc));

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Skip software renderer
				continue;
			}

#ifdef _DEBUG
				INFO("Device found: " + Common::ToString(desc.Description));
#endif
				break;
		}
	}
#else
	//#error Windows 10 version is too old
#endif
	// Workaround for built in graphics debugger
	if (!adapter.Get())
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
		for (UINT adapterID = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterID, &adapter1); ++adapterID)
		{
			DXGI_ADAPTER_DESC1 desc;
			CHECK(adapter1->GetDesc1(&desc));

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

#ifdef _DEBUG
				INFO("Device found: " + Common::ToString(desc.Description));
#endif
				break;
		}
		if (adapter1.Get())
		{
			adapter1.As(&adapter);
		}
	}



	// Handle as sofware rasterizer
	if (!adapter.Get())
	{
		if (SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
		{
			WARN("Running using WARP");
		}
		else
		{
			ERROR("Found no suitable adapters");
		}
	}

	return adapter;
}

Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> DX::Internal::CreateAnnotations(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& context)
{
	Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> annotations;
	context.As(&annotations);
	return annotations;
}


std::tuple<Microsoft::WRL::ComPtr<ID3D11Device5>, Microsoft::WRL::ComPtr<ID3D11DeviceContext4>> DX::Internal::CreateDevice(IDXGIAdapter3* adapter, D3D_FEATURE_LEVEL featureLevel, BOOL debugLayer)
{
	UINT creationFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Device5> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext4> context;

#if defined(_DEBUG)
	if (debugLayer)
	{
		if (SdkLayersAvailable())
		{
			// If the project is in a debug build, enable debugging via SDK Layers with this flag.
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		else
		{
			ERROR("Direct3D Debug Device is not available");
		}
	}

#endif
	// Determine DirectX hardware feature levels this app will support.
	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	Microsoft::WRL::ComPtr<ID3D11Device> deviceInt;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> intContext;


	UINT featLevelCount = _countof(featureLevels);

	HRESULT hr = D3D11CreateDevice(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		creationFlags,
		featureLevels,
		featLevelCount,
		D3D11_SDK_VERSION,
		deviceInt.GetAddressOf(),  // Returns the Direct3D device created.
		&featureLevel,         // Returns feature level of device created.
		intContext.GetAddressOf()  // Returns the device immediate context.
		);
	if (FAILED(hr))
	{
		// Create Software rasterizer if hardware is unavailable
		// Remote desktop ?
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
			nullptr,
			creationFlags,
			featureLevels,
			featLevelCount,
			D3D11_SDK_VERSION,
			deviceInt.GetAddressOf(),
			&featureLevel,
			intContext.GetAddressOf()
			);
		CHECK(hr);
		WARN("Running on Software comaptibility mode");
	}

#ifndef NDEBUG
	if (debugLayer)
	{
		Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
		if (SUCCEEDED(deviceInt.As(&d3dDebug)))
		{
			Microsoft::WRL::ComPtr<ID3D11InfoQueue> d3dInfoQueue;
			if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
			{

				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

				D3D11_MESSAGE_ID hide[] =
				{
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				};
				D3D11_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
			}
		}
	}
#endif

	CHECK(deviceInt.As(&device));
	CHECK(intContext.As(&context));

	return std::tuple<Microsoft::WRL::ComPtr<ID3D11Device5>, Microsoft::WRL::ComPtr<ID3D11DeviceContext4>>(device, context);
}
