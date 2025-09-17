#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tier2/GameObject.h>
#include <core/Engine.h>

// Can be specialized into a FPS camera, or a 
class ICameraComponent : public IComponent
{
public:
    void Init(IGameObject* pParent) override;
    virtual void Update() override;
    void Shutdown() override {}

    glm::mat4& GetView() { return m_View; }
    glm::mat4& GetProjection() { return m_Projection; }

    static ICameraComponent* GetActiveCamera() { return s_pActiveCamera; }
private:
    // The camera that is actively viewing the scene
    static ICameraComponent* s_pActiveCamera;

protected:
    glm::vec3 m_Forward;
    glm::vec3 m_Up;
    glm::vec3 m_Right;

    glm::vec3 m_Pos;

    float m_Fov;

    glm::mat4 m_View;
    glm::mat4 m_Projection;

    float m_Pitch;
    float m_Yaw;
    float m_CurPitch;
    float m_CurYaw;

    IGameObject* m_pParent;
};

inline void ICameraComponent::Init(IGameObject* pParent)
{
    m_pParent = pParent;

    if (!s_pActiveCamera)
        s_pActiveCamera = this;

    m_Fov = 45.0f;

    m_Forward = glm::vec3(0.0f, 0.0f, 1.0f);
    m_Up = glm::vec3(0.0f, -1.0f, 0.0f);
    m_Right = glm::vec3(1.0f, 0.0f, 0.0f);

    m_Pitch = 0;
    m_Yaw = 90.0f;
    m_CurPitch = 0;
    m_CurYaw = 90.0f;

    m_View = glm::lookAt(pParent->GetPosition(), pParent->GetPosition()+m_Forward, m_Up);
    m_Projection = glm::perspective(glm::radians(m_Fov), (float)g_pEngine->GetWindowSystem().GetWidth() / g_pEngine->GetWindowSystem().GetHeight(), 0.1f, 1000.0f);

    m_Pos = pParent->GetPosition();
}

inline void ICameraComponent::Update()
{
    bool rotChanged = false;
    if (m_Yaw != m_CurYaw || m_Pitch != m_CurPitch)
    {
        // Recalc forward, up vectors based on rotation
        m_Forward.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Forward.y = sin(glm::radians(m_Pitch));
        m_Forward.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

        m_Forward = glm::normalize(m_Forward);
        m_Right = glm::normalize(glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), m_Forward));
        m_Up = glm::cross(m_Forward, m_Right);
    
        m_CurPitch = m_Pitch;
        m_CurYaw = m_Yaw;
        rotChanged = true;
    }

    if (m_pParent->GetPosition() != m_Pos || rotChanged)
    {
        m_View = glm::lookAt(m_pParent->GetPosition(), m_pParent->GetPosition()+m_Forward, m_Up);
        m_Projection = glm::perspective(glm::radians(m_Fov), (float)g_pEngine->GetWindowSystem().GetWidth() / g_pEngine->GetWindowSystem().GetHeight(), 0.1f, 1000.0f);
    
        m_Pos = m_pParent->GetPosition();
    }
}