#pragma once

#include <memory>

#include "../../logger/logger.hpp"
#include "../../sdk/vmt/vmt.hpp"

#include <funchook/src/funchook.h>

inline funchook_t *g_funchookCtx = nullptr;
inline bool g_isShuttingDown = false;

template <typename T> class CHook
{
  public:
    // Template has been used to avoid casts.
    template <typename OriginalT, typename HookT>
    void Hook(OriginalT _pOriginalFn, HookT &pHookFn, const char *szFileName, int line)
    {
        SDK_ASSERT(!this->m_pOriginalFn, "Tried to hook an already initialized hook.")
        if (this->m_pOriginalFn)
        {
            SDK_WARN("'{}:{}' tried hooking an already installed hook.", szFileName, line);
            return;
        }

        void *pOriginalFn = static_cast<void *>(_pOriginalFn);

        SDK_ASSERT(pOriginalFn, "Can't hook 0x0!");
        if (!pOriginalFn)
        {
            SDK_ERROR("'{}:{}' tried hooking 0x0.", szFileName, line);
            return;
        }

        this->m_pOriginalFn = reinterpret_cast<decltype(this->m_pOriginalFn)>(pOriginalFn);
        int rv = funchook_prepare(g_funchookCtx, reinterpret_cast<void **>(&this->m_pOriginalFn),
                                  reinterpret_cast<void *>(pHookFn));

        SDK_ASSERT(rv == FUNCHOOK_ERROR_SUCCESS, "funchook_prepare() failed!");
        if (rv == FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_INFO("'{}:{}' funchook_prepare() returned {}.", szFileName, line, rv);
            SDK_INFO("Function '{}' is now being redirected to '{}'.", fmt::ptr(pOriginalFn), fmt::ptr(pHookFn));
        }
        else
        {
            this->m_pOriginalFn = nullptr;

            SDK_WARN("'{}:{}' funchook_prepare() returned {}.", szFileName, line, rv);
        }
    }

    template <typename HookT>
    void HookVirtual(void *pClass, int index, HookT &pHookFn, const char *szFileName, int line)
    {
        this->Hook(vmt::GetVirtual(pClass, index, szFileName, line), pHookFn, szFileName, line);
    }

    std::add_pointer_t<T> m_pOriginalFn;
};
