#include "hooks.hpp"

#include "../api/hook/chook.hpp"

#include "../memory/memory.hpp"
#include "../logger/logger.hpp"
#include "../config/config.hpp"
#include "../menu/menu.hpp"
#include "../sdk/sdk.hpp"

// These functions have different bodies on Linux!!!
// Check 'backend/win32' and 'backend/linux'.
void SDK_HookInputAPI();
void SDK_UnhookInputAPI();
void SDK_HookGraphicsAPI();
void SDK_UnhookGraphicsAPI();

static CHook<void FASTCALL_CALL(HOOK_ARGS)> g_lockCursor;
static void FASTCALL_CALL hkLockCursor(HOOK_ARGS)
{
    if (Menu::g_showMenu)
    {
        return g_pSurface->UnlockCursor();
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

    // Hook functions right here.
    g_lockCursor.Hook(GET_VIRTUAL(g_pSurface, 67), hkLockCursor, FILE_AND_LINE);
    g_isAccountPrime.Hook(Memory::pIsAccountPrimeFn, hkIsAccountPrime, FILE_AND_LINE);

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

    ::g_isShuttingDown = true;
    SDK_INFO("g_isShuttingDown set to 'true'.");

    if (g_funchookCtx)
    {
        ::SDK_UnhookInputAPI();
        ::SDK_UnhookGraphicsAPI();

        std::this_thread::sleep_for(std::chrono::milliseconds(75));

        rv = funchook_uninstall(g_funchookCtx, 0);
        if (rv != FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_ERROR("Could not uninstall hooks.");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(75));

        rv = funchook_destroy(g_funchookCtx);
        if (rv != FUNCHOOK_ERROR_SUCCESS)
        {
            SDK_ERROR("Could not destroy the funchook handle.");
        }

        g_funchookCtx = nullptr;
    }

    SDK_INFO("Shutdown hooks!");
}
