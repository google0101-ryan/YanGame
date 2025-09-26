#include <tier2/vulkan/RenderPass.h>
#include <tier2/vulkan/RenderBackend.h>

#include <core/Engine.h>

#include <tier0/assert.h>

#include <vector>
#include <string>

std::vector<uint32_t> ReadShader(std::string name)
{
    auto file = g_pEngine->GetFileSystem().OpenFileRead((std::string("shaders/") + name).c_str());

    file->Seek(0, SEEK_DIR_END);
    size_t size = file->Tell();
    file->Seek(0, SEEK_DIR_SET);

    std::vector<uint32_t> ret(size);
    file->Read((char*)ret.data(), size, 1);
    file->Close();
    delete file;

    return ret;
}

void CRenderPass::InitDefaults()
{
    m_CullMode = VK_CULL_MODE_BACK_BIT;
}

extern VkDescriptorSetLayout& GetSetLayout();

void CRenderPass::Finalize()
{
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &m_PushConstant;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &GetSetLayout();

    if (vkCreatePipelineLayout(g_pRenderBackend->GetDevice().GetDeviceHandle(), &layoutInfo, nullptr, &m_PipeLayout) != VK_SUCCESS)
        LOG_FATAL("Failed to create vulkan pipeline layout\n");
}

void CRenderPass::Shutdown()
{
    PFN_vkDestroyShaderEXT s_vkDestroyShaderEXT = (PFN_vkDestroyShaderEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkDestroyShaderEXT");
    if (m_VertShader != VK_NULL_HANDLE)
        s_vkDestroyShaderEXT(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_VertShader, nullptr);
    if (m_FragShader != VK_NULL_HANDLE)
        s_vkDestroyShaderEXT(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_FragShader, nullptr);
    if (m_ComputeShader != VK_NULL_HANDLE)
        s_vkDestroyShaderEXT(g_pRenderBackend->GetDevice().GetDeviceHandle(), m_ComputeShader, nullptr);
}

void CRenderPass::SetShader(str_t name)
{
    ASSERT_MSG(m_Type == PT_GRAPHICS, "TODO: Compute passes");

    static PFN_vkCreateShadersEXT s_CreateShaders;

    if (!s_CreateShaders)
    {
        s_CreateShaders = (PFN_vkCreateShadersEXT)glfwGetInstanceProcAddress(g_pRenderBackend->GetInstance().GetHandle(), "vkCreateShadersEXT");

        if (!s_CreateShaders)
            LOG_FATAL("Failed to load vkCreateShadersEXT function!\n");
    }

    if (m_Type == PT_GRAPHICS)
    {
        auto code = ReadShader(std::string(name) + ".vert.spirv");

        VkShaderCreateInfoEXT shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
        shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderInfo.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shaderInfo.codeSize = code.size();
        shaderInfo.pCode = code.data();
        shaderInfo.pName = "main";
        shaderInfo.pushConstantRangeCount = 1;
        shaderInfo.pPushConstantRanges = &m_PushConstant;
        shaderInfo.setLayoutCount = 1;
        shaderInfo.pSetLayouts = &GetSetLayout();

        if (s_CreateShaders(g_pRenderBackend->GetDevice().GetDeviceHandle(), 1, &shaderInfo, nullptr, &m_VertShader) != VK_SUCCESS)
            LOG_FATAL("Failed to compile shader \"{}\"\n", name);
        
        code = ReadShader(std::string(name) + ".frag.spirv");

        shaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderInfo.nextStage = 0;
        shaderInfo.codeSize = code.size();
        shaderInfo.pCode = code.data();

        if (s_CreateShaders(g_pRenderBackend->GetDevice().GetDeviceHandle(), 1, &shaderInfo, nullptr, &m_FragShader) != VK_SUCCESS)
            LOG_FATAL("Failed to compile shader \"{}\"\n", name);
    }
}

void CRenderPass::SetVertexBinding(size_t stride)
{
    inputBinding.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
    inputBinding.binding = 0;
    inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    inputBinding.stride = stride;
    inputBinding.divisor = 1;
}

void CRenderPass::AddInputAttribute(int loc, VkFormat fmt, size_t offs)
{
    VkVertexInputAttributeDescription2EXT attr = {};
    attr.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
    attr.binding = 0;
    attr.location = loc;
    attr.format = fmt;
    attr.offset = offs;

    m_Attributes.push_back(attr);
}

void CRenderPass::AddPushConstant(size_t size, size_t offs, VkShaderStageFlags stage)
{
    m_PushConstant.size = size;
    m_PushConstant.offset = offs;
    m_PushConstant.stageFlags = stage;
}