#pragma once

#include <stddef.h>
#include <array>

#include "../sdk_pointer.hpp"

class InterfaceReg;

class CModule
{
  public:
    CModule() = default;
    CModule(const char *moduleName, bool bParseModule = true);
    CModule &operator=(CModule &&rhs) noexcept;

    CModule(CModule &&) = delete;
    CModule(const CModule &) = delete;

    // This will call dlclose on Linux.
    ~CModule();

    bool Parse();
    bool IsLoaded();

    void *GetProcAddress(const char *lpProcName);

    template <size_t N> inline SDK_Pointer FindPattern(std::array<int, N> sigArr)
    {
        return SDK_Pointer{FindPatternEx(sigArr.data(), N)};
    }

    template <typename T> inline T FindInterface(const char *szInterfaceName, const char *szFileName, int line)
    {
        return reinterpret_cast<T>(FindInterfaceEx(szInterfaceName, szFileName, line));
    }

    inline auto GetName() const
    {
        return this->m_moduleName ? this->m_moduleName : "";
    }

    // Needed for linux (dl_iterate_phdr).
    inline auto SetModuleBounds(uintptr_t start, uintptr_t end)
    {
        this->m_start = start;
        this->m_end = end;
    }

  private:
    void ReleaseHandle();

    uintptr_t FindPatternEx(int *sigBytes, size_t sigSize);
    uintptr_t FindInterfaceEx(const char *szInterfaceName, const char *szFileName,
                              int line); // 'Ex' is used wrong here but it is what it is.
    InterfaceReg *GetRegisterLinkedList(const char *szFileName, int line);

    const char *m_moduleName;
    void *m_handle;
    uintptr_t m_start, m_end;
};
