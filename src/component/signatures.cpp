#include <stdinc.hpp>
#include "signatures.hpp"
#include <utils/hook.hpp>

namespace signatures
{
	size_t load_image_size()
	{
		MODULEINFO info{};
		GetModuleInformation(GetCurrentProcess(),
			GetModuleHandle("plutonium-bootstrapper-win32.exe"), &info, sizeof(MODULEINFO));
		return info.SizeOfImage;
	}

	size_t get_image_size()
	{
		static const auto image_size = load_image_size();
		return image_size;
	}

	void load_function_tables()
	{
		static const auto ptr = *reinterpret_cast<size_t*>(0x56CBDC + 0x1) + 0x56CBDC + 0x5;
		static const auto function_table = *reinterpret_cast<size_t*>(0x56C8EB + 0x3);
		static const auto method_table = *reinterpret_cast<size_t*>(ptr + 0xA);

		game::plutonium::function_table.set(function_table);
		game::plutonium::method_table.set(method_table);
	}

	size_t find_string_ptr(const std::string& string)
	{
		const char* string_ptr = nullptr;
		std::string mask(string.size(), 'x');
		const auto base = reinterpret_cast<size_t>(GetModuleHandle("plutonium-bootstrapper-win32.exe"));
		utils::hook::signature signature(base, get_image_size() - base);

		signature.add({
			string,
			mask,
			[&](char* address)
			{
				string_ptr = address;
			}
		});

		signature.process();
		return reinterpret_cast<size_t>(string_ptr);
	}

	size_t find_string_ref(const std::string& string)
	{
		char bytes[4] = {0};
		const auto string_ptr = find_string_ptr(string);
		if (!string_ptr)
		{
			return 0;
		}

		std::memcpy(bytes, &string_ptr, sizeof(bytes));
		return find_string_ptr({bytes, 4});
	}

	bool process_maps()
	{
		const auto string_ref = find_string_ref("couldn't resolve builtin function id for name '%s'!");
		if (!string_ref)
		{
			return false;
		}

		const auto map_ptr = *reinterpret_cast<size_t*>(string_ref - 0x2B);
		game::plutonium::function_map_rev.set(map_ptr);
		game::plutonium::method_map_rev.set(map_ptr + 0x20);
		game::plutonium::token_map_rev.set(map_ptr + 0xBC);
		return true;
	}

	bool process_printf()
	{
		const auto string_ref = find_string_ref("A critical exception occured!\n");
		if (!string_ref)
		{
			return false;
		}

		const auto offset = *reinterpret_cast<size_t*>(string_ref + 5);
		game::plutonium::printf.set(string_ref + 4 + 5 + offset);

		return true;
	}

	bool process()
	{
		load_function_tables();
		return process_printf() && process_maps();
	}
}
