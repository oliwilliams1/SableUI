#include "SableUI/texture.h"
#include "SableUI/console.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void SableUI::Texture::LoadTexture(const std::string& path)
{
    int width, height, channels;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &channels, 3);

    if (pixels)
    {
        SetTexture(pixels, width, height);
        stbi_image_free(pixels);
    }
    else
    {
        SableUI_Warn("Failed to load texture: %s", path.c_str());
        m_width = 512;
        m_height = 512;
        uint8_t* defaultPixels = GenerateDefaultTexture(m_width, m_height);
        if (defaultPixels != nullptr)
        {
            SetTexture(defaultPixels, m_width, m_height);
            delete[] defaultPixels;
        }
    }
}

uint8_t* SableUI::Texture::GenerateDefaultTexture(int width, int height)
{
    if (m_defaultTexID != 0)
    {
        m_texID = m_defaultTexID;
        return nullptr;
    }

    uint8_t* pixels = new uint8_t[width * height * 3];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 3;
            if ((x / 128) % 2 == (y / 128) % 2)
            {
                pixels[index]     = 0xFF;
                pixels[index + 1] = 0x00;
                pixels[index + 2] = 0xFF;
            }
            else
            {
                pixels[index]     = 0x00;
                pixels[index + 1] = 0x00;
                pixels[index + 2] = 0x00;
            }
        }
    }
    return pixels;
}

void SableUI::Texture::SetDefaultTexture(GLuint texID)
{
    m_defaultTexID = texID;
}

void SableUI::Texture::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_texID);
}

void SableUI::Texture::SetTexture(uint8_t* pixels, int width, int height)
{
    if (m_texID == 0) glGenTextures(1, &m_texID);

    glBindTexture(GL_TEXTURE_2D, m_texID);

    // Upload to dedicated memory instead of shared
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

SableUI::Texture::~Texture()
{
    if (m_texID != 0)
    {
        glDeleteTextures(1, &m_texID);
    }
}