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

		void Init(const SableString& label, bool checked, std::function<void(bool)> onChange,
			bool disabled = false);

		void SetLabel(const SableString& label) { setLabel(label); }
		void SetChecked(bool checked) { setChecked(checked); }
		void SetOnChange(std::function<void(bool)> callback) { onChangeCallback = callback; }
		void SetDisabled(bool disabled) { setDisabled(disabled); }
		bool IsChecked() const { return checked; }

	private:
		useState(label, setLabel, SableString, "");
		useState(checked, setChecked, bool, false);
		useState(disabled, setDisabled, bool, false);
		useState(isHovered, setIsHovered, bool, false);

		useRef(onChangeCallback, std::function<void(bool)>, nullptr);

		void HandleClick();
	};
}