#include <condition_variable>
#include <functional>
#include <exception>
#include <utility>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>

#include <SableUI/worker_pool.h>
#include <SableUI/SableUI.h>
#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Workers"
#include <SableUI/console.h>

static std::vector<std::thread> s_threads;
static std::queue<std::function<void()>> s_tasks;
static std::mutex s_mutex;
static std::condition_variable s_cv;
static std::atomic<bool> s_paused;
static std::atomic<bool> s_shutdown;
static bool s_initialised;

struct TimerEntry
{
	using TimePoint = std::chrono::steady_clock::time_point;

	TimePoint executeAt;
	std::function<void()> task;
	SableUI::WorkerPool::TimerHandle handle;
	float intervalSeconds;

	TimerEntry() : intervalSeconds(0.0f) {}

	TimerEntry(TimePoint at, std::function<void()> t,
		SableUI::WorkerPool::TimerHandle h, float interval)
		: executeAt(at),
		task(std::move(t)),
		handle(std::move(h)),
		intervalSeconds(interval) {};

	bool operator>(const TimerEntry& other) const
	{
		return executeAt > other.executeAt;
	}
};

static std::thread s_timerThread;
static std::priority_queue<TimerEntry, std::vector<TimerEntry>, std::greater<TimerEntry>> s_timerQueue;
static std::mutex s_timerMutex;
static std::condition_variable s_timerCV;
static std::atomic<bool> s_timerShutdown{ false };

void SableUI::WorkerPool::Initialise(int numThreads)
{
	if (s_initialised)
	{
		SableUI_Warn("WorkerPool already initialised");
		return;
	}

	s_initialised = true;
	s_shutdown = false;
	s_paused = false;
	s_timerShutdown = false;

	for (int i = 0; i < numThreads; i++)
	{
		s_threads.emplace_back(WorkerThreadFunc);
	}

	s_timerThread = std::thread(TimerThreadFunc);

	SableUI_Log("WorkerPool initialised with %d workers and 1 timer thread", numThreads);
}

void SableUI::WorkerPool::Shutdown()
{
	if (!s_initialised)
		return;

	{
		std::lock_guard<std::mutex> lock(s_timerMutex);
		s_timerShutdown = true;
	}

	s_timerCV.notify_one();
	if (s_timerThread.joinable())
		s_timerThread.join();

	s_shutdown = true;
	s_cv.notify_all();

	for (auto& thread : s_threads)
		if (thread.joinable())
			thread.join();

	s_threads.clear();
	s_initialised = false;
	SableUI_Log("WorkerPool shutdown");
}

void SableUI::WorkerPool::Submit(std::function<void()> task)
{
	if (!s_initialised)
	{
		SableUI_Error("WorkerPool not initialized");
		return;
	}

	{
		std::lock_guard<std::mutex> lock(s_mutex);
		s_tasks.push(std::move(task));
	}

	s_cv.notify_one();
}

SableUI::WorkerPool::TimerHandle SableUI::WorkerPool::SubmitInterval(float seconds, std::function<void()> task)
{
	auto handle = std::make_shared<TimerControl>();

	auto delay = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
		std::chrono::duration<float>(seconds)
	);

	auto executeAt = std::chrono::steady_clock::now() + delay;

	{
		std::lock_guard<std::mutex> lock(s_timerMutex);
		s_timerQueue.emplace(executeAt, std::move(task), handle, seconds);
	}

	s_timerCV.notify_one();
	return handle;
}

SableUI::WorkerPool::TimerHandle SableUI::WorkerPool::SubmitTimeout(float seconds, std::function<void()> task)
{
	TimerHandle handle = std::make_shared<TimerControl>();

	auto now = std::chrono::steady_clock::now();
	auto delay = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
		std::chrono::duration<float>(seconds)
	);
	auto executeAt = now + delay;

	TimerEntry entry(executeAt, std::move(task), handle, 0.0f);

	{
		std::lock_guard<std::mutex> lock(s_timerMutex);
		s_timerQueue.push(std::move(entry));
	}

	s_timerCV.notify_one();

	return handle;
}

void SableUI::WorkerPool::CancelTimer(TimerHandle handle)
{
	if (handle)
		handle->cancelled = true;
}

void SableUI::WorkerPool::PauseAll()
{
	s_paused = true;
}

void SableUI::WorkerPool::ResumeAll()
{
	s_paused = false;
}

bool SableUI::WorkerPool::IsPaused()
{
	return s_paused.load();
}

void SableUI::WorkerPool::WorkerThreadFunc()
{
	while (!s_shutdown)
	{
		std::function<void()> task;

		{
			std::unique_lock<std::mutex> lock(s_mutex);
			s_cv.wait(lock, [] { return s_shutdown || !s_tasks.empty(); });

			if (s_shutdown && s_tasks.empty())
				return;

			if (!s_tasks.empty())
			{
				task = std::move(s_tasks.front());
				s_tasks.pop();
			}
		}

		if (task)
		{
			try
			{
				task();
			}
			catch (const std::exception& e)
			{
				SableUI_Error("Worker threw exception: %s", e.what());
			}
		}
	}
}

void SableUI::WorkerPool::TimerThreadFunc()
{
	while (!s_timerShutdown)
	{
		std::unique_lock<std::mutex> lock(s_timerMutex);

		if (s_timerQueue.empty())
		{
			s_timerCV.wait(lock, [] {
				return s_timerShutdown.load() || !s_timerQueue.empty();
				});
			continue;
		}

		auto now = std::chrono::steady_clock::now();
		const auto& nextTimer = s_timerQueue.top();

		if (nextTimer.executeAt > now)
		{
			auto timeout = nextTimer.executeAt;
			lock.unlock();

			std::unique_lock<std::mutex> waitLock(s_timerMutex);
			s_timerCV.wait_until(waitLock, timeout, [] {
				return s_timerShutdown.load();
				});
			continue;
		}

		TimerEntry entry(nextTimer.executeAt,
			nextTimer.task,
			nextTimer.handle,
			nextTimer.intervalSeconds);
		s_timerQueue.pop();
		lock.unlock();

		if (entry.handle->cancelled || s_timerShutdown)
			continue;

		if (s_paused)
		{
			lock.lock();
			entry.executeAt = std::chrono::steady_clock::now() +
				std::chrono::milliseconds(100);
			s_timerQueue.push(entry);
			continue;
		}

		std::function<void()> taskCopy = entry.task;
		WorkerPool::TimerHandle handleCopy = entry.handle;

		Submit([taskCopy, handleCopy]() {
			if (!handleCopy->cancelled)
			{
				taskCopy();
				PostEmptyEvent();
			}
			});

		if (entry.intervalSeconds > 0.0f && !entry.handle->cancelled)
		{
			auto delay = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
				std::chrono::duration<float>(entry.intervalSeconds)
			);
			entry.executeAt = std::chrono::steady_clock::now() + delay;

			lock.lock();
			s_timerQueue.push(entry);
		}
	}
}