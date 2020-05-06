#include "RenderWindow.h"

RenderWindow::RenderWindow(HINSTANCE hInstance, WNDPROC winproc)
    : hwnd(nullptr), _hinst(nullptr), _wcex(nullptr), viewport{ 0 }, _open(0)
{
    RegisterWindow(hInstance, winproc);
}

RenderWindow::~RenderWindow()
{
    if (_open)
    {
        Close();
    }
}

void RenderWindow::Open(UINT width, UINT height)
{
    // Define rectangle struct
    RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    // Adjust window according to rectangle
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // Create window
    hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        APP_NAME,                       // Window class
        APP_NAME,                       // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, // Size and position
        nullptr,                        // Parent window    
        nullptr,                        // Menu
        _hinst,                         // Instance handle
        nullptr                         // Additional application data
    );

    if (!hwnd)
    {
        ERROR("Failure in creating window");
    }

    // Assign this Render Window as user data
    // Not actualy used in this example
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Get back actual rectangle size and update width and height
    GetClientRect(hwnd, &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    // Construct Viewport
    viewport = { 0 };
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.000001f;
    viewport.MaxDepth = 0.99999f;

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    _open = 1;
}

void RenderWindow::Close()
{
    // Close window
    CloseWindow(hwnd);
    DestroyWindow(hwnd);
    //ClosedEvent.Invoke();
    _open = 0;
}

void RenderWindow::RegisterWindow(HINSTANCE hInstance, WNDPROC winproc)
{
    // In case you want custom window icon
    //const char* icopath = "Icon.ico";
    //HICON icon = (HICON)LoadImageA(
    //    hInstance,
    //    icopath,
    //    IMAGE_ICON,
    //    0,
    //    0,
    //    LR_DEFAULTSIZE | LR_LOADFROMFILE
    //    );

    LPCSTR name = APP_NAME;
    _wcex = new WNDCLASSEX{ 0 };
    _wcex->cbSize = sizeof(WNDCLASSEX);
    _wcex->style = CS_HREDRAW | CS_VREDRAW;
    _wcex->lpfnWndProc = winproc;
    _wcex->cbClsExtra = 0;
    _wcex->cbWndExtra = 0;
    _wcex->hInstance = hInstance;
    _wcex->hIcon = nullptr;         //_wcex->hIcon = icon;
    _wcex->hCursor = LoadCursor(nullptr, IDC_ARROW);
    _wcex->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    _wcex->lpszMenuName = nullptr;
    _wcex->lpszClassName = name;
    _wcex->hIconSm = nullptr;       //_wcex->hIconSm = icon;

    // Register class
    ATOM cres = RegisterClassEx(_wcex);
    if (!cres)
    {
        DWORD error = GetLastError();
        ERROR("Could not create Windows class");
    }
}
