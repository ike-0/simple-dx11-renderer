#include "simple-dx11-renderer.h"
#include "Windowing/RenderWindow.h"
#include "examples/imgui_impl_win32.h"

// Include libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#ifdef _DEBUG
#pragma comment(lib, "dxguid.lib")
#endif


RenderWindow* pwnd = nullptr;
SimpleRenderer* papp = nullptr;


// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Forawrd declare callbacks
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// Entry point
INT WINAPI WinMain(
	_In_ HINSTANCE		hInstance,
	_In_opt_ HINSTANCE	hPrevInstance,
	_In_ LPSTR			lpCmdLine,
	_In_ int			nCmdShow
	)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// Check Intrinsic support
	if (!DirectX::XMVerifyCPUSupport())
	{
		return -1;
	}
#ifdef _DEBUG
	// Set debug reporting mode as assert
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

	// Allocate console
	BOOL result = AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);


    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
    {
        ERROR("Failed to initialize \"Windows Imaging Component\"");
    }

#ifdef _DEBUG
	BOOL dlayer = 1;
#else
	BOOL dlayer = 0;
#endif

    // In case we need to do thread pool execution
    // Not used in this example
    InternalHelper::Initialize();
	Concurrency::SchedulerPolicy policy(3,
		concurrency::MinConcurrency, 6,
		concurrency::MaxConcurrency, 8,
		concurrency::ContextPriority, THREAD_PRIORITY_HIGHEST
		);
	Concurrency::Scheduler::SetDefaultSchedulerPolicy(policy);
	Concurrency::task init = concurrency::create_task([]() {
		return std::string("SUCCESS");
	});
	std::string initresult = init.get();
	INFO("Multithreaded initialization: " + initresult);



	// NSight flag to disable debug layer, break on D3D11/D3D12 exception and so on
	std::string params(lpCmdLine);
	auto res = params.find("-nd");
	if (res != std::string::npos)
	{
		dlayer = 0;
	}

	// Setup DirectX
	DX::Initialize(
		DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D_FEATURE_LEVEL_11_1,
		dlayer
	);
    DX::HandleGraphicsOptions();


    // Actual application code
    {
        // Create Window
        pwnd = new RenderWindow(hInstance, WindowProc);
        pwnd->Open(1240, 860);

        papp = new SimpleRenderer(pwnd);
        papp->Run(); // Initialize and enter main loop

        // Closing
        pwnd->Close();
    }


	// Cleanup
	delete papp;
	delete pwnd;
    pwnd = nullptr;
    papp = nullptr;
	DX::Destroy();

	return 0;
}


// Classic
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool in_sizemove = false;
    static bool resizing = false;
    static bool in_suspend = false;
    static bool minimized = false;
    static bool maximizebutton = false;

    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_PAINT:
    {

        if (maximizebutton && (pwnd != nullptr))
        {
            RECT rc;
            GetClientRect(pwnd->hwnd, &rc);

            pwnd->OnResize(rc.right - rc.left, rc.bottom - rc.top);
            maximizebutton = false;
        }

        PAINTSTRUCT ps;

        // We are not rendering here, just respond
        HDC hdc = BeginPaint(hwnd, &ps);

        // FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1)); // Not actualy needed

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_SYSCOMMAND:
    {
        // Minimize/Maximize button
        // SC_RESTORE is actualy minimize...
        if ((pwnd != nullptr) & (wParam == SC_MAXIMIZE || wParam == SC_MINIMIZE || wParam == SC_RESTORE))
        {
            // Cannot resize now because window has not resized yet.
            // Just mark this flag and let on WM_PAINT resize
            maximizebutton = true;
        }
        break;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            if (!minimized)
            {
                minimized = true;
                if (!in_suspend && pwnd)
                    pwnd->OnMinimize();
                in_suspend = true;
            }
        }
        else if (minimized)
        {
            minimized = false;
            if (in_suspend && pwnd)
                pwnd->OnResume();
            in_suspend = false;
        }
        else if (pwnd)
        {
            resizing = true;
        }
        break;
    }
    case WM_ENTERSIZEMOVE:
    {
        in_sizemove = true;
        break;
    }
    case WM_EXITSIZEMOVE:
    {
        if (pwnd && resizing)
        {
            RECT rc;
            GetClientRect(pwnd->hwnd, &rc);

            pwnd->OnResize(rc.right - rc.left, rc.bottom - rc.top);
            resizing = false;
        }
        in_sizemove = false;

        break;
    }
    case WM_ACTIVATEAPP:
    {
        if (pwnd)
        {
            DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
            DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
        }
        break;
    }
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
    {
        DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        break;
    }
    case WM_SYSKEYDOWN:
    {
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        break;
    }
    case WM_MENUCHAR:
    {
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
    }
    case WM_CLOSE:
    {
        // Check if there is a need to close
        if (papp->OnClosing())
        {
            pwnd->Close();
        }
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}