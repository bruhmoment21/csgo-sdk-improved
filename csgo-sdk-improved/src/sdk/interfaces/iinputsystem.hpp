#pragma once

#include "../vmt/vmt.hpp"

class IInputSystem
{
  public:
    auto EnableInput(bool bEnable)
    {
        return CALL_VIRTUAL(void, 11, vmt::THISCALL_CONV, this, bEnable);
    }
};
