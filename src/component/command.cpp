#include <stdinc.hpp>
#include "command.hpp"
#include "gsc.hpp"

#include "game/scripting/array.hpp"
#include "game/scripting/entity.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/function.hpp"

/*
 * Console commands registered from gsc via addCommand(name, ::callback).
 *
 * Cmd_AddCommandInternal only takes a plain void() callback with no context,
 * so every command shares main_handler, which looks the real handler up by
 * name in `handlers`.
 *
 * The dispatch path here is the engine's command loop, NOT a gsc builtin, so
 * it does not inherit gsc.cpp's SEH net - a script error inside the callback
 * would otherwise take the server down. Hence the local guard below.
 */

namespace command
{
	namespace
	{
		std::unordered_map<std::string, std::function<void(const params&)>> handlers;

		std::vector<std::string> script_commands;
		utils::memory::allocator allocator;

		// POD-only frame: MSVC forbids __try in a function that owns objects
		// needing unwinding (C2712), so the body is a reference parameter.
		bool invoke_guarded(const std::function<void()>& body)
		{
			__try
			{
				body();
				return true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return false;
			}
		}

		void log(const std::string& msg)
		{
			gsc::log(msg);
		}

		void main_handler()
		{
			params params = {};

			const auto command = utils::string::to_lower(params[0]);
			const auto handler = handlers.find(command);
			if (handler == handlers.end())
			{
				return;
			}

			std::string error_message;
			bool threw = false;

			const auto ok = invoke_guarded([&]()
			{
				try
				{
					handler->second(params);
				}
				catch (const std::exception& e)
				{
					threw = true;
					error_message = e.what();
				}
			});

			if (!ok)
			{
				log("[iw5-gsc-utils] command '" + command +
					"': ACCESS VIOLATION in gsc callback (caught -- server did NOT crash)");
			}
			else if (threw)
			{
				log("[iw5-gsc-utils] command '" + command + "': " + error_message);
			}
		}
	}

	params::params()
		: nesting_(game::cmd_args->nesting)
	{
	}

	int params::size() const
	{
		return game::cmd_args->argc[this->nesting_];
	}

	const char* params::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::cmd_args->argv[this->nesting_][index];
	}

	std::string params::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}

		return result;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback,
			utils::memory::get_allocator()->allocate<game::cmd_function_t>());
	}

	void add(const char* name, const std::function<void(const params&)>& callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
		{
			add_raw(name, main_handler);
		}

		handlers[command] = callback;
	}

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
	{
		script_commands.push_back(name);
		// The engine keeps the name pointer, so it must outlive this call.
		const auto _name = allocator.duplicate_string(name);
		add(_name, callback);
	}

	void clear_script_commands()
	{
		for (const auto& name : script_commands)
		{
			handlers.erase(utils::string::to_lower(name));
			game::Cmd_RemoveCommand(name.data());
		}

		allocator.clear();
		script_commands.clear();
	}

	void init()
	{
		gsc::function::add("executecommand", [](const gsc::function_args& args) -> scripting::script_value
		{
			game::Cbuf_AddText(0, args[0].as<const char*>());
			return {};
		});

		gsc::function::add("addcommand", [](const gsc::function_args& args) -> scripting::script_value
		{
			const auto name = args[0].as<std::string>();
			const auto function = args[1].as<scripting::function>();

			add_script_command(name, [function](const params& params)
			{
				scripting::array array;
				for (auto i = 0; i < params.size(); i++)
				{
					array.push(params[i]);
				}

				function({array.get_raw()});
			});

			return {};
		});
	}
}
