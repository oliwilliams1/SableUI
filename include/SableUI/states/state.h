#pragma once
#include <SableUI/states/state_base.h>
#include <SableUI/core/component.h>
#include <concepts>

namespace SableUI
{
	template<typename T>
	concept HasEqualityOperator = requires(const T & a, const T & b) {
		{ a == b } -> std::same_as<bool>;
	};

	template<typename T>
	class State : public StateBase {
		static_assert(HasEqualityOperator<T>, "State<T> requires T to have an operator== overloaded");

	public:
		State(BaseComponent* owner, T initialValue)
			: m_value(initialValue), m_owner(owner) {
			owner->RegisterState(this);
		}

		const T& get() const { return m_value; }

		void set(const T& newValue) {
			if (m_value == newValue) return;
			m_value = newValue;
			m_owner->MarkDirty();
		}

		void Sync(StateBase* other) override {
			if (!other) return;
			auto* otherPtr = static_cast<State<T>*>(other);
			this->m_value = otherPtr->m_value;
		}

		operator const T& () const { return m_value; }

		State& operator=(const T& newValue) {
			set(newValue);
			return *this;
		}

		BaseComponent* _owner() { return m_owner; }

	private:
		T m_value;
		BaseComponent* m_owner;
	};
}