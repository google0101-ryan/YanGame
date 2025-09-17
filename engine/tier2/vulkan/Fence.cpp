#include <tier2/vulkan/Fence.h>
#include <tier2/vulkan/RenderBackend.h>

#include <tier0/log.h>

void CVkFence::Init(bool signalled)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signalled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if (vkCreateFence(g_pRenderBackend->GetDevice().GetDeviceHandle(), &info, nullptr, &m_Fence) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan fence!\n");
}

void CVkFence::Shutdown()
{
    vkDestroyFence(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_Fence, nullptr);
}