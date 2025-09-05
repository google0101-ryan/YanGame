#include <core/Application.h>
#include <core/Engine.h>

#include <tier0/Lexer.h>

#include <tier0/log.h>

class CEditorApp : public IApplication
{
public:
    virtual bool Init() { m_bIsRunning = true; return true; }
    // All engine components have been ticked by this point
    virtual void Tick();
    virtual void Shutdown() {}

    virtual bool IsRunning() { return m_bIsRunning; }

    virtual str_t GetName() const { return "Acrylic Editor"; }
private:
    bool m_bIsRunning;
};

void CEditorApp::Tick()
{
    auto& evtSys = g_pEngine->GetEventSystem();

    for (int i = 0; i < evtSys.GetEventCount(); i++)
    {
        Event e = evtSys.GetAtIndex(i);

        if (e.type == ET_EXIT)
            m_bIsRunning = false;
    }
}

int main(int argc, char** argv)
{
    g_pEngine->AttachApp(new CEditorApp());

    return g_pEngine->Main(argc, argv);
}