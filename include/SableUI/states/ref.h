#pragma once
#include <SableUI/states/state_base.h>
#include <SableUI/core/component.h>

namespace SableUI
{
	template<typename T>
	class Ref : public StateBase {
	public:
		Ref(BaseComponent* owner, T initialValue)
			: m_value(initialValue) {
			owner->RegisterState(this);
		}

		void Sync(StateBase* other) override {
			if (!other) return;
			auto* otherPtr = static_cast<Ref<T>*>(other);
			this->m_value = otherPtr->m_value;
		}

		T& get() { return m_value; }

		void set(const T& newValue) {
			m_value = newValue;
		}

		operator const T& () const { return m_value; }

		Ref& operator=(const T& newValue) {
			m_value = newValue;
			return *this;
		}

	private:
		T m_value;
	};
}