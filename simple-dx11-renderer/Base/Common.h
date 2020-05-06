#pragma once
#include <shlwapi.h>

// Application Defines
#define APP_NAME "Simple Renderer"
#define MAX_BACKBUFFER_COUNT 3


#if defined(_DEBUG)
// Check for SDK Layer support.
inline bool SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
        nullptr,                    // Any feature level will do.
        0,
        D3D11_SDK_VERSION,
        nullptr,                    // No need to keep the D3D device reference.
        nullptr,                    // No need to know the feature level.
        nullptr                     // No need to keep the D3D device context reference.
        );
    return SUCCEEDED(hr);
}
#endif

class Common
{
public:

    // Return Non SRGB colorspace type
    static inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt)
    {
        switch (fmt)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
        default:                                return fmt;
        }
    }


    // Converts UTF-8 string to unicode
    static inline std::wstring ToWString(const std::string& string) {
        return std::wstring(string.begin(), string.end());
    }

    // Converts back unicode to UTF-8
    // Note that this is a simple conversion not accounting for actual characters
    static std::string ToString(const std::wstring& string) {
        return std::string(string.begin(), string.end());
    }

    static inline std::string GetMessageFromHR(HRESULT hr)
    {
        CHAR   wszMsgBuff[512];  // Buffer for text.

        DWORD   dwChars;  // Number of chars returned.

        // Try to get the message from the system errors.
        dwChars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            hr,
            0,
            wszMsgBuff,
            512,
            NULL);

        return std::string(wszMsgBuff);
    }

};
namespace InternalHelper
{
    extern std::filesystem::path ExecutablePath;
    extern std::filesystem::path DataPath;
    extern WCHAR RawDataPath[MAX_PATH];
    extern std::filesystem::path ShaderSourcePath;

    void Initialize();

    constexpr void CheckResult(HRESULT x)
    {
        if (FAILED(x))
        {
            DebugBreak();
            throw std::exception();
        }
    }
    static inline void Terminate(const char* res)
    {
        MessageBox(nullptr, TEXT(res), TEXT("Fatal Failure"), MB_OK);
        ExitProcess(1);
    }
    static inline void Terminate(std::string res)
    {
        MessageBox(nullptr, TEXT(res.c_str()), TEXT("Fatal Failure"), MB_OK);
        ExitProcess(1);
    }

}
namespace Path
{
    // Gets relative path to data folder
    inline std::filesystem::path Relative(const std::string& path)
    {
        return std::filesystem::path(InternalHelper::DataPath).append(path).lexically_normal();
    }
    inline std::filesystem::path Relative(const char* path)
    {
        return std::filesystem::path(InternalHelper::DataPath).append(path).lexically_normal();
    }

    inline std::filesystem::path Relative(const std::filesystem::path& path)
    {
        auto npath = std::filesystem::path(InternalHelper::DataPath) / path;
        return path.lexically_normal();
    }
    

    // Gets relative path to executable
    inline std::filesystem::path RelativeToExecutable(const std::string& path)
    {
        return std::filesystem::path(InternalHelper::ExecutablePath).append(path).lexically_normal();
    }
    inline std::filesystem::path RelativeToExecutable(const std::filesystem::path& path)
    {
        auto npath = std::filesystem::path(InternalHelper::ExecutablePath) / path;
        return path.lexically_normal();
    }

    // Gets shader path
    inline std::filesystem::path GetShaderPath()
    {
        return std::filesystem::path(InternalHelper::ShaderSourcePath);
    }

}

#define SizeOfAsUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

// Helpers to break on debug build and message logging
#if defined(_DEBUG)
#define ERROR(x)                                            \
    std::cout << std::string("ERROR: ") + x << std::endl;   \
    OutputDebugStringA((std::string("ERROR: ") + x).c_str());   \
	DebugBreak();	                                        \
	_CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, "ERROR", NULL)

#define ERROR_A(x)                                          \
    std::cout << std::string("ERROR: ") + x << std::endl;   \
	DebugBreak();	                                        \
	_CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, x, NULL)

//#define CHECK(x) Internal::CheckResult(x)
#define CHECK(x) if(FAILED(x))                              \
    {                                                       \
        OutputDebugStringA(("CHECK FAILED: " + Common::GetMessageFromHR(x)).c_str()); \
        DebugBreak();                                       \
        throw std::exception();                             \
    }
#else
#define THROW(x)
#define CHECK(x) if(FAILED(x))                              \
    {                                                       \
        throw std::exception();                             \
    }
#endif

#if defined(_DEBUG)
#define WARN(x) std::cout << std::string("WARNING: ") + x << std::endl
#define INFO(x) std::cout << std::string("INFO: ") + x << std::endl
#else
#define WARN(x)
#define ERROR(x) InternalHelper::Terminate(x)
#define INFO(x)
#endif