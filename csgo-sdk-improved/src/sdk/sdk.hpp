#pragma once

#include "interfaces/iinputsystem.hpp"
#include "interfaces/isurface.hpp"

namespace Interfaces
{
    void Initialize();

    inline IInputSystem *g_pInputSystem;
    inline ISurface *g_pSurface;
} // namespace Interfaces
