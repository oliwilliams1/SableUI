#include <algorithm>
#include <cmath>
#include "SBUI_Renderer.h"

static SableUI::Renderer* renderer = nullptr;

void SableUI::Renderer::Init(SDL_Surface* surface)
{
	if (renderer == nullptr)
	{
		renderer = new Renderer(surface);
	}
	else
	{
		printf("Renderer already initialized!\n");
	}
}

void SableUI::Renderer::Shutdown()
{
	if (renderer != nullptr)
	{
		delete renderer;
		renderer = nullptr;
	}
	else
	{
		printf("Renderer not initialized!\n");
	}
}

void SableUI::Renderer::SetSurface(SDL_Surface* surface)
{
    Get().surface = surface;
}

SableUI::Renderer& SableUI::Renderer::Get()
{
	return *renderer;
}

void SableUI::Renderer::Clear(const SableUI::colour& colour)
{
    rect r = { 0, 0, (float)surface->w, (float)surface->h };
    Drawable::rect rect = { r, colour };

    rectQueue.push_back(rect);
}

void SableUI::Renderer::GetDrawableRect(Drawable::rect& drawableRect, const SableUI::rect& rect, const SableUI::colour& colour, float border, bool draw)
{
    drawableRect.rowBuffer.clear();

    SableUI::rect r = rect;

    r.x = std::clamp(r.x, 0.0f, surface->w - 1.0f);
    r.y = std::clamp(r.y, 0.0f, surface->h - 1.0f);

    r.w = std::clamp(r.w, 0.0f, surface->w - r.x);
    r.h = std::clamp(r.h, 0.0f, surface->h - r.y);

    if (border > 0.0f)
    {
        r.x += border;
        r.y += border;
        r.w -= border * 2;
        r.h -= border * 2;
    }

    r.w = std::max(r.w, 0.0f);
    r.h = std::max(r.h, 0.0f);

    drawableRect = { r, colour };

    int x = std::min(static_cast<int>(std::ceil(r.x)), surface->w - 1);
    int y = std::min(static_cast<int>(std::ceil(r.y)), surface->h - 1);
    int width = std::min(static_cast<int>(std::ceil(r.w)), surface->w - x);
    int height = std::min(static_cast<int>(std::ceil(r.h)), surface->h - y);

    drawableRect.rowBuffer.resize(width);
    std::fill(drawableRect.rowBuffer.begin(), drawableRect.rowBuffer.end(), colour.value);

    if (draw)
    {
        rectQueue.push_back(drawableRect);
    }
}

void SableUI::Renderer::DrawRects(int surfaceWidth, int surfaceHeight)
{
    for (Drawable::rect& rect : rectQueue)
    {
        uint32_t* surfacePixels = static_cast<uint32_t*>(surface->pixels);
        uint32_t* rowPixels = rect.rowBuffer.data();

        if (rowPixels == nullptr) continue;

        int x = std::min(static_cast<int>(std::ceil(rect.r.x)), surface->w - 1);
        int y = std::min(static_cast<int>(std::ceil(rect.r.y)), surface->h - 1);
        int width = std::min(static_cast<int>(std::ceil(rect.r.w)), surface->w - x);
        int height = std::min(static_cast<int>(std::ceil(rect.r.h)), surface->h - y);

        for (int i = 0; i < height; i++)
        {
            if (y + i < surfaceHeight)
            {
                std::memcpy(surfacePixels + ((y + i) * surfaceWidth) + x, rowPixels, width * sizeof(uint32_t));
            }
        }
    }

    rectQueue.clear();
}

void SableUI::Renderer::GetDrawableRectBorder(Drawable::rectBorder& drawableRectBorder, const SableUI::rect& rect, const SableUI::colour& colour, float border, bool draw)
{
    drawableRectBorder.topRowBuffer.clear();
    drawableRectBorder.sideRowBuffer.clear();
    drawableRectBorder.border = border;
    drawableRectBorder.colour = colour;
    drawableRectBorder.r = rect;

    SableUI::rect r = rect;

    r.x = std::clamp(r.x, 0.0f, surface->w - 1.0f);
    r.y = std::clamp(r.y, 0.0f, surface->h - 1.0f);

    r.w = std::clamp(r.w, 0.0f, surface->w - r.x);
    r.h = std::clamp(r.h, 0.0f, surface->h - r.y);

    drawableRectBorder.topRowBuffer.resize(static_cast<size_t>(r.w));
    drawableRectBorder.sideRowBuffer.resize(static_cast<size_t>(border));

    std::fill(drawableRectBorder.topRowBuffer.begin(), drawableRectBorder.topRowBuffer.end(), colour.value);
    std::fill(drawableRectBorder.sideRowBuffer.begin(), drawableRectBorder.sideRowBuffer.end(), colour.value);

    if (draw)
    {
        rectBorderQueue.push_back(drawableRectBorder);
    }
}

void SableUI::Renderer::DrawRectBorders(int surfaceWidth, int surfaceHeight)
{
    for (Drawable::rectBorder& rectBorder : rectBorderQueue)
    {
        uint32_t* surfacePixels = static_cast<uint32_t*>(surface->pixels);
        uint32_t* topRowPixels = rectBorder.topRowBuffer.data();
        uint32_t* sideRowPixels = rectBorder.sideRowBuffer.data();

        if (topRowPixels == nullptr || sideRowPixels == nullptr) continue;

        int x = std::min(static_cast<int>(std::ceil(rectBorder.r.x)), surface->w - 1);
        int y = std::min(static_cast<int>(std::ceil(rectBorder.r.y)), surface->h - 1);
        int width = std::min(static_cast<int>(std::ceil(rectBorder.r.w)), surface->w - x);
        int height = std::min(static_cast<int>(std::ceil(rectBorder.r.h)), surface->h - y);

        for (int i = 0; i < rectBorder.border; i++)
        {
            if (y + i < surfaceHeight)
            {
                std::memcpy(surfacePixels + ((y + i) * surfaceWidth) + x, topRowPixels, width * sizeof(uint32_t));
            }

			if (y + height - rectBorder.border + i < surfaceHeight)
			{
				std::memcpy(surfacePixels + ((y + height - (int)rectBorder.border + i) * surfaceWidth) + x, topRowPixels, width * sizeof(uint32_t));
			}
        }

        for (int i = 0; i < height; i++)
        {
            int f1 = x;
            int f2 = x + width - rectBorder.border;

            if (y + i < surfaceHeight)
            {
                if (f1 >= 0) {
                    std::memcpy(surfacePixels + ((y + i) * surfaceWidth) + f1, sideRowPixels, rectBorder.sideRowBuffer.size() * sizeof(uint32_t));
                }
                if (f2 < surfaceWidth) {
                    std::memcpy(surfacePixels + ((y + i) * surfaceWidth) + f2, sideRowPixels, rectBorder.sideRowBuffer.size() * sizeof(uint32_t));
                }
            }
        }
    }

    rectBorderQueue.clear();
}

void SableUI::Renderer::Draw()
{
    if (renderer == nullptr)
    {
        printf("Renderer not initialized!\n");
        return;
    }

    if (SDL_LockSurface(surface) < 0)
    {
        printf("Unable to lock surface! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    int surfaceWidth = surface->w;
    int surfaceHeight = surface->h;

    DrawRects(surfaceWidth, surfaceHeight);
    DrawRectBorders(surfaceWidth, surfaceHeight);

    SDL_UnlockSurface(surface);
    rectQueue.clear();
}