#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "gsc.hpp"
#include "json.hpp"

namespace io
{
	class component final : public component_interface
	{
	public:
		void on_after_dvar_init([[maybe_unused]] plugin::plugin* plugin) override
		{
			const auto fs_basegame = game::Dvar_FindVar("fs_basegame");
			if (fs_basegame == nullptr)
			{
				return;
			}

			printf("working directory: %s", fs_basegame->current.string);
			std::filesystem::current_path(fs_basegame->current.string);
		}

		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			gsc::function::add("jsonprint", [](const gsc::function_args& args) -> scripting::script_value
			{
				std::string buffer;

				for (const auto& arg : args.get_raw())
				{
					buffer.append(json::gsc_to_string(arg));
					buffer.append("\t");
				}

				printf("%s", buffer.data());
				return {};
			});

			gsc::function::add("fileexists", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_exists(path);
			});

			gsc::function::add("writefile", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				const auto data = args[1].as<std::string>();

				auto append = false;
				if (args.size() > 2)
				{
					append = args[2].as<bool>();
				}

				return utils::io::write_file(path, data, append);
			});

			gsc::function::add("readfile", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::read_file(path);
			});

			gsc::function::add("filesize", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_size(path);
			});

			gsc::function::add("createdirectory", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::create_directory(path);
			});

			gsc::function::add("directoryexists", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_exists(path);
			});

			gsc::function::add("directoryisempty", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_is_empty(path);
			});

			gsc::function::add("listfiles", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<std::string>();
				const auto files = utils::io::list_files(path);

				scripting::array array;
				for (const auto& file : files)
				{
					array.push(file);
				}

				return array;
			});

			gsc::function::add("copyfolder", [](const gsc::function_args& args)
			{
				const auto source = args[0].as<std::string>();
				const auto target = args[1].as<std::string>();
				utils::io::copy_folder(source, target);

				return scripting::script_value{};
			});

			gsc::function::add("httpget", [](const gsc::function_args& args) -> scripting::script_value
			{
				const auto url = args[0].as<std::string>();
				const auto object = scripting::entity(scripting::make_object());

				scheduler::once([object, url]()
				{
					const auto result = utils::http::get_data(url.data());
					scheduler::once([object, result]()
					{
						const auto value = result.has_value()
							? result.value().substr(0, 0x5000)
							: "";
						scripting::notify(object, "done", {value});
					});
				}, scheduler::pipeline::async);
			
				return object;
			});
		}
	};
}

REGISTER_COMPONENT(io::component)
