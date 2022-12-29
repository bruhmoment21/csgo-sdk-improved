#pragma once

#include <cstdint>

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
    // Defined in 'module.cpp'.
    uintptr_t Get(const char *szFileName, int line);

    uintptr_t m_ptr;
};
