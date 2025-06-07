#include "SableUI/texture.h"
#include "SableUI/console.h"
#include "SableUI/utils.h"
#include <memory>

SableUI::Texture::Texture(int width, int height)
    : width(width), height(height)
{
    initGPUTexture();
}

SableUI::Texture::~Texture()
{
    glDeleteTextures(1, &texID);
}

void SableUI::Texture::initGPUTexture()
{
    if (texID == 0)
    {
        glGenTextures(1, &texID);
    }

    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SableUI::Texture::Resize(int w, int h)
{
    bool u = false;

    if (w != width || h != height)
	{
		u = true;
	}
    width = f2i(w);
    height = f2i(h);

    if (u) Update();
}

void SableUI::Texture::Resize(float w, float h)
{
    Resize(SableUI::f2i(w), SableUI::f2i(h));
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
