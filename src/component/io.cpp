#include <stdinc.hpp>
#include "component/gsc.hpp"
#include "io.hpp"
#include "json.hpp"
#include "game/scripting/entity.hpp"
#include "game/scripting/function.hpp"

/*
 * Ported from upstream iw5-gsc-utils's component/io.cpp almost verbatim - the
 * function bodies are UNCHANGED.
 *
 * DROPPED: httpget - it used a risky dependency chain not needed here.
 */

namespace io
{
	namespace
	{
		/*
		 * Best-effort: point the process CWD at the game's base folder.
		 * Uses a SEH-guarded frame (POD-only locals) to read Dvar_FindVar safely,
		 * preventing server crashes on failure. std::filesystem operations follow
		 * in a separate frame to satisfy MSVC C2712.
		 */
		struct raw_path_result
		{
			bool ok;
			char buffer[MAX_PATH];
		};

		raw_path_result read_base_path_seh()
		{
			raw_path_result r{ false, {} };

			__try
			{
				const auto* dvar = game::Dvar_FindVar("fs_basegame");
				if (dvar && dvar->current.string)
				{
					strncpy_s(r.buffer, dvar->current.string, sizeof(r.buffer) - 1);
					r.ok = true;
				}
				return r;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				r.ok = false;
				return r;
			}
		}

		void set_base_path()
		{
			const auto result = read_base_path_seh();
			if (result.ok)
	{
				std::filesystem::current_path(result.buffer);
			}
		}
	}

	void init()
		{
		set_base_path();

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
			return static_cast<int>(utils::io::file_size(path));
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
		}
}
