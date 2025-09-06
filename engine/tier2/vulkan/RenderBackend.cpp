#include <tier2/vulkan/RenderBackend.h>

#include <core/Engine.h>

CVulkanBackend g_backend;
CVulkanBackend* g_pRenderBackend = &g_backend;

IRenderBackend* CreateBackend()
{
    return g_pRenderBackend;
}

bool CVulkanBackend::Init()
{
    m_Instance.Init();

    m_Surface = (VkSurfaceKHR)g_pEngine->GetWindowSystem().CreateSurface(m_Instance.GetHandle());

    m_Device.PushExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_Device.PushExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    m_Device.Init();

    m_SwapChain.Init();

    return true;
}

void CVulkanBackend::Shutdown()
{
}

void CVulkanBackend::SubmitCommand(RBCommand_t &cmd)
{
}