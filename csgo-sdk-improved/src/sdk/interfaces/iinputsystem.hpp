#pragma once

#include "../vmt/vmt.hpp"

class IInputSystem
{
  public:
    auto EnableInput(bool bEnable)
    {
#ifdef __linux__
        // Does not work on linux sadly ;-;
#else
        return CALL_VIRTUAL(void, 11, vmt::THISCALL_CONV, this, bEnable);
#endif
    }
};
