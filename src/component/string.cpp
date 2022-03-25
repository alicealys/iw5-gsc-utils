#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

#include <utils/string.hpp>

namespace string
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("toupper", [](const gsc::function_args& args)
			{
				return utils::string::to_upper(args[0].as<std::string>());
			});

			gsc::function::add("getchar", [](const gsc::function_args& args)
			{
				auto index = 0;
				if (args.size() > 1)
				{
					index = args[1].as<int>();
				}

				const auto string = args[0].as<std::string>();
				if (index >= static_cast<int>(string.size()))
				{
					throw std::runtime_error("Char index out of bounds");
				}

				return static_cast<int>(string[index]);
			});

			gsc::function::add("chartostring", [](const gsc::function_args& args)
			{
				const auto char_ = static_cast<char>(args[0].as<int>());
				return std::string(1, char_);
			});
		}
	};
}

REGISTER_COMPONENT(string::component)