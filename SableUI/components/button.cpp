#include <SableUI/SableUI.h>
#include <SableUI/components/button.h>
#include <SableUI/core/events.h>
#include <SableUI/styles/styles.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/element.h>
#include <SableUI/styles/theme.h>
#include <SableUI/core/text.h>
#include <functional>

using namespace SableUI;
using namespace SableUI::Style;

void Button::Init(
	const SableString& p_label,
	std::function<void()> p_callback,
	const ElementInfo& p_info)
{
	info = p_info;
	label = p_label;
	onClickCallback = p_callback;

	MarkDirty();
}

static Rect GetDefaultPadding(const ElementInfo& info)
{
	switch (info.appearance.size)
	{
	case ComponentSize::Small:  return { 8, 4, 8, 4 };
	case ComponentSize::Large:  return { 16, 10, 16, 10 };
	default:                    return { 12, 6, 12, 6 }; // medium
	}
}

static Rect ResolvePadding(const ElementInfo& info)
{
	if (info.layout.pB || info.layout.pT || info.layout.pL || info.layout.pR)
	{
		return {
			info.layout.pR,
			info.layout.pT,
			info.layout.pL,
			info.layout.pB
		};
	}

	return GetDefaultPadding(info);
}

static void ApplyButtonBackground(
	ElementInfo& i,
	const ElementInfo& src,
	bool pressed)
{
	const Theme& t = GetTheme();
	const float dFac = 0.9f;

	if (src.appearance.hasHoverBg)
	{
		Colour base = src.appearance.bg.value_or(t.primary);

		PackStylesToInfo(i,
			pressed ? hoverBg(base * dFac, src.appearance.hoverBg * dFac)
			: hoverBg(base, src.appearance.hoverBg)
		);
		return;
	}

	if (src.appearance.bg.has_value() && *src.appearance.bg != Colour(0, 0, 0, 0))
	{
		Colour base = *src.appearance.bg;

		PackStylesToInfo(i,
			pressed ? hoverBg(base * dFac, base * dFac * dFac)
			: hoverBg(base, base * dFac)
		);
		return;
	}

	PackStylesToInfo(i,
		pressed ? hoverBg(t.primary * dFac, t.primary * dFac * dFac)
		: hoverBg(t.primary, t.primary * dFac)
	);
}

static void ApplyDisabledStyle(ElementInfo& i, Colour& textColour)
{
	const Theme& t = GetTheme();

	PackStylesToInfo(i, bg(t.subtext0));
}

void Button::Layout()
{
	Rect padding = ResolvePadding(info);

	ElementInfo i;
	i.layout.pR = padding.x;
	i.layout.pT = padding.y;
	i.layout.pL = padding.w;
	i.layout.pB = padding.h;

	i.layout.wType = RectType::Fill;
	i.layout.hType = RectType::Fill;

	Colour col;
	if (info.text.colour.has_value())
		col = info.text.colour.value();
	else
		col = GetTheme().text;

	if (info.appearance.disabled)
		ApplyDisabledStyle(i, col);
	else
		ApplyButtonBackground(i, info, isPressed.get());

	i.appearance.radius = 4;

	i.onClickFunc = [this]() {
		if (!info.appearance.disabled && onClickCallback)
			onClickCallback();
		};

	if (SableUI::DivScope d(i); true)
	{
		Text(
			label,
			textColour(col),
			justify(i.text.justification.value_or(TextJustification::Center)),
			textWrap(false)
		);
	}
}

void Button::OnUpdate(const UIEventContext& ctx)
{
	Element* root = GetRootElement();
	if (!root)
	{
		SableUI_Warn("GetRootElement() failed in Button::OnUpdate()");
		return;
	}

	bool isHovered = RectBoundingBox(root->rect, ctx.mousePos);

	if (!info.appearance.disabled && isHovered)
	{
		if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
			isPressed.set(true);
		else if (ctx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
			isPressed.set(false);
	}
}