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

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, s_surface->h - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);

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

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, s_surface->h - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);

    for (int i = 0; i < height; i++)
    {
        if (y + i < s_surface->h)
        {
            std::memcpy(surfacePixels + ((y + i) * s_surface->w) + x, rowPixels, width * sizeof(uint32_t));
        }
    }
}

void Drawable::bSplitter::Update(SableUI::rect& rect, SableUI::colour colour, NodeType type, float pBSize, const std::vector<int>& segments, float borderSize, bool draw)
{
    this->r = rect;
    this->c = colour;
    this->type = type;
    this->b = borderSize;
    this->offsets = segments;

    int bufferSize = (int)borderSize;
    if (bufferSize > 0)
    {
        this->buffer.resize(bufferSize);
        std::fill(this->buffer.begin(), this->buffer.end(), colour.value);
    }

    if (draw)
    {
        SableUI::Renderer::Get().Draw(std::make_unique<Drawable::bSplitter>(*this));
    }
}

void Drawable::bSplitter::Draw()
{
    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);
    uint32_t* bufferPixels = buffer.data();

    if (bufferPixels == nullptr || surfacePixels == nullptr) return;

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, s_surface->h - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);

    switch (type)
    {
    case NodeType::HSPLITTER:
    {
        for (int offset : offsets)
        {
            int drawX = x + offset;

            for (int i = 0; i < height; i++)
            {
                if (y + i < s_surface->h && drawX < s_surface->w)
                {
                    std::memcpy(surfacePixels + (y + i) * s_surface->w + drawX, bufferPixels, b * sizeof(uint32_t));
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
        printf("Renderer not initialized!\n");
        return;
    }

    if (SDL_LockSurface(s_surface) < 0)
    {
        printf("Unable to lock surface! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            drawable->Draw();
        }
    }

    SDL_UnlockSurface(s_surface);
    drawStack.clear();
}