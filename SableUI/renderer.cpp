#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

static SableUI::Renderer* s_renderer = nullptr;
static SableUI::Texture* s_surface = nullptr;

static void DrawWindowBorder()
{
    static int borderWidth = 1;

    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);

    /* use std::fill to efficiently draw the border */
    uint32_t* topStart = surfacePixels;
    std::fill(topStart, topStart + s_surface->width, 0xFF333333);

    uint32_t* bottomStart = surfacePixels + (s_surface->height - borderWidth) * s_surface->width;
    std::fill(bottomStart, bottomStart + s_surface->width, 0xFF333333);

    /* draw left and right borders */
    for (int i = 0; i < s_surface->height; i++)
    {
        uint32_t* startL = surfacePixels + i * s_surface->width;
		std::fill(startL, startL + borderWidth, 0xFF333333);

        uint32_t* startR = surfacePixels + i * s_surface->width + s_surface->width - borderWidth;
		std::fill(startR, startR + borderWidth, 0xFF333333);
    }
}

void SableUI::Renderer::Init(Texture* surface)
{
	if (s_renderer == nullptr)
	{
		s_renderer = new Renderer();
        s_surface = surface;
	}
	else
	{
		SableUI_Warn("Renderer already initialized!");
	}
}

void SableUI::Renderer::Shutdown()
{
	if (s_renderer != nullptr)
	{
		delete s_renderer;
		s_renderer = nullptr;
	}
	else
	{
		SableUI_Warn("Renderer not initialized!");
	}
}

void SableUI::Renderer::SetSurface(Texture* surface)
{
    s_surface = surface;
}

void SableUI::Renderer::Flush()
{
    s_renderer->drawStack.clear();
}

SableUI::Renderer& SableUI::Renderer::Get()
{
	return *s_renderer;
}

void SableUI::Renderer::Draw(std::unique_ptr<SableUI::DrawableBase> drawable)
{
    drawStack.push_back(std::move(drawable));
}

void SableUI::Renderer::Draw()
{
    if (s_renderer == nullptr)
    {
        SableUI_Error("Renderer not initialized!");
        return;
    }

    std::sort(drawStack.begin(), drawStack.end(), [](const std::unique_ptr<SableUI::DrawableBase>& a, const std::unique_ptr<SableUI::DrawableBase>& b) {
        return a->z < b->z;
    });

    if (drawStack.size() == 0) return;

    /* iterate through queue and draw all types of drawables */
    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            drawable->Draw(s_surface);
        }
    }

    /* draw window border after queue is drawn */
    DrawWindowBorder();

    drawStack.clear();

    s_surface->Update();
}