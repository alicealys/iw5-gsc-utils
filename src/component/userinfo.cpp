#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"
#include "gsc.hpp"

namespace userinfo
{
	using userinfo_map = std::unordered_map<std::string, std::string>;
	std::unordered_map<int, userinfo_map> userinfo_overrides;

	namespace
	{
		utils::hook::detour sv_getuserinfo_hook;

		userinfo_map userinfo_to_map(std::string userinfo)
		{
			userinfo_map map{};

			if (userinfo[0] == '\\')
			{
				userinfo = userinfo.substr(1);
			}

			const auto args = utils::string::split(userinfo, '\\');
			for (size_t i = 0; !args.empty() && i < (args.size() - 1); i += 2)
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

			if (userinfo_overrides.find(index) == userinfo_overrides.end())
			{
				userinfo_overrides[index] = {};
			}

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
	}

	void clear_client_overrides(int client)
	{
		userinfo_overrides[client].clear();
	}

	void clear_overrides()
	{
		userinfo_overrides.clear();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			sv_getuserinfo_hook.create(0x573E00, sv_getuserinfo_stub);

			gsc::method::add("setname", [](const game::scr_entref_t ent, const gsc::function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("Not a player entity");
				}

				const auto name = args[0].as<std::string>();

				userinfo_overrides[ent.entnum]["name"] = name;
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetname", [](const game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("Not a player entity");
				}

				userinfo_overrides[ent.entnum].erase("name");
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("setclantag", [](const game::scr_entref_t ent, const gsc::function_args& args) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("Not a player entity");
				}

				const auto name = args[0].as<std::string>();

				userinfo_overrides[ent.entnum]["clantag"] = name;
				userinfo_overrides[ent.entnum]["ec_TagText"] = name;
				userinfo_overrides[ent.entnum]["ec_usingTag"] = "1";
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("resetclantag", [](const game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("Not a player entity");
				}

				userinfo_overrides[ent.entnum].erase("clantag");
				userinfo_overrides[ent.entnum].erase("ec_TagText");
				userinfo_overrides[ent.entnum].erase("ec_usingTag");
				game::ClientUserinfoChanged(ent.entnum);

				return {};
			});

			gsc::method::add("removeclantag", [](const game::scr_entref_t ent, const gsc::function_args&) -> scripting::script_value
			{
				if (ent.classnum != 0)
				{
					throw std::runtime_error("Invalid entity");
				}

				if (game::g_entities[ent.entnum].client == nullptr)
				{
					throw std::runtime_error("Not a player entity");
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