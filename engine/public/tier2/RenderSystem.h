#pragma once

#include <tier0/System.h>
#include <Geometry.h>

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

// Param struct for draw command
struct DrawData_t
{
    float fTimeMs; // Time, in milliseconds, for one frame
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

    void SubmitGeometry(Geometry* pGeo);

    void Tick();
private:
    IRenderBackend* m_pRenderBackend;
};