#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"

#include "gsc.hpp"

namespace io
{
    namespace
    {
        std::string load_path()
        {
            const auto fs_basegame = game::Dvar_FindVar("fs_basegame");

            return fs_basegame->current.string;
        }

        std::string get_path()
        {
            static const auto path = load_path();

            return path;
        }
    }

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
            const auto path = get_path();
            std::filesystem::current_path(path);

            gsc::function::add("fremove", [](gsc::function_args args)
            {
                const auto path = args[0].as<const char*>();

                const auto result = std::remove(path);

                return result;
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

            gsc::function::add("fgetc", [](gsc::function_args args)
            {
                const auto handle = args[0].as_ptr<FILE>();

                const char c = fgetc(handle);
                const char str[2] = {c, '\0'};

                return std::string(str);
            });

            gsc::function::add("fgets", [](gsc::function_args args)
            {
                const auto handle = args[0].as_ptr<FILE>();
                const auto n = args[1].as<int>();

                char* buffer = (char*)calloc(n, sizeof(char));

                fgets(buffer, n, handle);

                const std::string result = buffer;

                free(buffer);

                return result;
            });

            gsc::function::add("feof", [](gsc::function_args args)
            {
                const auto handle = args[0].as_ptr<FILE>();
                return feof(handle);
            });

            gsc::function::add("fclose", [](gsc::function_args args)
            {
                const auto handle = args[0].as_ptr<FILE>();
                return fclose(handle);
            });

            gsc::function::add("fputs", [](gsc::function_args args)
            {
                const auto text = args[0].as<const char*>();
                const auto handle = args[0].as_ptr<FILE>();

                return fputs(text, handle);
            });

            gsc::function::add("fprintf", [](gsc::function_args args)
            {
                const auto text = args[0].as<const char*>();
                const auto handle = args[1].as_ptr<FILE>();

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