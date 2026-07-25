#include <stdinc.hpp>
#include "component/gsc.hpp"
#include "userinfo.hpp"
#include "game/scripting/entity.hpp"
#include "game/scripting/function.hpp"

/*
 * Ported from upstream iw5-gsc-utils's component/userinfo.cpp almost
 * verbatim - logic unchanged.
 * 1. gsc::method::add now uses the SDK-based mechanism.
 * 2. sv_getuserinfo_hook is a NEW MinHook detour address.
 */

namespace userinfo
{
	using userinfo_map = std::unordered_map<std::string, std::string>;
	std::unordered_map<int, userinfo_map> userinfo_overrides;

	namespace
	{
		utils::hook::detour sv_getuserinfo_hook;
		utils::hook::detour clean_name_hook;
		utils::hook::detour client_userinfo_changed_hook;

		/*
		 * ClientCleanName strips ^colour codes from every name the server
		 * publishes, which is why setname() text arrives but colours do not.
		 *
		 * It is NOT enough to bypass it only while setname() drives
		 * ClientUserinfoChanged: the override lives on in userinfo_overrides,
		 * so the engine's own later calls (spawn, class change, ...) re-read
		 * the same name and strip it again - the last pass wins, and the
		 * colours vanish a moment after they were applied.
		 *
		 * So the gate is "does THIS client have a plugin-set name", decided in
		 * the ClientUserinfoChanged detour, which covers every caller. Real
		 * players keep being cleaned normally and cannot colour themselves.
		 */
		bool preserve_colour_codes = false;

		class colour_scope final
		{
		public:
			colour_scope() { preserve_colour_codes = true; }
			~colour_scope() { preserve_colour_codes = false; }

			colour_scope(const colour_scope&) = delete;
			colour_scope& operator=(const colour_scope&) = delete;
		};

		void clean_name_stub(const char* src, char* dest, int size)
		{
			// An empty result would leave the client nameless, so let the
			// original run and apply its "UnnamedPlayer" fallback.
			if (!preserve_colour_codes || !src || !*src || !dest || size <= 1)
			{
				clean_name_hook.invoke<void>(src, dest, size);
				return;
			}

			// Verbatim copy, still honouring the caller's buffer (16 bytes for
			// the name field). Plain loop, no CRT, since this runs inside the
			// engine's userinfo path.
			int i = 0;
			for (; i < size - 1 && src[i]; i++)
			{
				dest[i] = src[i];
			}
			dest[i] = '\0';
		}

		bool has_plugin_set_name(int client)
		{
			const auto it = userinfo_overrides.find(client);
			if (it == userinfo_overrides.end())
			{
				return false;
			}

			// ClientUserinfoChanged cleans both the name and the clantag, so
			// either override is reason enough to keep colour codes.
			for (const auto* key : { "name", "clantag", "ec_TagText" })
			{
				const auto value = it->second.find(key);
				if (value != it->second.end() && !value->second.empty())
				{
					return true;
				}
			}

			return false;
		}

		void client_userinfo_changed_stub(int client)
		{
			if (!has_plugin_set_name(client))
			{
				client_userinfo_changed_hook.invoke<void>(client);
				return;
			}

			colour_scope _;
			client_userinfo_changed_hook.invoke<void>(client);
		}

		userinfo_map userinfo_to_map(std::string userinfo)
		{
			userinfo_map map{};

			if (!userinfo.empty() && userinfo[0] == '\\')
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

	void init()
		{
			sv_getuserinfo_hook.create(0x573E00, sv_getuserinfo_stub);
			clean_name_hook.create(
				reinterpret_cast<size_t>(game::ClientCleanName.get()), clean_name_stub);

			// Covers every caller of ClientUserinfoChanged - ours and the
			// engine's - so a plugin-set name keeps its colours for good.
			client_userinfo_changed_hook.create(
				reinterpret_cast<size_t>(game::ClientUserinfoChanged.get()),
				client_userinfo_changed_stub);

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

				// Colour codes survive because the ClientUserinfoChanged
				// detour sees the override - no scope needed here.
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
}
