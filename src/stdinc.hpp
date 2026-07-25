#pragma once

#pragma warning(disable: 4018)
#pragma warning(disable: 4146)
#pragma warning(disable: 4129)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)
#pragma warning(disable: 26812)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <cassert>
#include <mutex>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <cstdint>
#include <stdexcept>

using namespace std::literals;

#include <gsl/gsl>
#include <MinHook.h>

#include "utils/memory.hpp"
#include "utils/string.hpp"
#include "utils/hook.hpp"
#include "utils/io.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"
