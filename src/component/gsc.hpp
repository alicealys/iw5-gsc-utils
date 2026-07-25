#pragma once
#include <plutonium_sdk.hpp>
#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

/*
 * Public API is intentionally IDENTICAL to upstream iw5-gsc-utils's gsc.hpp
 * so that component modules compile completely unchanged.
 * Implementation routes through plutonium-sdk's official GSC interfaces
 * instead of relying on stale hardcoded addresses and inline asm hooks.
 */

namespace gsc
{
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

	using script_function = std::function<scripting::script_value(const function_args&)>;
	using script_method = std::function<scripting::script_value(const game::scr_entref_t, const function_args&)>;

	namespace function
	{
		void add(const std::string& name, const script_function& func);
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func);
	}

	// Call this once from plugin::on_startup with the SDK interface pointer,
	// before any function::add/method::add calls.
	void init(plutonium::sdk::iinterface* interface_ptr);

	// Routes to the SDK logger; safe to call before init (no-op then).
	void log(const std::string& msg);
}
