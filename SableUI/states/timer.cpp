#include <SableUI/states/timer.h>
#include <SableUI/worker_pool.h>
#include <SableUI/component.h>
#include <functional>

SableUI::Interval::Interval(BaseComponent* owner, float interval, std::function<void()> task)
	: m_owner(owner), m_interval(interval), m_task(task)
{
	owner->RegisterState(this);
}

SableUI::Interval::~Interval()
{
	Stop();
}

void SableUI::Interval::Start()
{
	if (m_handle) return;

	m_handle = WorkerPool::SubmitInterval(m_interval, [this]() {
		m_task();
		m_owner->needsRerender = true;
	});
}

void SableUI::Interval::Stop()
{
	if (m_handle)
	{
		WorkerPool::CancelTimer(m_handle);
		m_handle = nullptr;
	}
}

SableUI::Timeout::Timeout(BaseComponent* owner)
	: m_owner(owner)
{
	owner->RegisterState(this);
}

SableUI::Timeout::~Timeout()
{
	Cancel();
}

void SableUI::Timeout::Schedule(float delay, std::function<void()> task)
{
	Cancel();

	m_handle = WorkerPool::SubmitTimeout(delay, [this, task] {
		task();
		m_owner->needsRerender = true;
		m_handle = nullptr;
	});
}

void SableUI::Timeout::Cancel()
{
	if (m_handle)
	{
		WorkerPool::CancelTimer(m_handle);
		m_handle = nullptr;
	}
}