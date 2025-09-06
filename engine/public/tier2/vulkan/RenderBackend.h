#pragma once

#include <tier2/RenderSystem.h>
#include <tier2/vulkan/Instance.h>
#include <tier2/vulkan/Device.h>
#include <tier2/vulkan/SwapChain.h>
#include <tier2/vulkan/RenderPass.h>

class CVulkanBackend : public IRenderBackend
{
public:
    virtual bool Init();
    virtual void Shutdown();

    virtual void SubmitCommand(RBCommand_t& cmd);

    const CVkInstance& GetInstance() const { return m_Instance; }
    const VkSurfaceKHR& GetSurface() const { return m_Surface; }
    const CVkDevice& GetDevice() const { return m_Device; }
private:
    CVkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    CVkDevice m_Device;
    CVkSwapChain m_SwapChain;

    CRenderPass m_MainRenderPass;
};

extern CVulkanBackend* g_pRenderBackend;