#pragma once

#include <tier2/RenderSystem.h>
#include <tier2/vulkan/Instance.h>
#include <tier2/vulkan/Device.h>
#include <tier2/vulkan/SwapChain.h>
#include <tier2/vulkan/RenderPass.h>
#include <tier2/vulkan/CommandBuffer.h>
#include <tier2/vulkan/VMA.h>
#include <tier2/vulkan/Buffer.h>
#include <tier2/vulkan/StagingBuffer.h>

#include <tier2/vulkan/Fence.h>
#include <tier2/vulkan/Semaphore.h>

#include <tier2/vulkan/RenderDefs.h>

class CVulkanBackend : public IRenderBackend
{
public:
    virtual bool Init();
    virtual void Shutdown();

    virtual void SubmitCommand(RBCommand_t& cmd);

    const CVkInstance& GetInstance() const { return m_Instance; }
    const VkSurfaceKHR& GetSurface() const { return m_Surface; }
    const CVkDevice& GetDevice() const { return m_Device; }
    const CVkSwapChain& GetSwapChain() const { return m_SwapChain; }
    CVkMemoryAllocator& GetAllocator() { return m_Allocator; }

    int GetCurFrame() const { return m_iCurFrame; }
private:
    void DrawFrame();
    void RecreateSwapchain();

    void AddGeometry(Geometry* pGeo);
private:
    CVkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    CVkDevice m_Device;
    CVkSwapChain m_SwapChain;

    CRenderPass m_MainRenderPass;

    CVkSemaphore m_RenderDoneSema[MAX_FRAMES];
    CVkFence m_InFlight[MAX_FRAMES];

    CCommandBuffer m_MainCommandBuffers[MAX_FRAMES];

    int m_iCurFrame = 0;

    CVkMemoryAllocator m_Allocator;
    CVkBuffer m_VertexBuffer;
    CVkStagingBuffer m_VertStagingBuffer;
    CVkBuffer m_IndexBuffer;
    CVkStagingBuffer m_IndexStagingBuffer;
};

extern CVulkanBackend* g_pRenderBackend;