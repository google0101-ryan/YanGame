#pragma once

#include <vulkan/vulkan.h>

class CVkSemaphore
{
public:
    void Init();
    void Shutdown();

    const VkSemaphore GetHandle() const { return m_Semaphore; }
private:
    VkSemaphore m_Semaphore;
};