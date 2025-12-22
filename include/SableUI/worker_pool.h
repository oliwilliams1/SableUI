#pragma once
#include <functional>
#include <atomic>
#include <memory>

namespace SableUI
{
	class WorkerPool
	{
	public:
		struct TimerControl
		{
			std::atomic<bool> cancelled = false;
		};

		using TimerHandle = std::shared_ptr<TimerControl>;

		static void Initialise(int numThreads = 4);
		static void Shutdown();

		static void Submit(std::function<void()> func);
		static TimerHandle SubmitInterval(float seconds, std::function<void()> task);
		static TimerHandle SubmitTimeout(float seconds, std::function<void()> task);
		static void CancelTimer(TimerHandle handle);
		static void PauseAll();
		static void ResumeAll();
		static bool IsPaused();

	private:
		static void WorkerThreadFunc();
		static void TimerThreadFunc();
	};
}