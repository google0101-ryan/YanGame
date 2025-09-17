#pragma once

#include <vulkan/vulkan.h>

class CVkImage
{
public:
    CVkImage() { m_Image = nullptr; m_ImageView = nullptr; }
    CVkImage(VkImage image, VkImageView view) { m_Image = image; m_ImageView = view; }

    VkImageView GetView() const { return m_ImageView; }
    VkImage GetHandle() const { return m_Image; }
private:
    VkImage m_Image;
    VkImageView m_ImageView;
};