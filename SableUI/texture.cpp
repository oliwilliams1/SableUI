#include "SableUI/texture.h"

#define SABLEUI_SUBSYSTEM "SableUI::Texture"
#include "SableUI/console.h"
#include <fstream>
#include <cmath>
#include <chrono>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <webp/decode.h>

struct HashTextureData
{
    int width = 0;
    int height = 0;
    GLuint texID = 0;
    bool currentlyUsed = true;
    std::chrono::steady_clock::time_point lastUsed = std::chrono::steady_clock::now();
};

struct ImageHash
{
    char data[16] = { 0 };

    bool operator<(const ImageHash& other) const
    {
        return std::memcmp(data, other.data, sizeof(data)) < 0;
    }
};

static std::map<ImageHash, HashTextureData> textureCache;

static ImageHash GetImageHash(const std::string& path, int width, int height)
{
    // inspired from djb2 hash
    ImageHash h;
    unsigned long long hashValue = 5381;

    hashValue = ((hashValue << 5) + hashValue) + width;
    hashValue = ((hashValue << 5) + hashValue) + height;

    for (char c : path)
    {
		hashValue = ((hashValue << 5) + hashValue) + c;
    }

    std::stringstream ss;
    ss << std::hex << hashValue;
    std::string hexHash = ss.str();

    size_t len = std::min(hexHash.length(), sizeof(h.data) - 1);
    memcpy(h.data, hexHash.c_str(), len);
    h.data[len] = '\0'; // terminate c-style string
    return h;
}

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
            GenerateDefaultTexture();
        }
        stbi_image_free(pixels);
    }
    else
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            SableUI_Warn("Unable to open file: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        if (buffer.size() < 12 || memcmp(buffer.data(), "RIFF", 4) != 0 || memcmp(buffer.data() + 8, "WEBP", 4) != 0)
        {
            SableUI_Warn("File is not a valid WebP format: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        int loadedWidth, loadedHeight;
        uint8_t* webpPixels = WebPDecodeRGB(buffer.data(), buffer.size(), &loadedWidth, &loadedHeight);
        if (!webpPixels)
        {
            SableUI_Warn("Failed to decode WebP image: %s. Generating default texture.", path.c_str());
            GenerateDefaultTexture();
            return;
        }
        SetTexture(webpPixels, loadedWidth, loadedHeight, 3);
    }
}

void SableUI::Texture::LoadTextureOptimised(const std::string& path, int width, int height)
{
    ImageHash hash = GetImageHash(path, width, height);

	auto it = textureCache.find(hash);
	if (it != textureCache.end())
	{
		m_width = it->second.width;
        m_height = it->second.height;
		m_texID = it->second.texID;
        it->second.currentlyUsed = true;
        it->second.lastUsed = std::chrono::high_resolution_clock::now();
		return;
	}

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
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            SableUI_Warn("Unable to open file: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        if (buffer.size() < 12 || memcmp(buffer.data(), "RIFF", 4) != 0 || memcmp(buffer.data() + 8, "WEBP", 4) != 0)
        {
            SableUI_Warn("File is not a valid WebP format: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        uint8_t* webpPixels = WebPDecodeRGB(buffer.data(), buffer.size(), &loadedWidth, &loadedHeight);
        if (!webpPixels)
        {
            SableUI_Warn("Failed to decode WebP image: %s. Generating default texture.", path.c_str());
            GenerateDefaultTexture();
            return;
        }
        loadedPixels = webpPixels;
        loadedChannels = 3;
    }

    const int targetChannels = 3;
    uint8_t* convertedToRgbPixels = nullptr;
    bool convertedAllocated = false;

    if (loadedChannels == targetChannels)
    {
        convertedToRgbPixels = loadedPixels;
    }
    else
    {
        convertedToRgbPixels = new uint8_t[loadedWidth * loadedHeight * targetChannels];
        convertedAllocated = true;

        for (int i = 0; i < loadedWidth * loadedHeight; i++)
        {
            if (loadedChannels == 4)
            {
                // RGBA to RGB
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
                SableUI_Warn("Unsupported loaded channel count for conversion: %d. Generating default texture.", loadedChannels);
                if (convertedAllocated) delete[] convertedToRgbPixels;
                if (loadedPixels) stbi_image_free(loadedPixels);
                GenerateDefaultTexture();
                return;
            }
        }
    }

    uint8_t* finalPixels = nullptr;
    bool finalPixelsAllocated = false;

    bool isDownsampling = (width < loadedWidth || height < loadedHeight);

    if (!isDownsampling)
    {
        SetTexture(convertedToRgbPixels, loadedWidth, loadedHeight, targetChannels);
    }
    else
    {
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

        if (resizeResult == nullptr)
        {
            SableUI_Warn("Failed to resize texture, using non-resized, 3-channel version");
            SetTexture(convertedToRgbPixels, loadedWidth, loadedHeight, targetChannels);

            if (loadedPixels) stbi_image_free(loadedPixels);
            if (intermediateAllocated) delete[] intermediatePixels;
            if (convertedAllocated) delete[] convertedToRgbPixels;
            return;
        }
        else
        {
            finalPixels = new uint8_t[width * height * targetChannels];
            finalPixelsAllocated = true;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
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
    SableUI_Log("Texture loading duration: %f seconds", duration.count());

    textureCache[hash] = { m_width, m_height, m_texID };
}

void SableUI::Texture::GenerateDefaultTexture()
{
    uint32_t pixels[16] = { 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
                            0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
                            0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
                            0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000, };

    SetTexture((uint8_t*)pixels, 4, 4, 4);
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
    for (auto& it : textureCache)
    {
        if (it.second.texID == m_texID)
        {
            it.second.currentlyUsed = false;
            it.second.lastUsed = std::chrono::high_resolution_clock::now();            
        }
    }
}

int iterations = 0;
void SableUI::StepCachedTexturesCleaner()
{
    if (iterations++ < 500) return;

    iterations = 0;
    
    std::vector<ImageHash> toDelete;

	auto now = std::chrono::high_resolution_clock::now();
	for (auto& it : textureCache)
	{
        if (it.second.currentlyUsed) continue;
		if (now - it.second.lastUsed > std::chrono::seconds(300))
		{
			toDelete.push_back(it.first);
		}
	}

    for (auto& it : toDelete)
	{
        glDeleteTextures(1, &textureCache[it].texID);
		textureCache.erase(it);
	}
}