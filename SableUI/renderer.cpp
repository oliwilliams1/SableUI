#include <algorithm>
#include <string>

#include "SableUI/renderer.h"
#include "SableUI/utils.h"

static SableUI::Renderer* s_renderer = nullptr;
static SDL_Surface* s_surface = nullptr;

static void DrawWindowBorder()
{
    static std::vector<uint32_t> TBRowBuffer(0);
    static std::vector<uint32_t> LRRowBuffer(0);

    static int borderWidth = 1;


    if (TBRowBuffer.size() != s_surface->w)
    {
        TBRowBuffer.resize(static_cast<size_t>(s_surface->w));
        std::fill(TBRowBuffer.begin(), TBRowBuffer.end(), 0xFF333333);
    }

    if (LRRowBuffer.size() != borderWidth)
    {
        LRRowBuffer.resize(borderWidth);
        std::fill(LRRowBuffer.begin(), LRRowBuffer.end(), 0xFF333333);
    }

    uint32_t* surfacePixels = static_cast<uint32_t*>(s_surface->pixels);
    uint32_t* TBRowPixels = TBRowBuffer.data();
    uint32_t* LRRowPixels = LRRowBuffer.data();


    std::memcpy(surfacePixels, TBRowPixels, TBRowBuffer.size() * sizeof(uint32_t));
    std::memcpy(surfacePixels + (s_surface->h - borderWidth) * s_surface->w, TBRowPixels, TBRowBuffer.size() * sizeof(uint32_t));

    for (int i = 0; i < s_surface->h; i++)
    {
        std::memcpy(surfacePixels + i * s_surface->w, LRRowPixels, borderWidth * sizeof(uint32_t));
        std::memcpy(surfacePixels + i * s_surface->w + s_surface->w - borderWidth, LRRowPixels, borderWidth * sizeof(uint32_t));
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
    this->rowBuffer.clear();

    this->r = rect;
    this->c = colour;

    int x = std::clamp(SableUI::f2i(r.x), 0, s_surface->w - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, s_surface->w - x);

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
    int height = std::clamp(SableUI::f2i(r.h), 0, s_surface->h - y);

    for (int i = 0; i < height; i++)
    {
        if (y + i < s_surface->h)
        {
            std::memcpy(surfacePixels + ((y + i) * s_surface->w) + x, rowPixels, rowBuffer.size() * sizeof(uint32_t));
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

    if (type == NodeType::HSPLITTER)
    {
        this->buffer.resize(static_cast<size_t>(b * 2));
        std::fill(this->buffer.begin(), this->buffer.end(), colour.value);
    }

    if (type == NodeType::VSPLITTER)
    {
        this->buffer.resize(static_cast<size_t>(r.w));
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
            int drawX = x + offset - b;

            for (int i = b; i < height; i++)
            {
                if (y + i < s_surface->h && drawX >= 0 && drawX < s_surface->w)
                {
                    std::memcpy(surfacePixels + (y + i) * s_surface->w + drawX, bufferPixels,
                        static_cast<size_t>(buffer.size() * sizeof(uint32_t)));
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
					std::memcpy(surfacePixels + (drawY + i) * s_surface->w + x, bufferPixels,
						static_cast<size_t>(buffer.size() * sizeof(uint32_t)));
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

    for (const auto& drawable : drawStack)
    {
        if (drawable)
        {
            drawable->Draw();
        }
    }

    DrawWindowBorder();

    SDL_UnlockSurface(s_surface);
    drawStack.clear();
}