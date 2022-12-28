#include "../../../logger/logger.hpp"
#include "../../../memory/memory.hpp"

void SDK_HookVulkanAPI(CModule *pLibdxvk_d3d9);
void SDK_UnhookVulkanAPI();
void SDK_HookOpenGLAPI();
void SDK_UnhookOpenGLAPI();

static bool g_isRunningVulkan = false;
bool IsRunningWithVulkan()
{
    return g_isRunningVulkan;
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_HookGraphicsAPI()
{
    CModule libdxvk_d3d9{"libdxvk_d3d9.so"};
    if (libdxvk_d3d9.IsLoaded())
        g_isRunningVulkan = true;

    if (g_isRunningVulkan)
    {
        SDK_INFO("Vulkan has been detected.");
        SDK_HookVulkanAPI(&libdxvk_d3d9);
    }
    else
    {
        SDK_INFO("OpenGL has been detected.");
        SDK_HookOpenGLAPI();
    }
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_UnhookGraphicsAPI()
{
    if (g_isRunningVulkan)
    {
        SDK_UnhookVulkanAPI();
    }
    else
    {
        SDK_UnhookOpenGLAPI();
    }
}
