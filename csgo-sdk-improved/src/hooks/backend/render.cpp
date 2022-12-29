#include "../../logger/logger.hpp"
#include "../../menu/menu.hpp"
#include "../../sdk/sdk.hpp"

#include <imgui/imgui.h>

// This function is called by every graphical hook.
// You may add features like ESP here.
void SDK_OnRender()
{
    ImDrawList *pBackgroundDrawList = ImGui::GetBackgroundDrawList();
    if (!pBackgroundDrawList)
    {
        SDK_WARN("ImGui::GetBackgroundDrawList() returned NULL.");
        return;
    }

    pBackgroundDrawList->AddText({8, 8}, IM_COL32(255, 255, 255, 255), SDK_NAME);
    if (g_pEngineClient->IsInGame())
    {
        pBackgroundDrawList->AddText({8, 20}, IM_COL32(255, 255, 255, 255), "You are currently in game.");
    }

    Menu::Render();
}
