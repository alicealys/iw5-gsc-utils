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

	bool process_gsc_ctx()
	{
		const auto string_ref = find_string_ref("in call to builtin %s \"%s\"");
		if (!string_ref)
		{
			return false;
		}
		
		const auto gsc_ctx_ptr = *reinterpret_cast<size_t*>(string_ref - 0xAD);
		OutputDebugString(utils::string::va("string_ref: %p\n", string_ref));
		OutputDebugString(utils::string::va("gsc_ctx_ptr: %p\n", gsc_ctx_ptr));
		game::plutonium::gsc_ctx.set(gsc_ctx_ptr);
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
		const auto printf_ptr = string_ref + 4 + 5 + offset;
		OutputDebugString(utils::string::va("printf_ptr: %p\n", printf_ptr));
		game::plutonium::printf.set(printf_ptr);

		return true;
	}

	bool process()
	{
		load_function_tables();
		return process_printf() && process_gsc_ctx();
	}
}
