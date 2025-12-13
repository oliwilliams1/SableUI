#include <SableUI/components/scrollView.h>
#include <SableUI/console.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <algorithm>
#include <string>

void SableUI::ScrollView::AttachChild(const std::string& p_childID, const ElementInfo& childInfo)
{
	if (!childID.empty())
	{
		SableUI_Warn("Scrollview cannot have more than 1 component, skipping addition of %s", p_childID.c_str());
		return;
	}
	this->childID = p_childID;
	this->childElInfo = childInfo;
	needsRerender = true;
}

void SableUI::ScrollView::Layout()
{
	Div(ID("Viewport") w_fill h_fill left_right overflow_hidden)
	{
		Div(ID("Content")
			w_fill h_fit
			ml(-scrollPos.x)
			mt(-scrollPos.y))
		{
			if (childID.empty())
			{
				Text("No child of scrollview", centerY justify_center);
			}
			else
			{
				childElInfo.setWType(SableUI::RectType::FILL).setHType(SableUI::RectType::FILL);
				AddComponent(childID.c_str())->BackendInitialiseChild(childID.c_str(), this, childElInfo);
			}
		}

		if (scrollData.contentSize.y <= 0 || scrollData.viewportSize.y <= 0) return;
		float fac = static_cast<float>(scrollData.viewportSize.y) / static_cast<float>(scrollData.contentSize.y);
		if (fac >= 1.0f) return;
		float thumbHeight = fac * scrollData.viewportSize.y;
		float progress = static_cast<float>(scrollPos.y) / static_cast<float>(scrollData.contentSize.y - scrollData.viewportSize.y);
		int parentPadding = 4;
		float topMargin = progress * (scrollData.viewportSize.y - thumbHeight - parentPadding * 2.0f);

		if (barHovered)
		{
			Div(ID("Bar") w_fit p(parentPadding) h_fill bg(28, 28, 28) rounded(4))
			{
				Rect(w(6) h((int)thumbHeight) mt((int)topMargin) rounded(3) bg(149, 149, 149));
			}
		}
		else
		{
			Div(ID("Bar") w_fit p(parentPadding) h_fill bg(32, 32, 32) rounded(4))
			{
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

	if (false) // scroll to end logic (to add)
	{
		float maxScrollY = 0.0f;
		if (contentEl->rect.h > viewportEl->rect.h)
		{
			maxScrollY = static_cast<float>(contentEl->rect.h - viewportEl->rect.h);
		}

		if (scrollPos.y != maxScrollY)
		{
			setScrollPos({ scrollPos.x, maxScrollY });
		}
	}

	Element* barEl = GetElementById("Bar");
	if (barEl)
	{
		bool hover = RectBoundingBox(barEl->rect, ctx.mousePos);
		if (barHovered != hover) setBarHovered(hover);
	}

	if (isDragging)
	{
		if (ctx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
		{
			setIsDragging(false);
			setBarHovered(false);
		}
		else
		{
			float viewportH = tempData.viewportSize.y;
			float contentH = tempData.contentSize.y;

			float padding = 4.0f * 2.0f;

			float fac = viewportH / contentH;
			float thumbHeight = fac * viewportH;

			float trackScrollableRange = viewportH - thumbHeight - padding;
			float contentScrollableRange = contentH - viewportH;

			if (trackScrollableRange > 0)
			{
				float ratio = contentScrollableRange / trackScrollableRange;
				float mouseDeltaY = static_cast<float>(ctx.mousePos.y - dragOrigPos.y);
				float newScrollY = dragStartScrollY + (mouseDeltaY * ratio);

				newScrollY = std::clamp(newScrollY, 0.0f, contentScrollableRange);

				if (newScrollY != scrollPos.y)
					setScrollPos({ scrollPos.x, newScrollY });
			}
		}

		return;
	}

	if (RectBoundingBox(viewportEl->rect, ctx.mousePos))
	{
		vec2 newPos = scrollPos;
		newPos = newPos - ctx.scrollDelta * scrollMultiplier;

		if (contentEl->rect.h > viewportEl->rect.h)
			newPos.y = std::clamp(newPos.y, 0.0f, static_cast<float>(contentEl->rect.h - viewportEl->rect.h));
		else
			newPos.y = 0.0f;

		if (newPos != scrollPos)
			setScrollPos(newPos);
	}

	if (!barEl) return;

	bool hover = RectBoundingBox(barEl->rect, ctx.mousePos);
	if (barHovered != hover) setBarHovered(hover);

	if (hover && ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
	{
		setIsDragging(true);
		setDragOrigPos(ctx.mousePos);
		setDragStartScrollY(scrollPos.y);
	}
}