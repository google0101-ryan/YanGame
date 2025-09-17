#include <tier2/vulkan/Buffer.h>
#include <tier2/vulkan/RenderBackend.h>

void CVkBuffer::Init(size_t size, VkBufferUsageFlags usage)
{
    m_Handle = g_pRenderBackend->GetAllocator().CreateBuffer(size, usage, m_AllocInfo, m_Allocation);
}

void* CVkBuffer::Map()
{
    void* ret;
    g_pRenderBackend->GetAllocator().MapMemory(m_Allocation, &ret);
    return ret;
}