#include <stdinc.hpp>
#include "plugin.hpp"
#include "component/command.hpp"
#include "component/gsc.hpp"
#include "component/io.hpp"
#include "component/json.hpp"
#include "component/userinfo.hpp"

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

		interface_ptr->logging()->info("[iw5-gsc-utils] on_startup - registering functions/methods");

		gsc::init(interface_ptr);

		io::init();
		json::init();
		userinfo::init();
		command::init();

		interface_ptr->logging()->info("[iw5-gsc-utils] registration complete (io + json + userinfo + command)");
	}

	void plugin::on_shutdown()
	{
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
