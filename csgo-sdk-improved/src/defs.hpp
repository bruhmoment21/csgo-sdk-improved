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
#define ENGINE_LIB WIN32_LINUX("engine.dll", "engine_client.so")

// Internal

// Used for paths.
#define SEPARATOR_CHAR WIN32_LINUX('\\', '/')

// Gets only filename from __FILE__.
#define __FILENAME__ (strrchr(__FILE__, SEPARATOR_CHAR) ? strrchr(__FILE__, SEPARATOR_CHAR) + 1 : __FILE__)

#ifdef SDK_ENABLE_VERBOSITY
#define FILE_AND_LINE __FILENAME__, __LINE__
#else
#define FILE_AND_LINE nullptr, 0
#endif
