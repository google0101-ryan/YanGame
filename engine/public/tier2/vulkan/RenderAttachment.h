#pragma once

#include <tier2/vulkan/Image.h>

// Represents an input or output
// TODO: Expand this?
class CRenderAttachment
{
public:
    // Allows this attachment to be changed to point to any image (used for swapchain images)
    void SetImage(CVkImage* pImage) { m_pBacking = pImage; }

    const CVkImage* GetImage() const { return m_pBacking; }
private:
    CVkImage* m_pBacking;
};