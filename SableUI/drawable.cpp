#include <memory>
#include <algorithm>
#include "SableUI/drawable.h"
#include "SableUI/renderer.h"
#include "SableUI/texture.h"

void SableUI::DrawableRect::Update(SableUI::rect& rect, SableUI::colour colour, float pBSize)
{
    this->r = rect;
    this->c = colour;
}

void SableUI::DrawableRect::Draw(SableUI::Texture* texture)
{
    /* get raw pointer data */
    uint32_t* surfacePixels = static_cast<uint32_t*>(texture->pixels);

    if (surfacePixels == nullptr) return;

    int x = std::clamp(SableUI::f2i(r.x), 0, texture->width - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, texture->height - 1);
    int height = std::clamp(SableUI::f2i(r.h), 0, texture->height - y);
    int width = std::clamp(SableUI::f2i(r.w), 0, texture->width - x);

    /* use std::fill to efficiently draw the rect */
    for (int i = 0; i < height; i++)
    {
        if (y + i < texture->height)
        {
            uint32_t* start = surfacePixels + ((y + i) * texture->width) + x;
            std::fill(start, start + width, c.value);
        }
    }
}

void SableUI::DrawableSplitter::Update(SableUI::rect& rect, SableUI::colour colour, SableUI::NodeType type, 
                                       float pBSize, const std::vector<int>& segments)
{
    this->r = rect;
    this->c = colour;
    this->type = type;
    this->b = SableUI::f2i(pBSize);
    this->offsets = segments;
}

void SableUI::DrawableSplitter::Draw(SableUI::Texture* texture)
{
    /* get raw pointer data */
    uint32_t* surfacePixels = static_cast<uint32_t*>(texture->pixels);

    if (surfacePixels == nullptr) return;

    int x = std::clamp(SableUI::f2i(r.x), 0, texture->width - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, texture->height - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, texture->width - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, texture->height - y);

    /* use std::fill to efficiently draw the splitter */
    switch (type)
    {
    case SableUI::NodeType::HSPLITTER:
    {
        for (int offset : offsets)
        {
            int drawX = x + offset - b;

            for (int i = b; i < height; i++)
            {
                if (y + i < texture->height && drawX >= 0 && drawX < texture->width)
                {
                    uint32_t* start = surfacePixels + (y + i) * texture->width + drawX;
                    std::fill(start, start + b * 2, c.value);
                }
            }
        }
        break;
    }

    case SableUI::NodeType::VSPLITTER:
    {
        for (int offset : offsets)
        {
            int drawY = y + offset - b;

            for (int i = 0; i < b * 2; i++)
            {
                if (drawY + i >= 0 && drawY + i < texture->height && x < texture->width)
                {
                    uint32_t* start = surfacePixels + (drawY + i) * texture->width + x;
                    std::fill(start, start + width, c.value);
                }
            }
        }
        break;
    }
    }
}