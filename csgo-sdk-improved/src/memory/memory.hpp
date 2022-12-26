#pragma once

#include "module/module.hpp"

struct CModulesContext;

namespace Memory
{
    void InitializeModuleContext();
    void InitializePointers();

    CModulesContext *GetModulesContext();
    void FreeModuleContext();

    void Shutdown();

    namespace Pointers
    {
        // Everything here should be inlined.
        inline void *pPrimeFn;
    } // namespace Pointers
} // namespace Memory

struct CModulesContext
{
    CModule g_client;
    CModule g_inputsystem;
    CModule g_surface;
};
