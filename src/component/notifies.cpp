#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include "game/scripting/entity.hpp"
#include "game/scripting/execution.hpp"
#include "notifies.hpp"

namespace notifies
{
	namespace
	{
		utils::hook::detour client_command_hook;

		utils::hook::detour scr_player_killed_hook;
		utils::hook::detour scr_player_damage_hook;

		void client_command_stub(int clientNum)
		{
			char cmd[1024] = { 0 };

			game::SV_Cmd_ArgvBuffer(0, cmd, 1024);

			if (cmd == "say"s)
			{
				std::string message = game::ConcatArgs(1);
				message.erase(0, 1);

				const scripting::entity level{*game::levelEntityId};
				const auto player = scripting::call("getEntByNum", {clientNum}).as<scripting::entity>();

				scripting::notify(level, "say", {player, message});
				scripting::notify(player, "say", {message});
			}

			return client_command_hook.invoke<void>(clientNum);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			client_command_hook.create(0x502CB0, client_command_stub);
		}
	};
}

REGISTER_COMPONENT(notifies::component)