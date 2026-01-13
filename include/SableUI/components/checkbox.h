#pragma once
#include <SableUI/core/component.h>
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
			const ElementInfo& info);

		void Init(bool checked, 
			const SableString& label,
			std::function<void(bool)> onChange,
			const ElementInfo& info);

		bool IsChecked() const;

	private:
		ElementInfo info;
		State<SableString> label{ this, "" };
		State<bool> isHovered{ this, false };
		State<bool> internalChecked{ this, false };

		State<bool>* externalCheckedState = nullptr;
		Ref<std::function<void(bool)>> onChangeCallback{ this, nullptr };

		void HandleClick();
	};
}

// Checkbox with State<bool> - auto-syncing to parent
#define CheckboxState(label, checked, ...)										\
	ComponentScopedWithStyle(													\
		checkbox,																\
		SableUI::Checkbox,														\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	checkbox->Init(																\
		checked,																\
		label,																	\
		nullptr,																\
		SableUI::PackStyles(__VA_ARGS__)										\
	)

// Checkbox with callback
#define Checkbox(label, checked, onChange, ...)									\
	ComponentScopedWithStyle(													\
		checkbox,																\
		SableUI::Checkbox,														\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	checkbox->Init(																\
		checked,																\
		label,																	\
		onChange,																\
		SableUI::PackStyles(__VA_ARGS__)										\
	)
