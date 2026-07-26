#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "plugin.hpp"

PLUTONIUM_API plutonium::sdk::plugin* PLUTONIUM_CALLBACK on_initialize()
{
    return plugin::get();
}

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD reason, LPVOID /*reserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {

    }

    if (reason == DLL_PROCESS_DETACH)
    {
        component_loader::on_shutdown();
    }

    return TRUE;
}
