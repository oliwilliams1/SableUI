#include <SableUI/states/timer.h>
#include <SableUI/core/component.h>
#include <SableUI/core/event_scheduler.h>

SableUI::Timer::Timer(BaseComponent* owner)
    : m_owner(owner), m_delay(0)
{
    owner->RegisterState(this);
}

SableUI::Timer::~Timer()
{
    Unregister();
}

void SableUI::Timer::Start(int milliseconds)
{
    m_delay = std::chrono::milliseconds(milliseconds);
    if (m_active) return;
    m_active = true;
    Register();
}

void SableUI::Timer::Stop()
{
    if (!m_active) return;
    m_active = false;
    Unregister();
}

void SableUI::Timer::Reset()
{
    if (m_handle != 0)
    {
        EventScheduler::GetInstance().UpdateTimer(m_handle, m_delay);
    }
    else if (m_active)
    {
        Register();
    }
}

void SableUI::Timer::Sync(StateBase* other)
{
    auto* otherPtr = static_cast<Timer*>(other);
    m_delay = otherPtr->m_delay;
    m_active = otherPtr->m_active;

    if (m_active && m_handle == 0)
    {
        Register();
    }
    else if (!m_active && m_handle != 0)
    {
        Unregister();
    }
}

void SableUI::Timer::Register()
{
    if (m_delay.count() <= 0) return;
    m_handle = EventScheduler::GetInstance().AddTimer(m_delay, m_delay, false);
}

void SableUI::Timer::Unregister()
{
    if (m_handle != 0)
    {
        EventScheduler::GetInstance().RemoveTimer(m_handle);
        m_handle = 0;
    }
}