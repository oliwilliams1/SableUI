#include <algorithm>
#include <string>

#include "SBUI_Renderer.h"
#include "SBUI_Utils.h"

static SableUI::Renderer* s_renderer = nullptr;
static SDL_Surface* s_surface = nullptr;

void SableUI::Renderer::Init(SDL_Surface* surface)
{
	if (s_renderer == nullptr)
	{
		s_renderer = new Renderer();
        s_surface = surface;
	}
	else
	{
		printf("Renderer already initialized!\n");
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
		printf("Renderer not initialized!\n");
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
    this->rowBuffer.clear();

    this->r = rect;
    this->c = colour;

    r.x = std::clamp(r.x, 0.0f, s_surface->w - 1.0f);
    r.y = std::clamp(r.y, 0.0f, s_surface->h - 1.0f);
    r.w = std::clamp(r.w, 0.0f, s_surface->w - r.x);
    r.h = std::clamp(r.h, 0.0f, s_surface->h - r.y);

    r.x += pBSize;
	r.y += pBSize;
	r.w -= pBSize * 2.0f;
	r.h -= pBSize * 2.0f;

    int x = std::clamp(static_cast<int>(std::ceil(r.x)), 0, s_surface->w - 1);
    int y = std::clamp(static_cast<int>(std::ceil(r.y)), 0, s_surface->h - 1);
    int width = std::clamp(static_cast<int>(std::ceil(r.w)), 0, s_surface->w - x);
    int height = std::clamp(static_cast<int>(std::ceil(r.h)), 0, s_surface->h - y);

    this->rowBuffer.resize(width);
    std::fill(this->rowBuffer.begin(), this->rowBuffer.end(), colour.value);

    if (draw)
    {
        SableUI::Renderer::Get().Draw(std::make_unique<Drawable::Rect>(*this));
    }
}

void Drawable::Rect::Draw()
{
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);
    uint32_t* rowPixels = rowBuffer.data();

    if (rowPixels == nullptr || surfacePixels == nullptr) return;

    int x = SableUI::f2i(std::clamp(r.x, 0.0f, static_cast<float>(s_surface->w)));
    int y = SableUI::f2i(std::clamp(r.y, 0.0f, static_cast<float>(s_surface->h)));
    int width = SableUI::f2i(std::clamp(r.w, 0.0f, static_cast<float>(s_surface->w - x)));
    int height = SableUI::f2i(std::clamp(r.h, 0.0f, static_cast<float>(s_surface->h - y)));

    for (int i = 0; i < height; i++)
    {
        if (y + i < s_surface->h)
        {
            std::memcpy(surfacePixels + ((y + i) * s_surface->w) + x, rowPixels, width * sizeof(uint32_t));
        }
    }
}

void Drawable::bSplitter::Update(SableUI::rect& rect, SableUI::colour colour, float pBSize, 
    const std::vector<int>& segments, float borderSize, bool draw)
{
    this->r = rect;
    this->c = colour;
    this->b = SableUI::f2i(borderSize);
    this->segments = segments;

    this->rowBuffer.clear();
    this->colBuffer.clear();

    rowBuffer.resize(r.w);
    std::fill(rowBuffer.begin(), rowBuffer.end(), c.value);
}

void Drawable::bSplitter::Draw()
{
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);
    uint32_t* rowPixels = rowBuffer.data();

    if (rowPixels == nullptr || surfacePixels == nullptr) return;

    int x = SableUI::f2i(std::clamp(r.x, 0.0f, static_cast<float>(s_surface->w)));
    int y = SableUI::f2i(std::clamp(r.y, 0.0f, static_cast<float>(s_surface->h)));
    int width = SableUI::f2i(std::clamp(r.w, 0.0f, static_cast<float>(s_surface->w - x)));
    int height = SableUI::f2i(std::clamp(r.h, 0.0f, static_cast<float>(s_surface->h - y)));

    for (int i = 0; i < b; i++)
    {
        if (y + i < s_surface->h)
        {
            std::memcpy(surfacePixels + ((y + i) * s_surface->w) + x, rowPixels, width * sizeof(uint32_t));
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
        printf("Renderer not initialized!\n");
        return;
    }

    if (SDL_LockSurface(s_surface) < 0)
    {
        printf("Unable to lock surface! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    int surfaceWidth = s_surface->w;
    int surfaceHeight = s_surface->h;

    for (const auto &drawable : drawStack)
	{
		drawable.get()->Draw();
	}

    printf("%i\n", (int)drawStack.size());

    SDL_UnlockSurface(s_surface);
    drawStack.clear();
}