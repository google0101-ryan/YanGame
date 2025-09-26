#pragma once

#include <vulkan/vulkan.h>
#include <tier2/vulkan/VMA.h>

class CVkImage
{
public:
    CVkImage() { m_Image = nullptr; m_ImageView = nullptr; }
    CVkImage(VkImage image, VkImageView view, u32 w, u32 h) { m_Image = image; m_ImageView = view; m_w = w; m_h = h; }
    CVkImage(const char* path);
    CVkImage(VkFormat format, u32 w, u32 h);

    VkImageView GetView() const { return m_ImageView; }
    VkImage GetHandle() const { return m_Image; }
    VkSampler GetSampler() const { return m_Sampler; }

    u32 GetWidth() const { return m_w; }
    u32 GetHeight() const { return m_h; }
private:
    VkImage m_Image;
    VkImageView m_ImageView;
    VkSampler m_Sampler;

    VmaAllocation m_Allocation;
    VmaAllocationInfo m_AllocInfo;

    u32 m_w, m_h;
};