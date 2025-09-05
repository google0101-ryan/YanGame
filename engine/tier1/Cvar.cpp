#include <tier1/Cvar.h>
#include <tier0/log.h>

#include <core/Engine.h>

CCvarSystem* g_pCvarSystem = nullptr;
CCvar* CCvar::pCvarRoot = nullptr;

CCvar g_cheats("cheats", "0", "Enables or disables cheats", CF_ROM);

CCvar::CCvar(str_t Name, str_t Value, str_t Description, int iFlags)
{
    m_Name = Name;
    m_Value = Value;
    m_Description = Description;
    m_iFlags = iFlags;

    m_iValue = atoi(m_Value);
    m_fValue = atof(m_Value);

    if (!g_pCvarSystem)
    {
        m_pNext = pCvarRoot;
        pCvarRoot = this;
    }
    else
        g_pCvarSystem->RegisterCvar(this);
}

void CCvar::Set(str_t newValue)
{
    if (m_iFlags & CF_ROM)
    {
        LOG_WARN("Tried to write to read-only cvar \"%s\"\n", m_Name);
        return;
    }

    if ((m_iFlags & CF_CHEATS) && !g_cheats.GetBool())
    {
        LOG_WARN("Tried to write to cheats-only cvar \"%s\" with cheats disabled\n", m_Name);
        return;
    }

    m_Value = newValue;
    m_iValue = atoi(m_Value);
    m_fValue = atof(m_Value);
}

bool CCvarSystem::Init()
{
    g_pCvarSystem = this;
    m_pCvars.clear();

    for (auto pCvar = CCvar::pCvarRoot; pCvar; pCvar = pCvar->m_pNext)
    {
        m_pCvars[pCvar->m_Name] = pCvar;
    }

    g_pEngine->GetCommandSystem().RegisterCommand("set", Set);

    return true;
}

void CCvarSystem::Shutdown()
{
}

void CCvarSystem::RegisterCvar(CCvar *pCvar)
{
    m_pCvars[pCvar->m_Name] = pCvar;
}

CCvar* CCvarSystem::SetCvar(str_t Name, str_t NewValue, bool AllowCreate)
{
    auto& pCvars = g_pCvarSystem->m_pCvars;
    if (pCvars.find(Name) == pCvars.end())
    {
        if (!AllowCreate)
        {
            LOG_WARN("Tried to write to non-existant cvar \"%s\"\n", Name);
            return NULL;
        }

        CCvar* newVar = new CCvar(Name, NewValue, "");
        newVar->Set(NewValue);
        return newVar;
    }

    CCvar* pCvar = pCvars[Name];
    pCvar->Set(NewValue);
    return pCvar;
}

void CCvarSystem::Set(int iArgc, CmdArgs_t& args)
{
    if (iArgc < 3)
    {
        LOG_INFO("Usage: set <cvar> <value>\n");
        return;
    }

    SetCvar(args[0].c_str(), args[1].c_str());
}