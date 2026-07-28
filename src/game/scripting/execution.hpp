#pragma once
#include "game/game.hpp"
#include "entity.hpp"
#include "array.hpp"
#include "object.hpp"
#include "function.hpp"
#include "script_value.hpp"

namespace scripting
{
	void push_value(const script_value& value);

	template <typename T = script_value>
	T call(const std::string& name, const std::vector<script_value>& arguments = {});

	template <>
	script_value call(const std::string& name, const std::vector<script_value>& arguments);

	template <typename T>
	T call(const std::string& name, const std::vector<script_value>& arguments)
	{
		return call<script_value>(name, arguments).as<T>();
	}

	script_value exec_ent_thread(const entity& entity, const char* pos, const std::vector<script_value>& arguments);

	void notify(const entity& entity, const std::string& event, const std::vector<script_value>& arguments);

	unsigned int make_array();
	unsigned int make_object();
}
