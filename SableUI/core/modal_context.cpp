#include <SableUI/core/modal_context.h>
#include <SableUI/components/button.h>
#include <SableUI/styles/styles.h>
#include <SableUI/styles/theme.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>

using namespace SableUI;
using namespace SableUI::Style;

void BaseModal::Init(const SableString& str)
{
	title = str;
}

void BaseModal::Layout()
{
	const Theme& t = GetTheme();
	Div(bg(t.surface1), rounded(8), p(16), centerXY, id(GetModalID(title) + "_root"))
	{
		Header();
		Content();
		Footer();
	}
}

void BaseModal::Header()
{
	Div(w_fill, left_right)
	{
		Text(title.bold(), fontSize(14), textWrap(false), mr(4), centerY);
		Button("Close", [this]() { QueueDestroyFloatingPanel("_modal_" + title); }, bg(GetTheme().error * 0.6), size_sm);
	}

	SplitterHorizontal();
}

void BaseModal::Content()
{
	Text("Example content");
}

void BaseModal::OnUpdate(const UIEventContext& ctx)
{
	Element* el = GetElementById(GetModalID(title) + "_root");
	if (el == nullptr) return;

	if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT)
		&& !RectBoundingBox(el->rect, ctx.mousePos))
	{
		QueueDestroyFloatingPanel("_modal_" + title);
		MarkDirty();
	}
}