#pragma once
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <functional>

namespace SableUI
{
	enum class ButtonVariant
	{
		Primary,
		Secondary,
		Danger,
		Ghost
	};

	class Button : public BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

		void Init(const SableString& label, std::function<void()> callback,
			ButtonVariant variant = ButtonVariant::Primary, bool disabled = false);

		void SetLabel(const SableString& label) { setLabel(label); }
		void SetOnClick(std::function<void()> callback) { onClickCallback = callback; }
		void SetVariant(ButtonVariant variant) { setVariant(variant); }
		void SetDisabled(bool disabled) { setDisabled(disabled); }

	private:
		useState(label, setLabel, SableString, "Button");
		useState(variant, setVariant, ButtonVariant, ButtonVariant::Primary);
		useState(disabled, setDisabled, bool, false);
		useState(isHovered, setIsHovered, bool, false);
		useState(isPressed, setIsPressed, bool, false);

		useRef(onClickCallback, std::function<void()>, nullptr);

		Colour GetBackgroundColour() const;
		Colour GetTextColour() const;
	};
}