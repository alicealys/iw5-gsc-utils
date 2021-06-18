#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"
#include "game/scripting/array.hpp"

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
			function_args args;

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

		void call_function(unsigned int id)
		{
			if (id < 0x200)
			{
				return reinterpret_cast<builtin_function*>(0x1D6EB34)[id]();
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
			catch (std::exception e)
			{
				printf("************** Script execution error **************\n");
				printf("Error executing function %s\n", function_name(id).data());
				printf("%s\n", e.what());
				printf("****************************************************\n");
			}
		}

		void call_method(game::scr_entref_t ent, unsigned int id)
		{
			if (id < 0x8400)
			{
				return reinterpret_cast<builtin_method*>(0x1D4F258)[id](ent);
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
			catch (std::exception e)
			{
				printf("************** Script execution error **************\n");
				printf("Error executing method %s\n", method_name(id).data());
				printf("%s\n", e.what());
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

		unsigned int replaced_pos = 0;

		void get_replaced_pos(unsigned int pos)
		{
			if (scripting::replaced_functions.find(pos) != scripting::replaced_functions.end())
			{
				replaced_pos = scripting::replaced_functions[pos];
			}
		}

		__declspec(naked) void vm_execute_stub()
		{
			__asm
			{
				pushad
				push esi
				call get_replaced_pos
				pop esi
				popad

				cmp replaced_pos, 0
				jne set_pos

				movzx eax, byte ptr[esi]
				inc esi

				jmp loc_1
			loc_1:
				mov [ebp - 0x18], eax
				mov [ebp - 0x8], esi

				push ecx

				mov ecx, 0x20B8E28
				mov [ecx], eax

				mov ecx, 0x20B4A5C
				mov[ecx], esi

				pop ecx

				cmp eax, 0x98

				push 0x56B740
				retn
			set_pos:
				mov esi, replaced_pos
				mov replaced_pos, 0

				movzx eax, byte ptr[esi]
				inc esi

				jmp loc_1
			}
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

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			function::add("executecommand", [](function_args args) -> scripting::script_value
			{
				game::Cbuf_AddText(0, args[0].as<const char*>());
				return {};
			});

			function::add("replacefunc", [](function_args args) -> scripting::script_value
			{
				const auto what = args[0].get_raw();
				const auto with = args[1].get_raw();

				if (what.type != game::SCRIPT_FUNCTION || with.type != game::SCRIPT_FUNCTION)
				{
					throw std::runtime_error("Invalid type");
				}

				scripting::replaced_functions[what.u.uintValue] = with.u.uintValue;

				return {};
			});

			function::add("addcommand", [](function_args args) -> scripting::script_value
			{
				const auto name = args[0].as<std::string>();
				const auto function = args[1].get_raw();

				if (function.type != game::SCRIPT_FUNCTION)
				{
					throw std::runtime_error("Invalid type");
				}

				const auto pos = function.u.codePosValue;
				command::add_script_command(name, [pos](const command::params& params)
				{
					scripting::array array;
					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					scripting::exec_ent_thread(*game::levelEntityId, pos, {array.get_raw()});
				});

				return {};
			});

			function::add("say", [](function_args args) -> scripting::script_value
			{
				const auto message = args[0].as<std::string>();
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("%c \"%s\"", 84, message.data()));

				return {};
			});

			method::add("tell", [](game::scr_entref_t ent, function_args args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid type");
				}

				const auto client = ent.entnum;
				const auto message = args[0].as<std::string>();

				game::SV_GameSendServerCommand(client, 0, utils::string::va("%c \"%s\"", 84, message.data()));

				return {};
			});

			utils::hook::jump(0x56C8EB, call_builtin_stub);
			utils::hook::jump(0x56CBDC, call_builtin_method_stub);
			utils::hook::jump(0x56B726, vm_execute_stub);
		}
	};
}

REGISTER_COMPONENT(gsc::component)