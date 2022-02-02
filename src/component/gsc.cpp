#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"
#include "game/scripting/array.hpp"
#include "game/scripting/function.hpp"

#include "gsc.hpp"

namespace gsc
{
	std::unordered_map<unsigned, script_function> functions;
	std::unordered_map<unsigned, script_method> methods;

	namespace
	{
		std::string method_name(unsigned int id)
		{
			const auto map = *game::plutonium::method_map_rev;

			for (const auto& function : map)
			{
				if (function.second == id)
				{
					return function.first;
				}
			}

			return {};
		}

		std::string function_name(unsigned int id)
		{
			const auto map = *game::plutonium::function_map_rev;

			for (const auto& function : map)
			{
				if (function.second == id)
				{
					return function.first;
				}
			}

			return {};
		}

		function_args get_arguments()
		{
			std::vector<scripting::script_value> args;

			const auto top = game::scr_VmPub->top;

			for (auto i = 0; i < game::scr_VmPub->outparamcount; i++)
			{
				const auto value = game::scr_VmPub->top[-i];
				args.push_back(value);
			}

			return args;
		}

		void return_value(const scripting::script_value& value)
		{
			if (game::scr_VmPub->outparamcount)
			{
				game::Scr_ClearOutParams();
			}

			scripting::push_value(value);
		}

		auto function_map_start = 0x200;
		auto method_map_start = 0x8400;
		auto token_map_start = 0x8000;
		auto field_offset_start = 0xA000;

		struct entity_field
		{
			std::string name;
			std::function<scripting::script_value(unsigned int entnum)> getter;
			std::function<void(unsigned int entnum, scripting::script_value)> setter;
		};

		std::vector<std::function<void()>> post_load_callbacks;
		std::unordered_map<unsigned int, std::unordered_map<unsigned int, entity_field>> custom_fields;

		void call_function(unsigned int id)
		{
			if (id < 0x200)
			{
				return reinterpret_cast<builtin_function*>(game::plutonium::function_table.get())[id]();
			}

			try
			{
				const auto result = functions[id](get_arguments());
				const auto type = result.get_raw().type;

				if (type)
				{
					return_value(result);
				}
			}
			catch (const std::exception& e)
			{
				printf("************** Script execution error **************\n");
				printf("Error executing function %s:\n", function_name(id).data());
				printf("    %s\n", e.what());
				printf("****************************************************\n");
			}
		}

		void call_method(game::scr_entref_t ent, unsigned int id)
		{
			if (id < 0x8400)
			{
				return reinterpret_cast<builtin_method*>(game::plutonium::method_table.get())[id - 0x8000](ent);
			}

			try
			{
				const auto result = methods[id](ent, get_arguments());
				const auto type = result.get_raw().type;

				if (type)
				{
					return_value(result);
				}
			}
			catch (const std::exception& e)
			{
				printf("************** Script execution error **************\n");
				printf("Error executing method %s:\n", method_name(id).data());
				printf("    %s\n", e.what());
				printf("****************************************************\n");
			}
		}

		__declspec(naked) void call_builtin_stub()
		{
			__asm
			{
				push eax

				mov eax, 0x20B4A5C
				mov [eax], esi

				mov eax, 0x20B4A90
				mov [eax], edx

				pop eax

				pushad
				push eax
				call call_function
				pop eax
				popad

				push 0x56C900
				retn
			}
		}

		__declspec(naked) void call_builtin_method_stub()
		{
			__asm
			{
				pushad
				push ecx
				push ebx
				call call_method
				pop ebx
				pop ecx
				popad

				push ebx
				add esp, 0xC

				push 0x56CBE9
				retn
			}
		}

		utils::hook::detour scr_get_object_field_hook;
		void scr_get_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
		{
			if (custom_fields[classnum].find(offset) == custom_fields[classnum].end())
			{
				return scr_get_object_field_hook.invoke<void>(classnum, entnum, offset);
			}

			const auto field = custom_fields[classnum][offset];

			try
			{
				const auto result = field.getter(entnum);
				return_value(result);
			}
			catch (const std::exception& e)
			{
				printf("************** Script execution error **************\n");
				printf("Error getting field %s:\n", field.name.data());
				printf("    %s\n", e.what());
				printf("****************************************************\n");
			}
		}

		utils::hook::detour scr_set_object_field_hook;
		void scr_set_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
		{
			if (custom_fields[classnum].find(offset) == custom_fields[classnum].end())
			{
				return scr_set_object_field_hook.invoke<void>(classnum, entnum, offset);
			}

			const auto args = get_arguments();
			const auto field = custom_fields[classnum][offset];

			try
			{
				field.setter(entnum, args[0]);
			}
			catch (const std::exception& e)
			{
				printf("************** Script execution error **************\n");
				printf("Error setting field %s:\n", field.name.data());
				printf("    %s\n", e.what());
				printf("****************************************************\n");
			}
		}

