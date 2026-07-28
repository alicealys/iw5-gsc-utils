#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

// lua/lstrlib.c
#define MAX_FORMAT	32
#define L_FMTFLAGSF	"-+#0 "
#define L_FMTFLAGSX	"-#0"
#define L_FMTFLAGSI	"-+0 "
#define L_FMTFLAGSU	"-0"
#define L_FMTFLAGSC	"-"

#include <utils/string.hpp>

namespace string
{
	namespace
	{
		// lua/lstrlib.c
		const char* getformat(const char* strfrmt, char* form)
		{
			const auto len = std::strspn(strfrmt, L_FMTFLAGSF "123456789.") + 1;
			if (len >= MAX_FORMAT - 10)
			{
				throw std::runtime_error("invalid format (too long)");
			}

			*(form++) = '%';
			std::memcpy(form, strfrmt, len * sizeof(char));
			*(form + len) = '\0';
			return strfrmt + len - 1;
		}

		// lua/lstrlib.c
		const char* get_2_digits(const char* s)
		{
			if (isdigit(static_cast<unsigned char>(*s)))
			{
				s++;
				if (isdigit(static_cast<unsigned char>(*s)))
				{
					s++;
				}
			}

			return s;
		}

		// lua/lstrlib.c
		void check_format(const char* form, const char* flags, int precision)
		{
			const char* spec = form + 1;
			spec += std::strspn(spec, flags);
			if (*spec != '0')
			{
				spec = get_2_digits(spec);
				if (*spec == '.' && precision)
				{
					spec++;
					spec = get_2_digits(spec);
				}
			}
			if (!std::isalpha(static_cast<unsigned char>(*spec)))
			{
				throw std::runtime_error(utils::string::va("invalid conversion specification: '%s'", form));
			}
		}

		// partially lua/lstrlib.c
		std::string format_string(const gsc::function_args& args)
		{
			std::string buffer{};
			size_t va_index = 1;

			const auto fmt = args[0].as<std::string>();

			const char* strfrmt = fmt.data();
			const char* strfrmt_end = strfrmt + fmt.size();

			while (strfrmt < strfrmt_end)
			{
				if (*strfrmt != '%')
				{
					buffer.push_back(*strfrmt++);
				}
				else if (*++strfrmt == '%')
				{
					buffer.push_back(*strfrmt++);
				}
				else
				{
					char form[MAX_FORMAT]{};
					const char* flags = "";
					strfrmt = getformat(strfrmt, form);

					switch (*strfrmt++)
					{
					case 'd':
					case 'i':
						flags = L_FMTFLAGSI;
						goto intcase;
					case 'u':
					case 'p':
						flags = L_FMTFLAGSU;
						goto intcase;
					case 'o':
					case 'x':
					case 'X':
						flags = L_FMTFLAGSX;
					intcase:
						{
							check_format(form, flags, 1);
							const auto value = args[va_index].as<int>();
							buffer.append(utils::string::va(form, value));
							va_index++;
							break;
						}
					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
					{
						check_format(form, L_FMTFLAGSF, 1);
						const auto value = args[va_index].as<float>();
						buffer.append(utils::string::va(form, value));
						va_index++;
						break;
					}
					case 'c':
					{
						const auto value = args[va_index].as<int>();
						check_format(form, L_FMTFLAGSC, 0);
						buffer.append(utils::string::va(form, static_cast<char>(value)));
						va_index++;
						break;
					}
					case 's':
					{
						const auto str = args[va_index].as<std::string>();
						buffer.append(str);
						va_index++;
						break;
					}
					default:
					{
						throw std::runtime_error(utils::string::va("invalid conversion '%s' to 'format'", form));
					}
					}
				}
			}

			return buffer;
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
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
					throw std::runtime_error("char index out of bounds");
				}

				return static_cast<int>(string[index]);
			});

			gsc::function::add("chartostring", [](const gsc::function_args& args)
			{
				const auto char_ = static_cast<char>(args[0].as<int>());
				return std::string(1, char_);
			});

			gsc::function::add("parsehex", [](const gsc::function_args& args)
			{
				const auto str = args[0].as<std::string>();
				return static_cast<unsigned int>(std::strtoul(str.data(), nullptr, 0x10));
			});

			gsc::function::add("string_format", format_string);
		}
	};
}

REGISTER_COMPONENT(string::component)
