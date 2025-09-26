#include <tier2/RenderSystem.h>

extern IRenderBackend* CreateBackend();

bool CRenderSystem::Init()
{
    m_pRenderBackend = CreateBackend();
    
    return m_pRenderBackend->Init();
}

void CRenderSystem::Shutdown()
{
    m_pRenderBackend->Shutdown();
}

void CRenderSystem::SubmitGeometry(Geometry *pGeo)
{
    RBCommand_t cmd;
    cmd.type = RB_SUBMIT_GEO;
    cmd.pData = pGeo;

    m_pRenderBackend->SubmitCommand(cmd);
}
void CRenderSystem::Tick()
{
    RBCommand_t cmd;
    cmd.type = RB_DRAW;

    m_pRenderBackend->SubmitCommand(cmd);
}