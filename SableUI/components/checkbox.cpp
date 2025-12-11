#include <SableUI/components/checkbox.h>
#include <SableUI/SableUI.h>

using namespace SableUI;

void Checkbox::Layout()
{
	Colour boxBorder = disabled ? Colour(100, 100, 100) :
		isHovered ? Colour(120, 180, 255) :
		Colour(100, 100, 100);

	Colour boxFill = checked ? (disabled ? Colour(120, 120, 120) : Colour(90, 160, 255)) :
		Colour(40, 40, 40);

	Colour textColour = disabled ? Colour(120, 120, 120) : Colour(200, 200, 200);

	Div(left_right w_fit h_fit
		onHover([this]() {
			if (!disabled) setIsHovered(true);
		})
		onHoverExit([this]() {
			setIsHovered(false);
		})
		onClick([this]() {
			HandleClick();
		}))
	{
		Rect(w(20) h(20) bg(boxFill) rounded(4) mr(8) centerY);

		if (label.size() > 0)
		{
			Text(label,
				textColour(textColour)
				fontSize(14)
				centerY
				wrapText(false));
		}
	}
}

void SableUI::Checkbox::Init(const SableString& label, bool checked, std::function<void(bool)> onChange, bool disabled)
{
	setLabel(label);
	setChecked(checked);
	onChangeCallback = onChange;
	setDisabled(disabled);
}

void Checkbox::HandleClick()
{
	if (disabled) return;

	bool newChecked = !checked;
	setChecked(newChecked);

	if (onChangeCallback)
	{
		onChangeCallback(newChecked);
	}
}