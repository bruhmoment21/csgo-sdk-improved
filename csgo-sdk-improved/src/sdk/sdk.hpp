#pragma once

#include "interfaces/iinputsystem.hpp"
#include "interfaces/isurface.hpp"

namespace sdk
{
    void FindInterfaces();

    inline IInputSystem *g_pInputSystem;
    inline ISurface *g_pSurface;
} // namespace sdk
