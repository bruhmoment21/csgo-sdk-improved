#include <GL/gl.h>

#include "../../chook.hpp"

#include "../../../logger/logger.hpp"
#include "../../../memory/memory.hpp"
#include "../../../menu/menu.hpp"

#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>

static bool g_shouldShutDownOpenGL = false;

static CHook<void(SDL_Window *)> g_GLSwapWindow;
static void hkGLSwapWindow(SDL_Window *window)
{
    if (!ImGui::GetCurrentContext())
    {
        SDK_WARN("SDL_Window has not been found! Try alt tabbing...");
        return g_GLSwapWindow.m_pOriginalFn(window);
    }

    void *pRendererUserData = ImGui::GetIO().BackendRendererUserData;
    if (g_shouldShutDownOpenGL && !pRendererUserData)
    {
        return g_GLSwapWindow.m_pOriginalFn(window);
    }
    else if (g_shouldShutDownOpenGL && pRendererUserData)
    {
        ImGui_ImplOpenGL3_Shutdown();
        return g_GLSwapWindow.m_pOriginalFn(window);
    }
    else if (!pRendererUserData)
    {
        ImGui_ImplOpenGL3_Init();
        Menu::Initialize();

        SDK_TRACE("Called ImGui_ImplOpenGL3_Init().");
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    Menu::Render();

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

    g_shouldShutDownOpenGL = true;
    SDK_TRACE("g_shouldShutDownOpenGL set to 'true'.");

    ImGuiIO &io = ImGui::GetIO();
    while (io.BackendRendererUserData)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (io.BackendPlatformUserData)
        ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();
}
