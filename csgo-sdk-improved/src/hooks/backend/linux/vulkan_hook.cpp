#include <vulkan/vulkan.h>

#include "../../../api/hook/chook.hpp"

#include "../../../logger/logger.hpp"
#include "../../../memory/memory.hpp"
#include "../../../menu/menu.hpp"

#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_sdl.h>

static VkAllocationCallbacks *g_Allocator = NULL;
static VkInstance g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice g_Device = VK_NULL_HANDLE;
static uint32_t g_QueueFamily = (uint32_t)-1;
static VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
static uint32_t g_MinImageCount = 2;
static VkRenderPass g_RenderPass = VK_NULL_HANDLE;
static ImGui_ImplVulkanH_Frame g_Frames[8] = {};
static VkExtent2D g_ImageExtent = {};

// Defined in 'render.cpp'.
void SDK_OnRender();

static void InitializeVulkanContext();
static void CleanupDeviceVulkan();
static void CleanupRenderTarget();
static void RenderImGui_Vulkan(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);

static CHook<VkResult VKAPI_CALL(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t *)>
    g_acquireNextImageKHR;
static VkResult VKAPI_CALL hkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                 VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
    g_Device = device;

    return g_acquireNextImageKHR.m_pOriginalFn(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

static CHook<VkResult VKAPI_CALL(VkQueue, const VkPresentInfoKHR *)> g_queuePresentKHR;
static VkResult VKAPI_CALL hkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
    RenderImGui_Vulkan(queue, pPresentInfo);

    return g_queuePresentKHR.m_pOriginalFn(queue, pPresentInfo);
}

static CHook<VkResult VKAPI_CALL(VkDevice, const VkSwapchainCreateInfoKHR *, const VkAllocationCallbacks *,
                                 VkSwapchainKHR *)>
    g_createSwapchainKHR;
static VkResult VKAPI_CALL hkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain)
{
    CleanupRenderTarget();
    g_ImageExtent = pCreateInfo->imageExtent;

    return g_createSwapchainKHR.m_pOriginalFn(device, pCreateInfo, pAllocator, pSwapchain);
}

static CHook<VkResult VKAPI_CALL(HOOK_ARGS)> g_presentImage;
static VkResult VKAPI_CALL hkPresentImage(HOOK_ARGS)
{
    // Checking if ecx is valid AND if we haven't done this already.
    if (ecx && !g_acquireNextImageKHR.m_pOriginalFn)
    {
        uintptr_t m_vkd = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(ecx) + 0x10);
        if (m_vkd)
        {
            // Found these with IDA in 'libdxvk_d3d9.so'.
            void *pAcquireNextImageKHRFn = *reinterpret_cast<void **>(m_vkd + 0x458);
            void *pQueuePresentKHRFn = *reinterpret_cast<void **>(m_vkd + 0x460);
            void *pCreateSwapchainKHRFn = *reinterpret_cast<void **>(m_vkd + 0x440);

            // This will happen after funchook is installed so
            // we need to uninstall it and reinstall it right after.
            int rv = funchook_uninstall(g_funchookCtx, 0);
            if (rv == FUNCHOOK_ERROR_SUCCESS)
            {
                SDK_INFO("Uninstalled hooks successfully in hkPresentImage().");
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                g_acquireNextImageKHR.Hook(pAcquireNextImageKHRFn, hkAcquireNextImageKHR, FILE_AND_LINE);
                g_queuePresentKHR.Hook(pQueuePresentKHRFn, hkQueuePresentKHR, FILE_AND_LINE);
                g_createSwapchainKHR.Hook(pCreateSwapchainKHRFn, hkCreateSwapchainKHR, FILE_AND_LINE);

                rv = funchook_install(g_funchookCtx, 0);
                if (rv == FUNCHOOK_ERROR_SUCCESS)
                {
                    SDK_INFO("Reinstalled hooks successfully in hkPresentImage().");
                }
                else
                {
                    SDK_ERROR("Failed to install hooks in hkPresentImage().");
                }
            }
            else
            {
                SDK_ERROR("Failed to uninstall hooks in hkPresentImage().");
            }
        }
    }

    return g_presentImage.m_pOriginalFn(HOOK_CALL);
}

