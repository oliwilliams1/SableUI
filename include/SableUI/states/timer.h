#pragma once
#include <SableUI/core/component.h>
#include <chrono>

namespace SableUI
{
	using TimerHandle = size_t;

	class Timer : public StateBase
	{
	public:
		Timer(BaseComponent* owner, int milliseconds, bool autoStart = true);
		~Timer();

		void Start();
		void Stop();
		void Reset();

		TimerHandle GetHandle() const { return m_handle; }

		void Sync(StateBase* other) override;

	private:
		void Register();
		void Unregister();

		std::chrono::milliseconds m_delay;
		bool m_active{ false };
		BaseComponent* m_owner;
		TimerHandle m_handle{ 0 };
	};
}