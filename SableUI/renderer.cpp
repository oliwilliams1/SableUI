#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

static SableUI::Renderer* s_renderer = nullptr;
static SDL_Surface* s_surface = nullptr;

static void DrawWindowBorder()
{
    static int borderWidth = 1;

    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);

    /* use std::fill to efficiently draw the border */
    uint32_t* topStart = surfacePixels;
    std::fill(topStart, topStart + s_surface->w, 0xFF333333);

    uint32_t* bottomStart = surfacePixels + (s_surface->h - borderWidth) * s_surface->w;
    std::fill(bottomStart, bottomStart + s_surface->w, 0xFF333333);

    /* draw left and right borders */
    for (int i = 0; i < s_surface->h; i++)
    {
        uint32_t* startL = surfacePixels + i * s_surface->w;
		std::fill(startL, startL + borderWidth, 0xFF333333);

        uint32_t* startR = surfacePixels + i * s_surface->w + s_surface->w - borderWidth;
		std::fill(startR, startR + borderWidth, 0xFF333333);
    }
}

void SableUI::Renderer::Init(SDL_Surface* surface)
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

void SableUI::Renderer::SetSurface(SDL_Surface* surface)
{
    s_surface = surface;
}

SableUI::Renderer& SableUI::Renderer::Get()
{
	return *s_renderer;
}

void Drawable::Rect::Update(SableUI::rect& rect, SableUI::colour colour, float pBSize, bool draw)
{

    this->r = rect;
    this->c = colour;

    if (draw)
    {
        SableUI::Renderer::Get().Draw(std::make_unique<Drawable::Rect>(*this));
    }
}

void Drawable::Rect::Draw()
{
    /* get raw pointer data */
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);

    if (surfacePixels == nullptr) return;

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, s_surface->h - 1);
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);

    /* use std::fill to efficiently draw the rect */
    for (int i = 0; i < height; i++)
    {
        if (y + i < s_surface->h)
        {
            uint32_t* start = surfacePixels + ((y + i) * s_surface->w) + x;
            std::fill(start, start + width, c.value);
        }
    }
}

void Drawable::bSplitter::Update(SableUI::rect& rect, SableUI::colour colour, NodeType type, float pBSize, const std::vector<int>& segments, bool draw)
{
    this->r = rect;
    this->c = colour;
    this->type = type;
    this->b = SableUI::f2i(pBSize);
    this->offsets = segments;

    if (draw)
    {
        SableUI::Renderer::Get().Draw(std::make_unique<Drawable::bSplitter>(*this));
    }
}

void Drawable::bSplitter::Draw()
{
    /* get raw pointer data */
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);

    if (surfacePixels == nullptr) return;

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, s_surface->h - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);

    /* use std::fill to efficiently draw the splitter */
    switch (type)
    {
    case NodeType::HSPLITTER:
    {
        for (int offset : offsets)
        {
            int drawX = x + offset - b;

            for (int i = b; i < height; i++)
            {
                if (y + i < s_surface->h && drawX >= 0 && drawX < s_surface->w)
                {
                    uint32_t* start = surfacePixels + (y + i) * s_surface->w + drawX;
                    std::fill(start, start + b * 2, c.value);
                }
            }
        }
        break;
    }

    case NodeType::VSPLITTER:
    {
        for (int offset : offsets)
        {
            int drawY = y + offset - b;

            for (int i = 0; i < b * 2; i++)
            {
                if (drawY + i >= 0 && drawY + i < s_surface->h && x < s_surface->w)
				{
                    uint32_t* start = surfacePixels + (drawY + i) * s_surface->w + x;
                    std::fill(start, start + width, c.value);
				}
            }
        }
        break;
    }
    }
}

void SableUI::Renderer::Draw(std::unique_ptr<Drawable::Base> drawable)
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

    if (SDL_LockSurface(s_surface) < 0)
    {
        SableUI_Error("Unable to lock surface! SDL_Error: %s", SDL_GetError());
        return;
    }

    /* iterate through queue and draw all types of drawables */
    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            drawable->Draw();
        }
    }

    /* draw window border after queue is drawn */
    DrawWindowBorder();

    SDL_UnlockSurface(s_surface);
    SDL_FreeSurface(s_surface);
    drawStack.clear();
}