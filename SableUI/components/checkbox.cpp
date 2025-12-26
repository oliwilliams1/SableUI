#include <SableUI/components/checkbox.h>
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;

void Checkbox::Init(
	State<bool>& checkedState, 
	const SableString& p_label,
	const ElementInfo& p_info,
	bool p_disabled)
{
	info = p_info;
	externalCheckedState = &checkedState;
	label.set(p_label);
	disabled.set(p_disabled);
	onChangeCallback.set(nullptr);
}

void Checkbox::Init(
	bool checked, 
	const SableString& p_label,
	std::function<void(bool)> onChange,
	const ElementInfo& p_info, 
	bool p_disabled)
{
	info = p_info;
	externalCheckedState = nullptr;
	internalChecked.set(checked);
	label.set(p_label);
	onChangeCallback.set(onChange);
	disabled.set(p_disabled);
}

bool Checkbox::IsChecked() const
{
	if (externalCheckedState)
		return externalCheckedState->get();
	return internalChecked.get();
}

static int GetBoxSize(const SableUI::ElementInfo& info)
{
	switch (info.appearance.size)
	{
	case ComponentSize::Small:  return 12;
	case ComponentSize::Large:  return 18;
	default:                    return 15; // medium
	}
}

static int GetFontSize(const SableUI::ElementInfo& info)
{
	switch (info.appearance.size)
	{
	case ComponentSize::Small:	return 10;
	case ComponentSize::Large:	return 13;
	default:					return 11; // medium
	}
}

static Colour GetCheckColour(const SableUI::ElementInfo& info, bool checked, bool disabled)
{
	if (disabled)
	{
		if (checked)
			return (info.appearance.bg != Colour(0, 0, 0, 0))
			? info.appearance.bg * 0.6f
			: Colour(90, 160, 255, 255) * 0.6f;

		return Colour(120, 120, 120, 255);
	}

	Colour base;

	if (checked)
	{
		if (info.appearance.bg != Colour(0, 0, 0, 0))
			base = info.appearance.bg;
		else
			base = Colour(90, 160, 255, 255);
	}
	else
	{
		base = Colour(40, 40, 40, 255);
	}

	return base;
}

void Checkbox::Layout()
{
	bool checked = IsChecked();

	Colour col = GetCheckColour(info, checked, disabled.get());
	int size = GetBoxSize(info);
	int fSize = GetFontSize(info);

	Div(left_right, w_fit, h_fit,
		onClick([this]() {
			HandleClick();
		}))
	{
		Rect(w(size), h(size), hoverBg(col, col * 0.8f), rounded(4), mr(6), centerY);

		if (!label.get().empty())
		{
			Text(label.get(), centerY, wrapText(false), fontSize(fSize));
		}
	}
}

void Checkbox::HandleClick()
{
	if (disabled.get())
		return;

	bool newValue = !IsChecked();

	if (externalCheckedState)
	{
		externalCheckedState->set(newValue);
	}
	else
	{
		internalChecked.set(newValue);
		if (onChangeCallback.get())
		{
			onChangeCallback.get()(newValue);
		}
	}
}