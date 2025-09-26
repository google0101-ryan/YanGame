#include <core/Application.h>
#include <core/Engine.h>

#include <tier0/Lexer.h>

#include <tier0/log.h>

#include <tier2/GameObject.h>
#include <components/Camera.h>
#include <components/RenderModel.h>

class CFlyCamera : public ICameraComponent
{
public:
    void Update();
private:
    void UpdateInput();
    void HandleKeyEvent(Event& e);

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    bool turnLeft = false;
    bool turnRight = false;

    bool up = false;
    bool down = false;

    float speed = 5.0f;
};

void CFlyCamera::UpdateInput()
{
    if (forward)
        m_pParent->GetPosition() += m_Forward * speed * g_engineTime.m_DeltaTime;
    if (backward)
        m_pParent->GetPosition() -= m_Forward * speed * g_engineTime.m_DeltaTime;
    if (right)
        m_pParent->GetPosition() -= m_Right * speed * g_engineTime.m_DeltaTime;
    if (left)
        m_pParent->GetPosition() += m_Right * speed * g_engineTime.m_DeltaTime;
    if (turnRight)
        m_Yaw += 50.0f * g_engineTime.m_DeltaTime;
    if (turnLeft)
        m_Yaw -= 50.0f * g_engineTime.m_DeltaTime;
    if (up)
        m_pParent->GetPosition() += m_Up * speed * g_engineTime.m_DeltaTime;
    if (down)
        m_pParent->GetPosition() -= m_Up * speed * g_engineTime.m_DeltaTime;
}

void CFlyCamera::HandleKeyEvent(Event &e)
{
    bool held = e.type != ET_KEY_RELEASE;

    switch (e.iData)
    {
    case GLFW_KEY_W:
        forward = held;
        break;
    case GLFW_KEY_A:
        left = held;
        break;
    case GLFW_KEY_S:
        backward = held;
        break;
    case GLFW_KEY_D:
        right = held;
        break;
    case GLFW_KEY_LEFT:
    case GLFW_KEY_Q:
        turnLeft = held;
        break;
    case GLFW_KEY_RIGHT:
    case GLFW_KEY_E:
        turnRight = held;
        break;
    case GLFW_KEY_Z:
        down = held;
        break;
    case GLFW_KEY_X:
        up = held;
        break;
    }
}

void CFlyCamera::Update()
{
    for (int i = 0; i < g_pEngine->GetEventSystem().GetEventCount(); i++)
    {
        auto evt = g_pEngine->GetEventSystem().GetAtIndex(i);

        switch (evt.type)
        {
        case ET_KEY_PRESS:
        case ET_KEY_HELD:
        case ET_KEY_RELEASE:
            HandleKeyEvent(evt);
            break;
        }
    }

    UpdateInput();

    ICameraComponent::Update();
}

class CEditorApp : public IApplication
{
public:
    virtual bool Init();
    // All engine components have been ticked by this point
    virtual void Tick();
    virtual void Shutdown() {}

    virtual bool IsRunning() { return m_bIsRunning; }

    virtual str_t GetName() const { return "Acrylic Editor"; }
private:
    bool m_bIsRunning;

    IGameObject* m_pPlayer;
    IGameObject* m_pSpinnyCubeWeeWeeWee;
};

bool CEditorApp::Init()
{
    m_pPlayer = new IGameObject();
    m_pPlayer->GetPosition() = glm::vec3(0.0f, 0.0f, -3.0f);

    m_pPlayer->AddComponent(new CFlyCamera());

    m_bIsRunning = true;

    m_pSpinnyCubeWeeWeeWee = new IGameObject();
    m_pSpinnyCubeWeeWeeWee->AddComponent(new CRenderModel("dummy"));

    m_pSpinnyCubeWeeWeeWee->GetPosition() = glm::vec3(0, 0, 9.0f);

    return true;
}

void CEditorApp::Tick()
{
    auto& evtSys = g_pEngine->GetEventSystem();

    m_pPlayer->Tick();
    m_pSpinnyCubeWeeWeeWee->Tick();

    for (int i = 0; i < evtSys.GetEventCount(); i++)
    {
        Event e = evtSys.GetAtIndex(i);

        if (e.type == ET_EXIT)
            m_bIsRunning = false;
    }
}

int main(int argc, char** argv)
{
    g_pEngine->AttachApp(new CEditorApp());

    return g_pEngine->Main(argc, argv);
}