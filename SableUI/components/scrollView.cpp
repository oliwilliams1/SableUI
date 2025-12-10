#include <algorithm>
#include <string>
#include <SableUI/components/scrollView.h>
#include <SableUI/console.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>

void SableUI::ScrollView::AttachChild(const std::string& p_childID)
{
	if (!childID.empty())
	{
		SableUI_Warn("Scrollview cannot have more than 1 component, skipping addition of %s", p_childID.c_str());
		return;
	}
	this->childID = p_childID;
	needsRerender = true;
}

void SableUI::ScrollView::Layout()
{
	Div(ID("Viewport") w_fill h_fill left_right overflow_hidden bg(32, 32, 32))
	{
		Div(ID("Content")
			w_fill h_fit
			ml(-scrollPos.x)
			mt(-scrollPos.y)
			bg(32, 32, 32))
		{
			if (childID.empty())
			{
				Text("No child of scrollview", centerY justify_center);
			}
			else
			{
				Component(childID.c_str(), w_fill h_fill);
			}
		}

		if (scrollData.contentSize.y <= 0 || scrollData.viewportSize.y <= 0) return;
		float fac = static_cast<float>(scrollData.viewportSize.y) / static_cast<float>(scrollData.contentSize.y);
		if (fac >= 1.0f) return;
		float thumbHeight = fac * scrollData.viewportSize.y;
		float progress = static_cast<float>(scrollPos.y) / static_cast<float>(scrollData.contentSize.y - scrollData.viewportSize.y);
		int parentPadding = 4;
		float topMargin = progress * (scrollData.viewportSize.y - thumbHeight - parentPadding * 2.0f);

		if (barHovered) {
			Div(ID("Bar") w_fit p(parentPadding) h_fill bg(28, 28, 28)) {
				Rect(w(6) h((int)thumbHeight) mt((int)topMargin) rounded(3) bg(149, 149, 149));
			}
		}
		else {
			Div(ID("Bar") w_fit p(parentPadding) h_fill bg(32, 32, 32)) {
				Rect(w(2) m(2) h((int)thumbHeight) mt((int)topMargin) rounded(1) bg(128, 128, 128));
			}
		}
	}
}

void SableUI::ScrollView::OnUpdate(const UIEventContext& ctx)
{
	Element* viewportEl = GetElementById("Viewport");
	Element* contentEl = GetElementById("Content");
	if (!viewportEl || !contentEl) return;

	ScrollData tempData{};
	tempData.viewportSize = vec2(viewportEl->rect.w, viewportEl->rect.h);
	tempData.contentSize = vec2(contentEl->rect.w, contentEl->rect.h);

	setScrollData(tempData);

	if (!RectBoundingBox(viewportEl->rect, ctx.mousePos))
		return;

	vec2 newPos = scrollPos;

	newPos = newPos - ctx.scrollDelta * scrollMultiplier;

	if (contentEl->rect.h > viewportEl->rect.h)
		newPos.y = std::clamp(newPos.y, 0.0f, static_cast<float>(contentEl->rect.h - viewportEl->rect.h));
	else
		newPos.y = 0.0f;

	if (newPos != scrollPos)
		setScrollPos(newPos);

	Element* barEl = GetElementById("Bar");
	if (!barEl) return;

	setBarHovered(RectBoundingBox(barEl->rect, ctx.mousePos));
}