#include <stdinc.hpp>
#include "loader/component_loader.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        const auto value = *reinterpret_cast<DWORD*>(0x20700000);
        if (value != 0x6FD5F7FB)
        {
            printf("\x1b[31m\n**************************************************************************************\n\n");
            printf("This version of \x1b[33miw5-gsc-utils\x1b[31m is outdated.\n");
            printf("Download the latest dll from here:\x1b[34m https://github.com/fedddddd/iw5-gsc-utils/releases \x1b[31m\n");
            printf("\n**************************************************************************************\n\n\x1b[37m");
            return FALSE;
        }

        component_loader::post_unpack();
    }

    return TRUE;
}