#include <SableUI/components/button.h>
#include <SableUI/SableUI.h>
#include <SableUI/events.h>
#include <SableUI/styles.h>
#include <SableUI/utils.h>
#include <functional>

using namespace SableUI;
using namespace SableUI::Style;

void Button::Init(
	const SableString& p_label,
	std::function<void()> p_callback,
	const ElementInfo& p_info)
{
	info = p_info;
	label.set(p_label);
	onClickCallback.set(p_callback);
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
	const float dFac = 0.8f;

	if (src.appearance.hasHoverBg)
	{
		PackStylesToInfo(i, 
			pressed ? hoverBg(src.appearance.bg * dFac, src.appearance.hoverBg * dFac)
			: hoverBg(src.appearance.bg, src.appearance.hoverBg)
		);
		return;
	}

	if (src.appearance.bg != Colour(0, 0, 0, 0))
	{
		PackStylesToInfo(i,
			pressed ? hoverBg(src.appearance.bg * dFac, src.appearance.bg * dFac * dFac)
			: hoverBg(src.appearance.bg, src.appearance.bg * dFac)
		);
		return;
	}

	PackStylesToInfo(i,
		pressed ? hoverBg(rgb(90, 160, 255) * 0.8f, rgb(80, 150, 240) * dFac)
		: hoverBg(rgb(90, 160, 255), rgb(80, 150, 240))
	);
}

static void ApplyDisabledStyle(ElementInfo& i, Colour& textColour)
{
	PackStylesToInfo(i, hoverBg(rgb(100, 100, 100), rgb(100, 100, 100)));

	textColour = Colour(150, 150, 150);
}

void Button::Layout()
{
	Rect padding = ResolvePadding(info);

	ElementInfo i;
	i.layout.pR = padding.x;
	i.layout.pT = padding.y;
	i.layout.pL = padding.w;
	i.layout.pB = padding.h;

	Colour textColor = info.text.colour;

	if (info.appearance.disabled)
		ApplyDisabledStyle(i, textColor);
	else
		ApplyButtonBackground(i, info, isPressed.get());

	i.appearance.radius = 4;
	i.layout.centerX = true;

	i.onClickFunc = [this]() {
		if (!info.appearance.disabled && onClickCallback.get())
			onClickCallback.get()();
	};

	if (SableUI::DivScope _d_(i); true)
	{
		Text(
			label.get(),
			textColour(textColor),
			justify_center,
			wrapText(false)
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
