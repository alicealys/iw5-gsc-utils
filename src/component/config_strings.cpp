#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>

namespace config_strings
{
	namespace
	{
		bool reserved_config_strings[600]{};
		std::uint32_t hudelem_config_strings[1024]{};

		bool is_string_reserved(const std::uint32_t string_index)
		{
			if (string_index >= 600)
			{
				return false;
			}

			return reserved_config_strings[string_index];
		}

		unsigned short get_scr_const_()
		{
			return *reinterpret_cast<unsigned short*>(0x1C8DF80);
		}

		std::uint32_t get_config_string()
		{
			auto index = 0;
			const auto scr_const_ = get_scr_const_();
			for (auto i = 1; i < 600; i++)
			{
				const auto string_value = game::sv_configstrings[i + 469];
				if (string_value == scr_const_ && !reserved_config_strings[i])
				{
					index = i;
					break;
				}
			}

			if (index == 0)
			{
				throw std::runtime_error("out of config strings");
			}

			reserved_config_strings[index] = true;
			return index;
		}

		utils::hook::detour scr_free_hud_elem_hook;
		void scr_free_hud_elem_stub(game::game_hudelem_s* hud)
		{
			const auto _0 = gsl::finally([&]
			{
				scr_free_hud_elem_hook.invoke<void>(hud);
			});

			const auto index = hud - &game::g_hudelems[0];
			const auto string = hudelem_config_strings[index];

			if (string != 0)
			{
				hudelem_config_strings[index] = 0;
				reserved_config_strings[string] = false;
				game::sv_configstrings[string + 469] = get_scr_const_();
				game::SV_SetConfigString(string + 496, nullptr);
			}
		}

		__declspec(naked) void g_localized_string_index_stub()
		{
			__asm
			{
				add esp, 4
				pushad
				lea ecx, [ecx - 469]
				push ecx
				call is_string_reserved
				test al, al
				jz not_reserved

				pop ecx
				popad
				push 0x52D95E
				retn

			not_reserved:
				pop ecx
				popad

				cmp eax, edx
				push 0x52D958
				retn
			};
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			utils::hook::jump(0x52D953, g_localized_string_index_stub);
			scr_free_hud_elem_hook.create(0x52B0F0, scr_free_hud_elem_stub);

			scripting::on_shutdown([&]
			{
				std::memset(reserved_config_strings, 0, sizeof(reserved_config_strings));
				std::memset(hudelem_config_strings, 0, sizeof(hudelem_config_strings));
			});

			gsc::method::add("settextunlimited", [](const game::scr_entref_t ent, const gsc::function_args& args)
				-> scripting::script_value
			{
				if (ent.classnum != gsc::class_hudelem || ent.entnum >= 1024)
				{
					throw std::runtime_error("not a hudelem");
				}

				const auto text = args[0].as<const char*>();
				const auto hudelem = &game::g_hudelems[ent.entnum];

				hudelem->type = 1;
				hudelem->flags |= 1;

				if (hudelem_config_strings[ent.entnum] == 0)
				{
					const auto index = get_config_string();
					hudelem->text = index;
					hudelem_config_strings[ent.entnum] = index;
				}
				else
				{
					hudelem->text = hudelem_config_strings[ent.entnum];
				}

				game::SV_SetConfigString(hudelem->text + 469, text);
				return {};
			});
		}
	};
}

REGISTER_COMPONENT(config_strings::component)
