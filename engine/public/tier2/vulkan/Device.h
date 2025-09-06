#pragma once

#include <vulkan/vulkan.h>
#include <tier0/types.h>

#include <vector>

class CVkDevice
{
public:
    void PushExtension(str_t ext);
    void Init();

    VkPhysicalDevice GetPhysicalHandle() const { return m_PhysicalHandle; }
    VkDevice GetDeviceHandle() const { return m_DeviceHandle; }

    u32 GetGraphicsQueueIdx() const { return m_GraphicsQueueIdx; }
    u32 GetPresentQueueIdx() const { return m_PresentQueueIdx; }
private:
    VkPhysicalDevice m_PhysicalHandle;
    VkDevice m_DeviceHandle;

    VkPhysicalDeviceProperties m_Props;

    u32 m_PresentQueueIdx, m_GraphicsQueueIdx;
    VkQueue m_PresentQueue, m_GraphicsQueue;

    std::vector<str_t> m_DesiredExtensions;
};