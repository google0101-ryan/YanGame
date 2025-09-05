#include <core/Engine.h>
#include <tier0/assert.h>

class CEngine : public IEngine
{
public:
    virtual int Main(int iArgc, const str_t* pArgv);

    bool Init(int iArgc, const str_t* pArgv);
    void Tick();
    void Shutdown();

    virtual void AttachApp(IApplication* pApplication);
    virtual const IApplication* GetApp() const;

    virtual CCommandSystem& GetCommandSystem();
    virtual CCvarSystem& GetCvarSystem();
    virtual CEventSystem& GetEventSystem();
    virtual CWindowSystem& GetWindowSystem();
private:
    IApplication* m_pParentApp = nullptr;

    CCommandSystem m_CommandSystem;
    CCvarSystem m_CvarSystem;
    CEventSystem m_EventSystem;
    CWindowSystem m_WindowSystem;
};

CEngine g_engine;
IEngine* g_pEngine = &g_engine;

int CEngine::Main(int iArgc, const str_t* pArgv)
{
    ASSERT_MSG(m_pParentApp != nullptr, "Please attach parent application before engine init!\n");

    if (!Init(iArgc, pArgv))
    {
        return MAIN_ERROR;
    }

    m_pParentApp->Init();

    while (m_pParentApp->IsRunning())
    {
        Tick();

        m_pParentApp->Tick();
    
        m_WindowSystem.EndTick(); // Special case to flip the fb
    }

    Shutdown();
    m_pParentApp->Shutdown();

    return MAIN_OKAY;
}

bool CEngine::Init(int iArgc, const str_t* pArgv)
{
    if (!m_CommandSystem.Init())
    {
        return false;
    }

    if (!m_CvarSystem.Init())
    {
        return false;
    }

    if (!m_EventSystem.Init())
    {
        return false;
    }

    if (!m_WindowSystem.Init())
    {
        return false;
    }

    return true;
}

void CEngine::Shutdown()
{
    m_EventSystem.Shutdown();
    m_CvarSystem.Shutdown();
    m_CommandSystem.Shutdown();
}

void CEngine::Tick()
{
    m_EventSystem.Reset(); // Clear the event queue

    m_WindowSystem.Tick();
}

void CEngine::AttachApp(IApplication* pApp)
{
    m_pParentApp = pApp;
}

const IApplication* CEngine::GetApp() const
{
    return m_pParentApp;
}

CCommandSystem& CEngine::GetCommandSystem()
{
    return m_CommandSystem;
}

CCvarSystem &CEngine::GetCvarSystem()
{
    return m_CvarSystem;
}

CEventSystem& CEngine::GetEventSystem()
{
    return m_EventSystem;
}

CWindowSystem& CEngine::GetWindowSystem()
{
    return m_WindowSystem;
}