#pragma once

#include <vulkan/vulkan.h>

#include <tier0/types.h>

#include <tier2/vulkan/RenderAttachment.h>

#define MAX_ATTACHMENTS 10

enum PassType
{
    PT_COMPUTE,
    PT_GRAPHICS,
};

class CRenderPass
{
public:
    void SetType(PassType type) { m_Type = type; }

    void SetShader(str_t name);

    void AddInput(int index, CRenderAttachment& attachment) { m_ShaderInputs[index] = attachment; m_iInputCount++; }
    void AddOutput(int index, CRenderAttachment& attachment) { m_Outputs[index] = attachment; m_iOutputCount++; }
private:
    CRenderAttachment m_ShaderInputs[MAX_ATTACHMENTS];
    CRenderAttachment m_Outputs[MAX_ATTACHMENTS];
    int m_iInputCount;
    int m_iOutputCount;

    PassType m_Type;

    VkShaderEXT m_VertShader, m_FragShader;
    VkShaderEXT m_ComputeShader;
};