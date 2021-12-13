#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "command.hpp"
#include "userinfo.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"

#include "gsc.hpp"

namespace scripting
{
	std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;
	std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;

	namespace
	{
		utils::hook::detour vm_notify_hook;
		utils::hook::detour scr_add_class_field_hook;

		utils::hook::detour scr_load_level_hook;
		utils::hook::detour g_shutdown_game_hook;

		utils::hook::detour scr_set_thread_position_hook;
		utils::hook::detour process_script_hook;

		std::string current_file;

		std::vector<std::function<void()>> shutdown_callbacks;

		void vm_notify_stub(const unsigned int notify_list_owner_id, const unsigned int string_value,
			                game::VariableValue* top)
		{
			const auto* name = game::SL_ConvertToString(string_value);

			if (name)
			{
				event e;
				e.name = name;
				e.entity = notify_list_owner_id;

				for (auto* value = top; value->type != game::SCRIPT_END; --value)
				{
					e.arguments.emplace_back(*value);
				}

				if (e.name == "connected")
				{
					const auto player = e.arguments[0].as<scripting::entity>();
					const auto client = player.call("getentitynumber").as<int>();
					userinfo::clear_client_overrides(client);
				}
			}

			vm_notify_hook.invoke<void>(notify_list_owner_id, string_value, top);
		}

		void scr_add_class_field_stub(unsigned int classnum, unsigned int _name, unsigned int canonicalString, unsigned int offset)
		{
			const auto name = game::SL_ConvertToString(_name);

			if (fields_table[classnum].find(name) == fields_table[classnum].end())
			{
				fields_table[classnum][name] = offset;
			}

			scr_add_class_field_hook.invoke<void>(classnum, _name, canonicalString, offset);
		}

		void scr_load_level_stub()
		{
			scr_load_level_hook.invoke<void>();
		}

		void g_shutdown_game_stub(const int free_scripts)
		{
			userinfo::clear_overrides();
			command::clear_script_commands();

			for (const auto& callback : shutdown_callbacks)
			{
				callback();
			}

			g_shutdown_game_hook.invoke<void>(free_scripts);
		}

		void process_script_stub(const char* filename)
		{
			current_file = filename;

			const auto file_id = atoi(filename);
			if (file_id)
			{
				current_file = scripting::file_list[file_id];
			}

			process_script_hook.invoke<void>(filename);
		}

		void scr_set_thread_position_stub(unsigned int threadName, const char* codePos)
		{
			const auto function_name = scripting::find_token(threadName);

			if (!function_name.empty())
			{
				script_function_table[current_file][function_name] = codePos;
			}

			scr_set_thread_position_hook.invoke<void>(threadName, codePos);
		}
	}

	void on_shutdown(const std::function<void()>& callback)
	{
		shutdown_callbacks.push_back(callback);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scr_load_level_hook.create(0x527AF0, scr_load_level_stub);
			g_shutdown_game_hook.create(0x50C100, g_shutdown_game_stub);

			scr_add_class_field_hook.create(0x567CD0, scr_add_class_field_stub);
			vm_notify_hook.create(0x569720, vm_notify_stub);

			scr_set_thread_position_hook.create(0x5616D0, scr_set_thread_position_stub);
			process_script_hook.create(0x56B130, process_script_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)