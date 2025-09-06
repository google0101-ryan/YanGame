#pragma once

#include <tier0/types.h>
#include <vulkan/vulkan.h>

class CVkSwapChain
{
public:
    void Init();
private:
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
};