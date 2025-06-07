#include "SableUI/texture.h"
#include "SableUI/console.h"
#include <memory>

SableUI::Texture::Texture(int width, int height, bool gpuTexture)
    : width(width), height(height), gpuTexture(gpuTexture)
{
    if (gpuTexture) initGPUTexture();
}

SableUI::Texture::~Texture()
{
    if (gpuTexture) glDeleteTextures(1, &texID);
}

void SableUI::Texture::initGPUTexture()
{
    if (texID == -1)
    {
        glGenTextures(1, &texID);
    }

    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    Update();
}

void SableUI::Texture::Resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    if (gpuTexture) Update();
}

void SableUI::Texture::SetColour(uint32_t v) const
{
    Update();
}

void SableUI::Texture::Update() const
{
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void SableUI::Texture::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, texID);
}
