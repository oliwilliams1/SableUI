#pragma once
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <unordered_map>
#include <vector>

namespace SableUI
{
	using TimerHandle = size_t;

	class EventScheduler
	{
	public:
		static EventScheduler& Get()
		{
			static EventScheduler instance;
			return instance;
		}

		EventScheduler(const EventScheduler&) = delete;
		EventScheduler& operator=(const EventScheduler&) = delete;

		TimerHandle AddTimer(
			std::chrono::milliseconds fireIn,
			std::chrono::milliseconds period,
			bool repeating = true);

		void RemoveTimer(TimerHandle handle);
		void UpdateTimer(TimerHandle handle, std::chrono::milliseconds fireIn);
		std::vector<TimerHandle> PollFiredTimers();
		void Shutdown();

	private:
		EventScheduler();
		~EventScheduler();
		void ThreadMain();

		std::thread m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cv;

		struct Timer
		{
			std::chrono::steady_clock::time_point nextFire;
			std::chrono::milliseconds period;
			bool repeating;
		};

		std::unordered_map<TimerHandle, Timer> m_timers;

		std::mutex m_firedMutex;
		std::vector<TimerHandle> m_firedTimers;

		std::atomic<bool> m_running{ true };
		TimerHandle m_nextHandle{ 1 };
	};
}