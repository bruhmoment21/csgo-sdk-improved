#ifdef __linux__
#else
#include <Windows.h>
#endif

#include "utils.hpp"

#ifdef WIN32
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

void Utils::Unload()
{
#ifdef __linux__
#else
    HANDLE hThread = CreateThread(
        NULL, 0, [](LPVOID pData) -> DWORD { FreeLibraryAndExitThread((HMODULE)(pData), EXIT_SUCCESS); },
        HINST_THISCOMPONENT, 0, NULL);

    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
#endif
}
