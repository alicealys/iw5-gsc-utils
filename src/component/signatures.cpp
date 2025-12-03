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
		OutputDebugString(utils::string::va("%p %p\n", base, get_image_size()));
		auto found = false;
		signature.add({
			string,
			mask,
			[&](char* address)
			{
				if (found)
				{
					return;
				}

				found = true;
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
		OutputDebugString("HELLOOO");

		const auto string_ref = find_string_ref("in call to builtin %s \"%s\"");
		if (!string_ref)
		{
			return false;
		}

		const auto gsc_ctx_ptr = *reinterpret_cast<size_t*>(string_ref + 215);
		OutputDebugString(utils::string::va("gsc_ctx_ptr: %p\n", gsc_ctx_ptr));
		game::plutonium::gsc_ctx.set(gsc_ctx_ptr);
		return true;
	}

	bool process()
	{
		load_function_tables();
		return process_gsc_ctx();
	}
}
