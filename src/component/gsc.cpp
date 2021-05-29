#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"

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

		auto function_map_start = 0x200;
		auto method_map_start = 0x8400;

		void call_function(unsigned int id)
		{
			if (id >= 0x200)
			{
				try
				{
					const auto result = functions[id](get_arguments());
					scripting::push_value(result);
				}
				catch (std::exception e)
				{
					printf("************** Script execution error **************\n");
					printf("Error executing function %s\n", function_name(id).data());
					printf("%s\n", e.what());
					printf("****************************************************\n");
				}

			}
			else
			{
				reinterpret_cast<builtin_function*>(0x1D6EB34)[id]();
			}
		}

		void call_method(game::scr_entref_t ent, unsigned int id)
		{
			if (id >= 0x8400)
			{
				try
				{
					const auto result = methods[id](ent, get_arguments());
					scripting::push_value(result);
				}
				catch (std::exception e)
				{
					printf("************** Script execution error **************\n");
					printf("Error executing method %s\n", method_name(id).data());
					printf("%s\n", e.what());
					printf("****************************************************\n");
				}

			}
			else
			{
				reinterpret_cast<builtin_method*>(0x1D4F258)[id](ent);
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
			function::add("lol", [](function_args args) -> scripting::script_value
			{
				const auto str = args[0].as<std::string>();

				return str;
			});

			method::add("test", [](game::scr_entref_t, function_args args) -> scripting::script_value
			{
				printf("here\n");

				return {};
			});

			utils::hook::jump(0x56C8EB, call_builtin_stub);
			utils::hook::jump(0x56CBDC, call_builtin_method_stub);
		}
	};
}

REGISTER_COMPONENT(gsc::component)