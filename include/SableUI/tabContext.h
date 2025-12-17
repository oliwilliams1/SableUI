#pragma once
#include <SableUI/SableUI.h>
#include <vector>
#include <string>

namespace SableUI
{
    struct TabContext
    {
        int activeTab = 0;
        std::vector<std::string> tabs;
        bool changed = false;

        TabContext& Add(const std::string& name) 
        { 
            tabs.push_back(name); 
            return *this; 
        }
    };

    inline void RenderTabHeader(TabContext& ctx, const ElementInfo& style = ElementInfo{})
    {
        Div(w_fill h_fit left_right bg(45, 45, 45))
        {
            for (size_t i = 0; i < ctx.tabs.size(); i++)
            {
                bool isActive = (i == (size_t)ctx.activeTab);
                
                Div(
                    bg(isActive ? Colour(70, 70, 70) : Colour(50, 50, 50))
                    p(4)
                    mr(2)
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
}