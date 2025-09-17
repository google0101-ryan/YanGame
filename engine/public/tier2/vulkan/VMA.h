#pragma once

#include <vk_mem_alloc.h>

class CVkMemoryAllocator
{
public:
    void Init();

    VkBuffer CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo& allocInfo, VmaAllocation& allocation) const;
    void MapMemory(VmaAllocation allocation, void** ppData);
private:
    VmaAllocator m_Allocator;
};