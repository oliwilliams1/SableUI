#include <algorithm>
#include <string>
#include <set>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"
#include "SableUI/shader.h"

SableUI::Renderer::~Renderer()
{
    DestroyShaders();
}

void SableUI::Renderer::ClearStack()
{
    drawStack.clear();
}

void SableUI::Renderer::Draw(DrawableBase* drawable)
{
    drawStack.push_back(drawable);
}

void SableUI::Renderer::StartDirectDraw()
{
	directDraw = true;
}

void SableUI::Renderer::DirectDrawRect(const Rect& rect, const Colour& colour)
{
    DrawableRect dr;
    dr.m_rect = rect;
    dr.m_colour = colour;
    dr.Draw(&renderTarget, GetContextResources());
}

void SableUI::Renderer::EndDirectDraw()
{
	directDraw = false;
    glFlush();
}

void SableUI::Renderer::Draw()
{
    if (drawStack.size() == 0) return;

    if (directDraw == true)
    {
        SableUI_Warn("Direct draw is enabled when drawing normally, forgot to end direct draw?");
    }

    if (renderTarget.target == TargetType::TEXTURE) renderTarget.Bind();

    ContextResources& res = GetContextResources();

    std::sort(drawStack.begin(), drawStack.end(), [](const DrawableBase* a, const DrawableBase* b) {
        return a->m_zIndex < b->m_zIndex;
    });

    std::set<unsigned int> drawnUUIDs;

    /* iterate through queue and draw all types of drawables */
    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            if (drawnUUIDs.find(drawable->uuid) != drawnUUIDs.end()) continue;
            drawnUUIDs.insert(drawable->uuid);
            drawable->Draw(&renderTarget, res);
        }
    }

    /* draw window border after queue is drawn */
    DrawWindowBorder(&renderTarget);

    drawStack.clear();

    glFlush();
}