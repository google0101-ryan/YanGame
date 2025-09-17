#pragma once

#include <tier0/types.h>
#include <vulkan/vulkan.h>

#include <tier2/vulkan/Image.h>
#include <tier2/vulkan/Semaphore.h>

#include <tier2/vulkan/RenderDefs.h>

class CVkSwapChain
{
public:
    void Init();
    void Shutdown();

    CVkImage* AcquireNextImage(u32* imageIndex);

    const VkExtent2D& GetExtent() const { return m_Extents; }
    VkFormat GetFormat() const { return m_Format; }

    CVkSemaphore& GetSema();
    VkSwapchainKHR GetHandle() const { return m_SwapChain; }
private:
    VkSwapchainKHR m_SwapChain;
    std::vector<CVkImage> m_Images;

    CVkSemaphore m_ImageAvailable[MAX_FRAMES];

    VkExtent2D m_Extents;
    VkFormat m_Format;
};