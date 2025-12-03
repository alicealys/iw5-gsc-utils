#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "plugin.hpp"

#include "component/signatures.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace plugin
{
	namespace
	{
		void printf_stub(const char* fmt, ...)
		{
			char buffer[0x2000] = {};

			va_list ap;
			va_start(ap, fmt);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);

			va_end(ap);

			get()->get_interface()->logging()->info(buffer);
		}
	}

	std::uint32_t plugin::plugin_version()
	{
		return 1;
	}

	const char* plugin::plugin_name()
	{
		return "iw5-gsc-utils";
	}

	bool  plugin::is_game_supported([[maybe_unused]] plutonium::sdk::game game)
	{
		return game == plutonium::sdk::game::iw5;
	}

	void plugin::on_startup(plutonium::sdk::iinterface* interface_ptr, plutonium::sdk::game game)
	{
		this->interface_ = interface_ptr;
		this->game_ = game;
		utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), printf_stub);

		if (!signatures::process())
		{
			MessageBoxA(NULL,
				"This version of iw5-gsc-utils is outdated.\n" \
				"Download the latest dll from here: https://github.com/alicealys/iw5-gsc-utils/releases",
				"ERROR", MB_ICONERROR);
		}
		else
		{
			component_loader::post_unpack();
		}
	}

	void plugin::on_shutdown()
	{
		component_loader::pre_destroy();
	}

	plutonium::sdk::iinterface* plugin::get_interface()
	{
		return this->interface_;
	}

	plutonium::sdk::game plugin::get_game()
	{
		return this->game_;
	}

	plugin* get()
	{
		static plugin instance;
		return &instance;
	}
}
