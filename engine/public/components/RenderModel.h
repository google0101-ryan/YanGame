#pragma once

#include <tier2/GameObject.h>
#include <core/Engine.h>
#include <Geometry.h>

class CRenderModel : public IComponent
{
public:
    CRenderModel(const char* modelName);

    void Init(IGameObject* pParent) override;
    virtual void Update() override;
    void Shutdown() override {}
private:
    void MakeDefault();
    void PurgeModel();
private:
    Geometry m_Geometry;
    const char* m_name;
};