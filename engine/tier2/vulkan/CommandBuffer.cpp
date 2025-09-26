#include <tier2/vulkan/CommandBuffer.h>
#include <tier2/vulkan/RenderBackend.h>

#include <tier0/log.h>

#include <core/Engine.h>

#include <GLFW/glfw3.h>

void CCommandBuffer::Init(u32 queueIdx)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIdx;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(g_pRenderBackend->GetDevice().GetDeviceHandle(), &poolInfo, nullptr, &m_Pool) != VK_SUCCESS)
        LOG_FATAL("Failed to create pool for command buffer\n");

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = m_Pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    if (vkAllocateCommandBuffers(g_pRenderBackend->GetDevice().GetDeviceHandle(), &allocInfo, &m_Buffer) != VK_SUCCESS)
        LOG_FATAL("Failed to allocate command buffer\n");
}

void CCommandBuffer::Shutdown()
{
    vkFreeCommandBuffers(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_Pool, 1, &m_Buffer);
    vkDestroyCommandPool(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_Pool, nullptr);
}

void CCommandBuffer::Reset()
{
    vkResetCommandBuffer(m_Buffer, 0);
}

void CCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo begin = {};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(m_Buffer, &begin) != VK_SUCCESS)
        LOG_FATAL("Error when beginning command buffer\n");
}

void CCommandBuffer::Draw(int vtxCount, int instCount, int firstVtx, int firstInst)
{
    vkCmdDraw(m_Buffer, vtxCount, instCount, firstVtx, firstInst);
}

void CCommandBuffer::DrawIndexed(int indexCount, int instCount, int firstIndex, int vertOffs, int firstInst)
{
    vkCmdDrawIndexed(m_Buffer, indexCount, instCount, firstIndex, vertOffs, firstInst);
}

void CCommandBuffer::End()
{
    vkEndCommandBuffer(m_Buffer);
}

