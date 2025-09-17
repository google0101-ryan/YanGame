#pragma once

#include <vulkan/vulkan.h>

#include <tier0/types.h>

#include <tier2/vulkan/RenderAttachment.h>

#include <vector>

#define MAX_ATTACHMENTS 10

enum PassType
{
    PT_COMPUTE,
    PT_GRAPHICS,
};

class CRenderPass
{
    friend class CCommandBuffer;
public:
    void InitDefaults();
    void Finalize();
    void Shutdown();

    void SetType(PassType type) { m_Type = type; }
    VkPipelineLayout GetPipeLayout() { return m_PipeLayout; }

    void SetShader(str_t name);

    void AddInput(int index, CRenderAttachment& attachment) { m_ShaderInputs[index] = attachment; m_iInputCount++; }
    void AddOutput(int index, CRenderAttachment& attachment) { m_Outputs[index] = attachment; m_iOutputCount++; }
    void AddDepthBuf(CRenderAttachment& attachment) { m_DepthAttachment = attachment; hasDepth = true; }

    CRenderAttachment& GetOutput(int index) { return m_Outputs[index]; }

    void SetCullMode(VkCullModeFlagBits flags) { m_CullMode = flags; }

    void SetVertexBinding(size_t stride);
    void AddInputAttribute(int loc, VkFormat fmt, size_t offs);

    void AddPushConstant(size_t size, size_t offs, VkShaderStageFlags stage);
private:
    CRenderAttachment m_ShaderInputs[MAX_ATTACHMENTS];
    CRenderAttachment m_Outputs[MAX_ATTACHMENTS];
    CRenderAttachment m_DepthAttachment = {};
    bool hasDepth = false;
    int m_iInputCount;
    int m_iOutputCount;

    PassType m_Type;

    VkShaderEXT m_VertShader, m_FragShader;
    VkShaderEXT m_ComputeShader;

    // Pipeline settings
    VkCullModeFlagBits m_CullMode;
    VkVertexInputBindingDescription2EXT inputBinding;
    std::vector<VkVertexInputAttributeDescription2EXT> m_Attributes;

    // Pipeline layout (needed for descriptor sets and push constants)
    VkPushConstantRange m_PushConstant; // TODO: Do we need more than one?
    VkPipelineLayout m_PipeLayout;
    VkPipeline m_Pipeline;
};