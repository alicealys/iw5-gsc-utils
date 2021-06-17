#pragma once

namespace gsc
{
	using function_args = std::vector<scripting::script_value>;

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(function_args)>;
	using script_method = std::function<scripting::script_value(game::scr_entref_t, function_args)>;

	namespace function
	{
		void add(const std::string& name, const script_function& func);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func);
	}

	unsigned int make_array();
	void add_array_key_value(unsigned int parent_id, const std::string& _key, const scripting::script_value& value);
	void add_array_value(unsigned int parent_id, const scripting::script_value& value);
}