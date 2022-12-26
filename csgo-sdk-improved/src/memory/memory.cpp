#include "memory.hpp"

#include "../logger/logger.hpp"

#include <stb/stb.hh>

#define SIG_TO_ARRAY(sig) stb::compiletime_string_to_byte_array_data::getter<sig>::value

static CModulesContext *g_pModulesContext;

void Memory::InitializeModuleContext()
{
    g_pModulesContext = new CModulesContext{};

    g_pModulesContext->g_client = CModule{CLIENT_LIB};
    g_pModulesContext->g_inputsystem = CModule{INPUTSYSTEM_LIB};
    g_pModulesContext->g_surface = CModule{SURFACE_LIB};

    SDK_INFO("Initialized module context!");
}

void Memory::InitializePointers()
{
    using namespace Pointers;

    CModulesContext *ctx = Memory::GetModulesContext();
    if (!ctx)
        return;

#ifdef __linux__
    pPrimeFn = ctx->g_client.FindPattern(SIG_TO_ARRAY("E8 ? ? ? ? 89 C3 8B 85 ? ? ? ?"))
                   .ToAbs(1, 0)
                   .GetAs<decltype(pPrimeFn)>(FILE_AND_LINE);
#else
    pPrimeFn = ctx->g_client.FindPattern(SIG_TO_ARRAY("E8 ? ? ? ? 88 46 14"))
                   .ToAbs(1, 0)
                   .GetAs<decltype(pPrimeFn)>(FILE_AND_LINE);
#endif

    SDK_INFO("Initialized pointers!");
}

CModulesContext *Memory::GetModulesContext()
{
    SDK_ASSERT(g_pModulesContext != nullptr,
               "Modules context not initialized.\n\tMake sure you've called Memory::InitializeModuleContext()");

    return g_pModulesContext;
}

void Memory::FreeModuleContext()
{
    if (g_pModulesContext)
    {
        delete g_pModulesContext;
        g_pModulesContext = nullptr;

        SDK_INFO("Freed module context!");
    }
}

void Memory::Shutdown()
{
    Memory::FreeModuleContext();

    SDK_INFO("Shutdown memory!");
}
