#include <Windows.h>

void InitializeSDK();
void UninitializeSDK();

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
