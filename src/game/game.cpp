#include <stdinc.hpp>

namespace game
{
	void SV_GameDropClient(int clientNum, const char* reason)
	{
		assert(sv_maxclients->current.integer >= 1 && sv_maxclients->current.integer <= 18);

		if (clientNum >= 0 && clientNum < sv_maxclients->current.integer)
		{
			SV_DropClient(&svs_clients[clientNum], reason, true);
		}
	}
}
