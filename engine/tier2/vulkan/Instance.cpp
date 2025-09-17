#include <tier2/vulkan/Instance.h>

#include <tier0/log.h>

#include <core/Engine.h>

const char* layers[] = {
#ifndef _NDEBUG
    "VK_LAYER_KHRONOS_validation"
#endif
};
size_t layerCount = sizeof(layers) / sizeof(layers[0]);

void CVkInstance::Init()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Acrylic";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
    appInfo.pApplicationName = g_pEngine->GetApp()->GetName();
    appInfo.applicationVersion = appInfo.engineVersion;
    appInfo.apiVersion = VK_API_VERSION_1_4; // Most likely to be supported by the vast majority of hw
    
    u32 extCount;
    str_t* extensions = g_pEngine->GetWindowSystem().GetExtensions(&extCount);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extCount;
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.ppEnabledLayerNames = layers;
    
    if (vkCreateInstance(&createInfo, nullptr, &m_Handle))
        LOG_FATAL("Failed to create vulkan instance!\n");
}

void CVkInstance::Shutdown()
{
    vkDestroyInstance(m_Handle, nullptr);
}