#include <SableUI/core/event_scheduler.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils/console.h>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

SableUI::EventScheduler::EventScheduler()
{
	m_thread = std::thread(&EventScheduler::ThreadMain, this);
}

SableUI::EventScheduler::~EventScheduler()
{
	Shutdown();
}

void SableUI::EventScheduler::Shutdown()
{
	m_running.store(false);
	m_cv.notify_all();
	if (m_thread.joinable())
		m_thread.join();
}

SableUI::TimerHandle SableUI::EventScheduler::AddTimer(
	std::chrono::milliseconds fireIn,
	std::chrono::milliseconds period,
	bool repeating)
{
	SableUI_Log("Timer added");
	if (period.count() == 0)
		SableUI_Runtime_Error("Timer period must be > 0ms");

	using clock = std::chrono::steady_clock;
	std::lock_guard lock(m_mutex);

	TimerHandle handle = m_nextHandle++;
	m_timers.emplace(handle, Timer{
		clock::now() + fireIn,
		period,
		repeating
	});

	m_cv.notify_one();
	return handle;
}

void SableUI::EventScheduler::RemoveTimer(TimerHandle handle)
{
	SableUI_Log("Timer removed");
	std::lock_guard lock(m_mutex);
	m_timers.erase(handle);
	m_cv.notify_one();
}
void SableUI::EventScheduler::UpdateTimer(TimerHandle handle, std::chrono::milliseconds fireIn)
{
	using clock = std::chrono::steady_clock;
	std::lock_guard lock(m_mutex);

	auto it = m_timers.find(handle);
	if (it != m_timers.end())
	{
		it->second.nextFire = clock::now() + fireIn;
		m_cv.notify_one();
	}
}

std::vector<SableUI::TimerHandle> SableUI::EventScheduler::PollFiredTimers()
{
	std::lock_guard lock(m_firedMutex);
	std::vector<TimerHandle> result = std::move(m_firedTimers);
	m_firedTimers.clear();
	return result;
}

void SableUI::EventScheduler::ThreadMain()
{
	using clock = std::chrono::steady_clock;

	while (m_running.load())
	{
		std::unique_lock lock(m_mutex);

		if (m_timers.empty())
		{
			m_cv.wait(lock);
			continue;
		}

		auto nextIt = std::min_element(
			m_timers.begin(), m_timers.end(),
			[](auto& a, auto& b) {
				return a.second.nextFire < b.second.nextFire;
			}
		);

		auto now = clock::now();
		if (nextIt->second.nextFire > now)
		{
			m_cv.wait_until(lock, nextIt->second.nextFire);
			continue;
		}

		TimerHandle handle = nextIt->first;
		auto& timer = nextIt->second;
		bool shouldRemove = !timer.repeating;

		{
			std::lock_guard firedLock(m_firedMutex);
			m_firedTimers.push_back(handle);
		}

		PostEmptyEvent();

		if (timer.repeating)
		{
			timer.nextFire += timer.period;

			if (timer.nextFire < now - timer.period)
			{
				timer.nextFire = now + timer.period;
			}
		}

		if (shouldRemove)
		{
			m_timers.erase(nextIt);
		}
	}
}