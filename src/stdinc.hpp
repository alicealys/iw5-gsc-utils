#pragma once

#pragma warning(disable: 4018)
#pragma warning(disable: 4146)
#pragma warning(disable: 4129)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)
#pragma warning(disable: 26812)

#define DLL_EXPORT extern "C" __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef min
#undef max

#include <vector>
#include <cassert>
#include <mutex>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <regex>
#include <queue>
#include <unordered_set>
#include <filesystem>
#include <map>
#include <csetjmp>
#include <atlcomcli.h>
#include <Psapi.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "delayimp.lib")
#pragma comment(lib, "secur32.lib")

#include "resource.hpp"

using namespace std::literals;

#define __has_warning(...) 0

#include <gsl/gsl>
#include <MinHook.h>

#include "utils/memory.hpp"
#include "utils/string.hpp"
#include "utils/hook.hpp"
#include "utils/io.hpp"
#include "utils/concurrency.hpp"
#include "utils/http.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"
