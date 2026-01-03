#include <SableUI/states/interval.h>
#include <SableUI/core/component.h>
#include <SableUI/core/event_scheduler.h>

SableUI::Interval::Interval(BaseComponent* owner)
    : m_owner(owner), m_period(0)
{
    owner->RegisterState(this);
}

SableUI::Interval::~Interval()
{
    Unregister();
}

void SableUI::Interval::Start(int milliseconds)
{
    m_period = std::chrono::milliseconds(milliseconds);
    if (m_active) return;

    m_active = true;
    Register();
}

void SableUI::Interval::Stop()
{
    if (!m_active) return;
    m_active = false;
    Unregister();
}

void SableUI::Interval::Reset()
{
    m_phase = std::chrono::milliseconds(0);
    if (m_handle != 0)
    {
        EventScheduler::Get().UpdateTimer(m_handle, m_period);
    }
    else if (m_active)
    {
        Register();
    }
}

void SableUI::Interval::Sync(StateBase* other)
{
    auto* otherPtr = static_cast<Interval*>(other);

    auto oldPeriod = m_period;
    bool wasActive = m_active;

    m_period = otherPtr->m_period;
    m_phase = otherPtr->m_phase;
    m_active = otherPtr->m_active;

    if (m_active && m_handle == 0)
    {
        Register();
    }
    else if (!m_active && m_handle != 0)
    {
        Unregister();
    }
    else if (m_active && m_handle != 0 && (m_period != oldPeriod))
    {
        EventScheduler::Get().UpdateTimer(m_handle, m_period);
    }
}

void SableUI::Interval::Register()
{
    if (m_period.count() <= 0) return;
    m_handle = EventScheduler::Get().AddTimer(m_period, m_period, true);
}

void SableUI::Interval::Unregister()
{
    if (m_handle != 0)
    {
        EventScheduler::Get().RemoveTimer(m_handle);
        m_handle = 0;
    }
}