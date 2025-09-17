#pragma once

#include <tier0/types.h>
#include <tier0/assert.h>
#include <array>

#include <glm/glm.hpp>

class IGameObject;

class IComponent
{
public:
    virtual void Init(IGameObject* pParent) = 0;
    virtual void Update() = 0;
    virtual void Shutdown() = 0;
};

static constexpr u32 MAX_COMPONENT = 10;

class IGameObject
{
public:
    IGameObject()
    {
        m_Pos = glm::vec3(0.0f);
        m_Rotation = glm::vec3(0.0f);
        m_Scale = glm::vec3(1.0f);
    }

    virtual ~IGameObject()
    {
        for (auto pComponent : m_pComponents)
            pComponent->Shutdown();
    }

    virtual void Update() {}

    void Tick()
    {
        Update();

        for (int i = 0; i < m_iComponentCount; i++)
        {
            m_pComponents[i]->Update();
        }
    }

    void AddComponent(IComponent* pComponent)
    {
        ASSERT_MSG(m_iComponentCount < MAX_COMPONENT, "Tried to add component to full go\n");

        m_pComponents[m_iComponentCount++] = pComponent;
        pComponent->Init(this);
    }

    glm::vec3& GetPosition()
    {
        return m_Pos;
    }
private:
    std::array<IComponent*, MAX_COMPONENT> m_pComponents;
    int m_iComponentCount;

    // All gameobjects exist in the world somewhere, probably
    glm::vec3 m_Pos;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
};