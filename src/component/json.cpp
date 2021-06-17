#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/functions.hpp"

#include "gsc.hpp"

#include <json.hpp>

namespace json
{
	namespace
	{
		nlohmann::json gsc_to_json(scripting::script_value value);

		nlohmann::json entity_to_array(unsigned int id)
		{
			nlohmann::json obj;
			auto string_indexed = -1;

			const auto offset = 0xC800 * (id & 1);
			auto current = game::scr_VarGlob->objectVariableChildren[id].firstChild;

			for (auto i = offset + current; current; i = offset + current)
			{
				const auto var = game::scr_VarGlob->childVariableValue[i];

				if (var.type == game::SCRIPT_NONE)
				{
					current = var.nextSibling;
					continue;
				}

				const auto string_value = (unsigned int)((unsigned __int8)var.name_lo + (var.k.keys.name_hi << 8));
				const auto* str = game::SL_ConvertToString(string_value);

				if (string_indexed == -1)
				{
					string_indexed = string_value < 0x40000 && str;
				}

				game::VariableValue variable{};
				variable.type = (game::scriptType_e)var.type;
				variable.u = var.u.u;

				if (!string_indexed)
				{
					obj.emplace_back(gsc_to_json(variable));
				}
				else 
				{
					obj.emplace(str, gsc_to_json(variable));
				}

				current = var.nextSibling;
			}

			return obj;
		}

		nlohmann::json vector_to_array(const float* value)
		{
			nlohmann::json obj;
			obj.push_back(value[0]);
			obj.push_back(value[1]);
			obj.push_back(value[2]);

			return obj;
		}

		nlohmann::json gsc_to_json(scripting::script_value _value)
		{
			const auto variable = _value.get_raw();
			const auto value = variable.u;
			const auto type = variable.type;

			switch (type)
			{
			case (game::SCRIPT_NONE):
				return {};
			case (game::SCRIPT_INTEGER):
				return value.intValue;
			case (game::SCRIPT_FLOAT):
				return value.floatValue;
			case (game::SCRIPT_STRING):
			case (game::SCRIPT_ISTRING):
				return game::SL_ConvertToString(static_cast<unsigned int>(value.stringValue));
			case (game::SCRIPT_VECTOR):
				return vector_to_array(value.vectorValue);
			case (game::SCRIPT_OBJECT):
			{
				const auto object_type = game::scr_VarGlob->objectVariableValue[value.uintValue].w.type;

				switch (object_type)
				{
				case (game::SCRIPT_STRUCT):
					return "[struct]";
				case (game::SCRIPT_ARRAY):
					return entity_to_array(value.uintValue);
				default:
					return "[entity]";
				}
			}
			case (game::SCRIPT_FUNCTION):
				return "[function]";
			default:
				return "[unknown type]";
			}
		}

		scripting::script_value json_to_gsc(nlohmann::json obj)
		{
			const auto type = obj.type();

			switch (type)
			{
			case (nlohmann::detail::value_t::number_integer):
			case (nlohmann::detail::value_t::number_unsigned):
				return obj.get<int>();
			case (nlohmann::detail::value_t::number_float):
				return obj.get<float>();
			case (nlohmann::detail::value_t::string):
				return obj.get<std::string>();
			case (nlohmann::detail::value_t::array):
			{
				const auto arr = gsc::make_array();

				for (const auto& [key, value] : obj.items())
				{
					gsc::add_array_value(arr, json_to_gsc(value));
				}

				return scripting::entity(arr);
			}
			case (nlohmann::detail::value_t::object):
			{
				const auto arr = gsc::make_array();

				for (const auto& [key, value] : obj.items())
				{
					gsc::add_array_key_value(arr, key, json_to_gsc(value));
				}

				return scripting::entity(arr);
			}
			}

			return {};
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add("array", [](gsc::function_args args)
			{
				const auto array = gsc::make_array();

				for (auto i = 0; i < args.size(); i++)
				{
					gsc::add_array_value(array, args[i]);
				}

				return scripting::entity(array);
			});

			gsc::function::add("map", [](gsc::function_args args)
			{
				const auto array = gsc::make_array();

				for (auto i = 0; i < args.size(); i += 2)
				{
					if (i >= args.size() - 1)
					{
						continue;
					}

					const auto key = args[i].as<std::string>();
					gsc::add_array_key_value(array, key, args[i + 1]);
				}

				return scripting::entity(array);
			});

			gsc::function::add("jsonparse", [](gsc::function_args args)
			{
				const auto json = args[0].as<std::string>();
				const auto obj = nlohmann::json::parse(json);

				return json_to_gsc(obj);
			});

			gsc::function::add("jsonserialize", [](gsc::function_args args)
			{
				const auto value = args[0];
				auto indent = -1;

				if (args.size() > 1)
				{
					indent = args[1].as<int>();
				}

				return gsc_to_json(value).dump(indent);
			});
		}
	};
}

REGISTER_COMPONENT(json::component)