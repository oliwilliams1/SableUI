#pragma once
#include <SableUI/core/component.h>
#include <chrono>

namespace SableUI
{
	using TimerHandle = size_t;

    class Interval : public StateBase
    {
    public:
        Interval(BaseComponent* owner);
        ~Interval();

        TimerHandle GetHandle() const { return m_handle; }
        void Start(int milliseconds);
        void Stop();
        void Reset();

        void Sync(StateBase* other) override;

    private:
        void Register();
        void Unregister();

        std::chrono::milliseconds m_phase{ 0 };
        std::chrono::milliseconds m_period{ 0 };
        bool m_active = false;
        BaseComponent* m_owner = nullptr;
        TimerHandle m_handle = 0;
    };
}