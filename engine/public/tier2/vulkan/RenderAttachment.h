#pragma once

#include <tier2/vulkan/Image.h>

// Represents an input or output
class CRenderAttachment
{
public:
    // Allows this attachment to be changed to point to any image (used for swapchain images)
    void SetImage(CVkImage* pImage) { m_pBacking = pImage; }
private:
    CVkImage* m_pBacking;
};