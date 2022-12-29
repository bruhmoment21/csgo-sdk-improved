#include "sdk.hpp"

#include "../memory/memory.hpp"

#define FIND_INTERFACE(src, mod, iface) src = mod.FindInterface<decltype(src)>(iface, FILE_AND_LINE)

void Interfaces::Initialize()
{
    CModulesContext *ctx = CModulesContext::Get();
    if (!ctx)
        return;

    FIND_INTERFACE(g_pInputSystem, ctx->inputsystem, "InputSystemVersion001");
    FIND_INTERFACE(g_pSurface, ctx->surface, "VGUI_Surface031");
    FIND_INTERFACE(g_pEngineClient, ctx->engine, "VEngineClient014");
}
