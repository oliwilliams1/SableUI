#include <algorithm>
#include "SBUI_Renderer.h"

static SbUI_Renderer* renderer = nullptr;

void SbUI_Renderer::Init(SDL_Surface* surface)
{
	if (renderer == nullptr)
	{
		renderer = new SbUI_Renderer(surface);
	}
	else
	{
		printf("Renderer already initialized!\n");
	}
}

void SbUI_Renderer::Shutdown()
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

SbUI_Renderer& SbUI_Renderer::Get()
{
	return *renderer;
}

void SbUI_Renderer::DrawRect(const SbUI_Rect& rect, const SbUIcolour& colour)
{
	SbUI_Rect r = rect;

	r.x = std::clamp(r.x, (uint16_t)0, (uint16_t)(surface->w - 1));
	r.y = std::clamp(r.y, (uint16_t)0, (uint16_t)(surface->h - 1));

	r.w = std::clamp(r.w, (uint16_t)0, (uint16_t)(surface->w - r.x));
	r.h = std::clamp(r.h, (uint16_t)0, (uint16_t)(surface->h - r.y));

	queue.push_back({ r, colour });
}

void SbUI_Renderer::Clear(const SbUIcolour& colour)
{
	DrawRect({ 0, 0, (uint16_t)surface->w, (uint16_t)surface->h }, colour);
}

void SbUI_Renderer::Draw()
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

    for (const Drawable::SbUI_DrawableRect& rect : queue)
    {
        uint32_t* pixelBuffer = new uint32_t[rect.rect.w];
		std::fill(pixelBuffer, pixelBuffer + rect.rect.w, rect.colour.value);

        for (int i = 0; i < rect.rect.h; i++)
        {
            uint32_t* dest = static_cast<uint32_t*>(surface->pixels) + 
				((rect.rect.y + i) * surface->w)+ rect.rect.x;

            std::memcpy(dest, pixelBuffer, rect.rect.w * sizeof(uint32_t));
        }

        delete[] pixelBuffer;
    }

    SDL_UnlockSurface(surface);
	queue.clear();
}