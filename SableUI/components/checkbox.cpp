#include <SableUI/components/checkbox.h>
#include <SableUI/SableUI.h>

using namespace SableUI;
using namespace SableUI::Style;

void Checkbox::Init(
	State<bool>& checkedState, 
	const SableString& p_label,
	const ElementInfo& p_info)
{
	info = p_info;
	externalCheckedState = &checkedState;
	label.set(p_label);
	onChangeCallback.set(nullptr);
}

void Checkbox::Init(
	bool checked, 
	const SableString& p_label,
	std::function<void(bool)> onChange,
	const ElementInfo& p_info)
{
	info = p_info;
	externalCheckedState = nullptr;
	internalChecked.set(checked);
	label.set(p_label);
	onChangeCallback.set(onChange);
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

static Colour GetCheckColour(const SableUI::ElementInfo& info, bool checked, const SableUI::Theme& t)
{
	if (info.appearance.disabled)
	{
		if (checked)
			return (info.appearance.bg != Colour(0, 0, 0, 0))
			? info.appearance.bg * 0.6f
			: t.overlay2 * 0.6f;

		return t.overlay2;
	}

	Colour base;

	if (checked)
	{
		if (info.appearance.bg != Colour(0, 0, 0, 0))
			base = info.appearance.bg;
		else
			base = t.primary;
	}
	else
	{
		base = t.surface2;
	}

	return base;
}

void Checkbox::Layout()
{
	const Theme& t = GetTheme();

	bool checked = IsChecked();

	Colour col = GetCheckColour(info, checked, t);
	int size = GetBoxSize(info);
	int fSize = GetFontSize(info);

	Div(left_right, w_fit, h_fit,
		onClick([this]() {
			HandleClick();
		}))
	{
		if (info.appearance.disabled)
			Rect(w(size), h(size), bg(col), rounded(4), mr(6), centerY);
		else
			Rect(w(size), h(size), hoverBg(col, col * 0.8f), rounded(4), mr(6), centerY);


		if (!label.get().empty())
		{
			Text(label.get(), centerY, wrapText(false), fontSize(fSize));
		}
	}
}

void Checkbox::HandleClick()
{
	if (info.appearance.disabled)
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