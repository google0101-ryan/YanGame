#pragma once

#include <tier0/System.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class CWindowSystem : public ISystem
{
public:
    virtual bool Init();
    virtual void Shutdown();

    // Called at the start of each frame to flush the event queue
    void Tick();
    // Called at the end of each frame to flip the backbuffer
    void EndTick();

    // Misc. Vulkan functions
    str_t* GetExtensions(u32* extensionCount);
    void* CreateSurface(void* instanceHandle);

    int GetWidth() const { return m_iWidth; }
    int GetHeight() const { return m_iHeight; }
private:
    GLFWwindow* m_pHandle;
    int m_iWidth, m_iHeight;
    str_t m_Title;
};