#pragma once

#include <tier0/System.h>
#include <tier1/Command.h>
#include <unordered_map>
#include <string>

enum CvarFlags
{
    CF_ROM = (1<<0),
    CF_CHEATS = (1<<1),
    CF_ARCHIVE = (1<<2)
};

class CCvar
{
    friend class CCvarSystem;
public:
    CCvar(str_t Name, str_t Value, str_t Description, int iFlags = 0);

    void Set(str_t newValue);

    bool GetBool() const { return (bool)m_iValue; }
    int GetInt() const { return m_iValue; }
    f64 GetFloat() const { return m_fValue; }
    str_t GetValue() const { return m_Value; }
private:
    static CCvar* pCvarRoot;
    CCvar* m_pNext;

    int m_iFlags;
    str_t m_Description;
    str_t m_Name;

    str_t m_Value;
    int m_iValue;
    f64 m_fValue;
};

class CCvarSystem : ISystem
{
    DEFINE_CLASS(CCvarSystem, ISystem);
public:
    virtual bool Init();
    virtual void Shutdown();

    void RegisterCvar(CCvar* pCvar);

    static CCvar* SetCvar(str_t Name, str_t NewValue, bool AllowCreate = true);
    static void Set(int iArgc, CmdArgs_t& Args);
private:
    std::unordered_map<std::string, CCvar*> m_pCvars;
};