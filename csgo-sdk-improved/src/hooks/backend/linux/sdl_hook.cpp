#include <SDL2/SDL.h>

#include "../../chook.hpp"

#include "../../../logger/logger.hpp"
#include "../../../memory/memory.hpp"
#include "../../../menu/menu.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>

#include "../../../sdk/vmt/vmt.hpp"

// Defined in 'pre_graphic_hook.cpp'.
bool IsRunningWithVulkan();

static CHook<int(SDL_Event *)> g_pollEvent;
static int hkPollEvent(SDL_Event *event)
{
    // Quite a long function but I don't think its that unreadable.
    int rv = g_pollEvent.m_pOriginalFn(event);
    if (rv == 0)
        return rv;

    if (!ImGui::GetCurrentContext())
    {
        // We will get a pointer to the window if there is an 'SDL_WINDOWEVENT' event.
        // (Opening the window from the taskbar.)
        if (event->type == SDL_WINDOWEVENT)
        {
            SDL_Window *pWindow = SDL_GetWindowFromID(event->window.windowID);
            if (pWindow)
            {
                ImGui::CreateContext();
                if (::IsRunningWithVulkan())
                {
                    ImGui_ImplSDL2_InitForVulkan(pWindow);
                    SDK_TRACE("Called ImGui_ImplSDL2_InitForVulkan().");
                }
                else
                {
                    ImGui_ImplSDL2_InitForOpenGL(pWindow, nullptr);
                    SDK_TRACE("Called ImGui_ImplSDL2_InitForOpenGL().");
                }

                ImGuiIO &io = ImGui::GetIO();
                io.IniFilename = io.LogFilename = nullptr;

                // If this is called 2 times it should be a problem.
                SDK_TRACE("Called ImGui::CreateContext().");
            }
        }
    }
    else
    {
        ImGui_ImplSDL2_ProcessEvent(event);
        event->type = Menu::g_showMenu ? 0 : event->type;
    }

    return rv;
}

static CHook<int(SDL_bool)> g_setRelativeMouseMode;
static int hkSetRelativeMouseMode(SDL_bool enabled)
{
    enabled = Menu::g_showMenu ? SDL_FALSE : enabled;
    return g_setRelativeMouseMode.m_pOriginalFn(enabled);
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_HookInputAPI()
{
    CModule libSDL2{"libSDL2-2.0.so.0"};
    if (!libSDL2.IsLoaded())
    {
        SDK_WARN("{} isn't loaded! Aborting hooking input...", libSDL2.GetName());
        return;
    }

    void *pPollEventFn = libSDL2.GetProcAddress("SDL_PollEvent");
    void *pSetRelativeMouseModeFn = libSDL2.GetProcAddress("SDL_SetRelativeMouseMode");

    g_pollEvent.Hook(pPollEventFn, hkPollEvent, FILE_AND_LINE);
    g_setRelativeMouseMode.Hook(pSetRelativeMouseModeFn, hkSetRelativeMouseMode, FILE_AND_LINE);
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_UnhookInputAPI()
{
    // Does not require any specific cleanup.
}
