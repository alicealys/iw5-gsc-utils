#include <stdinc.hpp>

namespace game
{
	void SV_GameDropClient(int clientNum, const char* reason)
	{
		if (clientNum >= 0 && clientNum < sv_maxclients->current.integer)
		{
			SV_DropClient(&svs_clients[clientNum], reason, true);
		}
	}
}
