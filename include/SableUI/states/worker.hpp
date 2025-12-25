#pragma once
#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <SableUI/worker_pool.h>
#include <thread>

#include <SableUI/console.h>
#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Worker-State"

#include <mutex>

namespace SableUI
{
	enum class WorkerStatus
	{
		Idle,
		Running,
		Completed,
		Failed
	};

	template<typename T>
	class WorkerState : public StateBase
	{
		struct SharedState
		{
			std::atomic<WorkerStatus> status = WorkerStatus::Idle;
			T result{};
			std::string error;
			std::mutex mutex;
			std::atomic<bool> cancelled = false;
		};

	public:
		WorkerState(BaseComponent* owner)
			: m_owner(owner), m_state(std::make_shared<SharedState>())
		{
			owner->RegisterState(this);
		}

		~WorkerState()
		{
			Cancel();
		}

		WorkerState& SetTask(std::function<T()> task)
		{
			if (GetStatus() == WorkerStatus::Running)
			{
				SableUI_Warn("Cannot change task while worker is running");
				return *this;
			}
			m_task = task;
			return *this;
		}

		void Start()
		{
			if (!m_task)
			{
				SableUI_Error("Cannot start worker without task");
				return;
			}

			// check if already running or completed
			WorkerStatus expected = WorkerStatus::Idle;
			if (!m_state->status.compare_exchange_strong(expected, WorkerStatus::Running))
				return;

			m_state->cancelled = false;
			m_state->error = "";

			auto state = m_state;
			auto task = m_task;
			auto owner = m_owner;

			WorkerPool::Submit([state, task, owner]()
			{
				try
				{
					while (WorkerPool::IsPaused() && !state->cancelled)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(25));
					}

					if (state->cancelled)
					{
						state->status = WorkerStatus::Idle;
						return;
					}

					T result = task();

					{
						std::lock_guard<std::mutex> lock(state->mutex);
						if (!state->cancelled)
						{
							state->result = std::move(result);
							state->status = WorkerStatus::Completed;
							owner->needsRerender = true;
						}
					}

					PostEmptyEvent();
				}
				catch (const std::exception& e)
				{
					std::lock_guard<std::mutex> lock(state->mutex);
					state->error = e.what();
					state->status = WorkerStatus::Failed;
					SableUI_Error("Worker throwed an exception: %s", e.what());
					PostEmptyEvent();
				}
			});
		}

		void Cancel()
		{
			m_state->cancelled = true;
		}

		WorkerStatus GetStatus() const
		{
			return m_state->status.load();
		}

		bool IsRunning() const
		{
			return GetStatus() == WorkerStatus::Running;
		}

		bool IsCompleted() const
		{
			return GetStatus() == WorkerStatus::Completed;
		}

		bool IsFailed() const
		{
			return GetStatus() == WorkerStatus::Failed;
		}

		const T& GetResult()
		{
			std::lock_guard<std::mutex> lock(m_state->mutex);
			if (m_state->status != WorkerStatus::Completed)
				SableUI_Runtime_Error("Getting result when worker is not complete");

			return m_state->result;
		}

		std::string GetError() const
		{
			std::lock_guard<std::mutex> lock(m_state->mutex);
			return m_state->error;
		}

		void Reset()
		{
			Cancel();
			std::lock_guard<std::mutex> lock(m_state->mutex);
			m_state->status = WorkerStatus::Idle;
			m_state->error.clear();
		}

		void Sync(StateBase* other) override
		{
			auto* otherWorker = static_cast<WorkerState<T>*>(other);

			if (otherWorker->m_state->status == WorkerStatus::Completed)
			{
				std::lock_guard<std::mutex> lock1(m_state->mutex);
				std::lock_guard<std::mutex> lock2(otherWorker->m_state->mutex);

				m_state->result = otherWorker->m_state->result;
				m_state->status = WorkerStatus::Completed;
			}
		}

	private:
		BaseComponent* m_owner;
		std::shared_ptr<SharedState> m_state;
		std::function<T()> m_task;
	};
}