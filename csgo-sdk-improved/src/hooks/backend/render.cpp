#include "../../logger/logger.hpp"
#include "../../menu/menu.hpp"

#include <imgui/imgui.h>

// This function is called in every graphical hook.
void SDK_OnRender()
{
    ImDrawList *pBackgroundDrawList = ImGui::GetBackgroundDrawList();
    if (!pBackgroundDrawList)
    {
        SDK_WARN("ImGui::GetBackgroundDrawList() returned NULL.");
        return;
    }

    pBackgroundDrawList->AddText({8, 8}, IM_COL32(255, 255, 255, 255), SDK_NAME);

    Menu::Render();
}
