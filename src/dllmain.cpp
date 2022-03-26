#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "component/signatures.hpp"

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (!signatures::process())
        {
            MessageBoxA(NULL,
                "This version of iw5-gsc-utils is outdated.\n" \
                "Download the latest dll from here: https://github.com/fedddddd/iw5-gsc-utils/releases",
                "ERROR", MB_ICONERROR);

            return FALSE;
        }

        if (game::plutonium::printf.get() != nullptr)
        {
            utils::hook::jump(reinterpret_cast<uintptr_t>(&printf), game::plutonium::printf);
        }

        component_loader::post_unpack();
    }

    return TRUE;
}