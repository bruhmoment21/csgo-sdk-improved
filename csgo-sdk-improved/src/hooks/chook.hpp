#pragma once

#include <memory>

#include "../logger/logger.hpp"

#include <funchook/src/funchook.h>

// Hook context
inline funchook_t *g_funchookCtx = nullptr;

template <typename T> class CHook
{
  public:
    // Template has been used to avoid casts.
    template <typename OriginalT, typename HookT>
    void Hook(OriginalT _pOriginalFn, HookT &pHookFn, const char *szFileName, int line)
    {
        void *pOriginalFn = static_cast<void *>(_pOriginalFn);

        if (!pOriginalFn)
        {
            SDK_ERROR("'{}:{}' tried hooking 0x0.", szFileName, line);
            return;
        }

        this->m_pOriginalFn = static_cast<decltype(this->m_pOriginalFn)>(pOriginalFn);

        int rv = funchook_prepare(g_funchookCtx, reinterpret_cast<void **>(&this->m_pOriginalFn),
                                  static_cast<void *>(pHookFn));

        SDK_ASSERT(rv == FUNCHOOK_ERROR_SUCCESS, "funchook_prepare() failed!");
        if (rv == FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_INFO("'{}:{}' funchook_prepare() returned {}.", szFileName, line, rv);
            SDK_INFO("Function '{}' is now being redirected to '{}'.", fmt::ptr(pOriginalFn), fmt::ptr(pHookFn));
        }
        else
        {
            SDK_WARN("'{}:{}' funchook_prepare() returned {}.", szFileName, line, rv);
        }
    }

    std::add_pointer_t<T> m_pOriginalFn;
};
