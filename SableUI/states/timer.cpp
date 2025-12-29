#include <SableUI/states/timer.h>
#include <SableUI/core/component.h>
#include <SableUI/core/event_scheduler.h>

SableUI::Timer::Timer(BaseComponent* owner, int milliseconds, bool autoStart)
	: m_owner(owner), m_delay(milliseconds)
{
	owner->RegisterState(this);
	if (autoStart)
		Start();
}

SableUI::Timer::~Timer()
{
	Unregister();
}

void SableUI::Timer::Start()
{
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
	Unregister();
	if (m_active)
		Register();
}

void SableUI::Timer::Sync(StateBase* other)
{
	auto* otherPtr = static_cast<Timer*>(other);
	m_delay = otherPtr->m_delay;
	m_active = otherPtr->m_active;
	m_handle = otherPtr->m_handle;
}

void SableUI::Timer::Register()
{
	if (!m_active)
		return;

	m_handle = EventScheduler::Get().AddTimer(
		m_delay,
		m_delay,
		false
	);
}

void SableUI::Timer::Unregister()
{
	if (m_handle != 0)
	{
		EventScheduler::Get().RemoveTimer(m_handle);
		m_handle = 0;
		m_active = false;
	}
}