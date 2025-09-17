#pragma once

#include <vulkan/vulkan.h>

class CVkFence
{
public:
    void Init(bool signalled);
    void Shutdown();

    VkFence GetHandle() const { return m_Fence; }
private:
    VkFence m_Fence;
};