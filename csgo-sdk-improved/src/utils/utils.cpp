#ifdef __linux__
#include <dlfcn.h>
#include <pthread.h>
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
    Dl_info info{};
    if (dladdr(reinterpret_cast<void *>(Utils::Unload), &info) != 0)
    {
        void *hCurrentLib = dlopen(info.dli_fname, RTLD_LAZY | RTLD_NOLOAD);
        if (hCurrentLib)
        {
            for (int i = 0; i < 2; ++i)
            {
                pthread_t thread;
                pthread_create(&thread, nullptr, reinterpret_cast<void *(*)(void *)>(dlclose), hCurrentLib);
                pthread_detach(thread);
            }
        }
    }
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
