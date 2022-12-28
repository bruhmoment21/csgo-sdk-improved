#ifdef __linux__
#include <SDL2/SDL.h>
#endif

#include "menu.hpp"

#include "fonts/cozette_bitmap.hpp"

#include "../logger/logger.hpp"
#include "../config/config.hpp"
#include "../utils/utils.hpp"
#include "../sdk/sdk.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_freetype.h>

void Menu::Initialize()
{
    // Prepare style/font/etc here.
    // It runs only once.

    ImGuiIO &io = ImGui::GetIO();

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;

    // Add your custom ranges right here.
    // https://github.com/ocornut/imgui/blob/master/docs/FONTS.md#using-custom-glyph-ranges

    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.BuildRanges(&ranges);

    ImFontConfig cfg{};
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Bitmap;

    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(cozzete_bitmap_compressed_data_base85, 13.f, &cfg, ranges.Data);
    io.Fonts->Build();

    SDK_INFO("Initialized menu!");
}

void Menu::Render()
{
    if (ImGui::IsKeyPressed(ImGuiKey_Insert, false))
    {
        g_showMenu = !g_showMenu;
        Menu::OnStateChange();
    }

#ifdef __linux__
    ImGui::GetIO().MouseDrawCursor = g_showMenu;
#endif

    sdk::g_pInputSystem->EnableInput(!g_showMenu);

    // This fixes a glitch where sometimes your cursor will appear when moving mouse fast.
    ImGuiIO &io = ImGui::GetIO();
    ImGuiConfigFlags oldCfgFlags = io.ConfigFlags;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    if (!g_showMenu)
        return;

    io.ConfigFlags = oldCfgFlags;

    ImGui::ShowDemoWindow();
    if (ImGui::Begin(SDK_NAME " window", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Checkbox("Fake prime", &g_variables.fakePrime);
        if (ImGui::Button("Unload", {ImGui::CalcItemWidth(), 0.f}))
        {
            Utils::Unload();
        }
    }
    ImGui::End();
}

void Menu::OnStateChange()
{
#ifdef __linux__
    static SDL_bool bMouseLastState;
    if (g_showMenu)
    {
        bMouseLastState = SDL_GetRelativeMouseMode();
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    else
    {
        SDL_SetRelativeMouseMode(bMouseLastState);
    }
#endif
}

void Menu::Shutdown()
{
    sdk::g_pInputSystem->EnableInput(true);

    SDK_INFO("Shutdown menu!");
}
