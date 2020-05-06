#include "Common.h"

std::filesystem::path InternalHelper::ExecutablePath;
std::filesystem::path InternalHelper::DataPath;
std::filesystem::path InternalHelper::ShaderSourcePath;

void InternalHelper::Initialize()
{
    HMODULE hModule = GetModuleHandleW(NULL);
    WCHAR path[MAX_PATH] = { 0 };

    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::filesystem::path pa(path);

    ExecutablePath = pa.remove_filename(); 

    auto base = std::filesystem::path(ExecutablePath);
    DataPath = std::filesystem::path(base).append(L"..\\Data\\").lexically_normal();
    ShaderSourcePath = std::filesystem::path(base).append(L"..\\Data\\Shaders\\").lexically_normal();

}



