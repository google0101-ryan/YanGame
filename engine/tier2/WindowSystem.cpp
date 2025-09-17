#include <tier2/WindowSystem.h>

#include <tier1/Event.h>
#include <tier1/Cvar.h>

#include <tier0/log.h>

#include <core/Engine.h>

CCvar g_windowWidth("r_width", "1920", "The width of the render target", 0);
CCvar g_windowHeight("r_height", "1080", "The height of the render target", 0);

static void HandleGlfwKeyPress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Event evt;

    switch (action)
    {
    case GLFW_PRESS:
        evt.type = ET_KEY_PRESS;
        break;
    case GLFW_RELEASE:
        evt.type = ET_KEY_RELEASE;
        break;
    case GLFW_REPEAT:
        evt.type = ET_KEY_HELD;
        break;
    }

    evt.iData = key;
    g_pEngine->GetEventSystem().PushEvent(evt);
}

bool CWindowSystem::Init()
{
    if (glfwInit() != GLFW_TRUE)
    {
        str_t err;
        glfwGetError(&err);
        LOG_FATAL("Failed to initialize GLFW3 context: {}\n", err);
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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

    glfwSetKeyCallback(m_pHandle, HandleGlfwKeyPress);

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

str_t *CWindowSystem::GetExtensions(u32 *extensionCount)
{
    return glfwGetRequiredInstanceExtensions(extensionCount);
}

void *CWindowSystem::CreateSurface(void* instanceHandle)
{
    VkSurfaceKHR surfaceHandle;
    if (glfwCreateWindowSurface((VkInstance)instanceHandle, m_pHandle, nullptr, &surfaceHandle) != VK_SUCCESS)
        LOG_ERROR("Failed to create window surface!\n");
    return surfaceHandle;
}