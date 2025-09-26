#pragma once

#include <vulkan/vulkan.h>

#include <tier0/types.h>

#include <tier2/vulkan/RenderPass.h>
#include <tier2/vulkan/Buffer.h>

class CCommandBuffer
{
public:
    void Init(u32 queueIdx);
    void Reset();
    void Shutdown();

    void Begin();
    void End();
    void Draw(int vtxCount, int instCount, int firstVtx, int firstInst);
    void DrawIndexed(int indexCount, int instCount, int firstIndex, int vertOffs, int firstInst);

    void SetRenderPass(CRenderPass& pass);
    void UpdatePushConstant(CRenderPass &pass, void* pData, size_t size);
    void BindDescriptorSet(VkDescriptorSet& set);
    void EndRenderPass();

    void TransitionImage(
        const CVkImage& image,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        const VkImageSubresourceRange &subresourceRange
    );

    // Buffer operations
    void CopyBuffer(CVkBuffer& a, CVkBuffer& b, size_t offs, size_t size);
    void CopyBufferToImage(CVkImage& a, VkBuffer b, size_t offs, size_t size);
    void BindVertexBuffer(CVkBuffer& buffer, VkDeviceSize offs);
    void BindIndexBuffer(CVkBuffer& buffer, VkDeviceSize offs);

    VkCommandBuffer GetHandle() { return m_Buffer; }
private:
    VkCommandPool m_Pool;
    VkCommandBuffer m_Buffer;
    VkPipelineLayout m_CurLayout;
};