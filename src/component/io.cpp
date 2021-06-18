#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"
#include "game/scripting/array.hpp"

#include "gsc.hpp"

namespace io
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			const auto path = game::Dvar_FindVar("fs_basegame")->current.string;
			std::filesystem::current_path(path);

			gsc::function::add("fremove", [](gsc::function_args args)
			{
				const auto path = args[0].as<const char*>();
				return std::remove(path);
			});

			gsc::function::add("fopen", [](gsc::function_args args)
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

			gsc::function::add("fclose", [](gsc::function_args args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				return fclose(handle);
			});

			gsc::function::add("fwrite", [](gsc::function_args args)
			{
				const auto handle = args[0].as_ptr<FILE>();
				const auto text = args[1].as<const char*>();

				return fprintf(handle, text);
			});

			gsc::function::add("fread", [](gsc::function_args args)
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

			gsc::function::add("fileexists", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_exists(path);
			});

			gsc::function::add("writefile", [](gsc::function_args args)
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

			gsc::function::add("readfile", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::read_file(path);
			});

			gsc::function::add("filesize", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::file_size(path);
			});

			gsc::function::add("createdirectory", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::create_directory(path);
			});

			gsc::function::add("directoryexists", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_exists(path);
			});

			gsc::function::add("directoryisempty", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				return utils::io::directory_is_empty(path);
			});

			gsc::function::add("listfiles", [](gsc::function_args args)
			{
				const auto path = args[0].as<std::string>();
				const auto files = utils::io::list_files(path);

				scripting::array array;
				for (const auto& file : files)
				{
					array.push(file);
				}

				return array.get_raw();
			});

			gsc::function::add("copyfolder", [](gsc::function_args args)
			{
				const auto source = args[0].as<std::string>();
				const auto target = args[1].as<std::string>();
				utils::io::copy_folder(source, target);

				return scripting::script_value{};
			});
		}
	};
}

REGISTER_COMPONENT(io::component)