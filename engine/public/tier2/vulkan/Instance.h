#pragma once

#include <vulkan/vulkan.h>

class CVkInstance
{
    friend class CVkInstanceBuilder;
public:
    void Init();

    VkInstance GetHandle() const { return m_Handle; }
private:
    VkInstance m_Handle;
};