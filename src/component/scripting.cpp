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
	namespace
	{
		utils::hook::detour g_shutdown_game_hook;

		std::vector<std::function<void()>> shutdown_callbacks;

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
	}

	void on_shutdown(const std::function<void()>& callback)
	{
		shutdown_callbacks.push_back(callback);
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			g_shutdown_game_hook.create(0x50C100, g_shutdown_game_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
