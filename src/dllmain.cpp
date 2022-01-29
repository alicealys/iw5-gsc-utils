#include <stdinc.hpp>
#include "loader/component_loader.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        const auto value = *reinterpret_cast<DWORD*>(0x21500000);
        if (value != 0x9C28D23B)
        {
            MessageBoxA(NULL,
                "This version of iw5-gsc-utils is outdated.\n" \
                "Download the latest dll from here: https://github.com/fedddddd/iw5-gsc-utils/releases", 
                "ERROR", MB_ICONERROR);

            return FALSE;
        }

        utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), game::plutonium::printf);
        component_loader::post_unpack();
    }

    return TRUE;
}