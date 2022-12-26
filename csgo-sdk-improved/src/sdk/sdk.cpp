#include "sdk.hpp"

#include "../memory/memory.hpp"

#define FIND_INTERFACE(src, mod, iface) src = mod.FindInterface<decltype(src)>(iface, FILE_AND_LINE)

void sdk::FindInterfaces()
{
    CModulesContext *ctx = Memory::GetModulesContext();
    if (!ctx)
        return;

    FIND_INTERFACE(g_pInputSystem, ctx->g_inputsystem, "InputSystemVersion001");
    FIND_INTERFACE(g_pSurface, ctx->g_surface, "VGUI_Surface031");
}
