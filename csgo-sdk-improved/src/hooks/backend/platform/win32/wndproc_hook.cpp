#include <Windows.h>

#include "../../../chook.hpp"

#include "../../../../logger/logger.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>

static HWND g_hWindow;
static WNDPROC g_oWndProc;
static LRESULT hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!ImGui::GetCurrentContext())
    {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hWnd);

        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = io.LogFilename = nullptr;

        // If this is called 2 times it should be a problem.
        SDK_TRACE("Called ImGui::CreateContext().");
    }

    LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    return CallWindowProc(g_oWndProc, hWnd, uMsg, wParam, lParam);
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_HookInputAPI()
{
    g_hWindow = FindWindow("Valve001", NULL);
    g_oWndProc =
        reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hkWndProc)));
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_UnhookInputAPI()
{
    if (g_oWndProc)
    {
        SetWindowLongPtr(g_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oWndProc));
    }
}
