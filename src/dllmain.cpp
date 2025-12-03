#include <stdinc.hpp>
#include "loader/component_loader.hpp"

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

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        component_loader::pre_destroy();
    }

    return TRUE;
}