#pragma once

typedef void *(*InstantiateInterfaceFn)();

// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/public/tier1/interface.h#L74
class InterfaceReg
{
  public:
    InstantiateInterfaceFn m_CreateFn;
    const char *m_pName;
    InterfaceReg *m_pNext; // For the global list.
};
