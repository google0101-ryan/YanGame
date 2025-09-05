#pragma once

#include <core/Application.h>

#include <tier1/Command.h>
#include <tier1/Cvar.h>
#include <tier1/Event.h>

#include <tier2/WindowSystem.h>

enum FailureCode_t
{
    MAIN_OKAY = 0,
    MAIN_ERROR = -1,
};

// The engine itself also implements the Init/Tick/Shutdown loop, so it inherits from IApplication
abstract_class IEngine
{
public:
    virtual int Main(int iArgc, const str_t* pArgv) = 0;

    virtual void AttachApp(IApplication* pApp) = 0;
    virtual const IApplication* GetApp() const = 0;

    virtual CCommandSystem& GetCommandSystem() = 0;
    virtual CCvarSystem& GetCvarSystem() = 0;
    virtual CEventSystem& GetEventSystem() = 0;
    virtual CWindowSystem& GetWindowSystem() = 0;
};

extern IEngine* g_pEngine;