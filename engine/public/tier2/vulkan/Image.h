#pragma once

#include <vulkan/vulkan.h>

class CVkImage
{
public:
    CVkImage(VkImage image, VkImageView view);
private:
    VkImage m_Image;
    VkImageView m_ImageView;
};