#pragma once

namespace Menu
{
    void Initialize();
    void Render();
    void OnStateChange();
    void Shutdown();

    inline bool g_showMenu;
} // namespace Menu
