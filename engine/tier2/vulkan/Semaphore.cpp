#include <tier2/vulkan/RenderBackend.h>
#include <tier2/vulkan/Semaphore.h>

#include <tier0/log.h>

void CVkSemaphore::Init()
{
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
    if (vkCreateSemaphore(g_pRenderBackend->GetDevice().GetDeviceHandle(), &createInfo, nullptr, &m_Semaphore) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan semaphore!\n");
}

void CVkSemaphore::Shutdown()
{
    vkDestroySemaphore(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_Semaphore, nullptr);
}