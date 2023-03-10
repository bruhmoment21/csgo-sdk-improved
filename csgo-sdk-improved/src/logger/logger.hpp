#pragma once

#include <spdlog/spdlog.h>

// This is included here because every file should have 'logger.hpp' included.
#include "../defs.hpp"

#ifdef SDK_ENABLE_ALL_LOGGING
#define SDK_TRACE(...) spdlog::trace(__VA_ARGS__)
#define SDK_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define SDK_INFO(...) spdlog::info(__VA_ARGS__)
#define SDK_WARN(...) spdlog::warn(__VA_ARGS__)
#define SDK_ERROR(...) spdlog::error(__VA_ARGS__)
#define SDK_CRITICAL(...) spdlog::critical(__VA_ARGS__)
#else
#define SDK_TRACE(...)
#define SDK_DEBUG(...)
#define SDK_INFO(...)
#define SDK_WARN(...)
#define SDK_ERROR(...)
#define SDK_CRITICAL(...)
#endif

#ifdef SDK_ENABLE_ASSERTS
#define SDK_ASSERT(cond, msg)                                                                                          \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        SDK_CRITICAL("Assertion!\n\t{} is false\n\t{}\n\tin file: {}\n\ton line: {}", #cond, msg, __FILE__, __LINE__); \
        SDK_DEBUG_BREAK();                                                                                             \
    }
#else
#define SDK_ASSERT(...)
#endif

namespace Logger
{
    void Initialize();
    void Shutdown();

    void TurnOn();
    void TurnOff();
} // namespace Logger
