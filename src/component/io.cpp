#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "gsc.hpp"
#include "json.hpp"
#include "scripting.hpp"

namespace io
{
	namespace
	{
		struct http_request_t
		{
			std::string url;
			bool completed;
			std::string result;
			scripting::entity handle;
		};

		utils::concurrency::container<std::vector<http_request_t>> requests;

		void run_requests()
		{
			requests.access([&](std::vector<http_request_t>& r)
			{
				for (auto& request : r)
				{
					const auto data = utils::http::get_data(request.url.data());
					if (data.has_value())
					{
						request.result = data->substr(0, 0x5000);
					}
					else
					{
						request.result = "";
					}

					request.completed = true;
				}
			});
		}

		void check_requests()
		{
			requests.access([&](std::vector<http_request_t>& r)
			{
				for (auto i = r.begin(); i != r.end(); )
				{
					if (!i->completed)
					{
						++i;
						continue;
					}
					else
					{
						scripting::notify(i->handle, "done", {i->result});
						i = r.erase(i);
					}
				}
			});
		}

		void add_request(const std::string& url, const scripting::entity& handle)
		{
			requests.access([&](std::vector<http_request_t>& r)
			{
				http_request_t request{};
				request.handle = handle;
				request.url = url;
				request.completed = false;
				r.emplace_back(request);
			});
		}
	}

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
			scheduler::loop(run_requests, scheduler::async);
			scheduler::loop(check_requests, scheduler::server);

			scripting::on_shutdown([]()
			{
				requests.access([&](std::vector<http_request_t>& r)
				{
					r.clear();
				});
			});

			gsc::function::add("jsonprint", [](const gsc::function_args& args) 
				-> scripting::script_value
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

			gsc::function::add("httpget", [](const gsc::function_args& args) 
				-> scripting::script_value
			{
				const auto url = args[0].as<std::string>();
				const auto object = scripting::entity(scripting::make_object());

				add_request(url, object);
			
				return object;
			});
		}
	};
}

REGISTER_COMPONENT(io::component)