void SDK_HookVulkanAPI(CModule *pLibdxvk_d3d9)
{
    // So basically DXVK has an import table for almost every function
    // and we need to somehow access that import table.
    // https://github.com/doitsujin/dxvk/blob/c7be18cb267d30ee3cc1fab0f9ccacbec22b8801/src/vulkan/vulkan_loader.h#L171
    // We need to access the 'DeviceFn' structure somehow to get our needed functions addresses.

    // After some researching I've found out that this function:
    // https://github.com/doitsujin/dxvk/blob/c7be18cb267d30ee3cc1fab0f9ccacbec22b8801/src/vulkan/vulkan_presenter.cpp#L53
    // is getting called every frame which is a member of 'Presenter'
    // WHICH also happens to have a pointer to 'DeviceFn'
    // https://github.com/doitsujin/dxvk/blob/c7be18cb267d30ee3cc1fab0f9ccacbec22b8801/src/vulkan/vulkan_presenter.h#L205

    // To access m_vkd you would do [presenter_class + 16].
    // 0x00: presenter_class_vtable
    // 0x08: m_vki
    // 0x10: m_vkd <---- WHAT WE NEED

    // TL;DR
    // 1. Hook 'Presenter::presentImage'.
    // 2. Get 'DeviceFn' pointer.
    // 3. Hook wanted functions.

    InitializeVulkanContext();

    void *pPresentImageFn = pLibdxvk_d3d9->GetProcAddress("_ZN4dxvk2vk9Presenter12presentImageEv");
    g_presentImage.Hook(pPresentImageFn, hkPresentImage, FILE_AND_LINE);
}

void SDK_UnhookVulkanAPI()
{
    if (ImGui::GetCurrentContext())
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.BackendRendererUserData)
            ImGui_ImplVulkan_Shutdown();

        if (io.BackendPlatformUserData)
            ImGui_ImplSDL2_Shutdown();

        ImGui::DestroyContext();
    }

    CleanupDeviceVulkan();
}

static void InitializeVulkanContext()
{
    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        constexpr const char *instance_extension = "VK_KHR_surface";

        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = &instance_extension;

        // Create Vulkan Instance without any debug feature
        vkCreateInstance(&create_info, g_Allocator, &g_Instance);
    }

    // Select GPU
    {
        uint32_t gpu_count;
        vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
        IM_ASSERT(gpu_count > 0);

        VkPhysicalDevice *gpus = new VkPhysicalDevice[sizeof(VkPhysicalDevice) * gpu_count];
        vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        int use_gpu = 0;
        for (int i = 0; i < (int)gpu_count; ++i)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                use_gpu = i;
                break;
            }
        }

        g_PhysicalDevice = gpus[use_gpu];

        delete[] gpus;
    }

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
        VkQueueFamilyProperties *queues = new VkQueueFamilyProperties[sizeof(VkQueueFamilyProperties) * count];
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; ++i)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                g_QueueFamily = i;
                break;
            }
        delete[] queues;
        IM_ASSERT(g_QueueFamily != (uint32_t)-1);
    }
}

