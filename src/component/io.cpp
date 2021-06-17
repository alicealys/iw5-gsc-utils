#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"

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
		}
	};
}

REGISTER_COMPONENT(io::component)