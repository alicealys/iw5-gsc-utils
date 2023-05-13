#include <stdinc.hpp>
#include "functions.hpp"

#include <utils/string.hpp>

namespace scripting
{
	namespace
	{
		int find_function_index(const std::string& name, [[maybe_unused]] const bool prefer_global)
		{
			const auto target = utils::string::to_lower(name);
			auto const& first = (*game::plutonium::gsc_ctx)->func_map();
			auto const& second = (*game::plutonium::gsc_ctx)->meth_map();

			if (const auto itr = first.find(name); itr != first.end())
			{
				return static_cast<int>(itr->second);
			}

			if (const auto itr = second.find(name); itr != second.end())
			{
				return static_cast<int>(itr->second);
			}

			return -1;
		}

		script_function get_function_by_index(const unsigned index)
		{
			static const auto function_table = game::plutonium::function_table.get();
			static const auto method_table = game::plutonium::method_table.get();

			if (index < 0x1C7)
			{
				return reinterpret_cast<script_function*>(function_table)[index];
			}

			return reinterpret_cast<script_function*>(method_table)[index - 0x8000];
		}
	}

	std::string find_token(unsigned int id)
	{
		return (*game::plutonium::gsc_ctx)->token_name(id);
	}

	std::string find_file(unsigned int id)
	{
		return find_token(id);
	}

	int find_token_id(const std::string& name)
	{
		return (*game::plutonium::gsc_ctx)->token_id(name);
	}

	script_function find_function(const std::string& name, const bool prefer_global)
	{
		const auto index = find_function_index(name, prefer_global);
		if (index < 0)
		{
			return nullptr;
		}

		return get_function_by_index(index);
	}
}
