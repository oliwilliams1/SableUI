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

void SableUI::Renderer::DrawRect(const SableUI::rect& rect, const SableUI::colour& colour)
{
	SableUI::rect r = rect;

	r.x = std::clamp(r.x, 0.0f, surface->w - 1.0f);
	r.y = std::clamp(r.y, 0.0f, surface->h - 1.0f);

	r.w = std::clamp(r.w, 0.0f, surface->w - r.x);
	r.h = std::clamp(r.h, 0.0f, surface->h - r.y);

	queue.push_back({ r, colour });
}

void SableUI::Renderer::Clear(const SableUI::colour& colour)
{
	DrawRect({ 0, 0, (float)surface->w, (float)surface->h }, colour);
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

    for (const Drawable::SbUI_DrawableRect& rect : queue)
    {
        int x = std::min(static_cast<int>(std::ceil(rect.rect.x)), surfaceWidth - 1);
        int y = std::min(static_cast<int>(std::ceil(rect.rect.y)), surfaceHeight - 1);
        int width = std::min(static_cast<int>(std::ceil(rect.rect.w)), surfaceWidth - x);
        int height = std::min(static_cast<int>(std::ceil(rect.rect.h)), surfaceHeight - y);

        if (width <= 0 || height <= 0)
            continue;

        uint32_t* pixelBuffer = new uint32_t[width];
        std::fill(pixelBuffer, pixelBuffer + width, rect.colour.value);

        uint32_t* surfacePixels = static_cast<uint32_t*>(surface->pixels);

        // Use a single memcpy call
        for (int i = 0; i < height; i++)
        {
            std::memcpy(surfacePixels + ((y + i) * surfaceWidth) + x, pixelBuffer, width * sizeof(uint32_t));
        }

        delete[] pixelBuffer;
    }

    SDL_UnlockSurface(surface);
    queue.clear();
}