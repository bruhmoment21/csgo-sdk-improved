#pragma once

#include <string.h>

// This is used everywhere in the SDK.
#define SDK_NAME "csgo-sdk-improved"

// Increases verbosity
#define SDK_ENABLE_VERBOSITY

// Platform defines.
#ifdef __linux__
#define WIN32_LINUX(x, y) y

// Mostly used in methods hooks.
#define HOOK_ARGS void *ecx
#define HOOK_CALL ecx
#else
#define WIN32_LINUX(x, y) x

#define HOOK_ARGS void *ecx, void *edx
#define HOOK_CALL ecx, edx
#endif

// Calling conventions
#define CDECL_CALL WIN32_LINUX(__cdecl, )
#define STDCALL_CALL WIN32_LINUX(__stdcall, )
#define FASTCALL_CALL WIN32_LINUX(__fastcall, )
#define THISCALL_CALL WIN32_LINUX(__thiscall, )

// Module names.
#define CLIENT_LIB WIN32_LINUX("client.dll", "csgo/bin/linux64/client_client.so")
#define SERVERBROWSER_LIB WIN32_LINUX("serverbrowser.dll", "serverbrowser_client.so")
#define INPUTSYSTEM_LIB WIN32_LINUX("inputsystem.dll", "inputsystem_client.so")
#define SURFACE_LIB WIN32_LINUX("vguimatsurface.dll", "vguimatsurface_client.so")

// Used for paths.
#define SEPARATOR_CHAR WIN32_LINUX('\\', '/')

// Internal
#ifdef SDK_ENABLE_VERBOSITY
#define VERBOSE_PARM(...) __VA_ARGS__
#else
#define VERBOSE_PARM(...)
#endif

#define __FILENAME__ (strrchr(__FILE__, SEPARATOR_CHAR) ? strrchr(__FILE__, SEPARATOR_CHAR) + 1 : __FILE__)
#define FILE_AND_LINE VERBOSE_PARM(__FILENAME__, __LINE__)
