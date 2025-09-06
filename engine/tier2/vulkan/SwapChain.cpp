#include <tier2/vulkan/RenderBackend.h>
#include <tier2/vulkan/SwapChain.h>

#include <core/Engine.h>

#include <tier0/log.h>

void CVkSwapChain::Init()
{
    VkSurfaceCapabilitiesKHR surfCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_pRenderBackend->GetDevice().GetPhysicalHandle(),
                                                g_pRenderBackend->GetSurface(),
                                            &surfCaps);

    VkSurfaceFormatKHR format;
    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_pRenderBackend->GetDevice().GetPhysicalHandle(),
        g_pRenderBackend->GetSurface(),
        &formatCount, nullptr);
    
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_pRenderBackend->GetDevice().GetPhysicalHandle(),
        g_pRenderBackend->GetSurface(),
        &formatCount, formats.data());

    format.format = VK_FORMAT_UNDEFINED;
    for (auto& fmt : formats)
    {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = fmt;
        }
    }

    if (format.format == VK_FORMAT_UNDEFINED)
        format = formats[0];

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = g_pRenderBackend->GetSurface();
    createInfo.minImageCount = surfCaps.minImageCount + 1;
    if (createInfo.minImageCount > surfCaps.maxImageCount && surfCaps.maxImageCount != 0)
        createInfo.minImageCount = surfCaps.maxImageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = surfCaps.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    u32 indices[] = { g_pRenderBackend->GetDevice().GetGraphicsQueueIdx(), g_pRenderBackend->GetDevice().GetPresentQueueIdx() };
    if (indices[0] != indices[1])
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    else
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    createInfo.clipped = VK_FALSE;
    
    if (vkCreateSwapchainKHR(g_pRenderBackend->GetDevice().GetDeviceHandle(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan swapchain!\n");

    u32 imageCount;
    vkGetSwapchainImagesKHR(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_SwapChain, &imageCount, nullptr);

    m_Images.resize(imageCount);
    m_ImageViews.resize(imageCount);
    vkGetSwapchainImagesKHR(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_SwapChain, &imageCount, m_Images.data());

    for (int i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = m_Images[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = format.format;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(g_pRenderBackend->GetDevice().GetDeviceHandle(), &imageViewInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
            LOG_FATAL("Failed to create image view for image {}\n", i);
    }

    LOG_INFO("Swapchain created with {} images, {}x{}\n", m_Images.size(), surfCaps.currentExtent.width, surfCaps.currentExtent.height);
}