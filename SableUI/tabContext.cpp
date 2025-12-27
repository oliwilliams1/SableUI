#include <SableUI/tabContext.h>
#include <string>
#include <SableUI/element.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>

using namespace SableUI::Style;

SableUI::TabContext& SableUI::TabContext::Add(const std::string& name)
{
	tabs.push_back(name);
	return *this;
}

void SableUI::RenderTabHeader(TabContext& ctx, const ElementInfo& style)
{
	const Theme& t = GetTheme();

	Div(w_fill, h_fit, left_right, bg(t.surface1))
	{
		for (size_t i = 0; i < ctx.tabs.size(); i++)
		{
			bool isActive = (i == (size_t)ctx.activeTab);

			Div(
				bg(isActive ? t.overlay0 : t.surface1),
				p(4),
				mr(2),
				onClick([&ctx, i]() {
					if (ctx.activeTab != i) {
						ctx.activeTab = (int)i;
						ctx.changed = true;
					}
				})
			)
			{
				Text(ctx.tabs[i], wrapText(false));
			}
		}
	}
}