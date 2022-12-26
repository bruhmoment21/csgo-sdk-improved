#pragma once

#include <string.h>

// This is used everywhere in the SDK.
#define SDK_NAME "csgo-sdk-improved"

// Increases verbosity
#define SDK_ENABLE_VERBOSITY

// Mostly used in methods hooks.
#define HOOK_ARGS void *ecx, void *edx
#define HOOK_CALL ecx, edx

// Calling conventions
#define CDECL_CALL __cdecl
#define STDCALL_CALL __stdcall
#define FASTCALL_CALL __fastcall
#define THISCALL_CALL __thiscall

// Module names.
#define CLIENT_LIB "client.dll"
#define SERVERBROWSER_LIB "serverbrowser.dll"
#define INPUTSYSTEM_LIB "inputsystem.dll"
#define SURFACE_LIB "vguimatsurface.dll"

// Used for paths.
#define SEPARATOR_CHAR '\\'

// Internal
#ifdef SDK_ENABLE_VERBOSITY
#define VERBOSE_PARM(...) __VA_ARGS__
#else
#define VERBOSE_PARM(...)
#endif

#define __FILENAME__ (strrchr(__FILE__, SEPARATOR_CHAR) ? strrchr(__FILE__, SEPARATOR_CHAR) + 1 : __FILE__)
#define FILE_AND_LINE VERBOSE_PARM(__FILENAME__, __LINE__)
