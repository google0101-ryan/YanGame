#include <tier2/vulkan/RenderBackend.h>
#include <tier0/log.h>

#include <core/Engine.h>

#include <Geometry.h>

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include <components/Camera.h>

CVulkanBackend g_backend;
CVulkanBackend* g_pRenderBackend = &g_backend;

IRenderBackend* CreateBackend()
{
    return g_pRenderBackend;
}

Geometry basicTriangle;

// Push the model matrix on the command buffer so we can change it per-model
struct PushConstants
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

bool CVulkanBackend::Init()
{
    m_Instance.Init();

    m_Surface = (VkSurfaceKHR)g_pEngine->GetWindowSystem().CreateSurface(m_Instance.GetHandle());

    m_Device.PushExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_Device.PushExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    m_Device.PushExtension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    m_Device.Init();

    m_SwapChain.Init();

    for (int i = 0; i < MAX_FRAMES; i++)
    {
        m_RenderDoneSema[i].Init();
        m_InFlight[i].Init(true);
        m_MainCommandBuffers[i].Init(m_Device.GetGraphicsQueueIdx());
    }

    CRenderAttachment swapImage; // Leave uninitialized, gets filled out on a per-frame basis

    m_MainRenderPass.SetType(PT_GRAPHICS);

    m_MainRenderPass.AddPushConstant(sizeof(PushConstants), 0, VK_SHADER_STAGE_VERTEX_BIT);
    m_MainRenderPass.SetShader("basic");

    m_MainRenderPass.AddOutput(0, swapImage);
    
    m_MainRenderPass.SetVertexBinding(sizeof(DrawVert));
    m_MainRenderPass.AddInputAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, pos));
    m_MainRenderPass.AddInputAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, color));
    
    m_MainRenderPass.Finalize();

    basicTriangle.iNumVerts = 4;
    basicTriangle.pVerts = new DrawVert[4];
    basicTriangle.iNumIndices = 6;
    basicTriangle.pIndices = new Index_t[6];

    basicTriangle.pVerts[0] = {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}};
    basicTriangle.pVerts[1] = {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    basicTriangle.pVerts[2] = {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    basicTriangle.pVerts[3] = {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}};

    basicTriangle.pIndices[0] = 0;
    basicTriangle.pIndices[1] = 1;
    basicTriangle.pIndices[2] = 2;
    basicTriangle.pIndices[3] = 2;
    basicTriangle.pIndices[4] = 3;
    basicTriangle.pIndices[5] = 0;

    m_Allocator.Init();

    m_VertStagingBuffer.Init();
    m_VertexBuffer.Init(16*1024*1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_IndexStagingBuffer.Init();
    m_IndexBuffer.Init(16*1024*1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    return true;
}

void CVulkanBackend::Shutdown()
{
    LOG_INFO("Backend shutting down...\n");

    vkDeviceWaitIdle(m_Device.GetDeviceHandle());

    // Cleanup time...
    m_MainRenderPass.Shutdown();
    for (int i = 0; i < MAX_FRAMES; i++)
    {
        m_InFlight[i].Shutdown();
        m_RenderDoneSema[i].Shutdown();
        m_MainCommandBuffers[i].Shutdown();
    }
    m_SwapChain.Shutdown();
    m_Device.Shutdown();

    vkDestroySurfaceKHR(m_Instance.GetHandle(), m_Surface, nullptr);

    m_Instance.Shutdown();
}

void CVulkanBackend::SubmitCommand(RBCommand_t &cmd)
{
    switch (cmd.type)
    {
    case RB_DRAW:
        DrawFrame();
        break;
    default:
        LOG_ERROR("Invalid command {} sent to rendering backend\n", (int)cmd.type);    
    }
}

PushConstants cur = { glm::identity<glm::mat4>() };

void CVulkanBackend::DrawFrame()
{
    m_iCurFrame = (m_iCurFrame + 1) % MAX_FRAMES;

    auto handle = m_VertStagingBuffer.Alloc(basicTriangle.pVerts, sizeof(DrawVert)*basicTriangle.iNumVerts);
    m_VertStagingBuffer.DoTransfer(m_VertexBuffer);

    auto indexHandle = m_IndexStagingBuffer.Alloc(basicTriangle.pIndices, sizeof(Index_t)*basicTriangle.iNumIndices);
    m_IndexStagingBuffer.DoTransfer(m_IndexBuffer);

    cur.view = ICameraComponent::GetActiveCamera()->GetView();
    cur.proj = ICameraComponent::GetActiveCamera()->GetProjection();

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer = 0;
    range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    auto fence = m_InFlight[m_iCurFrame].GetHandle();
    vkWaitForFences(m_Device.GetDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);

    u32 index;
    auto image = m_SwapChain.AcquireNextImage(&index);
    if (image == nullptr)
        return;
    m_MainRenderPass.GetOutput(0).SetImage(image);

    vkResetFences(m_Device.GetDeviceHandle(), 1, &fence);

    m_MainCommandBuffers[m_iCurFrame].Begin();

    // First, we transition our swapchain image into a color attachment so that our shader can write to it
    // TODO: These are ugly and verbose and get vulkan all over my nice abstraction. Clean them up.
    m_MainCommandBuffers[m_iCurFrame].TransitionImage(
        *m_MainRenderPass.GetOutput(0).GetImage(),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	    VK_ACCESS_NONE,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		range);

    m_MainCommandBuffers[m_iCurFrame].SetRenderPass(m_MainRenderPass);

    m_MainCommandBuffers[m_iCurFrame].BindVertexBuffer(m_VertexBuffer, handle);
    m_MainCommandBuffers[m_iCurFrame].BindIndexBuffer(m_IndexBuffer, indexHandle);
    m_MainCommandBuffers[m_iCurFrame].UpdatePushConstant(m_MainRenderPass, &cur, sizeof(PushConstants));

    m_MainCommandBuffers[m_iCurFrame].DrawIndexed(basicTriangle.iNumIndices, 1, 0, 0, 0);

    m_MainCommandBuffers[m_iCurFrame].EndRenderPass();

    // Transition the swapchain image into present layout, so it can be shown on-screen
    m_MainCommandBuffers[m_iCurFrame].TransitionImage(
        *m_MainRenderPass.GetOutput(0).GetImage(),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_NONE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        range
    );

    m_MainCommandBuffers[m_iCurFrame].End();

    VkSemaphore semas[] = { m_SwapChain.GetSema().GetHandle() };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    VkCommandBuffer cmdBuf = m_MainCommandBuffers[m_iCurFrame].GetHandle();
    VkSemaphore signalSemas[] = { m_RenderDoneSema[m_iCurFrame].GetHandle() };
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = semas;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemas;

    vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlight[m_iCurFrame].GetHandle());

    auto sc = m_SwapChain.GetHandle();
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &sc;
    presentInfo.pImageIndices = &index;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemas;

    vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        cur.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));;
    }
}

void CVulkanBackend::RecreateSwapchain()
{
    // Rebuild the swapchain to account for window resizing
    m_SwapChain.Shutdown();
    m_SwapChain.Init();
}
