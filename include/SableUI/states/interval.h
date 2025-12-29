#pragma once
#include <SableUI/core/component.h>
#include <chrono>

namespace SableUI
{
	using TimerHandle = size_t;

	class Interval : public StateBase
	{
	public:
		Interval(BaseComponent* owner, int milliseconds);
		~Interval();

		void Start();
		void Stop();
		void Reset();

		TimerHandle GetHandle() const { return m_handle; }

		void Sync(StateBase* other) override;

	private:
		void Register();
		void Unregister();

		std::chrono::milliseconds m_period;
		std::chrono::milliseconds m_phase{ 0 };
		bool m_active{ true };
		BaseComponent* m_owner;
		TimerHandle m_handle{ 0 };
	};
}