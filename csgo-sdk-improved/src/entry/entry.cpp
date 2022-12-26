#include <thread>

#ifdef WIN32
#include <Windows.h>
#endif

void InitializeSDK();
void UninitializeSDK();

#ifdef __linux__
static void __attribute__((constructor)) OnAttach()
{
    std::thread hThread{InitializeSDK};
    hThread.detach();
}

static void __attribute__((destructor)) OnDetach()
{
    UninitializeSDK();
}
#else
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);

        HANDLE hThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(InitializeSDK), NULL, 0, NULL);
        if (hThread != NULL)
        {
            CloseHandle(hThread);
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH && !lpvReserved)
    {
        UninitializeSDK();
    }

    return TRUE;
}
#endif
