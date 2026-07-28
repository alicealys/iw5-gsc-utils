#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"
#include "game/scripting/array.hpp"
#include "game/scripting/function.hpp"

#include "gsc.hpp"

namespace gsc
{
	namespace
	{
		void return_value(const scripting::script_value& value)
		{
			if (game::scr_VmPub->outparamcount)
			{
				game::Scr_ClearOutParams();
			}

			scripting::push_value(value);
		}

		auto field_offset_start = 0xA000u;

		std::unordered_map<unsigned int, entity_field_t> custom_fields[class_id_t::class_count];

		utils::hook::detour scr_get_object_field_hook;
		utils::hook::detour scr_set_object_field_hook;
		utils::hook::detour scr_post_load_scripts_hook;

		const char* get_script_loc()
		{
			return "unknown location";
		}

		void print_error(const char* fmt, ...)
		{
			static char buffer[0x1000];

			va_list ap;
			va_start(ap, fmt);
			_vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer), fmt, ap);
			va_end(ap);

			printf("\n"
				"******* script runtime error *******\n"
				"%s\n"
				"\tat %s\n"
				"************************************", 
				buffer, get_script_loc());
		}

		const gsc::entity_field_t* find_field(unsigned int classnum, unsigned int offset)
		{
			const auto& class_map = custom_fields[classnum];
			const auto field_iter = class_map.find(offset);
			if (field_iter == class_map.end())
			{
				return nullptr;
			}

			return &field_iter->second;
		}

		void scr_get_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
		{
			const auto field = find_field(classnum, offset);
			if (field == nullptr)
			{
				return scr_get_object_field_hook.invoke<void>(classnum, entnum, offset);
			}

			try
			{
				const auto result = field->getter(entnum);
				return_value(result);
			}
			catch (const std::exception& e)
			{
				print_error("while getting builtin field \"%s\"\n%s", field->name.data(), e.what());
			}
		}

		void scr_set_object_field_stub(unsigned int classnum, int entnum, unsigned int offset)
		{
			const auto field = find_field(classnum, offset);
			if (field == nullptr)
			{
				return scr_set_object_field_hook.invoke<void>(classnum, entnum, offset);
			}

			function_args args;

			try
			{
				field->setter(entnum, args[0]);
			}
			catch (const std::exception& e)
			{
				print_error("while setting builtin field \"%s\"\n%s", field->name.data(), e.what());
			}
		}

		void scr_post_load_scripts_stub()
		{
			for (auto i = 0; i < class_id_t::class_count; i++)
			{
				const auto& class_map = custom_fields[i];
				for (const auto& [offset, field] : class_map)
				{
					const auto str_id = game::SL_GetString(field.name.data(), 0);
					const auto canon_str = game::SL_GetCanonicalString(field.name.data());
					game::Scr_AddClassField(i, str_id, canon_str, offset);
				}
			}

			return scr_post_load_scripts_hook.invoke<void>();
		}
	}

	void call_function(const script_function& function, const std::string& name)
	{
		try
		{
			function_args args;
			const auto result = function.operator()(args);
			const auto type = result.get_raw().type;

			if (type)
			{
				return_value(result);
			}
		}
		catch (const std::exception& e)
		{
			print_error("in call to builtin function \"%s\"\n%s", name.data(), e.what());
		}
	}

	void call_method(const script_method& method, const std::string& name, game::scr_entref_t ent)
	{
		try
		{
			function_args args;
			const auto result = method.operator()(ent, args);
			const auto type = result.get_raw().type;

			if (type)
			{
				return_value(result);
			}
		}
		catch (const std::exception& e)
		{
			print_error("in call to builtin method \"%s\"\n%s", name.data(), e.what());
		}
	}

	namespace field
	{
		void add(const class_id_t classnum, const std::string& name, const field_getter_t getter, const field_setter_t& setter)
		{
			const auto offset = field_offset_start++;
			auto& class_map = custom_fields[classnum];

			entity_field_t field{};
			field.name = name;
			field.getter = getter;
			field.setter = setter;
			class_map.insert(std::make_pair(offset, field));
		}
	}

	function_args::function_args()
	{
		for (auto i = 0u; i < game::scr_VmPub->outparamcount; i++)
		{
			const auto value = game::scr_VmPub->top[-i];
			this->values_.emplace_back(value);
		}
	}

	function_args::function_args(const std::vector<scripting::script_value>& values)
		: values_(values)
	{
	}

	std::uint32_t function_args::size() const
	{
		return this->values_.size();
	}

	const std::vector<scripting::script_value>& function_args::get_raw() const
	{
		return this->values_;
	}

	std::vector<scripting::script_value>& function_args::get_raw()
	{
		return this->values_;
	}

	scripting::value_wrap function_args::get(const std::uint32_t index) const
	{
		if (index >= this->values_.size())
		{
			throw std::runtime_error(utils::string::va("parameter %d does not exist", index));
		}

		return {this->values_[index], index};
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			scr_get_object_field_hook.create(0x52BDB0, scr_get_object_field_stub);
			scr_set_object_field_hook.create(0x52BCC0, scr_set_object_field_stub);
			scr_post_load_scripts_hook.create(0x628B50, scr_post_load_scripts_stub);

			field::add(class_id_t::class_entity, "entityflags",
				[](unsigned int entnum) -> scripting::script_value
				{
					const auto entity = &game::g_entities[entnum];
					return entity->flags;
				},
				[](unsigned int entnum, const scripting::script_value& value)
				{
					const auto entity = &game::g_entities[entnum];
					entity->flags = value.as<int>();
				}
			);

			field::add(class_id_t::class_entity, "clientflags",
				[](unsigned int entnum) -> scripting::script_value
				{
					const auto entity = &game::g_entities[entnum];
					return entity->client->flags;
				},
				[](unsigned int entnum, const scripting::script_value& value)
				{
					const auto entity = &game::g_entities[entnum];
					entity->client->flags = value.as<int>();
				}
			);

			function::add("executecommand", [](const function_args& args)
				-> scripting::script_value
			{
				game::Cbuf_AddText(0, args[0].as<const char*>());
				return {};
			});

			function::add("addcommand", [](const function_args& args)
				-> scripting::script_value
			{
				const auto name = args[0].as<std::string>();
				const auto function = args[1].as<scripting::function>();
				command::add_script_command(name, [function](const command::params& params)
				{
					scripting::array array;
					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					function({array.get_raw()});
				});

				return {};
			});

			function::add("dropallbots", [](const function_args&) 
				-> scripting::script_value
			{
				for (auto i = 0; i < *game::svs_clientCount; i++)
				{
					if (game::svs_clients[i].header.state != game::CS_FREE
						&& game::svs_clients[i].header.netchan.remoteAddress.type == game::NA_BOT)
					{
						game::SV_GameDropClient(i, "GAME_GET_TO_COVER");
					}
				}

				return {};
			});

			method::add("specialtymarathon", [](const game::scr_entref_t ent, const function_args& args) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				const auto client = ent.entnum;

				if (game::g_entities[client].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				const auto toggle = args[0].as<int>();
				auto flags = game::g_entities[client].client->ps.perks[0];

				game::g_entities[client].client->ps.perks[0] = toggle
					? flags | 0x4000u 
					: flags & ~0x4000u;

				return {};
			});

			method::add("isbot", [](const game::scr_entref_t ent, const function_args&) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				const auto client = ent.entnum;

				if (game::g_entities[client].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				return game::svs_clients[client].bIsTestClient;
			});

			method::add("arecontrolsfrozen", [](const game::scr_entref_t ent, const function_args&) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				const auto client = ent.entnum;

				if (game::g_entities[client].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				return (game::g_entities[client].client->flags & 4) != 0;
			});
		}
	};
}

REGISTER_COMPONENT(gsc::component)
