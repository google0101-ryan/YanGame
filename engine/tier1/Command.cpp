#include <tier1/Command.h>

#include <tier0/log.h>

bool CCommandSystem::Init()
{
    m_Commands.clear();
    m_Commands.reserve(20); // Stave off allocations for a bit

    return true;
}

void CCommandSystem::Shutdown()
{
}

void CCommandSystem::RegisterCommand(str_t name, CmdFunc_t func)
{
    if (m_Commands.find(name) != m_Commands.end())
        LOG_WARN("Command \"{}\" already registered!\n", name);

    m_Commands[name] = func;
}