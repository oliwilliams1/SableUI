#pragma once
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <functional>

namespace SableUI
{
	class Checkbox : public BaseComponent
	{
	public:
		void Layout() override;

		void Init(State<bool>& checkedState, 
			const SableString& label,
			const ElementInfo& info, 
			bool disabled = false);

		void Init(bool checked, 
			const SableString& label,
			std::function<void(bool)> onChange,
			const ElementInfo& info,
			bool disabled = false);

		bool IsChecked() const;

	private:
		ElementInfo info;
		State<SableString> label{ this, "" };
		State<bool> disabled{ this, false };
		State<bool> isHovered{ this, false };
		State<bool> internalChecked{ this, false };

		State<bool>* externalCheckedState = nullptr;
		Ref<std::function<void(bool)>> onChangeCallback{ this, nullptr };

		void HandleClick();
	};
}