#include <tier2/vulkan/Device.h>
#include <tier2/vulkan/RenderBackend.h>

#include <tier0/log.h>

#include <vector>
#include <cstring>

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

        // Query shader object support
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &count, extensions.data());

        bool foundShaderObjs = false;
        for (auto& ext : extensions)
        {
            if (!strcmp(ext.extensionName, "VK_EXT_shader_object"))
            {
                foundShaderObjs = true;
                break;
            }
        }

        if (!foundShaderObjs)
            continue;

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || !m_PhysicalHandle)
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

    VkPhysicalDeviceFeatures features = {};
    features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features vk12Feats = {};
    vk12Feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Feats.runtimeDescriptorArray = VK_TRUE;
    vk12Feats.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vk12Feats.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    vk12Feats.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
    vk12Feats.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    vk12Feats.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    vk12Feats.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    vk12Feats.descriptorBindingPartiallyBound = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &vk12Feats;
    deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamic = {};
    dynamic.pNext = &deviceFeatures2;
    dynamic.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamic.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceShaderObjectFeaturesEXT enableShaders = {};
    enableShaders.pNext = &dynamic;
    enableShaders.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
    enableShaders.shaderObject = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &enableShaders;
    createInfo.queueCreateInfoCount = queueInfos.size();
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = m_DesiredExtensions.size();
    createInfo.ppEnabledExtensionNames = m_DesiredExtensions.data();

    if (vkCreateDevice(m_PhysicalHandle, &createInfo, nullptr, &m_DeviceHandle) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan device!\n");

    vkGetDeviceQueue(m_DeviceHandle, m_GraphicsQueueIdx, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_DeviceHandle, m_PresentQueueIdx, 0, &m_PresentQueue);
}

void CVkDevice::Shutdown()
{
    vkDestroyDevice(m_DeviceHandle, nullptr);
}