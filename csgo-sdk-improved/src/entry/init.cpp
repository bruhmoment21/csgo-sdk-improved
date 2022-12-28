#include <thread>
#include <chrono>

#include "../sdk/sdk.hpp"
#include "../menu/menu.hpp"
#include "../hooks/hooks.hpp"
#include "../memory/memory.hpp"
#include "../logger/logger.hpp"

void InitializeSDK()
{
    Logger::Initialize();

    // Waits for 'serverbrowser.dll' to load then proceeds.
    SDK_INFO("Will wait for '{}' if not loaded...", SERVERBROWSER_LIB);
    Logger::TurnOff();
    CModule serverbrowser{SERVERBROWSER_LIB};
    while (!serverbrowser.Parse())
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    Logger::TurnOn();

    auto start = std::chrono::high_resolution_clock::now();

    Memory::InitializeModuleContext();
    Memory::InitializePointers();
    Interfaces::Initialize();
    Hooks::Initialize();

    Memory::FreeModuleContext();

    auto finish = std::chrono::high_resolution_clock::now();
    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    SDK_INFO("Initialization took {}ms.", ms_int.count());
}

void UninitializeSDK()
{
    Hooks::Shutdown();
    Memory::Shutdown();
    Menu::Shutdown(); // This just enables back input.
    Logger::Shutdown();
}
