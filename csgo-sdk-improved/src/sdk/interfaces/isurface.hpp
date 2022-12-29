#pragma once

#include "../vmt/vmt.hpp"

class ISurface
{
  public:
    auto UnlockCursor()
    {
        return CALL_VIRTUAL(void, 66, vmt::THISCALL_CONV, this);
    }
} inline *g_pSurface;
