#include <GL/gl.h>

#include "../../../api/hook/chook.hpp"

#include "../../../logger/logger.hpp"
#include "../../../memory/memory.hpp"
#include "../../../menu/menu.hpp"

#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>

// Defined in 'render.cpp'.
void SDK_OnRender();

static CHook<void(SDL_Window *)> g_GLSwapWindow;
static void hkGLSwapWindow(SDL_Window *window)
{
    if (!ImGui::GetCurrentContext())
    {
        if (!::g_isShuttingDown)
        {
            SDK_WARN("SDL_Window has not been found! Try alt tabbing...");
        }

        return g_GLSwapWindow.m_pOriginalFn(window);
    }

    void *pRendererUserData = ImGui::GetIO().BackendRendererUserData;
    if (::g_isShuttingDown && !pRendererUserData)
    {
        return g_GLSwapWindow.m_pOriginalFn(window);
    }
    else if (::g_isShuttingDown && pRendererUserData)
    {
        ImGui_ImplOpenGL3_Shutdown();
        return g_GLSwapWindow.m_pOriginalFn(window);
    }
    else if (!pRendererUserData)
    {
        ImGui_ImplOpenGL3_Init();
        Menu::Initialize();

        // If this is called 2 times it should be a problem.
        SDK_TRACE("Called ImGui_ImplOpenGL3_Init().");
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ::SDK_OnRender();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return g_GLSwapWindow.m_pOriginalFn(window);
}

void SDK_HookOpenGLAPI()
{
    CModule libSDL2{"libSDL2-2.0.so.0"};
    if (!libSDL2.IsLoaded())
    {
        SDK_WARN("{} isn't loaded! Aborting hooking graphics...", libSDL2.GetName());
        return;
    }

    void *pGLSwapWindowFn = libSDL2.GetProcAddress("SDL_GL_SwapWindow");
    g_GLSwapWindow.Hook(pGLSwapWindowFn, hkGLSwapWindow, FILE_AND_LINE);
}

void SDK_UnhookOpenGLAPI()
{
    if (!ImGui::GetCurrentContext())
        return;

    // 'ImGui_ImplOpenGL3_Shutdown' is being called in our hook because Init and Shutdown
    // must be called from the same thread for some reason.

    ImGuiIO &io = ImGui::GetIO();
    while (io.BackendRendererUserData)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (io.BackendPlatformUserData)
        ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();
}
