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