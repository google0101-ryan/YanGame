#define VMA_IMPLEMENTATION
#include <tier2/vulkan/VMA.h>

#include <tier2/vulkan/RenderBackend.h>

#include <tier0/log.h>

void CVkMemoryAllocator::Init()
{
    VmaAllocatorCreateInfo info = {};
    info.device = g_pRenderBackend->GetDevice().GetDeviceHandle();
    info.physicalDevice = g_pRenderBackend->GetDevice().GetPhysicalHandle();
    info.instance = g_pRenderBackend->GetInstance().GetHandle();
    info.vulkanApiVersion = VK_API_VERSION_1_4;

    if (vmaCreateAllocator(&info, &m_Allocator) != VK_SUCCESS)
        LOG_FATAL("Failed to initialize vulkan allocator!\n");
}

VkBuffer CVkMemoryAllocator::CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo& allocInfo, VmaAllocation& allocation) const
{
    bool mappable = false;
    if (usage == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        mappable = true;

    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = usage;

    VmaAllocationCreateInfo createInfo = {};
    createInfo.usage = VMA_MEMORY_USAGE_AUTO;
    createInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

    VkBuffer ret;
    if (vmaCreateBuffer(m_Allocator, &info, &createInfo, &ret, &allocation, &allocInfo) != VK_SUCCESS)
        LOG_FATAL("VMA allocation failed!\n");
    return ret;
}

void CVkMemoryAllocator::MapMemory(VmaAllocation allocation, void **ppData)
{
    vmaMapMemory(m_Allocator, allocation, ppData);
}