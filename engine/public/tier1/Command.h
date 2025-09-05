#pragma once

#include <tier0/System.h>
#include <array>
#include <string>
#include <functional>
#include <unordered_map>

#define MAX_ARGS 10
typedef std::array<std::string, MAX_ARGS> CmdArgs_t;

using CmdFunc_t = std::function<void(int iArgc, CmdArgs_t& Args)>;

class CCommandSystem : public ISystem
{
    DEFINE_CLASS(CCommandSystem, ISystem);
public:
    virtual bool Init();
    virtual void Shutdown();

    virtual void RegisterCommand(str_t name, CmdFunc_t func);
private:
    std::unordered_map<std::string, CmdFunc_t> m_Commands;
};