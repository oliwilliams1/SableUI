#include <SableUI/states/interval.h>
#include <SableUI/core/component.h>
#include <SableUI/core/event_scheduler.h>

SableUI::Interval::Interval(BaseComponent* owner, int milliseconds)
	: m_owner(owner), m_period(milliseconds)
{
	owner->RegisterState(this);
	Register();
}

SableUI::Interval::~Interval()
{
	Unregister();
}

void SableUI::Interval::Start()
{
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
	Unregister();
	if (m_active)
		Register();
}

void SableUI::Interval::Sync(StateBase* other)
{
	auto* otherPtr = static_cast<Interval*>(other);
	m_period = otherPtr->m_period;
	m_phase = otherPtr->m_phase;
	m_active = otherPtr->m_active;
	m_handle = otherPtr->m_handle;
}

void SableUI::Interval::Register()
{
	if (!m_active)
		return;

	m_handle = EventScheduler::Get().AddTimer(
		m_period - m_phase,
		m_period,
		true
	);
}

void SableUI::Interval::Unregister()
{
	if (m_handle != 0)
	{
		EventScheduler::Get().RemoveTimer(m_handle);
		m_handle = 0;
	}
}