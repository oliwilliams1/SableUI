#include "SableUI/texture.h"
#include "SableUI/console.h"
#include <cmath>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

void SableUI::Texture::LoadTexture(const std::string& path)
{
    int width, height, channels;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (pixels)
    {
        if (channels == 4)
        {
            SetTexture(pixels, width, height, channels);
        }
        else if (channels == 3)
        {
            SetTexture(pixels, width, height, channels);
        }
        else {
            SableUI_Warn("Unsupported channel count: %d for texture: %s", channels, path.c_str());
            uint8_t* defaultPixels = GenerateDefaultTexture(m_width, m_height);
            if (defaultPixels != nullptr)
            {
                SetTexture(defaultPixels, m_width, m_height, 3);
                delete[] defaultPixels;
            }
        }
        stbi_image_free(pixels);
    }
    else
    {
        SableUI_Warn("Failed to load texture: %s", path.c_str());
        uint8_t* defaultPixels = GenerateDefaultTexture(m_width, m_height);
        if (defaultPixels != nullptr)
        {
            SetTexture(defaultPixels, m_width, m_height, 3);
            delete[] defaultPixels;
        }
    }
}


void SableUI::Texture::LoadTextureOptimised(const std::string& path, int width, int height)
{
    auto start = std::chrono::high_resolution_clock::now();
    if (width < 16 || height < 16)
    {
        SableUI_Warn("Optimized texture load requested for dimensions (%d, %d) less than 16. Using normal LoadTexture.", width, height);
        LoadTexture(path);
        return;
    }

    int loadedWidth, loadedHeight, loadedChannels;
    uint8_t* loadedPixels = stbi_load(path.c_str(), &loadedWidth, &loadedHeight, &loadedChannels, 0);

    if (!loadedPixels)
    {
        SableUI_Warn("Failed to load texture: %s. Generating default texture.", path.c_str());
        uint8_t* defaultPixels = GenerateDefaultTexture(width, height);
        if (defaultPixels != nullptr)
        {
            SetTexture(defaultPixels, width, height, 3);
            delete[] defaultPixels;
        }
        return;
    }

    const int targetChannels = 3;
    uint8_t* convertedToRgbPixels = nullptr;
    bool convertedAllocated = false;

    if (loadedChannels == targetChannels)
    {
        convertedToRgbPixels = loadedPixels;
    }
    else {
        convertedToRgbPixels = new uint8_t[loadedWidth * loadedHeight * targetChannels];
        convertedAllocated = true;

        for (int i = 0; i < loadedWidth * loadedHeight; i++)
        {
            if (loadedChannels == 4)
            {
                // RGBA to RGB (dropping alpha)
                convertedToRgbPixels[i * 3 + 0] = loadedPixels[i * 4 + 0];
                convertedToRgbPixels[i * 3 + 1] = loadedPixels[i * 4 + 1];
                convertedToRgbPixels[i * 3 + 2] = loadedPixels[i * 4 + 2];
            }
            else if (loadedChannels == 1)
            {
                // Grayscale to RGB
                convertedToRgbPixels[i * 3 + 0] = loadedPixels[i * 1 + 0];
                convertedToRgbPixels[i * 3 + 1] = loadedPixels[i * 1 + 0];
                convertedToRgbPixels[i * 3 + 2] = loadedPixels[i * 1 + 0];
            }
            else
            {
                SableUI_Warn("Unsupported loaded channel count for conversion: %d. Using default texture.", loadedChannels);
                if (convertedAllocated) delete[] convertedToRgbPixels;
                stbi_image_free(loadedPixels);
                uint8_t* defaultPixels = GenerateDefaultTexture(width, height);
                if (defaultPixels != nullptr)
                {
                    SetTexture(defaultPixels, width, height, 3);
                    delete[] defaultPixels;
                }
                return;
            }
        }
    }

    uint8_t* finalPixels = nullptr;
    bool finalPixelsAllocated = false;

    bool isDownsampling = (width < loadedWidth || height < loadedHeight);

    if (!isDownsampling)
    {
        /* upscaling or no scaling, upload direct to gpu */
        SetTexture(finalPixels, width, height, targetChannels);
    }
    else
    {
        /* downsampling, do 2-step resizing, linear to 2x target, then 2x nearest downsample */
        int scaleWidth1 = width * 2;
        int scaleHeight1 = height * 2;

        scaleWidth1 = std::min(scaleWidth1, loadedWidth);
        scaleHeight1 = std::min(scaleHeight1, loadedHeight);

        uint8_t* intermediatePixels = new uint8_t[scaleWidth1 * scaleHeight1 * targetChannels];
        bool intermediateAllocated = true;

        unsigned char* resizeResult = stbir_resize_uint8_linear(
            convertedToRgbPixels, loadedWidth, loadedHeight, 0,
            intermediatePixels, scaleWidth1, scaleHeight1, 0, STBIR_RGB
        );

        if (resizeResult == 0)
        {
            SableUI_Warn("Failed to resize texture, using non-resized, 3-channel version");
            SetTexture(convertedToRgbPixels, loadedWidth, loadedHeight, targetChannels);

            if (loadedPixels) stbi_image_free(loadedPixels);
            if (intermediateAllocated) delete[] intermediatePixels;
            if (convertedAllocated) delete[] convertedToRgbPixels;
            if (finalPixelsAllocated) delete[] finalPixels;
            return;
        }
        else {
            /* 2x nearest neighbout downsample */
            finalPixels = new uint8_t[width * height * targetChannels];
            finalPixelsAllocated = true;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    /* bounds check */
                    int srcX = (int)std::round((float)x * scaleWidth1 / width);
                    int srcY = (int)std::round((float)y * scaleHeight1 / height);

                    srcX = std::min(srcX, scaleWidth1 - 1);
                    srcY = std::min(srcY, scaleHeight1 - 1);

                    int srcIndex = (srcY * scaleWidth1 + srcX) * targetChannels;
                    int destIndex = (y * width + x) * targetChannels;

                    finalPixels[destIndex + 0] = intermediatePixels[srcIndex + 0];
                    finalPixels[destIndex + 1] = intermediatePixels[srcIndex + 1];
                    finalPixels[destIndex + 2] = intermediatePixels[srcIndex + 2];
                }
            }
            SetTexture(finalPixels, width, height, targetChannels);
        }

        if (intermediateAllocated) delete[] intermediatePixels;
    }

    if (convertedAllocated) delete[] convertedToRgbPixels;
    if (loadedPixels) stbi_image_free(loadedPixels);
    if (finalPixelsAllocated) delete[] finalPixels;

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    SableUI_Log("Texture thing duration %f", duration.count());
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

void SableUI::Texture::SetTexture(uint8_t* pixels, int width, int height, int channels)
{
    if (m_texID == 0) glGenTextures(1, &m_texID);

    glBindTexture(GL_TEXTURE_2D, m_texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum internalFormat = GL_RGB8;
    GLenum format = GL_RGB;

    if (channels == 4)
    {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    else if (channels == 3)
    {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }
    else
    {
        SableUI_Warn("Attempted to set texture with unsupported channel count in SetTexture: %d", channels);
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_width = width;
    m_height = height;
}

SableUI::Texture::~Texture()
{
    if (m_texID != 0)
    {
        glDeleteTextures(1, &m_texID);
    }
}