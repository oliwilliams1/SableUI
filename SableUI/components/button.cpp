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

void ButtonComponent::Init(
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
	case ComponentSize::Medium: return { 12, 6, 12, 6 };
	case ComponentSize::Large:  return { 16, 10, 16, 10 };
	default:					return { 0, 0, 0, 0 };
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

static void ApplyDisabledStyle(ElementInfo& i, Colour& textColour)
{
	const Theme& t = GetTheme();

	PackStylesToInfo(i, bg(t.subtext0));
}

void ButtonComponent::Layout()
{
	Rect padding = ResolvePadding(info);

	ElementInfo i{};
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

	i.appearance.bg = (info.appearance.disabled) 
		? GetTheme().subtext0 
		: info.appearance.bg.value_or(GetTheme().primary) * (isPressed.get() ? 0.9f : 1.0f);

	i.appearance.rTL = info.appearance.rTL > 0.0f ? info.appearance.rTL : 4.0f;
	i.appearance.rTR = info.appearance.rTR > 0.0f ? info.appearance.rTR : 4.0f;
	i.appearance.rBL = info.appearance.rBL > 0.0f ? info.appearance.rBL : 4.0f;
	i.appearance.rBR = info.appearance.rBR > 0.0f ? info.appearance.rBR : 4.0f;

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
			textWrap(false),
			fontSize(info.text.fontSize),
			centerY
		);
	}
}

void ButtonComponent::OnUpdate(const UIUpdateContext& ctx)
{
	Element* root = GetRootElement();
	if (!root)
	{
		SableUI_Warn("GetRootElement() failed in Button::OnUpdate()");
		return;
	}

	bool isHovered = RectBoundingBox(root->rect, ctx.input.mousePos, ctx.input.obscurers, ctx.zIndex);

	if (!info.appearance.disabled)
	{
		if (isHovered)
		{
			if (ctx.input.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
				isPressed.set(true);
			else if (!ctx.input.mouseDown.test(SABLE_MOUSE_BUTTON_LEFT))
				isPressed.set(false);
		}
		else if (isPressed.get() && !ctx.input.mouseDown.test(SABLE_MOUSE_BUTTON_LEFT))
			isPressed.set(false);
	}
}