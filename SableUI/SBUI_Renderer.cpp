#include <algorithm>
#include <cmath>
#include <string>

#include "SBUI_Renderer.h"

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

void Drawable::Rect::Update(SableUI::rect& rect, SableUI::colour colour, float border, SableUI::colour bColour, bool draw)
{
    this->rowBuffer.clear();

    this->r = rect;
    this->colour = colour;
    this->bColour = bColour;
    this->border = border;

    r.x = std::clamp(r.x, 0.0f, s_surface->w - 1.0f);
    r.y = std::clamp(r.y, 0.0f, s_surface->h - 1.0f);

    r.w = std::clamp(r.w, 0.0f, s_surface->w - r.x);
    r.h = std::clamp(r.h, 0.0f, s_surface->h - r.y);

    if (border > 0.0f)
    {
		r.x += border;
		r.y += border;
		r.w -= border * 2;
		r.h -= border * 2;
    }

	r.w = std::max(r.w, 0.0f);
	r.h = std::max(r.h, 0.0f);

    int x = std::min(static_cast<int>(std::ceil(r.x)), s_surface->w - 1);
    int y = std::min(static_cast<int>(std::ceil(r.y)), s_surface->h - 1);
    int width = std::min(static_cast<int>(std::ceil(r.w)), s_surface->w - x);
    int height = std::min(static_cast<int>(std::ceil(r.h)), s_surface->h - y);

    this->rowBuffer.resize(width);
    std::fill(this->rowBuffer.begin(), this->rowBuffer.end(), colour.value);

    if (border > 0.0f)
    {
        SableUI::rect temp_br = rect;

        bTBRowBuffer.resize(static_cast<size_t>(r.w + 1 + border * 2));
        bLRRowBuffer.resize(static_cast<size_t>(border));

        std::fill(bTBRowBuffer.begin(), bTBRowBuffer.end(), bColour.value);
        std::fill(bLRRowBuffer.begin(), bLRRowBuffer.end(), bColour.value);
    }

    if (draw)
    {
        SableUI::Renderer::Get().Draw(std::make_unique<Drawable::Rect>(*this));
    }
}

void Drawable::Rect::Draw()
{
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);
    uint32_t* rowPixels = rowBuffer.data();

    if (rowPixels == nullptr) return;

    int x      = static_cast<int>(std::min(r.x, static_cast<float>(s_surface->w)));
	int y      = static_cast<int>(std::min(r.y, static_cast<float>(s_surface->h)));
	int width  = static_cast<int>(std::min(r.w, static_cast<float>(s_surface->w - x)));
	int height = static_cast<int>(std::min(r.h, static_cast<float>(s_surface->h - y)));

	for (int i = 0; i < height; i++)
	{
		if (y + i < s_surface->h)
		{
			std::memcpy(surfacePixels + ((y + i) * s_surface->w) + x, rowPixels, width * sizeof(uint32_t));
		}
	}

    if (border <= 0.0f) return;

    if (bLRRowBuffer.size() == 0 || bTBRowBuffer.size() == 0) return;

	uint32_t* bTBRowPixels = bTBRowBuffer.data();
	uint32_t* bLRRowPixels = bLRRowBuffer.data();

    size_t sH1 = (width + 1 + 2 * border) * sizeof(uint32_t);

    for (int i = 0; i < border; i++)
    {
        if (y + i < s_surface->h)
        {
            std::memcpy(surfacePixels + ((y + i - (int)border) * s_surface->w) + x - (int)border, bTBRowPixels, sH1);
        }

        if (y + i < s_surface->h)
		{
            std::memcpy(surfacePixels + ((y + height + i) * s_surface->w) + x - (int)border, bTBRowPixels, sH1);
		}
    }

    int f1 = static_cast<int>(x - border);
    int f2 = static_cast<int>(x + width);

    size_t sH2 = border * sizeof(uint32_t);

    for (int i = 0; i < height; i++)
    {
        if (y + i < s_surface->h)
        {
            if (f1 >= 0)
            {
				std::memcpy(surfacePixels + ((y + i) * s_surface->w) + f1, bLRRowPixels, sH2);
            }
            if (f2 < s_surface->w)
            {
                std::memcpy(surfacePixels + ((y + i) * s_surface->w) + f2, bLRRowPixels, sH2);
			}
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

    SDL_UnlockSurface(s_surface);
    drawStack.clear();
}

Drawable::size::size(const char* str)
{
    if (str == nullptr) return;

    std::string s(str);
    if (s.length() > 1 && s.substr(s.length() - 2) == "px")
    {
        type = SizeType::PX;
        v = std::stof(s.substr(0, s.length() - 2));
    }
    else if (s.back() == '%')
    {
        type = SizeType::PERCENT;
        v = std::stof(s.substr(0, s.length() - 1));
    }
    else
    {
        v = -1.0f;
    }
}
