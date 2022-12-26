#pragma once

#include <array>

struct SDK_Pointer;
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

struct SDK_Pointer
{
    template <typename T> explicit SDK_Pointer(T ptr)
    {
        this->m_ptr = (uintptr_t)(ptr);
    }

    template <typename T = void *> inline T GetAs(const char *szFileName, int line)
    {
        return reinterpret_cast<T>(this->Get(szFileName, line));
    }

    inline SDK_Pointer &AddOffset(int offset)
    {
        if (!this->m_ptr)
            return *this;

        this->m_ptr += offset;
        return *this;
    }

    inline SDK_Pointer &Deref(int n)
    {
        if (!this->m_ptr)
            return *this;

        while (n-- != 0)
            this->m_ptr = *reinterpret_cast<decltype(this->m_ptr) *>(this->m_ptr);

        return *this;
    }

    inline SDK_Pointer &ToAbs(int preOffset, int postOffset)
    {
        if (!this->m_ptr)
            return *this;

        this->AddOffset(preOffset);
        this->m_ptr = this->m_ptr + 4 + *reinterpret_cast<int *>(this->m_ptr);
        this->AddOffset(postOffset);

        return *this;
    }

  private:
    uintptr_t Get(const char *szFileName, int line);

    uintptr_t m_ptr;
};
