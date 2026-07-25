#pragma once
#include "entity.hpp"
#include "script_value.hpp"

/*
 * Wraps a SCRIPT_FUNCTION code position received from gsc (the `::my_func`
 * syntax). call() runs it through exec_ent_thread, which is what lets a
 * builtin store a gsc callback and fire it later (addCommand).
 */

namespace scripting
{
	class function
	{
	public:
		function(const char*);

		script_value get_raw() const;
		const char* get_pos() const;

		script_value call(const entity& self, std::vector<script_value> arguments) const;

		script_value operator()(const entity& self, std::vector<script_value> arguments) const
		{
			return this->call(self, arguments);
		}

		script_value operator()(std::vector<script_value> arguments) const
		{
			return this->call(*game::levelEntityId, arguments);
		}

		script_value operator()() const
		{
			return this->call(*game::levelEntityId, {});
		}

	private:
		const char* pos_;
	};
}
