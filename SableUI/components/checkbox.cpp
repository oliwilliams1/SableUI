#include <SableUI/components/checkbox.h>
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;

void Checkbox::Layout()
{
	Colour boxBorder = disabled.get() ? Colour(100, 100, 100) :
		isHovered ? Colour(120, 180, 255) :
		Colour(100, 100, 100);

	Colour boxFill = checked ? (disabled.get() ? Colour(120, 120, 120) : Colour(90, 160, 255)) :
		Colour(40, 40, 40);

	Div(left_right, w_fit, h_fit,
		onHoverEnter([this]() {
			if (!disabled.get()) isHovered.set(true);
		}),
		onHoverLeave([this]() {
			isHovered.set(false);
		}),
		onClick([this]() {
			HandleClick();
		}))
	{
		Rect(w(15), h(15), bg(boxFill), rounded(4), mr(4), centerY);

		if (label.get().size() > 0)
		{
			Text(label, centerY, wrapText(false));
		}
	}
}

void SableUI::Checkbox::Init(const SableString& p_label, bool p_checked, std::function<void(bool)> p_onChange, bool p_disabled)
{
	label.set(p_label);
	checked.set(p_checked);
	onChangeCallback.set(p_onChange);
	disabled.set(p_disabled);
}

void Checkbox::HandleClick()
{
	if (disabled) return;

	checked.set(!checked.get());

	if (onChangeCallback.get())
	{
		onChangeCallback.get()(checked.get());
	}
}