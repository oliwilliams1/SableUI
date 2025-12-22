#pragma once
#include <SableUI/component.h>
#include <SableUI/worker_pool.h>
#include <functional>

namespace SableUI
{
	class Interval : public StateBase
	{
	public:
		Interval(BaseComponent* owner, float interval, std::function<void()> task);
		~Interval();

		void Start();
		void Stop();
		bool IsRunning() const { return m_handle != nullptr; }
		void Sync(StateBase* other) override {};

	private:
		BaseComponent* m_owner;
		float m_interval;
		std::function<void()> m_task;
		WorkerPool::TimerHandle m_handle;
	};

	class Timeout : public StateBase
	{
	public:
		Timeout(BaseComponent* owner);
		~Timeout();

		void Schedule(float delay, std::function<void()> task);
		void Cancel();
		bool IsPending() const { return m_handle != nullptr; }
		void Sync(StateBase* other) override {};

	private:
		BaseComponent* m_owner;
		WorkerPool::TimerHandle m_handle;
	};
}