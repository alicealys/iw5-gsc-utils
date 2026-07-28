#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "gsc.hpp"

namespace userinfo
{
	using userinfo_map = std::unordered_map<std::string, std::string>;
	std::array<userinfo_map, 18> userinfo_overrides;

	namespace
	{
		utils::hook::detour sv_getuserinfo_hook;

		userinfo_map userinfo_to_map(const char* userinfo)
		{
			userinfo_map map{};

			if (userinfo[0] == '\\')
			{
				++userinfo;
			}

			const auto args = utils::string::split(userinfo, '\\');
			for (auto i = 0u; !args.empty() && i < (args.size() - 1); i += 2)
			{
				map[args[i]] = args[i + 1];
			}

			return map;
		}

		std::string map_to_userinfo(const userinfo_map& map)
		{
			std::string buffer{};

			for (const auto& value : map)
			{
				buffer.append("\\");
				buffer.append(value.first);
				buffer.append("\\");
				buffer.append(value.second);
			}

			return buffer;
		}

		void sv_getuserinfo_stub(int index, char* buffer, int bufferSize)
		{
			sv_getuserinfo_hook.invoke<void>(index, buffer, bufferSize);

			auto map = userinfo_to_map(buffer);

			for (const auto& values : userinfo_overrides[index])
			{
				if (values.second.empty())
				{
					map.erase(values.first);
				}
				else
				{
					map[values.first] = values.second;
				}
			}

			const auto userinfo = map_to_userinfo(map);
			strcpy_s(buffer, 1024, userinfo.data());
		}

		void clear_client_overrides(unsigned int client)
		{
			userinfo_overrides[client].clear();
		}

		void* client_connect_stub(int client_num, int script_pers_id)
		{
			const auto res = utils::hook::invoke<void*>(0x4FAFB0, client_num, script_pers_id);

			if (res == nullptr)
			{
				const scripting::entity player = game::Scr_GetEntityId(client_num, 0);
				scripting::notify(*game::levelEntityId, "direct_connect", {player});
			}

			return res;
		}
	}

	void clear_overrides()
	{
		for (auto& entry : userinfo_overrides)
		{
			entry.clear();
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			sv_getuserinfo_hook.create(0x573E00, sv_getuserinfo_stub);
			utils::hook::call(0x573042, client_connect_stub);

			plugin->get_interface()->callbacks()->on_player_connect(clear_client_overrides);
			plugin->get_interface()->callbacks()->on_player_disconnect(clear_client_overrides);

			gsc::method::add("setname", [](const game::scr_entref_t ent, const gsc::function_args& args) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				const auto name = args[0].as<std::string>();

				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetname", [](const game::scr_entref_t ent, const gsc::function_args&)
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				userinfo_overrides[ent.entnum].erase("name");
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("setclantag", [](const game::scr_entref_t ent, const gsc::function_args& args) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				const auto name = args[0].as<std::string>();

				userinfo_overrides[ent.entnum]["clantag"] = name;
				userinfo_overrides[ent.entnum]["ec_TagText"] = name;
				userinfo_overrides[ent.entnum]["ec_usingTag"] = "1";
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetclantag", [](const game::scr_entref_t ent, const gsc::function_args&) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				userinfo_overrides[ent.entnum].erase("clantag");
				userinfo_overrides[ent.entnum].erase("ec_TagText");
				userinfo_overrides[ent.entnum].erase("ec_usingTag");
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("removeclantag", [](const game::scr_entref_t ent, const gsc::function_args&) 
				-> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("not a player entity");
				}

				userinfo_overrides[ent.entnum]["clantag"] = "";
				userinfo_overrides[ent.entnum]["ec_TagText"] = "";
				userinfo_overrides[ent.entnum]["ec_usingTag"] = "0";
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});
		}
	};
}

REGISTER_COMPONENT(userinfo::component)
