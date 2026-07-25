#pragma once
#include "game/scripting/script_value.hpp"

namespace json
{
	std::string gsc_to_string(const scripting::script_value& value);
	void init();
}