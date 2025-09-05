#pragma once

#include <tier0/System.h>

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
private:
    GLFWwindow* m_pHandle;
    int m_iWidth, m_iHeight;
    str_t m_Title;
};