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
		void post_unpack() override
		{
			const auto path = game::Dvar_FindVar("fs_basegame")->current.string;
			std::filesystem::current_path(path);

			gsc::function::add("jsonprint", [](const gsc::function_args& args) -> scripting::script_value
			{
				std::string buffer;

				for (const auto arg : args.get_raw())
				{
					buffer.append(json::gsc_to_string(arg));
					buffer.append("\t");
				}

				printf("%s\n", buffer.data());
				return {};
			});

			gsc::function::add("fremove", [](const gsc::function_args& args)
			{
				const auto path = args[0].as<const char*>();
				return std::remove(path);
			});

			gsc::function::add("fopen", [](const gsc::function_args& args)
			{
				const auto* path = args[0].as<const char*>();
				const auto* mode = args[1].as<const char*>();

				const auto handle = fopen(path, mode);

				if (!handle)
				{
					printf("fopen: Invalid path\n");
				}

				return handle;
			});

			gsc::function::add("fclose", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				return fclose(handle);
			});

			gsc::function::add("fwrite", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				const auto text = args[1].as<const char*>();

				return fprintf(handle, text);
			});

			gsc::function::add("fread", [](const gsc::function_args& args)
			{
				const auto handle = args[0].as_ptr<FILE>();

				fseek(handle, 0, SEEK_END);
				const auto length = ftell(handle);

				fseek(handle, 0, SEEK_SET);
				char* buffer = (char*)calloc(length, sizeof(char));

				fread(buffer, sizeof(char), length, handle);

				const std::string result = buffer;

				free(buffer);

				return result;
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