static void CreateRenderTarget(VkDevice device, VkSwapchainKHR swapchain)
{
    uint32_t uImageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &uImageCount, NULL);

    VkImage backbuffers[8] = {};
    vkGetSwapchainImagesKHR(device, swapchain, &uImageCount, backbuffers);

    for (uint32_t i = 0; i < uImageCount; ++i)
    {
        g_Frames[i].Backbuffer = backbuffers[i];

        ImGui_ImplVulkanH_Frame *fd = &g_Frames[i];
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = g_QueueFamily;

            vkCreateCommandPool(device, &info, g_Allocator, &fd->CommandPool);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->CommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;

            vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
        }
    }

    // Create the Render Pass
    {
        VkAttachmentDescription attachment = {};
        attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment = {};
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;

        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;

        vkCreateRenderPass(device, &info, g_Allocator, &g_RenderPass);
    }

    // Create The Image Views
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_B8G8R8A8_UNORM;

        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        for (uint32_t i = 0; i < uImageCount; ++i)
        {
            ImGui_ImplVulkanH_Frame *fd = &g_Frames[i];
            info.image = fd->Backbuffer;

            vkCreateImageView(device, &info, g_Allocator, &fd->BackbufferView);
        }
    }

    // Create Framebuffer
    {
        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = g_RenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.layers = 1;

        for (uint32_t i = 0; i < uImageCount; ++i)
        {
            ImGui_ImplVulkanH_Frame *fd = &g_Frames[i];
            attachment[0] = fd->BackbufferView;

            vkCreateFramebuffer(device, &info, g_Allocator, &fd->Framebuffer);
        }
    }

    if (!g_DescriptorPool) // Create Descriptor Pool.
    {
        constexpr VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                       {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                                       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                       {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
    }
}

static void CleanupRenderTarget()
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (g_Frames[i].CommandBuffer)
        {
            vkFreeCommandBuffers(g_Device, g_Frames[i].CommandPool, 1, &g_Frames[i].CommandBuffer);
            g_Frames[i].CommandBuffer = VK_NULL_HANDLE;
        }
        if (g_Frames[i].CommandPool)
        {
            vkDestroyCommandPool(g_Device, g_Frames[i].CommandPool, g_Allocator);
            g_Frames[i].CommandPool = VK_NULL_HANDLE;
        }
        if (g_Frames[i].BackbufferView)
        {
            vkDestroyImageView(g_Device, g_Frames[i].BackbufferView, g_Allocator);
            g_Frames[i].BackbufferView = VK_NULL_HANDLE;
        }
        if (g_Frames[i].Framebuffer)
        {
            vkDestroyFramebuffer(g_Device, g_Frames[i].Framebuffer, g_Allocator);
            g_Frames[i].Framebuffer = VK_NULL_HANDLE;
        }
    }
}

static void CleanupDeviceVulkan()
{
    CleanupRenderTarget();

    if (g_DescriptorPool)
    {
        vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
        g_DescriptorPool = NULL;
    }
    if (g_Instance)
    {
        vkDestroyInstance(g_Instance, g_Allocator);
        g_Instance = NULL;
    }

    g_ImageExtent = {};
    g_Device = NULL;

    SDK_TRACE("Called CleanupDeviceVulkan().");
}

static void RenderImGui_Vulkan(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
    if (!g_Device || ::g_isShuttingDown)
        return;

    if (!ImGui::GetCurrentContext())
    {
        SDK_WARN("SDL_Window has not been found! Try alt tabbing...");
        return;
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i)
    {
        VkSwapchainKHR swapchain = pPresentInfo->pSwapchains[i];
        if (g_Frames[0].Framebuffer == VK_NULL_HANDLE)
        {
            CreateRenderTarget(g_Device, swapchain);
        }

        ImGui_ImplVulkanH_Frame *fd = &g_Frames[pPresentInfo->pImageIndices[i]];
        {
            vkResetCommandBuffer(fd->CommandBuffer, 0);

            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(fd->CommandBuffer, &info);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = g_RenderPass;
            info.framebuffer = fd->Framebuffer;
            if (g_ImageExtent.width == 0 || g_ImageExtent.height == 0)
            {
                // We don't know the window size the first time. So we just set it to 4K.
                info.renderArea.extent.width = 3840;
                info.renderArea.extent.height = 2160;
            }
            else
            {
                info.renderArea.extent = g_ImageExtent;
            }

            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        if (!ImGui::GetIO().BackendRendererUserData)
        {
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = g_Instance;
            init_info.PhysicalDevice = g_PhysicalDevice;
            init_info.Device = g_Device;
            init_info.QueueFamily = g_QueueFamily;
            init_info.Queue = queue;
            init_info.PipelineCache = g_PipelineCache;
            init_info.DescriptorPool = g_DescriptorPool;
            init_info.Subpass = 0;
            init_info.MinImageCount = g_MinImageCount;
            init_info.ImageCount = g_MinImageCount;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = g_Allocator;

            Menu::Initialize();

            ImGui_ImplVulkan_Init(&init_info, g_RenderPass);
            ImGui_ImplVulkan_CreateFontsTexture(fd->CommandBuffer);

            // If this is called 2 times it should be a problem.
            SDK_TRACE("Called ImGui_ImplVulkan_Init().");
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ::SDK_OnRender();

        ImGui::Render();

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;

            vkEndCommandBuffer(fd->CommandBuffer);
            vkQueueSubmit(queue, 1, &info, NULL);
        }
    }
}
