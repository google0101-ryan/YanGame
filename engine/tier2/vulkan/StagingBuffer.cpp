#include <tier2/vulkan/StagingBuffer.h>
#include <tier2/vulkan/RenderBackend.h>

#include <tier1/Cvar.h>

#include <tier0/log.h>

#include <cstring>

static CCvar g_StagingBufferSize("staging_size_mb", "16", "Size of the staging buffers in MiBs");

void CVkStagingBuffer::Init()
{
    m_Size = g_StagingBufferSize.GetInt()*1024*1024;
    
    m_StagingBuffer.Init(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    m_pMappedData = m_StagingBuffer.Map();

    m_CopyBuffer.Init(g_pRenderBackend->GetDevice().GetGraphicsQueueIdx());

    m_usedThisFrame = 0;
}

void CVkStagingBuffer::Reset()
{
    m_usedThisFrame = 0;
}

StagingHandle_t CVkStagingBuffer::Alloc(void *pData, size_t size)
{
    if (m_usedThisFrame+size >= m_Size)
        LOG_FATAL("Buffer overflow in staging buffer!\n");
    memcpy((u8*)m_pMappedData+m_usedThisFrame, pData, size);
    StagingHandle_t ret = m_usedThisFrame;
    m_usedThisFrame += size;
    return ret;
}

void CVkStagingBuffer::DoTransfer(CVkBuffer &dst)
{
    m_CopyBuffer.Reset();
    m_CopyBuffer.Begin();

    m_CopyBuffer.CopyBuffer(dst, m_StagingBuffer, 0, m_Size);

    m_CopyBuffer.End();

    auto buffer = m_CopyBuffer.GetHandle();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &buffer;

    // Transfer
    vkQueueSubmit(g_pRenderBackend->GetDevice().GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_pRenderBackend->GetDevice().GetGraphicsQueue());
}
