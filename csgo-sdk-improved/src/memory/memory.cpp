#include "memory.hpp"

#include "../logger/logger.hpp"

#include <stb/stb.hh>

#define SIG_TO_ARRAY(sig) stb::compiletime_string_to_byte_array_data::getter<sig>::value

static CModulesContext *g_pModulesContext;

void Memory::Initialize()
{
    CModulesContext *ctx = CModulesContext::Get();
    if (!ctx)
        return;

#ifdef __linux__
    pIsAccountPrimeFn = ctx->client.FindPattern(SIG_TO_ARRAY("E8 ? ? ? ? 89 C3 8B 85 ? ? ? ?"))
                            .ToAbs(1, 0)
                            .GetAs<decltype(pIsAccountPrimeFn)>(FILE_AND_LINE);
#else
    pIsAccountPrimeFn = ctx->client.FindPattern(SIG_TO_ARRAY("E8 ? ? ? ? 88 46 14"))
                            .ToAbs(1, 0)
                            .GetAs<decltype(pIsAccountPrimeFn)>(FILE_AND_LINE);
#endif

    SDK_INFO("Initialized pointers!");
}

void Memory::Shutdown()
{
    CModulesContext::Shutdown();

    SDK_INFO("Shutdown memory!");
}

// CModulesContext implementation.
void CModulesContext::Initialize()
{
    g_pModulesContext = new CModulesContext{};

    g_pModulesContext->client = CModule{CLIENT_LIB};
    g_pModulesContext->inputsystem = CModule{INPUTSYSTEM_LIB};
    g_pModulesContext->surface = CModule{SURFACE_LIB};
    g_pModulesContext->engine = CModule{ENGINE_LIB};

    SDK_INFO("Initialized module context!");
}

CModulesContext *CModulesContext::Get()
{
    SDK_ASSERT(g_pModulesContext != nullptr,
               "Modules context not initialized.\n\tMake sure you've called CModulesContext::Initialize()");

    return g_pModulesContext;
}

void CModulesContext::Shutdown()
{
    if (g_pModulesContext)
    {
        delete g_pModulesContext;
        g_pModulesContext = nullptr;

        SDK_INFO("Freed module context!");
    }
}
