#pragma once


#include <WinSDKVer.h>
#define _WIN32_WINNT     0x0A00
#include <SDKDDKVer.h>

// Remove unecessary includes
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP

// Classic :)
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <typeinfo>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <crtdbg.h>
#include <d3d11_4.h>

#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#error Old windows 10 versions are not supported
#endif

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <string>
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <atomic>
#include <mutex>
#include <future>
#include <type_traits>
#include <vector>
#include <array>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <regex>
#include <math.h>
#include <limits>


#include <ppl.h>
#include <concurrent_queue.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <d3dcompiler.h>

#include "imgui.h"
#include <Keyboard.h>
#include <Mouse.h>
#include <WICTextureLoader.h>

#include <Audio.h>

#include "Extern/Json/json.hpp"
using json = nlohmann::json;

#include "Base/Common.h"
#include "Base/Events/Events.h"
#include "Options/Options.h"
#include "Base/DX.h"
#include "Profiling/Timer.h"
#include "Base/Time.h"
#include "Base/Bit.h"
