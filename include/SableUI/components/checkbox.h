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

		void SetLabel(const SableString& p_label) { label.set(p_label); }
		void SetChecked(bool p_checked) { checked.set(p_checked); }
		void SetOnChange(std::function<void(bool)> callback) { onChangeCallback.set(callback); }
		void SetDisabled(bool p_disabled) { disabled.set(p_disabled); }
		bool IsChecked() const { return checked.get(); }

	private:
		State<SableString> label{ this, "" };
		State<bool> checked{ this, false };
		State<bool> disabled{ this, false };
		State<bool> isHovered{ this, false };
		Ref<std::function<void(bool)>> onChangeCallback{ this, nullptr };

		void HandleClick();
	};
}