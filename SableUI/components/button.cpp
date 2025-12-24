#include <SableUI/components/button.h>
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;

void Button::Layout()
{
	Colour bgColour = GetBackgroundColour();
	Colour col = GetTextColour();

	Div(bg(bgColour),
		p(6), px(12),
		rounded(4),
		w_fit, h_fit,
		centerX,
		onHoverEnter([this]() {
			if (!disabled.get()) isHovered.set(true);
		}),
		onHoverLeave([this]() {
			isHovered.set(false);
			isPressed.set(false);
		}),
		onClick([this]() {
			if (!disabled.get() && onClickCallback.get()) {
				onClickCallback.get()();
			}
		}))
	{
		Text(label.get(),
			textColour(col),
			justify_center,
			wrapText(false));
	}
}

void Button::OnUpdate(const UIEventContext& ctx)
{
	if (!disabled.get() && isHovered.get())
	{
		if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
		{
			isPressed.set(true);
		}
		else if (ctx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
		{
			isPressed.set(false);
		}
	}
	else
	{
		if (isPressed.get())
			isPressed.set(false);
	}
}

void SableUI::Button::Init(const SableString& p_label, std::function<void()> p_callback, ButtonVariant p_variant, bool p_disabled)
{
	label.set(p_label);
	onClickCallback.set(p_callback);
	variant.set(p_variant);
	disabled.set(p_disabled);
}

Colour Button::GetBackgroundColour() const
{
	if (disabled.get())
	{
		switch (variant.get())
		{
		case ButtonVariant::Primary:    return Colour(120, 120, 120);
		case ButtonVariant::Secondary:  return Colour(100, 100, 100);
		case ButtonVariant::Danger:     return Colour(150, 100, 100);
		case ButtonVariant::Ghost:      return Colour(80, 80, 80, 0);
		}
	}

	if (isPressed.get())
	{
		switch (variant.get())
		{
		case ButtonVariant::Primary:    return Colour(70, 130, 220);
		case ButtonVariant::Secondary:  return Colour(60, 60, 60);
		case ButtonVariant::Danger:     return Colour(200, 80, 80);
		case ButtonVariant::Ghost:      return Colour(255, 255, 255, 20);
		}
	}

	if (isHovered.get())
	{
		switch (variant.get())
		{
		case ButtonVariant::Primary:    return Colour(80, 150, 240);
		case ButtonVariant::Secondary:  return Colour(70, 70, 70);
		case ButtonVariant::Danger:     return Colour(230, 100, 100);
		case ButtonVariant::Ghost:      return Colour(255, 255, 255, 15);
		}
	}

	switch (variant.get())
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
	if (disabled.get())
	{
		return Colour(150, 150, 150);
	}

	switch (variant.get())
	{
	case ButtonVariant::Primary:    return Colour(255, 255, 255);
	case ButtonVariant::Secondary:  return Colour(200, 200, 200);
	case ButtonVariant::Danger:     return Colour(255, 255, 255);
	case ButtonVariant::Ghost:      return Colour(200, 200, 200);
	default:                        return Colour(255, 255, 255);
	}
}