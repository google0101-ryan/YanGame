#include <tier2/Texture.h>
#include <tier2/vulkan/Image.h>
#include <tier2/vulkan/CommandBuffer.h>
#include <tier2/vulkan/RenderBackend.h>
#include <tier2/vulkan/StagingBuffer.h>

#include <core/Engine.h>

#include <tier0/log.h>

#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include <common/external/stb/stb_image.h>

// TODO: This is a lot of global state. Can we clean this up? A lot?
#define MAX_TEXTURES 65356

CCommandBuffer* g_CmdBuf = nullptr;
CVkStagingBuffer* g_ImageStagingBuffer = nullptr;

enum DescriptorType
{
    DESC_TYPE_SAMPLER,
    DESC_TYPE_UBO,
    DESC_TYPE_STORAGE_BUFFER,
    DESC_TYPE_COUNT
};

VkDescriptorSetLayout g_DescLayout;
VkDescriptorPool g_DescPool;
VkDescriptorSet g_DescSet;
int m_iCurSamplerIdx = 0;

CVkImage::CVkImage(const char* path)
{
    auto file = g_pEngine->GetFileSystem().OpenFileRead(path);

    stbi_uc* data;
    int w, h, comp;

    void* tmp;

    VkFormat fmt = VK_FORMAT_R8G8B8A8_SRGB;

    if (!file)
    {
        LOG_WARN("Failed to open file '{}' for texture\n", path);
        w = 200;
        h = 200;
        comp = 4;

        u32 purple = 0xffff00ff;
        u32 black = 0xff000000;

        int checkerboardWidth = 25;
        int checkerboardHeight = 25;

        tmp = new u32[w*h];

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                if (((i / 25) + (j / 25)) % 2 == 0)
                {
                    ((u32*)tmp)[i*w + j] = purple;
                }
                else
                {
                    ((u32*)tmp)[i * w + j] = black;
                }
            }
        }

        data = (stbi_uc*)tmp;
    }
    else
    {
        file->Seek(0, SEEK_DIR_END);
        size_t size = file->Tell();
        file->Seek(0, SEEK_DIR_SET);

        tmp = new char[size];
        file->Read((char*)tmp, 1, size);
        file->Close();

        delete file;

        data = stbi_load_from_memory((stbi_uc*)tmp, size, &w, &h, &comp, STBI_rgb_alpha);
        // delete[] buf;
    }

    auto& allocator = g_pRenderBackend->GetAllocator();
    
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // TODO: Other types of images
    imageInfo.format = fmt;
    imageInfo.extent.width = w;
    imageInfo.extent.height = h;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_Image = allocator.CreateImage(imageInfo, m_AllocInfo, m_Allocation);

    m_w = w;
    m_h = h;

    LOG_TRACE("Copying image of {}x{}@{} to memory\n", m_w, m_h, comp);

    g_ImageStagingBuffer->Reset();
    auto offs = g_ImageStagingBuffer->Alloc(data, w*h*4);

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = 1;
    range.levelCount = 1;

    g_CmdBuf->Reset();
    g_CmdBuf->Begin();
    g_CmdBuf->TransitionImage(
        *this,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        range
    );
    g_CmdBuf->CopyBufferToImage(*this, g_ImageStagingBuffer->GetHandle(), 0, w*h*4);
    g_CmdBuf->TransitionImage(
        *this,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        range
    );
    g_CmdBuf->End();

    auto buf = g_CmdBuf->GetHandle();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &buf;

    // Transfer
    vkQueueSubmit(g_pRenderBackend->GetDevice().GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_pRenderBackend->GetDevice().GetGraphicsQueue());

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = fmt;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    if (vkCreateImageView(g_pRenderBackend->GetDevice().GetDeviceHandle(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
        LOG_FATAL("Failed to create image view for '{}'\n", path);

    // Create sampler
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(g_pRenderBackend->GetDevice().GetDeviceHandle(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
        LOG_FATAL("Failed to create sampler!\n");

    stbi_image_free(data);
    if (data != (void*)tmp)
        delete[] tmp;
}

CVkImage::CVkImage(VkFormat format, u32 w, u32 h)
{
    auto& allocator = g_pRenderBackend->GetAllocator();
    
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // TODO: Other types of images
    imageInfo.format = format;
    imageInfo.extent.width = w;
    imageInfo.extent.height = h;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    m_Image = allocator.CreateImage(imageInfo, m_AllocInfo, m_Allocation);

    m_w = w;
    m_h = h;

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    if (vkCreateImageView(g_pRenderBackend->GetDevice().GetDeviceHandle(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
        LOG_FATAL("Failed to create image view for image\n");

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = 1;
    range.levelCount = 1;

    g_CmdBuf->Reset();
    g_CmdBuf->Begin();
    if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D16_UNORM_S8_UINT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        g_CmdBuf->TransitionImage(
            *this,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            range
        );
    }
    else
        LOG_FATAL("TODO: Non-depth blank images!\n");
    g_CmdBuf->End();

    auto buf = g_CmdBuf->GetHandle();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &buf;

    // Transfer
    vkQueueSubmit(g_pRenderBackend->GetDevice().GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_pRenderBackend->GetDevice().GetGraphicsQueue());
}

std::set<TexHandle_t> g_TexSlots;
std::unordered_map<TexHandle_t, CVkImage*> textures;

void Texture::InitTextures()
{
    auto hint = g_TexSlots.begin();
    for (TexHandle_t i = 0; i < MAX_TEXTURES; i++)
        g_TexSlots.insert(hint, i);
    textures.reserve(MAX_TEXTURES);

    g_CmdBuf = new CCommandBuffer();
    g_CmdBuf->Init(g_pRenderBackend->GetDevice().GetGraphicsQueueIdx());

    g_ImageStagingBuffer = new CVkStagingBuffer();
    g_ImageStagingBuffer->Init(1024*1024*4);

    VkDescriptorSetLayoutBinding descSetBindings[DESC_TYPE_COUNT];
    VkDescriptorBindingFlags descSetFlags[DESC_TYPE_COUNT];
    VkDescriptorType descType[DESC_TYPE_COUNT];

    VkDescriptorPoolSize poolSizes[DESC_TYPE_COUNT] = {};

    for (int i = 0; i < DESC_TYPE_COUNT; i++)
    {
        descSetBindings[i].binding = i;
        descSetBindings[i].descriptorCount = 1000;
        descSetBindings[i].stageFlags = VK_SHADER_STAGE_ALL;

        switch (i)
        {
        case DESC_TYPE_UBO:
            descSetBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case DESC_TYPE_STORAGE_BUFFER:
            descSetBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        case DESC_TYPE_SAMPLER:
            descSetBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        }

        descSetFlags[i] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

        poolSizes[i].descriptorCount = 1000;
        poolSizes[i].type = descSetBindings[i].descriptorType;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = {};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.pBindingFlags = descSetFlags;
    bindingFlags.bindingCount = DESC_TYPE_COUNT;

    VkDescriptorSetLayoutCreateInfo setLayoutInfo = {};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = DESC_TYPE_COUNT;
    setLayoutInfo.pBindings = descSetBindings;
    setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    setLayoutInfo.pNext = &bindingFlags;
    
    VkResult result;
    if ((result = vkCreateDescriptorSetLayout(g_pRenderBackend->GetDevice().GetDeviceHandle(), &setLayoutInfo, nullptr, &g_DescLayout)) != VK_SUCCESS)
        LOG_FATAL("Failed to create descriptor set layout ({})\n", (int)result);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = DESC_TYPE_COUNT;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = MAX_FRAMES;

    if (vkCreateDescriptorPool(g_pRenderBackend->GetDevice().GetDeviceHandle(), &poolInfo, nullptr, &g_DescPool) != VK_SUCCESS)
        LOG_FATAL("Failed to allocate descriptor pool\n");

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_DescPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &g_DescLayout;

    vkAllocateDescriptorSets(g_pRenderBackend->GetDevice().GetDeviceHandle(), &allocInfo, &g_DescSet);
}

void AllocateDescriptor(TexHandle_t handle, CVkImage* pImage)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = pImage->GetSampler();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = pImage->GetView();

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = DESC_TYPE_SAMPLER;
    write.dstArrayElement = handle;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;
    write.dstSet = g_DescSet;

    vkUpdateDescriptorSets(g_pRenderBackend->GetDevice().GetDeviceHandle(), 1, &write, 0, nullptr);
}

TexHandle_t Texture::CreateTexture(const char *path)
{
    if (g_TexSlots.empty())
    {
        LOG_FATAL("Failed to allocate ID for texture {}\n", path);
    }

    TexHandle_t ret = *g_TexSlots.begin();
    g_TexSlots.erase(g_TexSlots.begin());

    // Create a new Vulkan texture
    textures[ret] = new CVkImage(path);

    AllocateDescriptor(ret, textures[ret]);

    return ret;
}

void BindDescriptor(CCommandBuffer& c)
{
    c.BindDescriptorSet(g_DescSet);
}

VkDescriptorSetLayout& GetSetLayout()
{
    return g_DescLayout;
}