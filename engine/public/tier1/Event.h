#pragma once

#include <tier0/types.h>
#include <tier0/System.h>

enum EventType
{
    ET_NONE = -1,
    ET_CLICK,
    ET_KEY_PRESS,
    ET_KEY_HELD,
    ET_KEY_RELEASE,
    ET_EXIT
};

struct Event
{
    EventType type;
    int iData;
    void* pData;
};

class CEventSystem : public ISystem
{
    DEFINE_CLASS(CEventSystem, ISystem);
public:
    virtual bool Init();
    virtual void Shutdown();

    void Reset();
    int GetEventCount() const;
    const Event& GetAtIndex(int iIndex) const;
    void PushEvent(Event& event);
private:
    Event* m_EventQueue;
    int m_iEventCount;
    int m_iEventIndex;
};