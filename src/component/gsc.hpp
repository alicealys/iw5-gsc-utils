#pragma once
#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

namespace gsc
{
	enum classid
	{
		entity,
		hudelem,
		pathnode,
		node,
		count
	};

	class function_args
	{
	public:
		function_args(std::vector<scripting::script_value>);

		unsigned int size() const;
		std::vector<scripting::script_value> get_raw() const;
		scripting::value_wrap get(const int index) const;

		scripting::value_wrap operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		std::vector<scripting::script_value> values_;
	};

	using builtin_function = void(*)();
	using builtin_method = void(*)(game::scr_entref_t);

	using script_function = std::function<scripting::script_value(const function_args&)>;
	using script_method = std::function<scripting::script_value(const game::scr_entref_t, const function_args&)>;

	void* make_function_thunk(const char* name, const script_function* func);
	void* make_method_thunk(const char* name, const script_method* meth);

	namespace function
	{
		template <typename F>
		void add(const std::string& name, F&& f)
		{
			const auto name_str = utils::memory::duplicate_string(name);
			const auto func = new script_function(std::forward<F>(f));
			const auto thunk = reinterpret_cast<
				plutonium::sdk::interfaces::gsc::function_callback>(make_function_thunk(name_str, func));
			plugin::get()->get_interface()->gsc()->register_function(name, thunk);
		}
	}

	namespace method
	{
		template <typename F>
		void add(const std::string& name, F&& f)
		{
			const auto name_str = utils::memory::duplicate_string(name);
			const auto func = new script_method(std::forward<F>(f));
			const auto thunk = reinterpret_cast<
				plutonium::sdk::interfaces::gsc::method_callback>(make_method_thunk(name_str, func));
			plugin::get()->get_interface()->gsc()->register_method(name, thunk);
		}
	}

	namespace field
	{
		void add(const classid classnum, const std::string& name,
			const std::function<scripting::script_value(unsigned int entnum)>& getter,
			const std::function<void(unsigned int entnum, const scripting::script_value&)>& setter);
	}
}