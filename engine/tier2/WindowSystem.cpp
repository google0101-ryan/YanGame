#include <tier2/WindowSystem.h>

#include <tier1/Event.h>
#include <tier1/Cvar.h>

#include <tier0/log.h>

#include <core/Engine.h>

CCvar g_windowWidth("r_width", "1920", "The width of the render target", 0);
CCvar g_windowHeight("r_height", "1080", "The height of the render target", 0);

bool CWindowSystem::Init()
{
    if (glfwInit() != GLFW_TRUE)
    {
        str_t err;
        glfwGetError(&err);
        LOG_FATAL("Failed to initialize GLFW3 context: {}\n", err);
        return false;
    }

    m_pHandle = glfwCreateWindow(g_windowWidth.GetInt(), g_windowHeight.GetInt(), g_pEngine->GetApp()->GetName(), NULL, NULL);

    if (!m_pHandle)
    {
        str_t err;
        glfwGetError(&err);
        LOG_FATAL("Failed to create GLFW3 window: {}\n", err);
        return false;
    }

    m_iWidth = g_windowWidth.GetInt();
    m_iHeight = g_windowHeight.GetInt();

    glfwMakeContextCurrent(m_pHandle);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
}

void CWindowSystem::Shutdown()
{
    glfwDestroyWindow(m_pHandle);
    glfwTerminate();
}

void CWindowSystem::Tick()
{
    glfwPollEvents();

    // Manually add the exit event

    if (glfwWindowShouldClose(m_pHandle))
    {
        Event evt;
        evt.type = ET_EXIT;
        evt.iData = 0; // exit code

        g_pEngine->GetEventSystem().PushEvent(evt);
    }
}

void CWindowSystem::EndTick()
{
    glfwSwapBuffers(m_pHandle);
}