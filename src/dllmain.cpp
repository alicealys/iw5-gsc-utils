#include <stdinc.hpp>
#include "plugin.hpp"

PLUTONIUM_API plutonium::sdk::plugin* PLUTONIUM_CALLBACK on_initialize()
{
    return plugin::get();
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
    }

    return TRUE;
}