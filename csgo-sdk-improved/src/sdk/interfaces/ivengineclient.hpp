#pragma once

#include "../vmt/vmt.hpp"

class IVEngineClient
{
  public:
    auto IsInGame()
    {
        return CALL_VIRTUAL(bool, 26, vmt::THISCALL_CONV, this);
    }
} inline *g_pEngineClient;
