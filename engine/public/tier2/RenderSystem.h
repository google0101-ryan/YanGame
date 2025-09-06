#pragma once

#include <tier0/System.h>

enum RBCommands_t
{
    RB_SUBMIT_GEO,
    RB_DRAW
};

struct RBCommand_t
{
    RBCommands_t type;
    void* pData;
};

abstract_class IRenderBackend : public ISystem
{
public:
    virtual void SubmitCommand(RBCommand_t& cmd) = 0;
};

class CRenderSystem : public ISystem
{
public:
    virtual bool Init();
    virtual void Shutdown();
private:
    IRenderBackend* m_pRenderBackend;
};