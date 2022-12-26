#include "hooks.hpp"
#include "chook.hpp"

#include "../memory/memory.hpp"
#include "../logger/logger.hpp"
#include "../config/config.hpp"
#include "../menu/menu.hpp"
#include "../sdk/sdk.hpp"

// These functions have different bodies on Linux!!!
// Check 'platform/win32' and 'platform/linux'.
void SDK_HookInputAPI();
void SDK_UnhookInputAPI();
void SDK_HookGraphicsAPI();
void SDK_UnhookGraphicsAPI();

static CHook<void FASTCALL_CALL(HOOK_ARGS)> g_lockCursor;
static void FASTCALL_CALL hkLockCursor(HOOK_ARGS)
{
    if (Menu::g_showMenu)
    {
        return sdk::g_pSurface->UnlockCursor();
    }

    return g_lockCursor.m_pOriginalFn(HOOK_CALL);
}

static CHook<bool CDECL_CALL()> g_isAccountPrime;
static bool CDECL_CALL hkIsAccountPrime()
{
    if (g_variables.fakePrime)
        return true;

    return g_isAccountPrime.m_pOriginalFn();
}

void Hooks::Initialize()
{
    g_funchookCtx = funchook_create();
    SDK_ASSERT(g_funchookCtx, "Couldn't allocate a funchook handle!");
    if (!g_funchookCtx)
    {
        SDK_CRITICAL("Funchook handle couldn't be allocated.");
        return;
    }

    ::SDK_HookInputAPI();
    ::SDK_HookGraphicsAPI();

    void *pLockCursorFn = vmt::GetVirtual(sdk::g_pSurface, 67);

    // Hook functions right here.
    g_lockCursor.Hook(pLockCursorFn, hkLockCursor, FILE_AND_LINE);
    g_isAccountPrime.Hook(Memory::Pointers::pPrimeFn, hkIsAccountPrime, FILE_AND_LINE);

    int rv = funchook_install(g_funchookCtx, 0);
    SDK_ASSERT(rv == FUNCHOOK_ERROR_SUCCESS, "funchook_install() failed!");
    if (rv != FUNCHOOK_ERROR_SUCCESS)
    {
        SDK_CRITICAL("Failed to install hooks.");
        return;
    }

    SDK_INFO("Initialized hooks!");
}

void Hooks::Shutdown()
{
    int rv = FUNCHOOK_ERROR_SUCCESS;

    if (g_funchookCtx)
    {
        ::SDK_UnhookInputAPI();
        ::SDK_UnhookGraphicsAPI();

        rv = funchook_uninstall(g_funchookCtx, 0);
        if (rv != FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_ERROR("Could not uninstall hooks.");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        rv = funchook_destroy(g_funchookCtx);
        if (rv != FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_ERROR("Could not destroy the funchook handle.");
        }

        g_funchookCtx = nullptr;
    }

    SDK_INFO("Shutdown hooks!");
}
