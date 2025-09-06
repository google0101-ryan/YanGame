#include <tier2/vulkan/Device.h>
#include <tier2/vulkan/RenderBackend.h>

#include <tier0/log.h>

#include <vector>

void CVkDevice::PushExtension(str_t ext)
{
    m_DesiredExtensions.push_back(ext);
}

void CVkDevice::Init()
{
    u32 physDevCount;
    vkEnumeratePhysicalDevices(g_pRenderBackend->GetInstance().GetHandle(), &physDevCount, nullptr);

    std::vector<VkPhysicalDevice> devices(physDevCount);
    vkEnumeratePhysicalDevices(g_pRenderBackend->GetInstance().GetHandle(), &physDevCount, devices.data());

    m_PhysicalHandle = VK_NULL_HANDLE;

    for (int i = 0; i < physDevCount; i++)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            m_PhysicalHandle = devices[i];
        else if (!m_PhysicalHandle)
            m_PhysicalHandle = devices[i];
    }

    if (!m_PhysicalHandle)
        LOG_FATAL("Failed to find valid vulkan device!\n");

    vkGetPhysicalDeviceProperties(m_PhysicalHandle, &m_Props);

    LOG_INFO("Vulkan device {} selected\n", m_Props.deviceName);

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalHandle, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalHandle, &queueFamilyCount, queueFamilies.data());

    m_GraphicsQueueIdx = 0xffffffff;
    m_PresentQueueIdx = 0xffffffff;
    for (int i = 0; i < queueFamilyCount; i++)
    {
        auto& props = queueFamilies[i];
        if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_GraphicsQueueIdx = i;

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalHandle, i, g_pRenderBackend->GetSurface(), &presentSupport);

        if (presentSupport)
            m_PresentQueueIdx = i;
    }

    if (m_PresentQueueIdx == 0xffffffff || m_GraphicsQueueIdx == 0xffffffff)
        LOG_FATAL("Failed to find suitable graphics and presentation queues for physical device!\n");

    float queuePrio = 1.0f;

    u32 queueCount = (m_PresentQueueIdx == m_GraphicsQueueIdx) ? 1 : 2;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (int i = 0; i < queueCount; i++)
    {
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePrio;
        queueInfo.queueFamilyIndex = i ? m_GraphicsQueueIdx : m_PresentQueueIdx;
        
        queueInfos.push_back(queueInfo);
    }


    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueInfos.size();
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = m_DesiredExtensions.size();
    createInfo.ppEnabledExtensionNames = m_DesiredExtensions.data();

    if (vkCreateDevice(m_PhysicalHandle, &createInfo, nullptr, &m_DeviceHandle) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan device!\n");

    vkGetDeviceQueue(m_DeviceHandle, m_GraphicsQueueIdx, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_DeviceHandle, m_PresentQueueIdx, 0, &m_PresentQueue);
}