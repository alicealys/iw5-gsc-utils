#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "gsc.hpp"
#include "scripting.hpp"

namespace notifies
{
	namespace
	{
		std::vector<scripting::function> say_callbacks;
		utils::hook::detour client_command_hook;

		void client_command_stub(int clientNum)
		{
			char cmd[1024] = {0};
			const auto* entity = &game::g_entities[clientNum];

			if (entity->client == nullptr)
			{
				return; // Client is not fully in game yet
			}

			game::SV_Cmd_ArgvBuffer(0, cmd, 1024);

			auto hidden = false;
			if (cmd == "say"s || cmd == "say_team"s)
			{
				std::string message = game::ConcatArgs(1);
				message.erase(0, 1);

				for (const auto& callback : say_callbacks)
				{
					const auto entity_id = game::Scr_GetEntityId(clientNum, 0);
					const auto result = callback(entity_id, {message, cmd == "say_team"s});

					if (result.is<int>() && !hidden)
					{
						hidden = result.as<int>() == 0;
					}
				}
			}

			if (!hidden)
			{
				client_command_hook.invoke<void>(clientNum);
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			client_command_hook.create(0x502CB0, client_command_stub);

			scripting::on_shutdown([]()
			{
				say_callbacks.clear();
			});

			gsc::function::add("onplayersay", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto function = args[0].as<scripting::function>();
				say_callbacks.push_back(function);
				return {};
			});
		}
	};
}

//REGISTER_COMPONENT(notifies::component)
