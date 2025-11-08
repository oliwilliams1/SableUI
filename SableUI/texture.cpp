#include "SableUI/texture.h"

#define SABLEUI_SUBSYSTEM "SableUI::Texture"
#include "SableUI/console.h"
#include "SableUI/renderer.h"

#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#include <cmath>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#include <webp/decode.h>

using namespace SableUI;

struct SableUI::CachedGpuTexture
{
    GpuTexture gpu;
    int width = 0;
    int height = 0;
    bool inUse = true;
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

static std::map<ImageHash, std::shared_ptr<CachedGpuTexture>> textureCache;
static int s_numTextures = 0;

static ImageHash GetImageHash(const std::string& path, int width, int height)
{
    // djb2-inspired hash
    unsigned long long hashValue = 5381;
    hashValue = ((hashValue << 5) + hashValue) + width;
    hashValue = ((hashValue << 5) + hashValue) + height;

    for (char c : path)
        hashValue = ((hashValue << 5) + hashValue) + static_cast<unsigned long long>(c);

    ImageHash h;
    std::stringstream ss;
    ss << std::hex << hashValue;
    std::string hexHash = ss.str();

    size_t len = std::min(hexHash.length(), sizeof(h.data) - 1);
    memcpy(h.data, hexHash.c_str(), len);
    h.data[len] = '\0';
    return h;
}

Texture::Texture()
{
    s_numTextures++;
}

Texture::Texture(int width, int height, uint32_t texID)
    : m_width(width), m_height(height)
{
    s_numTextures++;
    m_defaultTexID = texID;
}

Texture::~Texture()
{
    if (m_cachedGpu)
    {
        m_cachedGpu->inUse = false;
        m_cachedGpu->lastUsed = std::chrono::high_resolution_clock::now();
    }
    s_numTextures--;
}

int Texture::GetNumInstances()
{
    return s_numTextures;
}

void Texture::LoadTexture(const std::string& path)
{
    int width, height, channels;
    uint8_t* pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!pixels)
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
            SableUI_Warn("Invalid WebP format: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        int w, h;
        uint8_t* webpPixels = WebPDecodeRGB(buffer.data(), buffer.size(), &w, &h);
        if (!webpPixels)
        {
            SableUI_Warn("Failed to decode WebP image: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        auto tex = std::make_shared<CachedGpuTexture>();
        tex->gpu.SetData(webpPixels, w, h, 3);
        tex->width = w;
        tex->height = h;
        m_cachedGpu = tex;
        WebPFree(webpPixels);
        return;
    }

    auto tex = std::make_shared<CachedGpuTexture>();
    tex->gpu.SetData(pixels, width, height, channels);
    tex->width = width;
    tex->height = height;
    m_cachedGpu = tex;
    stbi_image_free(pixels);
}

void Texture::LoadTextureOptimised(const std::string& path, int width, int height)
{
    ImageHash hash = GetImageHash(path, width, height);

    auto it = textureCache.find(hash);
    if (it != textureCache.end())
    {
        m_cachedGpu = it->second;
        m_cachedGpu->inUse = true;
        m_cachedGpu->lastUsed = std::chrono::high_resolution_clock::now();
        m_width = m_cachedGpu->width;
        m_height = m_cachedGpu->height;
        return;
    }

    int loadedWidth, loadedHeight, loadedChannels;
    uint8_t* loadedPixels = stbi_load(path.c_str(), &loadedWidth, &loadedHeight, &loadedChannels, 0);

    if (!loadedPixels)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            SableUI_Warn("Unable to open texture: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        if (buffer.size() < 12 || memcmp(buffer.data(), "RIFF", 4) != 0 || memcmp(buffer.data() + 8, "WEBP", 4) != 0)
        {
            SableUI_Warn("Invalid WebP format: %s", path.c_str());
            GenerateDefaultTexture();
            return;
        }

        loadedPixels = WebPDecodeRGB(buffer.data(), buffer.size(), &loadedWidth, &loadedHeight);
        loadedChannels = 3;
    }

    const int targetChannels = 3;
    uint8_t* converted = nullptr;
    bool allocated = false;

    if (loadedChannels == targetChannels)
    {
        converted = loadedPixels;
    }
    else
    {
        converted = new uint8_t[loadedWidth * loadedHeight * targetChannels];
        allocated = true;

        for (int i = 0; i < loadedWidth * loadedHeight; i++)
        {
            if (loadedChannels == 4)
            {
                converted[i * 3 + 0] = loadedPixels[i * 4 + 0];
                converted[i * 3 + 1] = loadedPixels[i * 4 + 1];
                converted[i * 3 + 2] = loadedPixels[i * 4 + 2];
            }
            else if (loadedChannels == 1)
            {
                converted[i * 3 + 0] = converted[i * 3 + 1] = converted[i * 3 + 2] = loadedPixels[i];
            }
            else
            {
                SableUI_Warn("Unsupported channel count %d", loadedChannels);
                if (allocated) delete[] converted;
                stbi_image_free(loadedPixels);
                GenerateDefaultTexture();
                return;
            }
        }
    }

    uint8_t* finalPixels = converted;
    bool resized = false;

    if (width > 0 && height > 0 && (width < loadedWidth || height < loadedHeight))
    {
        finalPixels = new uint8_t[width * height * targetChannels];
        resized = stbir_resize_uint8_linear(
            converted, loadedWidth, loadedHeight, 0,
            finalPixels, width, height, 0, STBIR_RGB
        ) != nullptr;
        if (resized)
        {
            loadedWidth = width;
            loadedHeight = height;
        }
        else
        {
            delete[] finalPixels;
            finalPixels = converted;
        }
    }

    auto tex = std::make_shared<CachedGpuTexture>();
    tex->gpu.SetData(finalPixels, loadedWidth, loadedHeight, targetChannels);
    tex->width = loadedWidth;
    tex->height = loadedHeight;
    tex->lastUsed = std::chrono::high_resolution_clock::now();

    textureCache[hash] = tex;
    m_cachedGpu = tex;
    m_width = loadedWidth;
    m_height = loadedHeight;

    if (resized) delete[] finalPixels;
    if (allocated) delete[] converted;
    stbi_image_free(loadedPixels);

    SableUI_Log("Texture cached: %s (%dx%d)", path.c_str(), m_width, m_height);
}

void Texture::GenerateDefaultTexture()
{
    uint32_t pixels[16] = {
        0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
        0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
        0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
        0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
    };

    auto tex = std::make_shared<CachedGpuTexture>();
    tex->gpu.SetData((uint8_t*)pixels, 4, 4, 4);
    tex->width = 4;
    tex->height = 4;
    m_cachedGpu = tex;
}

void Texture::SetDefaultTexture(uint32_t texID)
{
    m_defaultTexID = texID;
}

void Texture::Bind()
{
    if (m_cachedGpu)
        m_cachedGpu->gpu.Bind();
}

void SableUI::StepCachedTexturesCleaner()
{
    static int iterations = 0;
    if (iterations++ < 500) return;
    iterations = 0;

    auto now = std::chrono::high_resolution_clock::now();
    std::vector<ImageHash> toDelete;

    for (auto& [hash, tex] : textureCache)
    {
        if (tex.use_count() > 1) continue;
        if (!tex->inUse && now - tex->lastUsed > std::chrono::seconds(300))
            toDelete.push_back(hash);
    }

    for (auto& h : toDelete)
        textureCache.erase(h);
}
