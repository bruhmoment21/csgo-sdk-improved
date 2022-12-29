#pragma once

#include "../api/module/module.hpp"

namespace Memory
{
    void Initialize();
    void Shutdown();

    // Everything below should be inlined.
    inline void *pIsAccountPrimeFn;
} // namespace Memory

struct CModulesContext
{
    static void Initialize();
    static void Shutdown();
    static CModulesContext *Get();

    CModule client;
    CModule inputsystem;
    CModule surface;
    CModule engine;
};
