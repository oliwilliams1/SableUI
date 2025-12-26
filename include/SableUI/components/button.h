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

		void SetLabel(const SableString& p_label) { label.set(p_label); }
		void SetOnClick(std::function<void()> callback) { onClickCallback = callback; }
		void SetVariant(ButtonVariant p_variant) { variant.set(p_variant); }
		void SetDisabled(bool p_disabled) { disabled.set(p_disabled); }

	private:
		State<SableString> label{ this, "Label" };
		State<ButtonVariant> variant{ this, ButtonVariant::Primary };
		State<bool> disabled{ this, false };
		State<bool> isHovered{ this, false };
		State<bool> isPressed{ this, false };
		Ref<std::function<void()>> onClickCallback{ this, nullptr };

		Colour GetBackgroundColour() const;
		Colour GetHoverColour() const;
		Colour GetTextColour() const;
	};
}