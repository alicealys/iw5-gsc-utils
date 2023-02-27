#include <stdinc.hpp>
#include "functions.hpp"

#include <utils/string.hpp>

namespace scripting
{
	namespace
	{
		std::unordered_map<std::string, uint16_t> lowercase_map(
			const std::unordered_map<std::string_view, uint16_t>& old_map)
		{
			std::unordered_map<std::string, uint16_t> new_map{};
			for (auto& entry : old_map)
			{
				new_map[utils::string::to_lower(entry.first.data())] = entry.second;
			}

			return new_map;
		}

		const std::unordered_map<std::string, uint16_t>& get_methods()
		{
			static auto methods = lowercase_map(*game::plutonium::method_map_rev);
			return methods;
		}

		const std::unordered_map<std::string, uint16_t>& get_functions()
		{
			static auto function = lowercase_map(*game::plutonium::function_map_rev);
			return function;
		}

		int find_function_index(const std::string& name, const bool prefer_global)
		{
			const auto target = utils::string::to_lower(name);

			const auto& primary_map = prefer_global
				                          ? get_functions()
				                          : get_methods();
			const auto& secondary_map = !prefer_global
				                            ? get_functions()
				                            : get_methods();

			auto function_entry = primary_map.find(target);
			if (function_entry != primary_map.end())
			{
				return function_entry->second;
			}

			function_entry = secondary_map.find(target);
			if (function_entry != secondary_map.end())
			{
				return function_entry->second;
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
		const auto& token_map = *game::plutonium::token_map_rev;
		for (const auto& token : token_map)
		{
			if (token.second == id)
			{
				return token.first.data();
			}
		}

		return {};
	}

	std::string find_file(unsigned int id)
	{
		return find_token(id);
	}

	int find_token_id(const std::string& name)
	{
		const auto& token_map = *game::plutonium::token_map_rev;
		const auto result = token_map.find(name);

		if (result != token_map.end())
		{
			return result->second;
		}

		return -1;
	}

	script_function find_function(const std::string& name, const bool prefer_global)
	{
		const auto index = find_function_index(name, prefer_global);
		if (index < 0) return nullptr;

		return get_function_by_index(index);
	}
}
