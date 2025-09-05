#include <tier1/Event.h>
#include <tier1/Cvar.h>
#include <tier0/log.h>

#include <cstring>

CCvar g_eventCount("event_count", "20", "Maximum number of events per frame");

bool CEventSystem::Init()
{
    m_iEventCount = g_eventCount.GetInt();
    if (m_iEventCount <= 0)
    {
        LOG_ERROR("Specified event count is less than 1\n");
        m_iEventCount = 20;
    }

    m_iEventIndex = 0;
    m_EventQueue = new Event[m_iEventCount];

    memset(m_EventQueue, 0, sizeof(Event) * m_iEventCount);

    return true;
}

void CEventSystem::Shutdown()
{
    delete m_EventQueue;
}

void CEventSystem::Reset()
{
    m_iEventIndex = 0;
}

int CEventSystem::GetEventCount() const
{
    return m_iEventIndex;
}

const Event &CEventSystem::GetAtIndex(int iIndex) const
{
    if (iIndex >= m_iEventIndex)
    {
        LOG_WARN("Tried to access event {}, which is > the max number of events {}\n", iIndex, m_iEventCount);
        return {};
    }

    return m_EventQueue[iIndex];
}

void CEventSystem::PushEvent(Event &event)
{
    if (m_iEventIndex >= m_iEventCount)
    {
        LOG_WARN("Tried to push event into queue at max capacity!\n");
        return;
    }

    m_EventQueue[m_iEventIndex++] = event;
}