void CCommandBuffer::SetRenderPass(CRenderPass &pass)
{
    static PFN_vkCmdBindShadersEXT s_CmdBindShadersExt = nullptr;

    if (!s_CmdBindShadersExt)
    {
        s_CmdBindShadersExt = (PFN_vkCmdBindShadersEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdBindShadersEXT");
    }

    std::vector<VkRenderingAttachmentInfoKHR> attachments;
    VkRenderingAttachmentInfoKHR depthInfo = {};
    for (int i = 0; i < pass.m_iOutputCount; i++)
    {
        VkRenderingAttachmentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.imageView = pass.m_Outputs[i].GetImage()->GetView();
        info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        attachments.push_back(info);
    }

    auto extent = g_pRenderBackend->GetSwapChain().GetExtent();

    VkRenderingInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.renderArea = VkRect2D{.offset = VkOffset2D{0, 0}, .extent = extent};
    info.layerCount = 1;
    info.colorAttachmentCount = attachments.size();
    info.pColorAttachments = attachments.data();

    if (pass.hasDepth)
    {
        depthInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthInfo.imageView = pass.m_DepthAttachment.GetImage()->GetView();
        depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthInfo.clearValue.depthStencil.depth = 1.0f;

        info.pDepthAttachment = &depthInfo;
    }

    vkCmdBeginRendering(m_Buffer, &info);

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    s_CmdBindShadersExt(m_Buffer, 1, &stage, &pass.m_VertShader);

    stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    s_CmdBindShadersExt(m_Buffer, 1, &stage, &pass.m_FragShader);

    vkCmdSetRasterizerDiscardEnable(m_Buffer, VK_FALSE);

    static PFN_vkCmdSetRasterizationSamplesEXT s_vkCmdSetRasterizationSamplesEXT = nullptr;
    static PFN_vkCmdSetColorWriteMaskEXT s_vkCmdSetColorWriteMaskEXT;
    static PFN_vkCmdSetPolygonModeEXT s_vkCmdSetPolygonModeEXT;
    static PFN_vkCmdSetSampleMaskEXT s_vkCmdSetSampleMaskEXT;
    static PFN_vkCmdSetAlphaToCoverageEnableEXT s_vkCmdSetAlphaToCoverageEnableEXT;
    static PFN_vkCmdSetColorBlendEnableEXT s_vkCmdSetColorBlendEnableEXT;
    static PFN_vkCmdSetVertexInputEXT s_vkCmdSetVertexInputEXT;

    if (!s_vkCmdSetRasterizationSamplesEXT)
    {
        s_vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetRasterizationSamplesEXT");
        s_vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetColorWriteMaskEXT");;
        s_vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetPolygonModeEXT");
        s_vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetSampleMaskEXT");
        s_vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetAlphaToCoverageEnableEXT");
        s_vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetColorBlendEnableEXT");
        s_vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCmdSetVertexInputEXT");
    }

    std::vector<VkColorComponentFlags> colorMask(attachments.size());
    std::vector<VkSampleMask> sampleMask(attachments.size());
    std::vector<VkBool32> colorBlend(attachments.size());
    for (size_t i = 0; i < attachments.size(); i++)
    {
        colorMask[i] = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        sampleMask[i] = 0;
        colorBlend[i] = VK_FALSE;
    }

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = extent.width;
    viewport.height = extent.height;
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    // Pipeline state
    // TODO: Modularize this
    vkCmdSetRasterizerDiscardEnable(m_Buffer, VK_FALSE);
    vkCmdSetPrimitiveTopology(m_Buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(m_Buffer, VK_FALSE);
    s_vkCmdSetRasterizationSamplesEXT(m_Buffer, VK_SAMPLE_COUNT_1_BIT);
    {
        const VkSampleMask sampleMask = 1;
        s_vkCmdSetSampleMaskEXT(m_Buffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    }

    s_vkCmdSetAlphaToCoverageEnableEXT(m_Buffer, VK_FALSE);
    s_vkCmdSetPolygonModeEXT(m_Buffer, VK_POLYGON_MODE_FILL);
    vkCmdSetFrontFace(m_Buffer, VK_FRONT_FACE_CLOCKWISE);

    if (pass.hasDepth)
    {
        vkCmdSetDepthWriteEnable(m_Buffer, VK_TRUE);
        vkCmdSetDepthTestEnable(m_Buffer, VK_TRUE);
        vkCmdSetDepthCompareOp(m_Buffer, VK_COMPARE_OP_LESS);
    }
    else
    {
        vkCmdSetDepthWriteEnable(m_Buffer, VK_FALSE);
        vkCmdSetDepthTestEnable(m_Buffer, VK_FALSE);
        vkCmdSetDepthCompareOp(m_Buffer, VK_COMPARE_OP_ALWAYS);
    }

    vkCmdSetStencilTestEnable(m_Buffer, VK_FALSE);
    vkCmdSetDepthBoundsTestEnable(m_Buffer, VK_FALSE);
    vkCmdSetDepthBiasEnable(m_Buffer, VK_FALSE);
    
    {
		// Disable color blending
		VkBool32 color_blend_enables[] = {VK_FALSE};
		s_vkCmdSetColorBlendEnableEXT(m_Buffer, 0, 1, color_blend_enables);
	}

	{
		// Use RGBA color write mask
		VkColorComponentFlags color_component_flags[] = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT};
		s_vkCmdSetColorWriteMaskEXT(m_Buffer, 0, 1, color_component_flags);
	}

    vkCmdSetViewportWithCount(m_Buffer, 1, &viewport);
    vkCmdSetScissorWithCount(m_Buffer, 1, &scissor);
    vkCmdSetFrontFace(m_Buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    vkCmdSetCullMode(m_Buffer, VK_CULL_MODE_BACK_BIT);

    
    s_vkCmdSetVertexInputEXT(m_Buffer, 1, &pass.inputBinding, pass.m_Attributes.size(), pass.m_Attributes.data());

    m_CurLayout = pass.m_PipeLayout;
}

void CCommandBuffer::UpdatePushConstant(CRenderPass &pass, void *pData, size_t size)
{
    vkCmdPushConstants(m_Buffer, pass.m_PipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, pData);
}

void CCommandBuffer::BindDescriptorSet(VkDescriptorSet& set)
{
    vkCmdBindDescriptorSets(m_Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurLayout, 0, 1, &set, 0, NULL);
}
void CCommandBuffer::EndRenderPass()
{
    vkCmdEndRendering(m_Buffer);

    m_CurLayout = VK_NULL_HANDLE;
}

void CCommandBuffer::TransitionImage(const CVkImage &image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout,
        const VkImageSubresourceRange &subresourceRange)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.GetHandle();
    barrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(m_Buffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CCommandBuffer::CopyBuffer(CVkBuffer &a, CVkBuffer &b, size_t offs, size_t size)
{
    VkBufferCopy region = {};
    region.dstOffset = offs;
    region.srcOffset = offs;
    region.size = size;

    vkCmdCopyBuffer(m_Buffer, b.GetHandle(), a.GetHandle(), 1, &region);
}

void CCommandBuffer::CopyBufferToImage(CVkImage &a, VkBuffer b, size_t offs, size_t size)
{
    VkBufferImageCopy copy = {};
    copy.bufferOffset = offs;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;

    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageSubresource.mipLevel = 0;

    copy.imageOffset = VkOffset3D{0, 0, 0};
    copy.imageExtent = {a.GetWidth(), a.GetHeight(), 1};

    vkCmdCopyBufferToImage(m_Buffer, b, a.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

void CCommandBuffer::BindVertexBuffer(CVkBuffer &buffer, VkDeviceSize offs)
{
    auto buf = buffer.GetHandle();
    vkCmdBindVertexBuffers(m_Buffer, 0, 1, &buf, &offs);
}

void CCommandBuffer::BindIndexBuffer(CVkBuffer &buffer, VkDeviceSize offs)
{
    auto buf = buffer.GetHandle();
    vkCmdBindIndexBuffer(m_Buffer, buf, offs, VK_INDEX_TYPE_UINT16);
}