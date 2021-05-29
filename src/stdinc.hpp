#pragma once

#pragma warning(disable: 4018)
#pragma warning(disable: 4146)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 26812)

#define DLL_EXPORT extern "C" __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

using namespace std::literals;

#include <gsl/gsl>
#include <MinHook.h>

#include "utils/memory.hpp"
#include "utils/string.hpp"
#include "utils/hook.hpp"
#include "utils/concurrent_list.hpp"
#include "utils/io.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"