		utils::hook::detour scr_post_load_scripts_hook;
		void scr_post_load_scripts_stub()
		{
			for (const auto& callback : post_load_callbacks)
			{
				callback();
			}

			return scr_post_load_scripts_hook.invoke<void>();
		}
	}

	namespace function
	{
		void add(const std::string& name, const script_function& func)
		{
			const auto index = function_map_start++;

			functions[index] = func;
			(*game::plutonium::function_map_rev)[name] = index;
		}
	}

	namespace method
	{
		void add(const std::string& name, const script_method& func)
		{
			const auto index = method_map_start++;

			methods[index] = func;
			(*game::plutonium::method_map_rev)[name] = index;
		}
	}

	namespace field
	{
		void add(const classid classnum, const std::string& name,
			const std::function<scripting::script_value(unsigned int entnum)>& getter,
			const std::function<void(unsigned int entnum, const scripting::script_value&)>& setter)
		{
			const auto token_id = token_map_start++;
			const auto offset = field_offset_start++;

			custom_fields[classnum][offset] = {name, getter, setter};
			(*game::plutonium::token_map_rev)[name] = token_id;

			post_load_callbacks.push_back([classnum, name, token_id, offset]()
			{
				const auto name_str = game::SL_GetString(name.data(), 0);
				game::Scr_AddClassField(classnum, name_str, token_id, offset);
			});
		}
	}

	function_args::function_args(std::vector<scripting::script_value> values)
		: values_(values)
	{
	}

	unsigned int function_args::size() const
	{
		return this->values_.size();
	}

	std::vector<scripting::script_value> function_args::get_raw() const
	{
		return this->values_;
	}

	scripting::value_wrap function_args::get(const int index) const
	{
		if (index >= this->values_.size())
		{
			throw std::runtime_error(utils::string::va("parameter %d does not exist", index));
		}

		return {this->values_[index], index};
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scr_get_object_field_hook.create(0x52BDB0, scr_get_object_field_stub);
			scr_set_object_field_hook.create(0x52BCC0, scr_set_object_field_stub);
			scr_post_load_scripts_hook.create(0x628B50, scr_post_load_scripts_stub);

			field::add(classid::entity, "flags",
				[](unsigned int entnum) -> scripting::script_value
				{
					const auto entity = &game::g_entities[entnum];
					return entity->flags;
				},
				[](unsigned int entnum, const scripting::script_value& value)
				{
					const auto entity = &game::g_entities[entnum];
					entity->flags = value.as<int>();
				}
			);

			field::add(classid::entity, "clientflags",
				[](unsigned int entnum) -> scripting::script_value
				{
					const auto entity = &game::g_entities[entnum];
					return entity->client->flags;
				},
				[](unsigned int entnum, const scripting::script_value& value)
				{
					const auto entity = &game::g_entities[entnum];
					entity->client->flags = value.as<int>();
				}
			);

			function::add("executecommand", [](const function_args& args) -> scripting::script_value
			{
				game::Cbuf_AddText(0, args[0].as<const char*>());
				return {};
			});

			function::add("addcommand", [](const function_args& args) -> scripting::script_value
			{
				const auto name = args[0].as<std::string>();
				const auto function = args[1].as<scripting::function>();
				command::add_script_command(name, [function](const command::params& params)
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

			function::add("say", [](const function_args& args) -> scripting::script_value
			{
				const auto message = args[0].as<std::string>();
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("%c \"%s\"", 84, message.data()));

				return {};
			});

			function::add("dropallbots", [](const function_args& args) -> scripting::script_value
			{
				for (auto i = 0; i < *game::svs_clientCount; i++)
				{
					if (game::svs_clients[i].header.state != game::CS_FREE
						&& game::svs_clients[i].header.netchan.remoteAddress.type == game::NA_BOT)
					{
						game::SV_GameDropClient(i, "GAME_GET_TO_COVER");
					}
				}

				return {};
			});

			method::add("tell", [](const game::scr_entref_t ent, const function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto client = ent.entnum;
				const auto message = args[0].as<std::string>();

				game::SV_GameSendServerCommand(client, 0, utils::string::va("%c \"%s\"", 84, message.data()));

				return {};
			});

			method::add("specialtymarathon", [](const game::scr_entref_t ent, const function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto num = ent.entnum;
				const auto toggle = args[0].as<int>();

				auto g_client = game::g_entities[num].client;
				auto playerState = &g_client->ps;
				auto flags = playerState->perks[0];

				playerState->perks[0] = toggle
					? flags | 0x4000u
					: flags & ~0x4000u;

				return {};
			});

			method::add("isbot", [](const game::scr_entref_t ent, const function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				const auto client = ent.entnum;
				if (game::g_entities[client].client == nullptr)
				{
					throw std::runtime_error("entity is not a player");
				}

				return game::svs_clients[client].bIsTestClient;
			});

			utils::hook::jump(0x56C8EB, call_builtin_stub);
			utils::hook::jump(0x56CBDC, call_builtin_method_stub);
		}
	};
}

REGISTER_COMPONENT(gsc::component)
