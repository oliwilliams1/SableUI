#include "SableUI/texture.h"
#include <memory>

SableUI::Texture::Texture(int width, int height)
    : width(width), height(height)
{
    pixels = new uint32_t[width * height];
}

SableUI::Texture::~Texture()
{
    delete[] pixels;
}

void SableUI::Texture::Resize(int newWidth, int newHeight)
{
    delete[] pixels;
    width = newWidth;
    height = newHeight;
    pixels = new uint32_t[width * height];
}

void SableUI::Texture::SetColour(uint32_t v)
{
    std::fill(pixels, pixels + width * height, v);
}