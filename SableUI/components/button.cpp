#include <SableUI/components/button.h>
#include <SableUI/SableUI.h>

using namespace SableUI;

void Button::Layout()
{
	Colour bgColour = GetBackgroundColour();
	Colour textColour = GetTextColour();

	Div(bg(bgColour)
		p(12) px(20)
		rounded(6)
		w_fit h_fit
		centerX
		onHover([this]() {
			if (!disabled) setIsHovered(true);
		})
		onHoverExit([this]() {
			setIsHovered(false);
			setIsPressed(false);
		})
		onClick([this]() {
			if (!disabled && onClickCallback) {
				onClickCallback();
			}
		}))
	{
		Text(label,
			textColour(textColour)
			fontSize(14)
			justify_center
			wrapText(false));
	}
}

void Button::OnUpdate(const UIEventContext& ctx)
{
	if (!disabled && isHovered)
	{
		if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
		{
			setIsPressed(true);
		}
		else if (ctx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
		{
			setIsPressed(false);
		}
	}
	else
	{
		if (isPressed)
			setIsPressed(false);
	}
}

void SableUI::Button::Init(const SableString& label, std::function<void()> callback, ButtonVariant variant, bool disabled)
{
	setLabel(label);
	onClickCallback = callback;
	setVariant(variant);
	setDisabled(disabled);
}

Colour Button::GetBackgroundColour() const
{
	if (disabled)
	{
		switch (variant)
		{
		case ButtonVariant::Primary:    return Colour(120, 120, 120);
		case ButtonVariant::Secondary:  return Colour(100, 100, 100);
		case ButtonVariant::Danger:     return Colour(150, 100, 100);
		case ButtonVariant::Ghost:      return Colour(80, 80, 80, 0);
		}
	}

	if (isPressed)
	{
		switch (variant)
		{
		case ButtonVariant::Primary:    return Colour(70, 130, 220);
		case ButtonVariant::Secondary:  return Colour(60, 60, 60);
		case ButtonVariant::Danger:     return Colour(200, 80, 80);
		case ButtonVariant::Ghost:      return Colour(255, 255, 255, 20);
		}
	}

	if (isHovered)
	{
		switch (variant)
		{
		case ButtonVariant::Primary:    return Colour(80, 150, 240);
		case ButtonVariant::Secondary:  return Colour(70, 70, 70);
		case ButtonVariant::Danger:     return Colour(230, 100, 100);
		case ButtonVariant::Ghost:      return Colour(255, 255, 255, 15);
		}
	}

	switch (variant)
	{
	case ButtonVariant::Primary:    return Colour(90, 160, 255);
	case ButtonVariant::Secondary:  return Colour(80, 80, 80);
	case ButtonVariant::Danger:     return Colour(255, 120, 120);
	case ButtonVariant::Ghost:      return Colour(0, 0, 0, 0);
	default:                        return Colour(90, 160, 255);
	}
}

Colour Button::GetTextColour() const
{
	if (disabled)
	{
		return Colour(150, 150, 150);
	}

	switch (variant)
	{
	case ButtonVariant::Primary:    return Colour(255, 255, 255);
	case ButtonVariant::Secondary:  return Colour(200, 200, 200);
	case ButtonVariant::Danger:     return Colour(255, 255, 255);
	case ButtonVariant::Ghost:      return Colour(200, 200, 200);
	default:                        return Colour(255, 255, 255);
	}
}