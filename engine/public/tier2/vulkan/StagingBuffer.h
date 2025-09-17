#pragma once

#include <tier0/types.h>

#include <tier2/vulkan/Buffer.h>
#include <tier2/vulkan/CommandBuffer.h>

typedef u32 StagingHandle_t;

class CVkStagingBuffer
{
public:
    void Init();
    void Reset();

    // Copy data into the staging buffer
    StagingHandle_t Alloc(void* pData, size_t size);

    // Copy data from the staging buffer into the GPU
    void DoTransfer(CVkBuffer& dst);
private:
    CVkBuffer m_StagingBuffer;
    void* m_pMappedData;
    size_t m_usedThisFrame;
    size_t m_Size;

    CCommandBuffer m_CopyBuffer;
};