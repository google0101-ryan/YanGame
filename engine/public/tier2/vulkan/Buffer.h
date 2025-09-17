#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class CVkBuffer
{
public:
    void Init(size_t size, VkBufferUsageFlags usage);

    void* Map();

    VkBuffer GetHandle() const { return m_Handle; }
private:
    VkBuffer m_Handle;
    VmaAllocation m_Allocation;
    VmaAllocationInfo m_AllocInfo;
};