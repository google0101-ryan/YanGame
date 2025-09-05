#include <core/Application.h>
#include <core/Engine.h>

#include <tier0/Lexer.h>

#include <tier0/log.h>

class CEditorApp : public IApplication
{
public:
    virtual bool Init() { return true; }
    virtual void Tick() {}
    virtual void Shutdown() {}

    virtual bool IsRunning() { return false; }
};

int main(int argc, char** argv)
{
    g_pEngine->AttachApp(new CEditorApp());

    return g_pEngine->Main(argc, argv);
}